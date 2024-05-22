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
#include <ooo/vba/word/XBorder.hpp>
#include <ooo/vba/word/WdBorderType.hpp>
#include <ooo/vba/word/WdLineStyle.hpp>
#include <sal/macros.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <utility>
#include "vbapalette.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

typedef ::cppu::WeakImplHelper<container::XIndexAccess > RangeBorders_Base;
typedef InheritedHelperInterfaceWeakImpl<word::XBorder > SwVbaBorder_Base;

// #TODO sort these indexes to match the order in which Word iterates over the
// borders, the enumeration will match the order in this list
const sal_Int16 supportedIndexTable[] = { word::WdBorderType::wdBorderBottom, word::WdBorderType::wdBorderDiagonalDown, word::WdBorderType::wdBorderDiagonalUp, word::WdBorderType::wdBorderHorizontal, word::WdBorderType::wdBorderLeft, word::WdBorderType::wdBorderRight, word::WdBorderType::wdBorderTop, word::WdBorderType::wdBorderVertical };

//  Equiv widths in 1/100 mm
const sal_Int32 OOLineHairline = 2;

namespace {

class SwVbaBorder : public SwVbaBorder_Base
{
private:
    uno::Reference< beans::XPropertySet > m_xProps;
    sal_Int32 m_LineType;
    void setBorderLine( table::BorderLine const & rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( u"TableBorder"_ustr ) >>= aTableBorder;

        switch ( m_LineType )
        {
            case word::WdBorderType::wdBorderLeft:
                aTableBorder.IsLeftLineValid = true;
                aTableBorder.LeftLine= rBorderLine;
                break;
            case word::WdBorderType::wdBorderTop:
                aTableBorder.IsTopLineValid = true;
                aTableBorder.TopLine = rBorderLine;
                break;

            case word::WdBorderType::wdBorderBottom:
                aTableBorder.IsBottomLineValid = true;
                aTableBorder.BottomLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderRight:
                aTableBorder.IsRightLineValid = true;
                aTableBorder.RightLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderVertical:
                aTableBorder.IsVerticalLineValid = true;
                aTableBorder.VerticalLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderHorizontal:
                aTableBorder.IsHorizontalLineValid = true;
                aTableBorder.HorizontalLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderDiagonalDown:
            case word::WdBorderType::wdBorderDiagonalUp:
                // #TODO have to ignore at the moment, would be
                // nice to investigate what we can do here
                break;
            default:
                return;
        }
        m_xProps->setPropertyValue( u"TableBorder"_ustr, uno::Any(aTableBorder) );
    }

    bool getBorderLine( table::BorderLine& rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( u"TableBorder"_ustr ) >>= aTableBorder;
        switch ( m_LineType )
        {
            case word::WdBorderType::wdBorderLeft:
                if ( aTableBorder.IsLeftLineValid )
                    rBorderLine = aTableBorder.LeftLine;
                break;
            case word::WdBorderType::wdBorderTop:
                if ( aTableBorder.IsTopLineValid )
                    rBorderLine = aTableBorder.TopLine;
                break;
            case word::WdBorderType::wdBorderBottom:
                if ( aTableBorder.IsBottomLineValid )
                    rBorderLine = aTableBorder.BottomLine;
                break;
            case word::WdBorderType::wdBorderRight:
                if ( aTableBorder.IsRightLineValid )
                    rBorderLine = aTableBorder.RightLine;
                break;
            case word::WdBorderType::wdBorderVertical:
                if ( aTableBorder.IsVerticalLineValid )
                    rBorderLine = aTableBorder.VerticalLine;
                break;
            case word::WdBorderType::wdBorderHorizontal:
                if ( aTableBorder.IsHorizontalLineValid )
                    rBorderLine = aTableBorder.HorizontalLine;
                break;

            case word::WdBorderType::wdBorderDiagonalDown:
            case word::WdBorderType::wdBorderDiagonalUp:
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
        return u"SwVbaBorder"_ustr;
    }

    virtual css::uno::Sequence<OUString> getServiceNames() override
    {
        static uno::Sequence< OUString > const aServiceNames
        {
            u"ooo.vba.word.Border"_ustr
        };
        return aServiceNames;
    }
public:
    SwVbaBorder( const uno::Reference< beans::XPropertySet > & xProps, const uno::Reference< uno::XComponentContext >& xContext, sal_Int32 lineType ) : SwVbaBorder_Base( uno::Reference< XHelperInterface >( xProps, uno::UNO_QUERY ), xContext ), m_xProps( xProps ), m_LineType( lineType ) {}

    uno::Any SAL_CALL getLineStyle() override
    {
        sal_Int32 nLineStyle = word::WdLineStyle::wdLineStyleNone;
        table::BorderLine aBorderLine;
        if ( getBorderLine( aBorderLine ) )
        {
            if( aBorderLine.InnerLineWidth !=0 && aBorderLine.OuterLineWidth !=0 )
            {
                nLineStyle = word::WdLineStyle::wdLineStyleDouble;
            }
            else if( aBorderLine.InnerLineWidth !=0 || aBorderLine.OuterLineWidth !=0 )
            {
                nLineStyle = word::WdLineStyle::wdLineStyleSingle;
            }
            else
            {
                nLineStyle = word::WdLineStyle::wdLineStyleNone;
            }
        }
        return uno::Any( nLineStyle );
    }
    void SAL_CALL setLineStyle( const uno::Any& _linestyle ) override
    {
        // Urk no choice but to silently ignore we don't support this attribute
        // #TODO would be nice to support the word line styles
        sal_Int32 nLineStyle = 0;
        _linestyle >>= nLineStyle;
        table::BorderLine aBorderLine;
        if ( !getBorderLine( aBorderLine ) )
            throw uno::RuntimeException(u"Method failed"_ustr );

        switch ( nLineStyle )
        {
            case word::WdLineStyle::wdLineStyleNone:
            {
                aBorderLine.InnerLineWidth = 0;
                aBorderLine.OuterLineWidth = 0;
                break;
            }
            case word::WdLineStyle::wdLineStyleDashDot:
            case word::WdLineStyle::wdLineStyleDashDotDot:
            case word::WdLineStyle::wdLineStyleDashDotStroked:
            case word::WdLineStyle::wdLineStyleDashLargeGap:
            case word::WdLineStyle::wdLineStyleDashSmallGap:
            case word::WdLineStyle::wdLineStyleDot:
            case word::WdLineStyle::wdLineStyleDouble:
            case word::WdLineStyle::wdLineStyleDoubleWavy:
            case word::WdLineStyle::wdLineStyleEmboss3D:
            case word::WdLineStyle::wdLineStyleEngrave3D:
            case word::WdLineStyle::wdLineStyleInset:
            case word::WdLineStyle::wdLineStyleOutset:
            case word::WdLineStyle::wdLineStyleSingle:
            case word::WdLineStyle::wdLineStyleSingleWavy:
            case word::WdLineStyle::wdLineStyleThickThinLargeGap:
            case word::WdLineStyle::wdLineStyleThickThinMedGap:
            case word::WdLineStyle::wdLineStyleThickThinSmallGap:
            case word::WdLineStyle::wdLineStyleThinThickLargeGap:
            case word::WdLineStyle::wdLineStyleThinThickMedGap:
            case word::WdLineStyle::wdLineStyleThinThickSmallGap:
            case word::WdLineStyle::wdLineStyleThinThickThinLargeGap:
            case word::WdLineStyle::wdLineStyleThinThickThinMedGap:
            case word::WdLineStyle::wdLineStyleThinThickThinSmallGap:
            case word::WdLineStyle::wdLineStyleTriple:
            {
                aBorderLine.InnerLineWidth = 0;
                aBorderLine.OuterLineWidth = OOLineHairline;
                break;
            }
            default:
                throw uno::RuntimeException(u"Bad param"_ustr );
        }
        setBorderLine( aBorderLine );

    }
};

class RangeBorders : public RangeBorders_Base
{
private:
    uno::Reference< table::XCellRange > m_xRange;
    uno::Reference< uno::XComponentContext > m_xContext;
    VbaPalette m_Palette;
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
    RangeBorders(  uno::Reference< table::XCellRange > xRange,  uno::Reference< uno::XComponentContext > xContext, VbaPalette aPalette ) : m_xRange(std::move( xRange )), m_xContext(std::move( xContext )), m_Palette(std::move( aPalette ))
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
            return uno::Any( uno::Reference< word::XBorder >( new SwVbaBorder( xProps, m_xContext, supportedIndexTable[ nIndex ] )) );
        }
        throw lang::IndexOutOfBoundsException();
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return  cppu::UnoType<word::XBorder>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
};

}

static uno::Reference< container::XIndexAccess >
rangeToBorderIndexAccess( const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, VbaPalette const & rPalette )
{
    return new RangeBorders( xRange, xContext, rPalette );
}

namespace {

class RangeBorderEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 m_nIndex;
public:
    explicit RangeBorderEnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : m_xIndexAccess(std::move( xIndexAccess )), m_nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( m_nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( m_nIndex++ );
        throw container::NoSuchElementException();
    }
};

}

// for Table borders
SwVbaBorders::SwVbaBorders( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< table::XCellRange >& xRange, VbaPalette const & rPalette  ):  SwVbaBorders_BASE( xParent, xContext, rangeToBorderIndexAccess( xRange ,xContext, rPalette ) )
{
    m_xProps.set( xRange, uno::UNO_QUERY_THROW );
}

uno::Reference< container::XEnumeration >
SwVbaBorders::createEnumeration()
{
    return new RangeBorderEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaBorders::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // it's already a Border object
}

uno::Type
SwVbaBorders::getElementType()
{
    return cppu::UnoType<word::XBorders>::get();
}

uno::Any
SwVbaBorders::getItemByIntIndex( const sal_Int32 nIndex )
{
    return createCollectionObject( m_xIndexAccess->getByIndex( nIndex ) );
}

sal_Bool SAL_CALL SwVbaBorders::getShadow()
{
    // always return False for table border in MS Word
    return false;
}

void SAL_CALL SwVbaBorders::setShadow( sal_Bool /*_shadow*/ )
{
    // not support in Table border in Word
    // TODO:
}

OUString
SwVbaBorders::getServiceImplName()
{
    return u"SwVbaBorders"_ustr;
}

uno::Sequence< OUString >
SwVbaBorders::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Borders"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
