/*************************************************************************
 *
 *  $RCSfile: chrlohdl.cxx,v $
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


#ifndef _XMLOFF_PROPERTYHANDLER_CHARLOCALETYPES_HXX
#include <chrlohdl.hxx>
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

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
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

XMLCharLanguageHdl::~XMLCharLanguageHdl()
{
    // nothing to do
}

sal_Bool XMLCharLanguageHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Language == aLocale2.Language );

    return bRet;
}

sal_Bool XMLCharLanguageHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( rStrImpValue.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_none ) ) != 0 )
        aLocale.Language = rStrImpValue;

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharLanguageHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Language;

    if( !rStrExpValue.getLength() )
        rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) );

    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementHeightPropHdl
//

XMLCharCountryHdl::~XMLCharCountryHdl()
{
    // nothing to do
}

sal_Bool XMLCharCountryHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Country == aLocale2.Country );

    return bRet;
}

sal_Bool XMLCharCountryHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( rStrImpValue.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_none ) ) != 0 )
        aLocale.Country = rStrImpValue;

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharCountryHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Country;

    if( !rStrExpValue.getLength() )
        rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) );

    return sal_True;
}
