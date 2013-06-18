/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "cppuhelper/implementationentry.hxx"
#include "basicmigration.hxx"
#include "wordbookmigration.hxx"
#include "oo3extensionmigration.hxx"

extern "C"
{

::cppu::ImplementationEntry entries [] =
{
    {
        migration::BasicMigration_create, migration::BasicMigration_getImplementationName,
        migration::BasicMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        migration::WordbookMigration_create, migration::WordbookMigration_getImplementationName,
        migration::WordbookMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        migration::OO3ExtensionMigration_create, migration::OO3ExtensionMigration_getImplementationName,
        migration::OO3ExtensionMigration_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};


void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, entries );
}

}
