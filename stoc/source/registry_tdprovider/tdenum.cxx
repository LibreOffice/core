/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
