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


#include <XMLPercentOrMeasurePropertyHandler.hxx>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


XMLPercentOrMeasurePropertyHandler::XMLPercentOrMeasurePropertyHandler()
{
}

XMLPercentOrMeasurePropertyHandler::~XMLPercentOrMeasurePropertyHandler()
{
}

bool XMLPercentOrMeasurePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    if( rStrImpValue.indexOf( '%' ) != -1 )
        return false;

    sal_Int32 nValue;

    if (!rUnitConverter.convertMeasureToCore( nValue, rStrImpValue ))
        return false;

    rValue <<= nValue;
    return true;
}

bool XMLPercentOrMeasurePropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    sal_Int32 nValue = 0;
    if( !(rValue >>= nValue ) )
        return false;

    rUnitConverter.convertMeasureToXML( aOut, nValue );

    rStrExpValue = aOut.makeStringAndClear();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
