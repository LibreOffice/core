/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"


#include <escphdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP    58
#define DFLT_ESC_AUTO_SUPER 101
#define DFLT_ESC_AUTO_SUB  -101

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementPropHdl
//

XMLEscapementPropHdl::~XMLEscapementPropHdl()
{
    // nothing to do
}

sal_Bool XMLEscapementPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int16 nVal;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return sal_False;

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
        if( !SvXMLUnitConverter::convertPercent( nNewEsc, aToken ) )
            return sal_False;

        nVal = (sal_Int16) nNewEsc;
    }

    rValue <<= nVal;
    return sal_True;
}

sal_Bool XMLEscapementPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
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
            SvXMLUnitConverter::convertPercent( aOut, nValue );
        }
    }

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementHeightPropHdl
//

XMLEscapementHeightPropHdl::~XMLEscapementHeightPropHdl()
{
    // nothing to do
}

sal_Bool XMLEscapementHeightPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if( IsXMLToken( rStrImpValue, XML_CASEMAP_SMALL_CAPS ) )
        return sal_False;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return sal_False;

    sal_Int8 nProp;
    if( aTokens.getNextToken( aToken ) )
    {
        sal_Int32 nNewProp;
        if( !SvXMLUnitConverter::convertPercent( nNewProp, aToken ) )
            return sal_False;
        nProp = (sal_Int8)nNewProp;
    }
    else
    {
        sal_Int32 nEscapementPosition=0;
        if( SvXMLUnitConverter::convertPercent( nEscapementPosition, aToken ) && nEscapementPosition==0 )
            nProp = 100; //if escapement position is zero and no escapement height is given the default height should be 100percent and not something smaller (#i91800#)
        else
            nProp = (sal_Int8) DFLT_ESC_PROP;
    }

    rValue <<= nProp;
    return sal_True;
}

sal_Bool XMLEscapementHeightPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut( rStrExpValue );

    sal_Int32 nValue = 0;
    if( rValue >>= nValue )
    {
        if( rStrExpValue.getLength() )
            aOut.append( sal_Unicode(' '));

        SvXMLUnitConverter::convertPercent( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength() != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
