/*************************************************************************
 *
 *  $RCSfile: cdouthdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:23:19 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_CROSSEDOUTTYPES_HXX
#include <cdouthdl.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::xmloff::token;

SvXMLEnumMapEntry pXML_CrossedoutType_Enum[] =
{
    { XML_NONE,                 FontStrikeout::NONE },
    { XML_SINGLE,   FontStrikeout::SINGLE },
    { XML_DOUBLE,               FontStrikeout::DOUBLE },
    { XML_SINGLE,    FontStrikeout::BOLD },
    { XML_SINGLE,    FontStrikeout::SLASH },
    { XML_SINGLE,    FontStrikeout::X },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutStyle_Enum[] =
{
    { XML_NONE,                         FontStrikeout::NONE },
    { XML_SOLID,                        FontStrikeout::SINGLE },
    { XML_SOLID,                        FontStrikeout::DOUBLE },
    { XML_SOLID,                        FontStrikeout::BOLD },
    { XML_SOLID,                        FontStrikeout::SLASH },
    { XML_SOLID,                        FontStrikeout::X },
    { XML_DOTTED,               FontStrikeout::SINGLE },
    { XML_DASH,             FontStrikeout::SINGLE },
    { XML_LONG_DASH,            FontStrikeout::SINGLE },
    { XML_DOT_DASH,         FontStrikeout::SINGLE },
    { XML_DOT_DOT_DASH,     FontStrikeout::SINGLE },
    { XML_WAVE,             FontStrikeout::SINGLE },
    { XML_TOKEN_INVALID,                0 }
};

SvXMLEnumMapEntry pXML_CrossedoutWidth_Enum[] =
{
    { XML_AUTO,                 FontStrikeout::NONE },
    { XML_AUTO,                 FontStrikeout::SINGLE },
    { XML_AUTO,                 FontStrikeout::DOUBLE },
    { XML_BOLD,     FontStrikeout::BOLD },
    { XML_AUTO,                 FontStrikeout::SLASH },
    { XML_AUTO,                 FontStrikeout::X },
    { XML_THIN,                 FontStrikeout::NONE },
    { XML_MEDIUM,               FontStrikeout::NONE },
    { XML_THICK,                FontStrikeout::NONE },
    { XML_TOKEN_INVALID,                0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutTypePropHdl
//

XMLCrossedOutTypePropHdl::~XMLCrossedOutTypePropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutTypePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewStrikeout;

    if( ( bRet = rUnitConverter.convertEnum( eNewStrikeout, rStrImpValue,
                                             pXML_CrossedoutType_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout;
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case FontStrikeout::NONE:
            case FontStrikeout::SINGLE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case FontStrikeout::DOUBLE:
                // A double line style has priority over a solid or a bold
                // line style,
                // but not about any other line style
                switch( eStrikeout )
                {
                case FontStrikeout::SINGLE:
                case FontStrikeout::BOLD:
                    break;
                default:
                    // If a double line style is not supported for the existing
                    // value, keep the new one
                    eNewStrikeout = eStrikeout;
                    break;
                }
                break;
            default:
                OSL_ENSURE( bRet, "unexpected line type value" );
                break;
            }
            if( eNewStrikeout != eStrikeout )
                rValue <<= (sal_Int16)eNewStrikeout;
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutTypePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && FontStrikeout::DOUBLE==nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue, pXML_CrossedoutType_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutStylePropHdl
//

XMLCrossedOutStylePropHdl::~XMLCrossedOutStylePropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutStylePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewStrikeout;

    if( ( bRet = rUnitConverter.convertEnum( eNewStrikeout, rStrImpValue,
                                             pXML_CrossedoutStyle_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout;
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
        {
            // one NONE a SINGLE are possible new values. For both, the
            // existing value is kept.
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutStylePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_CrossedoutStyle_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutWidthPropHdl
//

XMLCrossedOutWidthPropHdl::~XMLCrossedOutWidthPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutWidthPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 eNewStrikeout;

    if( ( bRet = rUnitConverter.convertEnum( eNewStrikeout, rStrImpValue,
                                             pXML_CrossedoutWidth_Enum ) ) )
    {
        // multi property: style and width might be set already.
        // If the old value is NONE, the new is used unchanged.
        sal_Int16 eStrikeout;
        if( (rValue >>= eStrikeout) && FontStrikeout::NONE!=eStrikeout )
        {
            switch( eNewStrikeout )
            {
            case FontStrikeout::NONE:
                // keep existing line style
                eNewStrikeout = eStrikeout;
                break;
            case FontStrikeout::BOLD:
                switch( eStrikeout )
                {
                case FontStrikeout::SINGLE:
                    break;
                default:
                    // If a double line style is not supported for the existing
                    // value, keep the new one
                    eNewStrikeout = eStrikeout;
                    break;
                }
            default:
                OSL_ENSURE( bRet, "unexpected line type value" );
                break;
            }
            if( eNewStrikeout != eStrikeout )
                rValue <<= (sal_Int16)eNewStrikeout;
        }
        else
        {
            rValue <<= (sal_Int16)eNewStrikeout;
        }
    }

    return bRet;
}

sal_Bool XMLCrossedOutWidthPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( (rValue >>= nValue) && (FontStrikeout::BOLD == nValue) )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue,
                                                 pXML_CrossedoutWidth_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLCrossedOutTextPropHdl
//

XMLCrossedOutTextPropHdl::~XMLCrossedOutTextPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutTextPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( rStrImpValue.getLength() )
    {
        sal_Int16 eStrikeout = ('/' == rStrImpValue[0]
                                        ? FontStrikeout::SLASH
                                        : FontStrikeout::X);
        rValue <<= (sal_Int16)eStrikeout;
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool XMLCrossedOutTextPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;

    if( (rValue >>= nValue) &&
        (FontStrikeout::SLASH == nValue || FontStrikeout::X == nValue) )
    {
        rStrExpValue = OUString::valueOf(
            static_cast< sal_Unicode>( FontStrikeout::SLASH == nValue ? '/'
                                                                      : 'X' ) );
        bRet = sal_True;
    }

    return bRet;
}

