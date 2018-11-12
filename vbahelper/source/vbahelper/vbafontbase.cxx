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
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <vbahelper/vbafontbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;


// form controls use other property name as the remaining OOo API
#define VBAFONTBASE_PROPNAME( ascii_normal, ascii_control ) \
    mbFormControl ? OUString( ascii_control ) : OUString( ascii_normal )

VbaFontBase::VbaFontBase(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< css::container::XIndexAccess >& xPalette,
        const uno::Reference< beans::XPropertySet >& xPropertySet,
        bool bFormControl ) :
    VbaFontBase_BASE( xParent, xContext ),
    mxFont( xPropertySet, uno::UNO_SET_THROW ),
    mxPalette( xPalette, uno::UNO_SET_THROW ),
    mbFormControl( bFormControl )
{
}

VbaFontBase::~VbaFontBase()
{
}

void SAL_CALL
VbaFontBase::setSuperscript( const uno::Any& aValue )
{
    // not supported in form controls
    if( mbFormControl )
        return;

    bool bValue = false;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

    if( bValue )
    {
        nValue = SUPERSCRIPT;
        nValue2 = SUPERSCRIPTHEIGHT;
    }
    mxFont->setPropertyValue( "CharEscapement" , uno::Any(nValue) );
    mxFont->setPropertyValue( "CharEscapementHeight" , uno::Any(nValue2) );
}

uno::Any SAL_CALL
VbaFontBase::getSuperscript()
{
    short nValue = NORMAL;
    // not supported in form controls
    if( !mbFormControl )
       mxFont->getPropertyValue( "CharEscapement" ) >>= nValue;
    return uno::makeAny( nValue == SUPERSCRIPT );
}

void SAL_CALL
VbaFontBase::setSubscript( const uno::Any& aValue )
{
    // not supported in form controls
    if( mbFormControl )
        return;

    bool bValue = false;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

    if( bValue )
    {
        nValue= SUBSCRIPT;
        nValue2 = SUBSCRIPTHEIGHT;
    }

    mxFont->setPropertyValue( "CharEscapementHeight" , uno::Any(nValue2) );
    mxFont->setPropertyValue( "CharEscapement" , uno::Any(nValue) );

}

uno::Any SAL_CALL
VbaFontBase::getSubscript()
{
    short nValue = NORMAL;
    // not supported in form controls
    if( !mbFormControl )
       mxFont->getPropertyValue( "CharEscapement" ) >>= nValue;
    return uno::makeAny( nValue == SUBSCRIPT );
}

void SAL_CALL
VbaFontBase::setSize( const uno::Any& aValue )
{
    // form controls need a sal_Int16 containing points, other APIs need a float
    uno::Any aVal( aValue );
    if( mbFormControl )
    {
        float fVal = 0.0;
        aVal >>= fVal;
        aVal <<= static_cast< sal_Int16 >( fVal );
    }
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharHeight", "FontHeight" ), aVal );
}

uno::Any SAL_CALL
VbaFontBase::getSize()
{
    return mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharHeight", "FontHeight" ) );
}

void SAL_CALL
VbaFontBase::setColorIndex( const uno::Any& _colorindex )
{
    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;

    --nIndex; // OOo indices are zero bases

    // setColor expects colors in XL RGB values
    // #FIXME this is daft we convert OO RGB val to XL RGB val and
    // then back again to OO RGB value
    setColor( OORGBToXLRGB(mxPalette->getByIndex( nIndex )) );
}


uno::Any SAL_CALL
VbaFontBase::getColorIndex()
{
    sal_Int32 nColor = 0;

    XLRGBToOORGB( getColor() ) >>= nColor;
    sal_Int32 nElems = mxPalette->getCount();
    sal_Int32 nIndex = -1;
    for ( sal_Int32 count=0; count<nElems; ++count )
           {
        sal_Int32 nPaletteColor = 0;
        mxPalette->getByIndex( count ) >>= nPaletteColor;
        if ( nPaletteColor == nColor )
        {
            nIndex = count + 1; // 1 based
            break;
        }
    }
    return uno::makeAny( nIndex );
}

void SAL_CALL
VbaFontBase::setBold( const uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    double fBoldValue = awt::FontWeight::NORMAL;
    if( bValue )
        fBoldValue = awt::FontWeight::BOLD;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharWeight", "FontWeight" ), uno::Any( fBoldValue ) );

}

uno::Any SAL_CALL
VbaFontBase::getBold()
{
    double fValue = 0.0;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharWeight", "FontWeight" ) ) >>= fValue;
    return uno::makeAny( fValue == awt::FontWeight::BOLD );
}

void SAL_CALL
VbaFontBase::setStrikethrough( const uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    short nValue = awt::FontStrikeout::NONE;
    if( bValue )
        nValue = awt::FontStrikeout::SINGLE;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharStrikeout", "FontStrikeout" ), uno::Any( nValue ) );
}

uno::Any SAL_CALL
VbaFontBase::getStrikethrough()
{
    short nValue = 0;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharStrikeout", "FontStrikeout" ) ) >>= nValue;
    return uno::Any( nValue == awt::FontStrikeout::SINGLE );
}

void  SAL_CALL
VbaFontBase::setShadow( const uno::Any& aValue )
{
    if( !mbFormControl )
       mxFont->setPropertyValue( "CharShadowed" , aValue );
}

uno::Any SAL_CALL
VbaFontBase::getShadow()
{
    return mbFormControl ? uno::Any( false ) : mxFont->getPropertyValue( "CharShadowed" );
}

void  SAL_CALL
VbaFontBase::setItalic( const uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    awt::FontSlant nValue = awt::FontSlant_NONE;
    if( bValue )
        nValue = awt::FontSlant_ITALIC;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharPosture", "FontSlant" ), uno::Any( static_cast<short>(nValue) ) );
}

uno::Any SAL_CALL
VbaFontBase::getItalic()
{
    awt::FontSlant aFS;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharPosture", "FontSlant" ) ) >>= aFS;
    return uno::makeAny( aFS == awt::FontSlant_ITALIC );
}

void  SAL_CALL
VbaFontBase::setName( const uno::Any& aValue )
{
    OUString sString;
    aValue >>= sString;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharFontName", "FontName" ), aValue );
}

uno::Any SAL_CALL
VbaFontBase::getName()
{
    return mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharFontName", "FontName" ) );
}

uno::Any
VbaFontBase::getColor()
{
    uno::Any aAny;
    aAny = OORGBToXLRGB( mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharColor", "TextColor" ) ) );
    return aAny;
}

void
VbaFontBase::setColor( const uno::Any& _color  )
{
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharColor", "TextColor" ), XLRGBToOORGB(_color) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
