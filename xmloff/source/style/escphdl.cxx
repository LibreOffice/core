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

#include <escphdl.hxx>

#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
#define DFLT_ESC_AUTO_SUB  -101


// class XMLEscapementPropHdl


XMLEscapementPropHdl::~XMLEscapementPropHdl()
{
    // nothing to do
}

bool XMLEscapementPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int16 nVal;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return false;

    if( IsXMLToken( aToken, XML_ESCAPEMENT_SUB ) )
    {
        nVal = DFLT_ESC_AUTO_SUB;
    }
    else if( IsXMLToken( aToken, XML_ESCAPEMENT_SUPER ) )
    {
        nVal = DFLT_ESC_AUTO_SUPER;
    }
    else
    {
        sal_Int32 nNewEsc;
        if (!::sax::Converter::convertPercent( nNewEsc, aToken ))
            return false;

        nVal = (sal_Int16) nNewEsc;
    }

    rValue <<= nVal;
    return true;
}

bool XMLEscapementPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( nValue == DFLT_ESC_AUTO_SUPER )
        {
            aOut.append( GetXMLToken(XML_ESCAPEMENT_SUPER) );
        }
        else if( nValue == DFLT_ESC_AUTO_SUB )
        {
            aOut.append( GetXMLToken(XML_ESCAPEMENT_SUB) );
        }
        else
        {
            ::sax::Converter::convertPercent( aOut, nValue );
        }
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}


// class XMLEscapementHeightPropHdl


XMLEscapementHeightPropHdl::~XMLEscapementHeightPropHdl()
{
    // nothing to do
}

bool XMLEscapementHeightPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if( IsXMLToken( rStrImpValue, XML_CASEMAP_SMALL_CAPS ) )
        return false;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return false;

    sal_Int8 nProp;
    if( aTokens.getNextToken( aToken ) )
    {
        sal_Int32 nNewProp;
        if (!::sax::Converter::convertPercent( nNewProp, aToken ))
            return false;
        nProp = (sal_Int8)nNewProp;
    }
    else
    {
        sal_Int32 nEscapementPosition=0;
        if (::sax::Converter::convertPercent( nEscapementPosition, aToken )
            && (nEscapementPosition == 0))
        {
            nProp = 100; //if escapement position is zero and no escapement height is given the default height should be 100percent and not something smaller (#i91800#)
        }
        else
            nProp = (sal_Int8) DFLT_ESC_PROP;
    }

    rValue <<= nProp;
    return true;
}

bool XMLEscapementHeightPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut( rStrExpValue );

    sal_Int32 nValue = 0;
    if( rValue >>= nValue )
    {
        if( !rStrExpValue.isEmpty() )
            aOut.append( ' ');

        ::sax::Converter::convertPercent( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
