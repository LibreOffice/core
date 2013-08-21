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
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/AttributeData.hpp>

#include <ooo/vba/excel/XlColorIndex.hpp>
#include <ooo/vba/excel/XlPattern.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <map>

#include <sal/macros.h>
#include <svx/xtable.hxx>

#include "vbainterior.hxx"
#include "vbapalette.hxx"
#include "document.hxx"

#define COLORMAST 0xFFFFFF
const sal_uInt16 EXC_COLOR_WINDOWBACK = 65;
typedef std::map< sal_Int32, sal_Int32 >  PatternMap;
typedef std::pair< sal_Int32, sal_Int32 > PatternPair;
using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlPattern;
static const OUString BACKCOLOR( "CellBackColor" );
static const OUString PATTERN( "Pattern" );
static const OUString PATTERNCOLOR( "PatternColor" );

static PatternMap lcl_getPatternMap()
{
    PatternMap aPatternMap;
    aPatternMap.insert( PatternPair( xlPatternAutomatic, 0 ) );
    aPatternMap.insert( PatternPair( xlPatternChecker, 9 ) );
    aPatternMap.insert( PatternPair( xlPatternCrissCross, 16 ) );
    aPatternMap.insert( PatternPair( xlPatternDown, 7 ) );
    aPatternMap.insert( PatternPair( xlPatternGray16, 17 ) );
    aPatternMap.insert( PatternPair( xlPatternGray25, 4 ) );
    aPatternMap.insert( PatternPair( xlPatternGray50, 2 ) );
    aPatternMap.insert( PatternPair( xlPatternGray75, 3 ) );
    aPatternMap.insert( PatternPair( xlPatternGray8, 18 ) );
    aPatternMap.insert( PatternPair( xlPatternGrid, 15 ) );
    aPatternMap.insert( PatternPair( xlPatternHorizontal, 5 ) );
    aPatternMap.insert( PatternPair( xlPatternLightDown, 13 ) );
    aPatternMap.insert( PatternPair( xlPatternLightHorizontal, 11 ) );
    aPatternMap.insert( PatternPair( xlPatternLightUp, 14 ) );
    aPatternMap.insert( PatternPair( xlPatternLightVertical, 12 ) );
    aPatternMap.insert( PatternPair( xlPatternNone, 0 ) );
    aPatternMap.insert( PatternPair( xlPatternSemiGray75, 10 ) );
    aPatternMap.insert( PatternPair( xlPatternSolid, 0 ) );
    aPatternMap.insert( PatternPair( xlPatternUp, 8 ) );
    aPatternMap.insert( PatternPair( xlPatternVertical, 6 ) );
    return aPatternMap;
}

static PatternMap aPatternMap( lcl_getPatternMap() );

ScVbaInterior::ScVbaInterior( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertySet >&  xProps, ScDocument* pScDoc ) throw ( lang::IllegalArgumentException) : ScVbaInterior_BASE( xParent, xContext ), m_xProps(xProps), m_pScDoc( pScDoc )
{
    // auto color
    m_aPattColor.SetColor( (sal_uInt32)0x0 );
    m_nPattern = 0L;
    if ( !m_xProps.is() )
        throw lang::IllegalArgumentException("properties", uno::Reference< uno::XInterface >(), 2 );
}

uno::Any
ScVbaInterior::getColor() throw (uno::RuntimeException)
{
    Color aBackColor( GetBackColor() );
    return uno::makeAny( OORGBToXLRGB( aBackColor.GetColor() ) );
}

void
ScVbaInterior::setColor( const uno::Any& _color  ) throw (uno::RuntimeException)
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
        m_aPattColor.SetColor( nPatternColor );
    }
    sal_Int32 nPatternColor = m_aPattColor.GetColor();
    // back color
    Color aBackColor( GetBackColor() );
    // set mixed color
    Color aMixedColor;
    if( nPattern > 0 )
        aMixedColor = GetPatternColor( Color(nPatternColor), aBackColor, (sal_uInt32)nPattern );
    else
        aMixedColor = GetPatternColor( aBackColor, aBackColor, (sal_uInt32)nPattern );
    sal_Int32 nMixedColor = aMixedColor.GetColor() & COLORMAST;
    m_xProps->setPropertyValue( BACKCOLOR , uno::makeAny( nMixedColor ) );
}

uno::Reference< container::XIndexAccess >
ScVbaInterior::getPalette()
{
    if ( !m_pScDoc )
        throw uno::RuntimeException();
    SfxObjectShell* pShell = m_pScDoc->GetDocumentShell();
    ScVbaPalette aPalette( pShell );
    return aPalette.getPalette();
}

void SAL_CALL
ScVbaInterior::setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException)
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
ScVbaInterior::GetIndexColor( const sal_Int32& nColorIndex )
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
ScVbaInterior::getColorIndex() throw ( css::uno::RuntimeException )
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
ScVbaInterior::GetMixedColorComp(  sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans ) const
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
ScVbaInterior::GetAttributeData( uno::Any aValue )
{
    xml::AttributeData aDataValue;
    if( aValue >>= aDataValue )
    {
        return aDataValue.Value.toInt32();
    }
    return sal_Int32( 0 );
}
uno::Any
ScVbaInterior::SetAttributeData( sal_Int32 nValue )
{
    xml::AttributeData aAttributeData;
    aAttributeData.Type = OUString( "sal_Int32" );
    aAttributeData.Value = OUString::number( nValue );
    return uno::makeAny( aAttributeData );
}
uno::Any
ScVbaInterior::GetUserDefinedAttributes( const OUString& sName )
{
    uno::Reference< container::XNameContainer > xNameContainer( GetAttributeContainer(), uno::UNO_QUERY_THROW );
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
        uno::Reference< container::XNameContainer > xNameContainer( GetAttributeContainer(), uno::UNO_QUERY_THROW );
        if( xNameContainer->hasByName( sName ) )
            xNameContainer->removeByName( sName );
        xNameContainer->insertByName( sName, aValue );
        m_xProps->setPropertyValue("UserDefinedAttributes", uno::makeAny( xNameContainer ) );
    }
}
// OOo do not support below API
uno::Any SAL_CALL
ScVbaInterior::getPattern() throw (uno::RuntimeException)
{
    // XlPattern
    uno::Any aPattern = GetUserDefinedAttributes( PATTERN );
    if( aPattern.hasValue() )
        return uno::makeAny( GetAttributeData( aPattern ) );
    return uno::makeAny( excel::XlPattern::xlPatternNone );
}
void SAL_CALL
ScVbaInterior::setPattern( const uno::Any& _pattern ) throw (uno::RuntimeException)
{
    if( _pattern >>= m_nPattern )
    {
        SetUserDefinedAttributes( PATTERN, SetAttributeData( m_nPattern ) );
        SetMixedColor();
    }
    else
        throw uno::RuntimeException("Invalid Pattern index", uno::Reference< uno::XInterface >() );
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
        aBackColor.SetColor( nColor );
    }
    else
    {
        uno::Any aAny;
        aAny = OORGBToXLRGB( m_xProps->getPropertyValue( BACKCOLOR ) );
        if( aAny >>= nColor )
        {
            nColor = XLRGBToOORGB( nColor );
            aBackColor.SetColor( nColor );
            SetUserDefinedAttributes( BACKCOLOR, SetAttributeData( nColor ) );
        }
    }
    return aBackColor;
}
uno::Any SAL_CALL
ScVbaInterior::getPatternColor() throw (uno::RuntimeException)
{
    // 0 is the default color. no filled.
    uno::Any aPatternColor = GetUserDefinedAttributes( PATTERNCOLOR );
    if( aPatternColor.hasValue() )
    {
        sal_uInt32 nPatternColor = GetAttributeData( aPatternColor );
        return uno::makeAny( OORGBToXLRGB( nPatternColor ) );
    }
    return uno::makeAny( sal_Int32( 0 ) );
}
void SAL_CALL
ScVbaInterior::setPatternColor( const uno::Any& _patterncolor ) throw (uno::RuntimeException)
{
    sal_Int32 nPattColor = 0;
    if( _patterncolor >>= nPattColor )
    {
        SetUserDefinedAttributes( PATTERNCOLOR, SetAttributeData( XLRGBToOORGB( nPattColor ) ) );
        SetMixedColor();
    }
    else
        throw uno::RuntimeException("Invalid Pattern Color", uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL
ScVbaInterior::getPatternColorIndex() throw (uno::RuntimeException)
{
    sal_Int32 nColor = 0;
    XLRGBToOORGB( getPatternColor() ) >>= nColor;

    return uno::makeAny( GetIndexColor( nColor ) );
}
void SAL_CALL
ScVbaInterior::setPatternColorIndex( const uno::Any& _patterncolorindex ) throw (uno::RuntimeException)
{
    sal_Int32 nColorIndex = 0;
    if( _patterncolorindex >>= nColorIndex )
    {
        if( nColorIndex == 0 )
            return;
        sal_Int32 nPattColor = 0;
        GetIndexColor( nColorIndex ) >>= nPattColor;
        setPatternColor( uno::makeAny( OORGBToXLRGB( nPattColor ) ) );
    }
    else
        throw uno::RuntimeException("Invalid Pattern Color", uno::Reference< uno::XInterface >() );
}

OUString
ScVbaInterior::getServiceImplName()
{
    return OUString("ScVbaInterior");
}

uno::Sequence< OUString >
ScVbaInterior::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.Interior" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
