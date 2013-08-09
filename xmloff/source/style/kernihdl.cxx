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

#include "kernihdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

// --
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

//
// class XMLKerningPropHdl
//

XMLKerningPropHdl::~XMLKerningPropHdl()
{
    // nothing to do
}

sal_Bool XMLKerningPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;
    sal_Int32 nKerning = 0;

    if( ! IsXMLToken( rStrImpValue, XML_KERNING_NORMAL ) )
    {
        bRet = rUnitConverter.convertMeasureToCore( nKerning, rStrImpValue );
    }

    rValue <<= (sal_Int16)nKerning;

    return bRet;
}

sal_Bool XMLKerningPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();

    if( rValue >>= nValue )
    {
        OUStringBuffer aOut;

        if( nValue == 0 )
            aOut.append( GetXMLToken(XML_KERNING_NORMAL) );
        else
        {
            rUnitConverter.convertMeasureToXML( aOut, nValue );
        }

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
