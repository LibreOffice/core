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
#include "precompiled_stoc.hxx"

#include <vector>
#include <osl/diagnose.h>
#include "base.hxx"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ModuleTypeDescriptionImpl::~ModuleTypeDescriptionImpl()
{
    delete _pMembers;

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ModuleTypeDescriptionImpl::getTypeClass()
    throw( RuntimeException )
{
    return TypeClass_MODULE;
}
//__________________________________________________________________________________________________
// virtual
OUString ModuleTypeDescriptionImpl::getName()
    throw( RuntimeException )
{
    return _aName;
}

// XModuleTypeDescription
//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XTypeDescription > > SAL_CALL
ModuleTypeDescriptionImpl::getMembers()
    throw ( RuntimeException )
{
    if ( !_pMembers )
    {
        Reference< XTypeDescriptionEnumeration > xEnum;
        try
        {
            xEnum = _xTDMgr->createTypeDescriptionEnumeration(
                        _aName,
                        Sequence< TypeClass >(),
                        TypeDescriptionSearchDepth_ONE );
        }
        catch ( NoSuchTypeNameException const & )
        {
        }
        catch ( InvalidTypeNameException const & )
        {
        }

        OSL_ENSURE( xEnum.is(),
                    "ModuleTypeDescriptionImpl::getMembers - No enumeration!" );

        std::vector< Reference< XTypeDescription > > aTDs;
        while ( xEnum->hasMoreElements() )
        {
            try
            {
                Reference< XTypeDescription > xTD(
                    xEnum->nextTypeDescription() );
                aTDs.push_back( xTD );
            }
            catch ( NoSuchElementException const & )
            {
                OSL_ENSURE( sal_False,
                    "ModuleTypeDescriptionImpl::getMembers - "
                    " Caught NoSuchElementException!" );
            }
        }

        Sequence< Reference< XTypeDescription > > * pMembers
            = new Sequence< Reference< XTypeDescription > >( aTDs.size() );
        for ( sal_Int32 n = 0; n < pMembers->getLength(); n++ )
            (*pMembers)[ n ] = aTDs[ n ];

        ClearableMutexGuard aGuard( getMutex() );
        if ( _pMembers )
        {
            aGuard.clear();
            delete pMembers;
        }
        else
        {
            _pMembers = pMembers;
        }
    }
    return *_pMembers;
}

}
