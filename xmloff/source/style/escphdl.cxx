/*************************************************************************
 *
 *  $RCSfile: escphdl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-12 17:11:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _XMLOFF_PROPERTYHANDLER_ESCAPETYPES_HXX
#include <escphdl.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

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

sal_Bool XMLEscapementPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nVal;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return sal_False;

    if( 0 == aToken.compareToAscii( sXML_escapement_sub ) )
    {
        nVal = DFLT_ESC_AUTO_SUB;
    }
    else if( 0 == aToken.compareToAscii( sXML_escapement_super ) )
    {
        nVal = DFLT_ESC_AUTO_SUPER;
    }
    else
    {
        sal_Int32 nNewEsc;
        if( !rUnitConverter.convertPercent( nNewEsc, aToken ) )
            return sal_False;

        nVal = (sal_Int16) nNewEsc;
    }

    rValue <<= nVal;
    return sal_True;
}

sal_Bool XMLEscapementPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( nValue == DFLT_ESC_AUTO_SUPER )
        {
            aOut.appendAscii( sXML_escapement_super );
        }
        else if( nValue == DFLT_ESC_AUTO_SUB )
        {
            aOut.appendAscii( sXML_escapement_sub );
        }
        else
        {
            rUnitConverter.convertPercent( aOut, nValue );
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

sal_Bool XMLEscapementHeightPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    if( !rStrImpValue.compareToAscii( sXML_casemap_small_caps ) )
        return sal_False;

    SvXMLTokenEnumerator aTokens( rStrImpValue );

    OUString aToken;
    if( ! aTokens.getNextToken( aToken ) )
        return sal_False;

    sal_Int8 nProp;
    if( aTokens.getNextToken( aToken ) )
    {
        sal_Int32 nNewProp;
        if( !rUnitConverter.convertPercent( nNewProp, aToken ) )
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

sal_Bool XMLEscapementHeightPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut( rStrExpValue );

    sal_Int32 nValue;
    if( rValue >>= nValue )
    {
        if( rStrExpValue.getLength() )
            aOut.append( sal_Unicode(' '));

        rUnitConverter.convertPercent( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength();
}
