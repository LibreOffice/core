/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: escphdl.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:51:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"


#ifndef _XMLOFF_PROPERTYHANDLER_ESCAPETYPES_HXX
#include <escphdl.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlelement.hxx>
#endif

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
