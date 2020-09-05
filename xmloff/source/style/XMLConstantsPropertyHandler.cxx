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

#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>

using namespace ::com::sun::star::uno;

XMLConstantsPropertyHandler::~XMLConstantsPropertyHandler()
{
}

bool XMLConstantsPropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    bool bRet = SvXMLUnitConverter::convertEnum(
        nEnum, rStrImpValue, pMap );

    if( bRet )
        rValue <<= static_cast<sal_Int16>(nEnum);

    return bRet;
}

bool XMLConstantsPropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    sal_Int32 nEnum = 0;

    if( rValue.hasValue() && (rValue.getValueTypeClass() == TypeClass_ENUM))
    {
        nEnum = *static_cast<sal_Int32 const *>(rValue.getValue());
        bRet = true;
    }
    else
    {
        bRet = (rValue >>= nEnum );
    }

    if( bRet )
    {
        if( (nEnum >= 0) && (nEnum <= 0xffff) )
        {
            sal_uInt16 nConst = static_cast<sal_uInt16>( nEnum );
            OUStringBuffer aOut;

            bRet = SvXMLUnitConverter::convertEnum(
                aOut, nConst, pMap, eDefault );

            rStrExpValue = aOut.makeStringAndClear();
        }
        else
        {
            OSL_FAIL("XMLConstantsPropertyHandler::exportXML() constant is out of range for implementation using sal_uInt16");
        }
    }
    else
    {
        OSL_FAIL("XMLConstantsPropertyHandler::exportXML() could not convert any to sal_Int32");
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
