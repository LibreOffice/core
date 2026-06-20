/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <vbahelper/vbafontbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;


// form controls use other property name as the remaining OOo API
#define VBAFONTBASE_PROPNAME( ascii_normal, ascii_control ) \
    mbFormControl ? u"" ascii_control ""_ustr : u"" ascii_normal ""_ustr

VbaFontBase::VbaFontBase(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< css::container::XIndexAccess >& xPalette,
        const uno::Reference< beans::XPropertySet >& xPropertySet,
        Component eWhich,
        bool bFormControl ) :
    VbaFontBase_BASE( xParent, xContext ),
    mxFont( xPropertySet, uno::UNO_SET_THROW ),
    mxPalette( xPalette, uno::UNO_SET_THROW ),
    meWhich( eWhich ),
    mbFormControl( bFormControl )
{
}

VbaFontBase::~VbaFontBase()
{
}

void SAL_CALL
VbaFontBase::setSuperscript( const cpo::uno::Any& aValue )
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
    mxFont->setPropertyValue( u"CharEscapement"_ustr , cpo::uno::Any(nValue) );
    mxFont->setPropertyValue( u"CharEscapementHeight"_ustr , cpo::uno::Any(nValue2) );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getSuperscript()
{
    short nValue = NORMAL;
    // not supported in form controls
    if( !mbFormControl )
       mxFont->getPropertyValue( u"CharEscapement"_ustr ) >>= nValue;
    return cpo::uno::Any( nValue == SUPERSCRIPT );
}

void SAL_CALL
VbaFontBase::setSubscript( const cpo::uno::Any& aValue )
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

    mxFont->setPropertyValue( u"CharEscapementHeight"_ustr , cpo::uno::Any(nValue2) );
    mxFont->setPropertyValue( u"CharEscapement"_ustr , cpo::uno::Any(nValue) );

}

cpo::uno::Any SAL_CALL
VbaFontBase::getSubscript()
{
    short nValue = NORMAL;
    // not supported in form controls
    if( !mbFormControl )
       mxFont->getPropertyValue( u"CharEscapement"_ustr ) >>= nValue;
    return cpo::uno::Any( nValue == SUBSCRIPT );
}

void SAL_CALL
VbaFontBase::setSize( const cpo::uno::Any& aValue )
{
    // form controls need a sal_Int16 containing points, other APIs need a float
    cpo::uno::Any aVal( aValue );
    if( mbFormControl )
    {
        float fVal = 0.0;
        aVal >>= fVal;
        aVal <<= static_cast< sal_Int16 >( fVal );
    }
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharHeight", "FontHeight" ), aVal );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getSize()
{
    return mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharHeight", "FontHeight" ) );
}

void SAL_CALL
VbaFontBase::setColorIndex( const cpo::uno::Any& _colorindex )
{
    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;

    --nIndex; // OOo indices are zero bases

    if (meWhich == EXCEL){
        setColor( OORGBToXLRGB(mxPalette->getByIndex( nIndex )) );
    }
    else{
        setColor( mxPalette->getByIndex( nIndex ));
    }
}


cpo::uno::Any SAL_CALL
VbaFontBase::getColorIndex()
{
    sal_Int32 nColor = 0;

    if (meWhich == EXCEL){
        XLRGBToOORGB( getColor() ) >>= nColor;
    }
    else{
        getColor() >>= nColor;
    }

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
    return cpo::uno::Any( nIndex );
}

void SAL_CALL
VbaFontBase::setBold( const cpo::uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    double fBoldValue = awt::FontWeight::NORMAL;
    if( bValue )
        fBoldValue = awt::FontWeight::BOLD;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharWeight", "FontWeight" ), cpo::uno::Any( fBoldValue ) );

}

cpo::uno::Any SAL_CALL
VbaFontBase::getBold()
{
    double fValue = 0.0;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharWeight", "FontWeight" ) ) >>= fValue;
    return cpo::uno::Any( fValue == awt::FontWeight::BOLD );
}

void SAL_CALL
VbaFontBase::setStrikethrough( const cpo::uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    short nValue = awt::FontStrikeout::NONE;
    if( bValue )
        nValue = awt::FontStrikeout::SINGLE;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharStrikeout", "FontStrikeout" ), cpo::uno::Any( nValue ) );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getStrikethrough()
{
    short nValue = 0;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharStrikeout", "FontStrikeout" ) ) >>= nValue;
    return cpo::uno::Any( nValue == awt::FontStrikeout::SINGLE );
}

void  SAL_CALL
VbaFontBase::setShadow( const cpo::uno::Any& aValue )
{
    if( !mbFormControl )
       mxFont->setPropertyValue( u"CharShadowed"_ustr , aValue );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getShadow()
{
    return mbFormControl ? cpo::uno::Any( false ) : mxFont->getPropertyValue( u"CharShadowed"_ustr );
}

void  SAL_CALL
VbaFontBase::setItalic( const cpo::uno::Any& aValue )
{
    bool bValue = false;
    aValue >>= bValue;
    awt::FontSlant nValue = awt::FontSlant_NONE;
    if( bValue )
        nValue = awt::FontSlant_ITALIC;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharPosture", "FontSlant" ), cpo::uno::Any( static_cast<short>(nValue) ) );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getItalic()
{
    awt::FontSlant aFS;
    mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharPosture", "FontSlant" ) ) >>= aFS;
    return cpo::uno::Any( aFS == awt::FontSlant_ITALIC );
}

void  SAL_CALL
VbaFontBase::setName( const cpo::uno::Any& aValue )
{
    OUString sString;
    aValue >>= sString;
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharFontName", "FontName" ), aValue );
}

cpo::uno::Any SAL_CALL
VbaFontBase::getName()
{
    return mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharFontName", "FontName" ) );
}

cpo::uno::Any
VbaFontBase::getColor()
{
    if (meWhich == EXCEL){
        cpo::uno::Any aAny = OORGBToXLRGB( mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharColor", "TextColor" ) ) );
        return aAny;
    }
    else{
        cpo::uno::Any aAny = mxFont->getPropertyValue( VBAFONTBASE_PROPNAME( "CharColor", "TextColor" ) );
        return aAny;
    }
}

void
VbaFontBase::setColor( const cpo::uno::Any& _color  )
{
    mxFont->setPropertyValue( VBAFONTBASE_PROPNAME( "CharColor", "TextColor" ), XLRGBToOORGB(_color) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
