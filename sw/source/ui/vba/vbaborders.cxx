/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbaborders.hxx"
#include <ooo/vba/word/XBorder.hpp>
#include <ooo/vba/word/WdBorderType.hpp>
#include <ooo/vba/word/WdLineStyle.hpp>
#include <sal/macros.h>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include "vbapalette.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;


typedef ::cppu::WeakImplHelper1<container::XIndexAccess > RangeBorders_Base;
typedef InheritedHelperInterfaceImpl1<word::XBorder > SwVbaBorder_Base;

// #TODO sort these indexes to match the order in which Word iterates over the
// borders, the enumeration will match the order in this list
static const sal_Int16 supportedIndexTable[] = { word::WdBorderType::wdBorderBottom, word::WdBorderType::wdBorderDiagonalDown, word::WdBorderType::wdBorderDiagonalUp, word::WdBorderType::wdBorderHorizontal, word::WdBorderType::wdBorderLeft, word::WdBorderType::wdBorderRight, word::WdBorderType::wdBorderTop, word::WdBorderType::wdBorderVertical };

const static rtl::OUString sTableBorder( RTL_CONSTASCII_USTRINGPARAM("TableBorder") );

//  Equiv widths in in 1/100 mm
const static sal_Int32 OOLineThin = 35;
const static sal_Int32 OOLineMedium = 88;
const static sal_Int32 OOLineThick = 141;
const static sal_Int32 OOLineHairline = 2;

class SwVbaBorder : public SwVbaBorder_Base
{
private:
    uno::Reference< beans::XPropertySet > m_xProps;
    sal_Int32 m_LineType;
    VbaPalette m_Palette;
    bool setBorderLine( table::BorderLine& rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;

        switch ( m_LineType )
        {
            case word::WdBorderType::wdBorderLeft:
                aTableBorder.IsLeftLineValid = sal_True;
                aTableBorder.LeftLine= rBorderLine;
                break;
            case word::WdBorderType::wdBorderTop:
                aTableBorder.IsTopLineValid = sal_True;
                aTableBorder.TopLine = rBorderLine;
                break;

            case word::WdBorderType::wdBorderBottom:
                aTableBorder.IsBottomLineValid = sal_True;
                aTableBorder.BottomLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderRight:
                aTableBorder.IsRightLineValid = sal_True;
                aTableBorder.RightLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderVertical:
                aTableBorder.IsVerticalLineValid = sal_True;
                aTableBorder.VerticalLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderHorizontal:
                aTableBorder.IsHorizontalLineValid = sal_True;
                aTableBorder.HorizontalLine = rBorderLine;
                break;
            case word::WdBorderType::wdBorderDiagonalDown:
            case word::WdBorderType::wdBorderDiagonalUp:
                // #TODO have to ignore at the momement, would be
                // nice to investigate what we can do here
                break;
            default:
                    return false;
        }
        m_xProps->setPropertyValue( sTableBorder, uno::makeAny(aTableBorder) );
        return true;
    }

    bool getBorderLine( table::BorderLine& rBorderLine )
    {
        table::TableBorder aTableBorder;
        m_xProps->getPropertyValue( sTableBorder ) >>= aTableBorder;
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
                // #TODO have to ignore at the momement, would be
                // nice to investigate what we can do here
                break;
            default:
                    return false;
        }
        return true;
    }
    SwVbaBorder(); // no impl
protected:
    virtual rtl::OUString& getServiceImplName()
    {
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaBorder") );
    return sImplName;
    }
    virtual css::uno::Sequence<rtl::OUString> getServiceNames()
    {
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
            aServiceNames.realloc( 1 );
            aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Border" ) );
        }
        return aServiceNames;
    }
public:
    SwVbaBorder( const uno::Reference< beans::XPropertySet > & xProps, const uno::Reference< uno::XComponentContext >& xContext, sal_Int32 lineType, VbaPalette& rPalette) : SwVbaBorder_Base( uno::Reference< XHelperInterface >( xProps, uno::UNO_QUERY ), xContext ), m_xProps( xProps ), m_LineType( lineType ), m_Palette( rPalette ) {}

    uno::Any SAL_CALL getLineStyle() throw (uno::RuntimeException)
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
        return uno::makeAny( nLineStyle );
    }
    void SAL_CALL setLineStyle( const uno::Any& _linestyle ) throw (uno::RuntimeException)
    {
        // Urk no choice but to silently ignore we don't support this attribute
        // #TODO would be nice to support the word line styles
        sal_Int32 nLineStyle = 0;
        _linestyle >>= nLineStyle;
        table::BorderLine aBorderLine;
        if ( getBorderLine( aBorderLine ) )
        {
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
                    throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Bad param" ) ), uno::Reference< uno::XInterface >() );
            }
            setBorderLine( aBorderLine );
        }
        else
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Method failed" ) ), uno::Reference< uno::XInterface >() );
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
        // hokay return position of the index in the table
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
    RangeBorders(  const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, VbaPalette& rPalette ) : m_xRange( xRange ), m_xContext( xContext ), m_Palette( rPalette )
    {
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return SAL_N_ELEMENTS( supportedIndexTable );
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {

        sal_Int32 nIndex = getTableIndex( Index );
        if ( nIndex >= 0 && nIndex < getCount() )
        {
            uno::Reference< beans::XPropertySet > xProps( m_xRange, uno::UNO_QUERY_THROW );
            return uno::makeAny( uno::Reference< word::XBorder >( new SwVbaBorder( xProps, m_xContext, supportedIndexTable[ nIndex ], m_Palette )) );
        }
        throw lang::IndexOutOfBoundsException();
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return  word::XBorder::static_type(0);
    }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
};

uno::Reference< container::XIndexAccess >
rangeToBorderIndexAccess( const uno::Reference< table::XCellRange >& xRange,  const uno::Reference< uno::XComponentContext > & xContext, VbaPalette& rPalette )
{
    return new RangeBorders( xRange, xContext, rPalette );
}

class RangeBorderEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 nIndex;
public:
    RangeBorderEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( nIndex++ );
        throw container::NoSuchElementException();
    }
};

// for Table borders
SwVbaBorders::SwVbaBorders( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< table::XCellRange >& xRange, VbaPalette& rPalette  ):  SwVbaBorders_BASE( xParent, xContext, rangeToBorderIndexAccess( xRange ,xContext, rPalette ) )
{
    m_xProps.set( xRange, uno::UNO_QUERY_THROW );
}

uno::Reference< container::XEnumeration >
SwVbaBorders::createEnumeration() throw (uno::RuntimeException)
{
    return new RangeBorderEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaBorders::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // its already a Border object
}

uno::Type
SwVbaBorders::getElementType() throw (uno::RuntimeException)
{
    return word::XBorders::static_type(0);
}

uno::Any
SwVbaBorders::getItemByIntIndex( const sal_Int32 nIndex )  throw (uno::RuntimeException)
{
    return createCollectionObject( m_xIndexAccess->getByIndex( nIndex ) );
}

sal_Bool SAL_CALL SwVbaBorders::getShadow() throw (uno::RuntimeException)
{
    /*
    table::ShadowFormat aShadowFormat;
    m_xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShadowFormat") ) ) >>= aShadowFormat;
    return ( aShadowFormat.Location != table::ShadowLocation_NONE );
    */
    // always return False for table border in MS Word
    return sal_False;
}

void SAL_CALL SwVbaBorders::setShadow( sal_Bool /*_shadow*/ ) throw (uno::RuntimeException)
{
    // not support in Table border in Word
    // TODO:
}

rtl::OUString&
SwVbaBorders::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaBorders") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaBorders::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Borders" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
