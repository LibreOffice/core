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
