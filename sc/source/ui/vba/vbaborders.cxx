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
#include "vbaborders.hxx"

#include <sal/macros.h>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XlBordersIndex.hpp>
#include <ooo/vba/excel/XlBorderWeight.hpp>
#include <ooo/vba/excel/XlLineStyle.hpp>
#include <ooo/vba/excel/XlColorIndex.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>

#include "vbapalette.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel;

typedef ::cppu::WeakImplHelper<container::XIndexAccess > RangeBorders_Base;
typedef InheritedHelperInterfaceWeakImpl<excel::XBorder > ScVbaBorder_Base;

// #TODO sort these indexes to match the order in which Excel iterates over the
// borders, the enumeration will match the order in this list
const sal_Int16 supportedIndexTable[] = {  XlBordersIndex::xlEdgeLeft, XlBordersIndex::xlEdgeTop, XlBordersIndex::xlEdgeBottom, XlBordersIndex::xlEdgeRight, XlBordersIndex::xlDiagonalDown, XlBordersIndex::xlDiagonalUp, XlBordersIndex::xlInsideVertical, XlBordersIndex::xlInsideHorizontal };

constexpr OUStringLiteral sTableBorder = u"TableBorder";

//  Equiv widths in 1/100 mm
const sal_Int32 OOLineThin = 26;
const sal_Int32 OOLineMedium = 88;
const sal_Int32 OOLineThick = 141;
const sal_Int32 OOLineHairline = 2;

namespace {

class ScVbaBorder : public ScVbaBorder_Base
{
private:
    uno::Reference< beans::XPropertySet > m_xProps;
    sal_Int32 m_LineType;
    ScVbaPalette m_Palette;
    void setBorderLine( const table::BorderLine& rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;

        switch ( m_LineType )
        {
            case XlBordersIndex::xlEdgeLeft:
                aTableBorder.IsLeftLineValid = true;
                aTableBorder.LeftLine= rBorderLine;
                break;
            case XlBordersIndex::xlEdgeTop:
                aTableBorder.IsTopLineValid = true;
                aTableBorder.TopLine = rBorderLine;
                break;

            case XlBordersIndex::xlEdgeBottom:
                aTableBorder.IsBottomLineValid = true;
                aTableBorder.BottomLine = rBorderLine;
                break;
            case XlBordersIndex::xlEdgeRight:
                aTableBorder.IsRightLineValid = true;
                aTableBorder.RightLine = rBorderLine;
                break;
            case XlBordersIndex::xlInsideVertical:
                aTableBorder.IsVerticalLineValid = true;
                aTableBorder.VerticalLine = rBorderLine;
                break;
            case XlBordersIndex::xlInsideHorizontal:
                aTableBorder.IsHorizontalLineValid = true;
                aTableBorder.HorizontalLine = rBorderLine;
                break;
            case XlBordersIndex::xlDiagonalDown:
            case XlBordersIndex::xlDiagonalUp:
                // #TODO have to ignore at the moment, would be
                // nice to investigate what we can do here
                break;
            default:
                    return;
        }
        m_xProps->setPropertyValue( sTableBorder, uno::makeAny(aTableBorder) );
    }

    bool getBorderLine( table::BorderLine& rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;
        switch ( m_LineType )
        {
            case XlBordersIndex::xlEdgeLeft:
                if ( aTableBorder.IsLeftLineValid )
                    rBorderLine = aTableBorder.LeftLine;
                break;
            case XlBordersIndex::xlEdgeTop:
                if ( aTableBorder.IsTopLineValid )
                    rBorderLine = aTableBorder.TopLine;
                break;

            case XlBordersIndex::xlEdgeBottom:
                if ( aTableBorder.IsBottomLineValid )
                    rBorderLine = aTableBorder.BottomLine;
                break;
            case XlBordersIndex::xlEdgeRight:
                if ( aTableBorder.IsRightLineValid )
                    rBorderLine = aTableBorder.RightLine;
                break;
            case XlBordersIndex::xlInsideVertical:
                if ( aTableBorder.IsVerticalLineValid )
                    rBorderLine = aTableBorder.VerticalLine;
                break;
            case XlBordersIndex::xlInsideHorizontal:
                if ( aTableBorder.IsHorizontalLineValid )
                    rBorderLine = aTableBorder.HorizontalLine;
                break;

            case XlBordersIndex::xlDiagonalDown:
            case XlBordersIndex::xlDiagonalUp:
                // #TODO have to ignore at the moment, would be
                // nice to investigate what we can do here
                break;
            default:
                    return false;
        }
        return true;
    }

protected:
    virtual OUString getServiceImplName() override
    {
        return "ScVbaBorder";
    }
    virtual css::uno::Sequence<OUString> getServiceNames() override
    {
        static uno::Sequence< OUString > const aServiceNames
        {
            "ooo.vba.excel.Border"
        };
        return aServiceNames;
    }
public:
    ScVbaBorder( const uno::Reference< beans::XPropertySet > & xProps, const uno::Reference< uno::XComponentContext >& xContext, sal_Int32 lineType, const ScVbaPalette& rPalette) : ScVbaBorder_Base( uno::Reference< XHelperInterface >( xProps, uno::UNO_QUERY ), xContext ), m_xProps( xProps ), m_LineType( lineType ), m_Palette( rPalette ) {}

    // XBorder
    uno::Any SAL_CALL getColor() override
    {
        table::BorderLine aBorderLine;
        if ( getBorderLine( aBorderLine ) )
            return uno::makeAny( OORGBToXLRGB( Color(ColorTransparency, aBorderLine.Color) ) );
        throw uno::RuntimeException("No Implementation available" );
    }
    void SAL_CALL setColor( const uno::Any& _color ) override
    {
        sal_Int32 nColor = 0;
        _color >>= nColor;
        table::BorderLine aBorderLine;
        if ( !getBorderLine( aBorderLine ) )
            throw uno::RuntimeException("No Implementation available" );

        aBorderLine.Color = XLRGBToOORGB( nColor );
        setBorderLine( aBorderLine );

    }

    uno::Any SAL_CALL getColorIndex() override
    {
        sal_Int32 nColor = 0;
        XLRGBToOORGB( getColor() ) >>= nColor;
        uno::Reference< container::XIndexAccess > xIndex = m_Palette.getPalette();
        sal_Int32 nElems = xIndex->getCount();
        sal_Int32 nIndex = -1;
        for ( sal_Int32 count=0; count<nElems; ++count )
        {
            sal_Int32 nPaletteColor = 0;
            xIndex->getByIndex( count ) >>= nPaletteColor;
            if ( nPaletteColor == nColor )
            {
                nIndex = count + 1;
                break;
            }
        }
        return uno::makeAny(nIndex);
    }

    void SAL_CALL setColorIndex( const uno::Any& _colorindex ) override
    {
        sal_Int32 nColor = 0;
        _colorindex >>= nColor;
        if ( !nColor || nColor == XlColorIndex::xlColorIndexAutomatic )
            nColor = 1;
        setColor( OORGBToXLRGB( m_Palette.getPalette()->getByIndex( --nColor )  ) );
    }
    uno::Any SAL_CALL getWeight() override
    {
        table::BorderLine aBorderLine;
        if ( getBorderLine( aBorderLine ) )
        {
            switch ( aBorderLine.OuterLineWidth )
            {
                case 0: // Thin = default OO thickness
                case OOLineThin:
                    return uno::makeAny( XlBorderWeight::xlThin );
                case OOLineMedium:
                    return uno::makeAny( XlBorderWeight::xlMedium );
                case OOLineThick:
                    return uno::makeAny( XlBorderWeight::xlThick );
                case OOLineHairline:
                    return uno::makeAny( XlBorderWeight::xlHairline );
                default:
                    break;
            }
        }
        throw uno::RuntimeException("Method failed" );
    }
    void SAL_CALL setWeight( const uno::Any& _weight ) override
    {
        sal_Int32 nWeight = 0;
        _weight >>= nWeight;
        table::BorderLine aBorderLine;
        if ( !getBorderLine( aBorderLine ) )
                    throw uno::RuntimeException("Method failed" );

        switch ( nWeight )
        {
            case XlBorderWeight::xlThin:
                aBorderLine.OuterLineWidth = OOLineThin;
                break;
            case XlBorderWeight::xlMedium:
                aBorderLine.OuterLineWidth = OOLineMedium;
                break;
            case XlBorderWeight::xlThick:
                aBorderLine.OuterLineWidth = OOLineThick;
                break;
            case XlBorderWeight::xlHairline:
                aBorderLine.OuterLineWidth = OOLineHairline;
                break;
            default:
                throw uno::RuntimeException("Bad param" );
        }
        setBorderLine( aBorderLine );

    }

    void SAL_CALL setTintAndShade( const uno::Any& /*rAny*/ ) override
    {
        // TODO implement
    }
    uno::Any SAL_CALL getTintAndShade() override
    {
        // TODO implement
        return uno::makeAny(static_cast<double>(0));
    }

    uno::Any SAL_CALL getLineStyle() override
    {
        // always return xlContinuous;
        return uno::makeAny( XlLineStyle::xlContinuous );
    }
    void SAL_CALL setLineStyle( const uno::Any& _linestyle ) override
    {
        // Urk no choice but to silently ignore we don't support this attribute
        // #TODO would be nice to support the excel line styles
        sal_Int32 nLineStyle = 0;
        _linestyle >>= nLineStyle;
        table::BorderLine aBorderLine;
        if ( !getBorderLine( aBorderLine ) )
            throw uno::RuntimeException("Method failed" );

        switch ( nLineStyle )
        {
            case XlLineStyle::xlContinuous:
            case XlLineStyle::xlDash:
            case XlLineStyle::xlDashDot:
            case XlLineStyle::xlDashDotDot:
            case XlLineStyle::xlDot:
            case XlLineStyle::xlDouble:
            case XlLineStyle::xlLineStyleNone:
            case XlLineStyle::xlSlantDashDot:
                break;
            default:
                throw uno::RuntimeException("Bad param" );
        }
        setBorderLine( aBorderLine );

    }
};

class RangeBorders : public RangeBorders_Base
{
private:
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > m_xContext;
    ScVbaPalette m_Palette;
    sal_Int32 getTableIndex( sal_Int32 nConst )
    {
        // okay return position of the index in the table
        sal_Int32 nIndexes = getCount();
        sal_Int32 realIndex = 0;
        const sal_Int16* pTableEntry = supportedIndexTable;
        for ( ; realIndex < nIndexes; ++realIndex, ++pTableEntry )
        {
            if ( *pTableEntry == nConst )
                return realIndex;
        }
        return getCount(); // error condition
    }
public:
    RangeBorders(  const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, const ScVbaPalette& rPalette ) : m_xRange( xRange ), m_xContext( xContext ), m_Palette( rPalette )
    {
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return SAL_N_ELEMENTS( supportedIndexTable );
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {

        sal_Int32 nIndex = getTableIndex( Index );
        if ( nIndex >= 0 && nIndex < getCount() )
        {
            uno::Reference< beans::XPropertySet > xProps( m_xRange, uno::UNO_QUERY_THROW );
            return uno::makeAny( uno::Reference< excel::XBorder >( new ScVbaBorder( xProps, m_xContext, supportedIndexTable[ nIndex ], m_Palette )) );
        }
        throw lang::IndexOutOfBoundsException();
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return  cppu::UnoType<excel::XBorder>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
};

}

static uno::Reference< container::XIndexAccess >
rangeToBorderIndexAccess( const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, const ScVbaPalette& rPalette )
{
    return new RangeBorders( xRange, xContext, rPalette );
}

namespace {

class RangeBorderEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 nIndex;
public:
    explicit RangeBorderEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( nIndex++ );
        throw container::NoSuchElementException();
    }
};

}

ScVbaBorders::ScVbaBorders( const uno::Reference< XHelperInterface >& xParent,
                            const uno::Reference< uno::XComponentContext > & xContext,
                            const uno::Reference< table::XCellRange >& xRange,
                            const ScVbaPalette& rPalette  )
    :  ScVbaBorders_BASE( xParent, xContext, rangeToBorderIndexAccess( xRange ,xContext, rPalette ) ), bRangeIsSingleCell( false )
{
    uno::Reference< table::XColumnRowRange > xColumnRowRange(xRange, uno::UNO_QUERY_THROW );
    if ( xColumnRowRange->getRows()->getCount() == 1 && xColumnRowRange->getColumns()->getCount() == 1 )
        bRangeIsSingleCell = true;
    m_xProps.set( xRange, uno::UNO_QUERY_THROW );
}

uno::Reference< container::XEnumeration >
ScVbaBorders::createEnumeration()
{
    return new RangeBorderEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaBorders::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // it's already a Border object
}

uno::Type
ScVbaBorders::getElementType()
{
    return cppu::UnoType<excel::XBorders>::get();
}

uno::Any
ScVbaBorders::getItemByIntIndex( const sal_Int32 nIndex )
{
    return createCollectionObject( m_xIndexAccess->getByIndex( nIndex ) );
}

uno::Any SAL_CALL ScVbaBorders::getColor()
{
    sal_Int32 count = getCount();
    uno::Any color;
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        if( XlBordersIndex::xlDiagonalDown != supportedIndexTable[i] && XlBordersIndex::xlDiagonalUp != supportedIndexTable[i] )
        {
            uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
            if( color.hasValue() )
            {
                if( color != xBorder->getColor() )
                    return uno::makeAny( uno::Reference< uno::XInterface >() );
            }
            else
                color = xBorder->getColor();
        }
    }
    return  color;
}
void SAL_CALL ScVbaBorders::setColor( const uno::Any& _color )
{
    sal_Int32 count = getCount();
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
        xBorder->setColor( _color );
    }
}
uno::Any SAL_CALL ScVbaBorders::getColorIndex()
{
    sal_Int32 count = getCount();
    uno::Any nColorIndex;
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        if( XlBordersIndex::xlDiagonalDown != supportedIndexTable[i] && XlBordersIndex::xlDiagonalUp != supportedIndexTable[i] )
        {
            uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
            if( nColorIndex.hasValue() )
            {
                if( nColorIndex != xBorder->getColorIndex() )
                    return uno::makeAny( uno::Reference< uno::XInterface >() );
            }
            else
                nColorIndex = xBorder->getColorIndex();
        }
    }
    return  nColorIndex;
}
void SAL_CALL ScVbaBorders::setColorIndex( const uno::Any& _colorindex )
{
    sal_Int32 count = getCount();
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
        xBorder->setColorIndex( _colorindex );
    }
}

static bool
lcl_areAllLineWidthsSame( const table::TableBorder& maTableBorder, bool bIsCell )
{

    bool bRes = false;
    if (bIsCell)
    {
        bRes = ((maTableBorder.TopLine.OuterLineWidth == maTableBorder.BottomLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.LeftLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.RightLine.OuterLineWidth));
    }
    else
    {
        bRes = ((maTableBorder.TopLine.OuterLineWidth == maTableBorder.BottomLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.LeftLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.HorizontalLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.VerticalLine.OuterLineWidth) &&
(maTableBorder.TopLine.OuterLineWidth == maTableBorder.RightLine.OuterLineWidth));
    }
    return bRes;
}

uno::Any SAL_CALL ScVbaBorders::getLineStyle()
{
    table::TableBorder aTableBorder;
    m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;

    sal_Int32 aLinestyle =  XlLineStyle::xlLineStyleNone;

    if ( lcl_areAllLineWidthsSame( aTableBorder, bRangeIsSingleCell ))
    {
        if (aTableBorder.TopLine.LineDistance != 0)
        {
            aLinestyle = XlLineStyle::xlDouble;
        }
        else if ( aTableBorder.TopLine.OuterLineWidth != 0 )
        {
            aLinestyle = XlLineStyle::xlContinuous;
        }
    }
    return uno::makeAny( aLinestyle );
}
void SAL_CALL ScVbaBorders::setLineStyle( const uno::Any& _linestyle )
{
    sal_Int32 count = getCount();
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
        xBorder->setLineStyle( _linestyle );
    }
}
uno::Any SAL_CALL ScVbaBorders::getWeight()
{
    sal_Int32 count = getCount();
    uno::Any weight;
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        if( XlBordersIndex::xlDiagonalDown != supportedIndexTable[i] && XlBordersIndex::xlDiagonalUp != supportedIndexTable[i] )
        {
            uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
            if( weight.hasValue() )
            {
                if( weight != xBorder->getWeight() )
                    return uno::makeAny( uno::Reference< uno::XInterface >() );
            }
            else
                weight = xBorder->getWeight();
        }
    }
    return  weight;
}

uno::Any SAL_CALL ScVbaBorders::getTintAndShade()
{
    // TODO implement
    return uno::makeAny(static_cast<double>(0));
}

void SAL_CALL ScVbaBorders::setTintAndShade(const uno::Any& /*rAny*/)
{
    // TODO implement
}

void SAL_CALL ScVbaBorders::setWeight( const uno::Any& _weight )
{
    sal_Int32 count = getCount();
    for( sal_Int32 i = 0; i < count ; i++ )
    {
        uno::Reference< XBorder > xBorder( getItemByIntIndex( supportedIndexTable[i] ), uno::UNO_QUERY_THROW );
        xBorder->setWeight( _weight );
    }
}

OUString
ScVbaBorders::getServiceImplName()
{
    return "ScVbaBorders";
}

uno::Sequence< OUString >
ScVbaBorders::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Borders"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
