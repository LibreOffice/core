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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/AttributeData.hpp>

#include <ooo/vba/excel/XlColorIndex.hpp>
#include <ooo/vba/excel/XlPattern.hpp>

#include <map>

#include <sal/macros.h>

#include "vbainterior.hxx"
#include "vbapalette.hxx"
#include <document.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlPattern;

const OUStringLiteral BACKCOLOR = u"CellBackColor";
const OUStringLiteral PATTERN = u"Pattern";
const OUStringLiteral PATTERNCOLOR = u"PatternColor";

static std::map< sal_Int32, sal_Int32 > aPatternMap {
    { xlPatternAutomatic, 0 },
    { xlPatternChecker, 9 },
    { xlPatternCrissCross, 16 },
    { xlPatternDown, 7 },
    { xlPatternGray16, 17 },
    { xlPatternGray25, 4 },
    { xlPatternGray50, 2 },
    { xlPatternGray75, 3 },
    { xlPatternGray8, 18 },
    { xlPatternGrid, 15 },
    { xlPatternHorizontal, 5 },
    { xlPatternLightDown, 13 },
    { xlPatternLightHorizontal, 11 },
    { xlPatternLightUp, 14 },
    { xlPatternLightVertical, 12 },
    { xlPatternNone, 0 },
    { xlPatternSemiGray75, 10 },
    { xlPatternSolid, 0 },
    { xlPatternUp, 8 },
    { xlPatternVertical, 6 }
};

ScVbaInterior::ScVbaInterior( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertySet >&  xProps, ScDocument* pScDoc ) : ScVbaInterior_BASE( xParent, xContext ), m_xProps(xProps), m_pScDoc( pScDoc )
{
    // auto color
    m_aPattColor = Color(0);
    m_nPattern = 0;
    if ( !m_xProps.is() )
        throw lang::IllegalArgumentException("properties", uno::Reference< uno::XInterface >(), 2 );
}

uno::Any
ScVbaInterior::getColor()
{
    return uno::makeAny( OORGBToXLRGB( GetBackColor() ) );
}

void
ScVbaInterior::setColor( const uno::Any& _color  )
{
    sal_Int32 nColor = 0;
    if( _color >>= nColor )
    {
        SetUserDefinedAttributes( BACKCOLOR, SetAttributeData( XLRGBToOORGB( nColor ) ) );
        SetMixedColor();
    }
}

void
ScVbaInterior::SetMixedColor()
{
    // pattern
    uno::Any aPattern = GetUserDefinedAttributes( PATTERN );
    if( aPattern.hasValue() )
    {
        m_nPattern = GetAttributeData( aPattern );
    }
    sal_Int32 nPattern = aPatternMap[ m_nPattern ];
    // pattern color
    uno::Any aPatternColor = GetUserDefinedAttributes( PATTERNCOLOR );
    if( aPatternColor.hasValue() )
    {
        sal_uInt32 nPatternColor = GetAttributeData( aPatternColor );
        m_aPattColor = Color(nPatternColor);
    }
    Color nPatternColor = m_aPattColor;
    // back color
    Color aBackColor( GetBackColor() );
    // set mixed color
    Color aMixedColor;
    if( nPattern > 0 )
        aMixedColor = GetPatternColor( nPatternColor, aBackColor, static_cast<sal_uInt32>(nPattern) );
    else
        aMixedColor = GetPatternColor( aBackColor, aBackColor, static_cast<sal_uInt32>(nPattern) );
    Color nMixedColor = aMixedColor.GetRGBColor();
    m_xProps->setPropertyValue( BACKCOLOR , uno::makeAny( nMixedColor ) );
}

uno::Reference< container::XIndexAccess >
ScVbaInterior::getPalette() const
{
    if ( !m_pScDoc )
        throw uno::RuntimeException();
    SfxObjectShell* pShell = m_pScDoc->GetDocumentShell();
    ScVbaPalette aPalette( pShell );
    return aPalette.getPalette();
}

void SAL_CALL
ScVbaInterior::setColorIndex( const css::uno::Any& _colorindex )
{
    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;

    // hackly for excel::XlColorIndex::xlColorIndexNone
    if( nIndex == excel::XlColorIndex::xlColorIndexNone )
    {
        m_xProps->setPropertyValue( BACKCOLOR, uno::makeAny( sal_Int32( -1 ) ) );
    }
    else
    {
        // setColor expects colors in XL RGB values
        // #FIXME this is daft we convert OO RGB val to XL RGB val and
        // then back again to OO RGB value
        setColor( OORGBToXLRGB( GetIndexColor( nIndex ) ) );
    }
}
uno::Any
ScVbaInterior::GetIndexColor( sal_Int32 nColorIndex )
{
    sal_Int32 nIndex = nColorIndex;
    // #FIXME  xlColorIndexAutomatic & xlColorIndexNone are not really
    // handled properly here
    if ( !nIndex || ( nIndex == excel::XlColorIndex::xlColorIndexAutomatic ) || ( nIndex == excel::XlColorIndex::xlColorIndexNone )  )
        nIndex = 2; // default is white ( this maybe will probably break, e.g. we may at some stage need to know what this interior is,  a cell or something else and then pick a default colour based on that )
    --nIndex; // OOo indices are zero bases
    uno::Reference< container::XIndexAccess > xIndex = getPalette();
    return xIndex->getByIndex( nIndex );
}

sal_Int32
ScVbaInterior::GetColorIndex( const sal_Int32 nColor )
{
    uno::Reference< container::XIndexAccess > xIndex = getPalette();
    sal_Int32 nElems = xIndex->getCount();
    sal_Int32 nIndex = -1;
    for ( sal_Int32 count=0; count<nElems; ++count )
           {
        sal_Int32 nPaletteColor = 0;
        xIndex->getByIndex( count ) >>= nPaletteColor;
        if ( nPaletteColor == nColor )
        {
            nIndex = count + 1; // 1 based
            break;
        }
    }
    return nIndex;
}

uno::Any SAL_CALL
ScVbaInterior::getColorIndex()
{
    sal_Int32 nColor = 0;
    // hackly for excel::XlColorIndex::xlColorIndexNone
    uno::Any aColor = m_xProps->getPropertyValue( BACKCOLOR );
    if( ( aColor >>= nColor ) && ( nColor == -1 ) )
    {
        nColor = excel::XlColorIndex::xlColorIndexNone;
        return uno::makeAny( nColor );
    }

    // getColor returns Xl ColorValue, need to convert it to OO val
    // as the palette deals with OO RGB values
    // #FIXME this is daft in getColor we convert OO RGB val to XL RGB val
    // and then back again to OO RGB value
    XLRGBToOORGB( getColor() ) >>= nColor;

    return uno::makeAny( GetColorIndex( nColor ) );
}
Color
ScVbaInterior::GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt32 nXclPattern )
{
    // 0x00 == 0% transparence (full rPattColor)
    // 0x80 == 100% transparence (full rBackColor)
    static const sal_uInt8 pnRatioTable[] =
    {
        0x80, 0x00, 0x40, 0x20, 0x60, 0x40, 0x40, 0x40,     // 00 - 07
        0x40, 0x40, 0x20, 0x60, 0x60, 0x60, 0x60, 0x48,     // 08 - 15
        0x50, 0x70, 0x78                                    // 16 - 18
    };
    return ( nXclPattern < SAL_N_ELEMENTS( pnRatioTable ) ) ?
        GetMixedColor( rPattColor, rBackColor, pnRatioTable[ nXclPattern ] ) : rPattColor;
}
Color
ScVbaInterior::GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans )
{
    return Color(
        nTrans,
        GetMixedColorComp( rFore.GetRed(), rBack.GetRed(), nTrans ),
        GetMixedColorComp( rFore.GetGreen(), rBack.GetGreen(), nTrans ),
        GetMixedColorComp( rFore.GetBlue(), rBack.GetBlue(), nTrans ));
}
sal_uInt8
ScVbaInterior::GetMixedColorComp(  sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans )
{
    sal_uInt32 nTemp = ((static_cast< sal_Int32 >( nBack ) - nFore) * nTrans) / 0x80 + nFore;
    return static_cast< sal_uInt8 >( nTemp );
}
uno::Reference< container::XNameContainer >
ScVbaInterior::GetAttributeContainer()
{
    return uno::Reference < container::XNameContainer > ( m_xProps->getPropertyValue("UserDefinedAttributes"), uno::UNO_QUERY_THROW );
}
sal_Int32
ScVbaInterior::GetAttributeData( uno::Any const & aValue )
{
    xml::AttributeData aDataValue;
    if( aValue >>= aDataValue )
    {
        return aDataValue.Value.toInt32();
    }
    return 0;
}
uno::Any
ScVbaInterior::SetAttributeData( sal_Int32 nValue )
{
    xml::AttributeData aAttributeData;
    aAttributeData.Type = "sal_Int32";
    aAttributeData.Value = OUString::number( nValue );
    return uno::makeAny( aAttributeData );
}
uno::Any
ScVbaInterior::GetUserDefinedAttributes( const OUString& sName )
{
    uno::Reference< container::XNameContainer > xNameContainer( GetAttributeContainer(), uno::UNO_SET_THROW );
    if( xNameContainer->hasByName( sName ) )
    {
        return xNameContainer->getByName( sName );
    }
    return uno::Any();
}
void
ScVbaInterior::SetUserDefinedAttributes( const OUString& sName, const uno::Any& aValue )
{
    if( aValue.hasValue() )
    {
        uno::Reference< container::XNameContainer > xNameContainer( GetAttributeContainer(), uno::UNO_SET_THROW );
        if( xNameContainer->hasByName( sName ) )
            xNameContainer->removeByName( sName );
        xNameContainer->insertByName( sName, aValue );
        m_xProps->setPropertyValue("UserDefinedAttributes", uno::makeAny( xNameContainer ) );
    }
}
// OOo do not support below API
uno::Any SAL_CALL
ScVbaInterior::getPattern()
{
    // XlPattern
    uno::Any aPattern = GetUserDefinedAttributes( PATTERN );
    if( aPattern.hasValue() )
        return uno::makeAny( GetAttributeData( aPattern ) );
    return uno::makeAny( excel::XlPattern::xlPatternNone );
}
void SAL_CALL
ScVbaInterior::setPattern( const uno::Any& _pattern )
{
    if( !(_pattern >>= m_nPattern) )
        throw uno::RuntimeException("Invalid Pattern index" );

    SetUserDefinedAttributes( PATTERN, SetAttributeData( m_nPattern ) );
    SetMixedColor();

}
Color
ScVbaInterior::GetBackColor()
{
    sal_Int32 nColor = 0;
    Color aBackColor;
    uno::Any aColor = GetUserDefinedAttributes( BACKCOLOR );
    if( aColor.hasValue() )
    {
        nColor = GetAttributeData( aColor );
        aBackColor = Color(nColor);
    }
    else
    {
        uno::Any aAny = OORGBToXLRGB( m_xProps->getPropertyValue( BACKCOLOR ) );
        if( aAny >>= nColor )
        {
            nColor = XLRGBToOORGB( nColor );
            aBackColor = Color(nColor);
            SetUserDefinedAttributes( BACKCOLOR, SetAttributeData( nColor ) );
        }
    }
    return aBackColor;
}
uno::Any SAL_CALL
ScVbaInterior::getPatternColor()
{
    // 0 is the default color. no filled.
    uno::Any aPatternColor = GetUserDefinedAttributes( PATTERNCOLOR );
    if( aPatternColor.hasValue() )
    {
        sal_uInt32 nPatternColor = GetAttributeData( aPatternColor );
        return uno::makeAny( OORGBToXLRGB( Color(nPatternColor) ) );
    }
    return uno::makeAny( sal_Int32( 0 ) );
}
void SAL_CALL
ScVbaInterior::setPatternColor( const uno::Any& _patterncolor )
{
    sal_Int32 nPattColor = 0;
    if( !(_patterncolor >>= nPattColor) )
        throw uno::RuntimeException("Invalid Pattern Color" );

    SetUserDefinedAttributes( PATTERNCOLOR, SetAttributeData( XLRGBToOORGB( nPattColor ) ) );
    SetMixedColor();

}
uno::Any SAL_CALL
ScVbaInterior::getPatternColorIndex()
{
    sal_Int32 nColor = 0;
    XLRGBToOORGB( getPatternColor() ) >>= nColor;

    return uno::makeAny( GetColorIndex( nColor ) );
}
void SAL_CALL
ScVbaInterior::setPatternColorIndex( const uno::Any& _patterncolorindex )
{
    sal_Int32 nColorIndex = 0;
    if( !(_patterncolorindex >>= nColorIndex) )
        throw uno::RuntimeException("Invalid Pattern Color" );

    if( nColorIndex == 0 )
        return;
    Color nPattColor;
    GetIndexColor( nColorIndex ) >>= nPattColor;
    setPatternColor( uno::makeAny( OORGBToXLRGB( nPattColor ) ) );

}

uno::Any SAL_CALL ScVbaInterior::getThemeColor()
{
    // Just a stub for now.
    return uno::makeAny(static_cast<sal_Int32>(0));
}

void SAL_CALL ScVbaInterior::setThemeColor(const uno::Any& /*rAny*/)
{
    // Just a stub for now.
}

uno::Any SAL_CALL ScVbaInterior::getTintAndShade()
{
    // Just a stub for now.
    return uno::makeAny(static_cast<double>(0));
}

void SAL_CALL ScVbaInterior::setTintAndShade(const uno::Any& /*rAny*/)
{
    // Just a stub for now.
}

uno::Any SAL_CALL ScVbaInterior::getPatternTintAndShade()
{
    // Just a stub for now.
    return uno::makeAny(static_cast<double>(0));
}

void SAL_CALL ScVbaInterior::setPatternTintAndShade(const uno::Any& /*rAny*/)
{
    // Just a stub for now.
}

OUString
ScVbaInterior::getServiceImplName()
{
    return "ScVbaInterior";
}

uno::Sequence< OUString >
ScVbaInterior::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Interior"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
