/*************************************************************************
 *
 *  $RCSfile: undlihdl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-23 11:51:54 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_UNDERLINETYPES_HXX
#include <undlihdl.hxx>
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
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP
#include <com/sun/star/awt/FontUnderline.hpp>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;

SvXMLEnumMapEntry __READONLY_DATA pXML_Underline_Enum[] =
{
    { sXML_underline_none,              FontUnderline::NONE },
    { sXML_underline_single,            FontUnderline::SINGLE },
    { sXML_underline_double,            FontUnderline::DOUBLE },
    { sXML_underline_dotted,            FontUnderline::DOTTED },
    { sXML_underline_dash,              FontUnderline::DASH },
    { sXML_underline_long_dash,         FontUnderline::LONGDASH },
    { sXML_underline_dot_dash,          FontUnderline::DASHDOT },
    { sXML_underline_dot_dot_dash,      FontUnderline::DASHDOTDOT },
    { sXML_underline_wave,              FontUnderline::WAVE },
    { sXML_underline_bold,              FontUnderline::BOLD },
    { sXML_underline_bold_dotted,       FontUnderline::BOLDDOTTED },
    { sXML_underline_bold_dash,         FontUnderline::BOLDDASH },
    { sXML_underline_bold_long_dash,    FontUnderline::BOLDLONGDASH },
    { sXML_underline_bold_dot_dash,     FontUnderline::BOLDDASHDOT },
    { sXML_underline_bold_dot_dot_dash, FontUnderline::BOLDDASHDOTDOT },
    { sXML_underline_bold_wave,         FontUnderline::BOLDWAVE },
    { sXML_underline_double_wave,       FontUnderline::DOUBLEWAVE },
    { sXML_underline_small_wave,        FontUnderline::SMALLWAVE },
    { 0,                                0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPosturePropHdl
//

XMLUnderlinePropHdl::~XMLUnderlinePropHdl()
{
    // nothing to do
}

sal_Bool XMLUnderlinePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eUnderline;

    if( ( bRet = rUnitConverter.convertEnum( eUnderline, rStrImpValue, pXML_Underline_Enum ) ) )
        rValue <<= (sal_Int16)eUnderline;

    return bRet;
}

sal_Bool XMLUnderlinePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_Underline_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

