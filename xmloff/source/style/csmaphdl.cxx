/*************************************************************************
 *
 *  $RCSfile: csmaphdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:04 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_CASEMAPTYPES_HXX
#include <csmaphdl.hxx>
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

#ifndef _COM_SUN_STAR_STYLE_CASEMAP_HPP_
#include <com/sun/star/style/CaseMap.hpp>
#endif


#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

static SvXMLEnumMapEntry pXML_Casemap_Enum[] =
{
    { sXML_none,                style::CaseMap::NONE },
    { sXML_casemap_lowercase,   style::CaseMap::LOWERCASE },
    { sXML_casemap_uppercase,   style::CaseMap::UPPERCASE },
    { sXML_casemap_capitalize,  style::CaseMap::TITLE },
    { 0,0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPosturePropHdl
//

XMLCaseMapPropHdl::~XMLCaseMapPropHdl()
{
    // nothing to do
}

sal_Bool XMLCaseMapPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nVal;

    if( ( bRet = rUnitConverter.convertEnum( nVal, rStrImpValue, pXML_Casemap_Enum ) ) )
        rValue <<= nVal;

    return bRet;
}

sal_Bool XMLCaseMapPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, nValue, pXML_Casemap_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCaseMapVariantHdl
//

XMLCaseMapVariantHdl::~XMLCaseMapVariantHdl()
{
    // nothing to do
}

sal_Bool XMLCaseMapVariantHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    if( !rStrImpValue.compareToAscii( sXML_casemap_small_caps ) )
        return sal_False;

    rValue <<= (sal_Int16)style::CaseMap::SMALLCAPS;
    return sal_True;
}

sal_Bool XMLCaseMapVariantHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_uInt16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        switch( nValue )
        {
        case style::CaseMap::NONE:
            aOut.appendAscii( sXML_casemap_normal );
            break;
        case style::CaseMap::SMALLCAPS:
            aOut.appendAscii( sXML_casemap_small_caps );
            break;
        }
    }

    rStrExpValue = aOut.makeStringAndClear();
    return rStrExpValue.getLength();
}
