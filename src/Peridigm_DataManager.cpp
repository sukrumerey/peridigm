/*! \file Peridigm_DataManager.cpp */

// ***********************************************************************
//
//                             Peridigm
//                 Copyright (2009) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// 
// Questions? 
// David J. Littlewood   djlittl@sandia.gov 
// John A. Mitchell      jamitch@sandia.gov
// Michael L. Parks      mlparks@sandia.gov
// Stewart A. Silling    sasilli@sandia.gov
//
// ***********************************************************************

#include <Teuchos_Exceptions.hpp>
#include "Peridigm_DataManager.hpp"

void PeridigmNS::DataManager::allocateData(Teuchos::RCP< std::vector<Field_NS::FieldSpec> > specs)
{
  fieldSpecs = specs;
  
  // loop over the specs and determine:
  // 1) the number of scalar, vector2d, and vector3d fields
  // 2) the FieldType for each of the data
  // 3) whether the data has one or two states
  statelessScalarFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  statelessVector3DFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  statelessBondFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  statefulScalarFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  statefulVector3DFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  statefulBondFieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>());
  for(unsigned int i=0; i<fieldSpecs->size() ; ++i){
    Field_NS::FieldSpec& spec = (*fieldSpecs)[i];
    if(spec.getLength() == Field_NS::FieldSpec::SCALAR){
      if(spec.getStateArchitecture() == Field_NS::FieldSpec::STATELESS)
        statelessScalarFieldSpecs->push_back(spec);
      else
        statefulScalarFieldSpecs->push_back(spec);
    }
    else if(spec.getLength() == Field_NS::FieldSpec::VECTOR3D){
      if(spec.getStateArchitecture() == Field_NS::FieldSpec::STATELESS)
        statelessVector3DFieldSpecs->push_back(spec);
      else
        statefulVector3DFieldSpecs->push_back(spec);
    }
    else if(spec.getLength() == Field_NS::FieldSpec::BOND){
      if(spec.getStateArchitecture() == Field_NS::FieldSpec::STATELESS)
        statelessBondFieldSpecs->push_back(spec);
      else
        statefulBondFieldSpecs->push_back(spec);
    }
    else{
      TEST_FOR_EXCEPTION(false, Teuchos::RangeError, 
                         "PeridigmNS::DataManager::allocateData, invalid FieldSpec!");
    }
  }

//   cout << "\nDEBUGGING:" << endl;
//   cout << "  numStatelessScalar    " << statelessScalarFieldSpecs->size() << endl;
//   cout << "  numStatefulScalar     " << statefulScalarFieldSpecs->size() << endl;
//   cout << "  numStatelessBond      " << statelessBondFieldSpecs->size() << endl;
//   cout << "  numStatelessVector3D  " << statelessVector3DFieldSpecs->size() << endl;
//   cout << "  numStatefulVector3D   " << statefulVector3DFieldSpecs->size() << endl;
//   cout << "  numStatefulBond       " << statefulBondFieldSpecs->size() << endl;
//   cout << endl;

  // make sure maps exist before trying to create states
  if(statelessScalarFieldSpecs->size() + statefulScalarFieldSpecs->size() > 0)
    TEST_FOR_EXCEPTION(scalarMap == Teuchos::null, Teuchos::NullReferenceError, 
                       "Error in PeridigmNS::DataManager::allocateData(), attempting to allocate scalar data with no map (forget setScalarMap()?).");
  if(statelessVector3DFieldSpecs->size() + statefulVector3DFieldSpecs->size() > 0)
    TEST_FOR_EXCEPTION(vector3DMap == Teuchos::null, Teuchos::NullReferenceError, 
                       "Error in PeridigmNS::DataManager::allocateData(), attempting to allocate vector3D data with no map (forget setVector3DMap()?).");
  if(statelessBondFieldSpecs->size() + statefulBondFieldSpecs->size() > 0)
    TEST_FOR_EXCEPTION(bondMap == Teuchos::null, Teuchos::NullReferenceError, 
                       "Error in PeridigmNS::DataManager::allocateData(), attempting to allocate bond data with no map (forget setBondMap()?).");

  // create the states
  if(statelessScalarFieldSpecs->size() + statelessVector3DFieldSpecs->size() > 0){
    stateNONE = Teuchos::rcp(new State);
    if(statelessScalarFieldSpecs->size() > 0)
      stateNONE->allocateScalarData(statelessScalarFieldSpecs, scalarMap);
    if(statelessVector3DFieldSpecs->size() > 0)
      stateNONE->allocateVector3DData(statelessVector3DFieldSpecs, vector3DMap);
    if(statelessBondFieldSpecs->size() > 0)
      stateNONE->allocateBondData(statelessBondFieldSpecs, bondMap);
  }
  if(statefulScalarFieldSpecs->size() + statefulVector3DFieldSpecs->size() > 0){
    stateN = Teuchos::rcp(new State);
    stateNP1 = Teuchos::rcp(new State);
    if(statefulScalarFieldSpecs->size() > 0){
      stateN->allocateScalarData(statefulScalarFieldSpecs, scalarMap);
      stateNP1->allocateScalarData(statefulScalarFieldSpecs, scalarMap);
    }
    if(statefulVector3DFieldSpecs->size() > 0){
      stateN->allocateVector3DData(statefulVector3DFieldSpecs, vector3DMap);
      stateNP1->allocateVector3DData(statefulVector3DFieldSpecs, vector3DMap);
    }   
    if(statefulBondFieldSpecs->size() > 0){
      stateN->allocateBondData(statefulBondFieldSpecs, bondMap);
      stateNP1->allocateBondData(statefulBondFieldSpecs, bondMap);
    }
  }
}

Teuchos::RCP<Epetra_Vector> PeridigmNS::DataManager::getData(Field_NS::FieldSpec fieldSpec, Field_NS::FieldSpec::FieldStep fieldStep)
{
  Teuchos::RCP<Epetra_Vector> data;
  if(fieldStep == Field_NS::FieldSpec::STEP_NONE){
    data = stateNONE->getData(fieldSpec);
  }
  else if(fieldStep == Field_NS::FieldSpec::STEP_N){
    data = stateN->getData(fieldSpec);
  }
  else if(fieldStep == Field_NS::FieldSpec::STEP_NP1){
    data = stateNP1->getData(fieldSpec);
  }
  else{
    TEST_FOR_EXCEPTION(false, Teuchos::RangeError, 
                       "PeridigmNS::DataManager::getData, invalid FieldStep!");
  }
  return data;
}
