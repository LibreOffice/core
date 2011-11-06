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
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include "registry/reader.hxx"
#include "registry/version.h"
#include "base.hxx"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ConstantsTypeDescriptionImpl::~ConstantsTypeDescriptionImpl()
{
    delete _pMembers;

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ConstantsTypeDescriptionImpl::getTypeClass()
    throw( RuntimeException )
{
    return TypeClass_CONSTANTS;
}
//__________________________________________________________________________________________________
// virtual
OUString ConstantsTypeDescriptionImpl::getName()
    throw( RuntimeException )
{
    return _aName;
}

// XConstantsTypeDescription
//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XConstantTypeDescription > > SAL_CALL
ConstantsTypeDescriptionImpl::getConstants()
    throw ( RuntimeException )
{
    if ( !_pMembers )
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< Reference< XConstantTypeDescription > > * pTempConsts
            = new Sequence< Reference< XConstantTypeDescription > >( nFields );
        Reference< XConstantTypeDescription > * pConsts
            = pTempConsts->getArray();

        while ( nFields-- )
        {
            rtl::OUStringBuffer aName( _aName );
            aName.appendAscii( "." );
            aName.append( aReader.getFieldName( nFields ) );

            Any aValue( getRTValue( aReader.getFieldValue( nFields ) ) );

            pConsts[ nFields ]
                = new ConstantTypeDescriptionImpl( aName.makeStringAndClear(),
                                                   aValue );
        }

        ClearableMutexGuard aGuard( getMutex() );
        if ( _pMembers )
        {
            aGuard.clear();
            delete pTempConsts;
        }
        else
        {
            _pMembers = pTempConsts;
        }
    }
    return *_pMembers;
}

}
