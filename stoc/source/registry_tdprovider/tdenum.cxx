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
#include "base.hxx"

#include "registry/reader.hxx"
#include "registry/version.h"

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
EnumTypeDescriptionImpl::~EnumTypeDescriptionImpl()
{
    delete _pEnumNames;
    delete _pEnumValues;
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass EnumTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_ENUM;
}
//__________________________________________________________________________________________________
OUString EnumTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XEnumTypeDescription
//__________________________________________________________________________________________________
sal_Int32 EnumTypeDescriptionImpl::getDefaultEnumValue()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nDefaultValue;
}
//__________________________________________________________________________________________________
Sequence< OUString > EnumTypeDescriptionImpl::getEnumNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pEnumNames)
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< OUString > * pTempEnumNames = new Sequence< OUString >( nFields );
        OUString * pEnumNames = pTempEnumNames->getArray();

        while (nFields--)
        {
            pEnumNames[nFields] = aReader.getFieldName( nFields );
        }

        ClearableMutexGuard aGuard( getMutex() );
        if (_pEnumNames)
        {
            aGuard.clear();
            delete pTempEnumNames;
        }
        else
        {
            _pEnumNames = pTempEnumNames;
        }
    }
    return *_pEnumNames;
}
//__________________________________________________________________________________________________
Sequence< sal_Int32 > EnumTypeDescriptionImpl::getEnumValues()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pEnumValues)
    {
        typereg::Reader aReader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);

        sal_uInt16 nFields = aReader.getFieldCount();
        Sequence< sal_Int32 > * pTempEnumValues = new Sequence< sal_Int32 >( nFields );
        sal_Int32 * pEnumValues = pTempEnumValues->getArray();

        while (nFields--)
        {
            pEnumValues[nFields] = getRTValueAsInt32(
                aReader.getFieldValue( nFields ) );
        }

        ClearableMutexGuard aGuard( getMutex() );
        if (_pEnumValues)
        {
            aGuard.clear();
            delete pTempEnumValues;
        }
        else
        {
            _pEnumValues = pTempEnumValues;
        }
    }
    return *_pEnumValues;
}

}


