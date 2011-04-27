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

#include <vbanewfont.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

VbaNewFont::VbaNewFont(
        const uno::Reference< XHelperInterface >& rxParent,
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< beans::XPropertySet >& rxModelProps ) throw (uno::RuntimeException) :
    VbaNewFont_BASE( rxParent, rxContext ),
    mxProps( rxModelProps, uno::UNO_SET_THROW )
{
}

// XNewFont attributes

::rtl::OUString SAL_CALL VbaNewFont::getName() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontName" ) ) );
    return aAny.get< ::rtl::OUString >();
}

void SAL_CALL VbaNewFont::setName( const ::rtl::OUString& rName ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontName" ) ),
        uno::Any( rName ) );
}

double SAL_CALL VbaNewFont::getSize() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontHeight" ) ) );
    return aAny.get< float >();
}

void SAL_CALL VbaNewFont::setSize( double fSize ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontHeight" ) ),
        uno::Any( static_cast< float >( fSize ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getCharset() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontCharset" ) ) );
    return rtl_getBestWindowsCharsetFromTextEncoding( static_cast< rtl_TextEncoding >( aAny.get< sal_Int16 >() ) );
}

void SAL_CALL VbaNewFont::setCharset( sal_Int16 nCharset ) throw (uno::RuntimeException)
{
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= nCharset) && (nCharset <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( nCharset ) );
    if( eFontEnc == RTL_TEXTENCODING_DONTKNOW )
        throw uno::RuntimeException();
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontCharset" ) ),
        uno::Any( static_cast< sal_Int16 >( eFontEnc ) ) );
}

sal_Int16 SAL_CALL VbaNewFont::getWeight() throw (uno::RuntimeException)
{
    return getBold() ? 700 : 400;
}

void SAL_CALL VbaNewFont::setWeight( sal_Int16 nWeight ) throw (uno::RuntimeException)
{
    setBold( nWeight >= 700 );
}

sal_Bool SAL_CALL VbaNewFont::getBold() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontWeight" ) ) );
    return aAny.get< float >() > awt::FontWeight::NORMAL;
}

void SAL_CALL VbaNewFont::setBold( sal_Bool bBold ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontWeight" ) ),
        uno::Any( bBold ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
}

sal_Bool SAL_CALL VbaNewFont::getItalic() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontSlant" ) ) );
    return aAny.get< awt::FontSlant >() != awt::FontSlant_NONE;
}

void SAL_CALL VbaNewFont::setItalic( sal_Bool bItalic ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontSlant" ) ),
        uno::Any( bItalic ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getUnderline() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontUnderline" ) ) );
    return aAny.get< sal_Int16 >() != awt::FontUnderline::NONE;
}

void SAL_CALL VbaNewFont::setUnderline( sal_Bool bUnderline ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontUnderline" ) ),
        uno::Any( bUnderline ? awt::FontUnderline::SINGLE : awt::FontUnderline::NONE ) );
}

sal_Bool SAL_CALL VbaNewFont::getStrikethrough() throw (uno::RuntimeException)
{
    uno::Any aAny = mxProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontStrikeout" ) ) );
    return aAny.get< sal_Int16 >() != awt::FontStrikeout::NONE;
}

void SAL_CALL VbaNewFont::setStrikethrough( sal_Bool bStrikethrough ) throw (uno::RuntimeException)
{
    mxProps->setPropertyValue(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FontStrikeout" ) ),
        uno::Any( bStrikethrough ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( VbaNewFont, "ooo.vba.msforms.NewFont" )

// ============================================================================
