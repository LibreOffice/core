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
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <ooo/vba/excel/XlColorIndex.hpp>
#include <ooo/vba/excel/XlUnderlineStyle.hpp>
#include <svl/itemset.hxx>
#include "excelvbahelper.hxx"
#include "vbafont.hxx"
#include "scitems.hxx"
#include "cellsuno.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaFont::ScVbaFont(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const ScVbaPalette& dPalette,
        const uno::Reference< beans::XPropertySet >& xPropertySet,
        ScCellRangeObj* pRangeObj, bool bFormControl ) throw ( uno::RuntimeException ) :
    ScVbaFont_BASE( xParent, xContext, dPalette.getPalette(), xPropertySet, bFormControl ),
    mPalette( dPalette ),
    mpRangeObj( pRangeObj )
{
}

SfxItemSet*
ScVbaFont::GetDataSet()
{
    return mpRangeObj ? excel::ScVbaCellRangeAccess::GetDataSet( mpRangeObj ) : nullptr;
}

ScVbaFont::~ScVbaFont()
{
}

static uno::Reference< beans::XPropertySet > lcl_TextProperties( uno::Reference< table::XCell >& xIf ) throw ( uno::RuntimeException )
{
    uno::Reference< text::XTextRange > xTxtRange( xIf, uno::UNO_QUERY_THROW );
    uno::Reference< text::XSimpleText > xTxt( xTxtRange->getText(), uno::UNO_QUERY_THROW ) ;
    uno::Reference< beans::XPropertySet > xProps( xTxt->createTextCursor(), uno::UNO_QUERY_THROW );
    return xProps;
}
void SAL_CALL
ScVbaFont::setSuperscript( const uno::Any& aValue ) throw ( uno::RuntimeException, std::exception )
{
    // #FIXEME create some sort of generic get/set code where
    // you can pass a functor
    // get/set - Super/sub script code is exactly the same
    // except for the call applied at each cell position
        uno::Reference< table::XCell> xCell( mxFont, uno::UNO_QUERY );
        uno::Reference< table::XCellRange > xCellRange( mxFont, uno::UNO_QUERY );
    if ( !xCell.is() )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xCellRange, uno::UNO_QUERY_THROW );
        sal_Int32 nCols = xColumnRowRange->getColumns()->getCount();
        sal_Int32 nRows = xColumnRowRange->getRows()->getCount();
        for ( sal_Int32 col = 0; col < nCols; ++col )
        {
            for ( sal_Int32 row = 0; row < nRows; ++row )
            {
                uno::Reference< beans::XPropertySet > xProps( xCellRange->getCellByPosition( col, row ) , uno::UNO_QUERY_THROW );
                uno::Reference< ScVbaFont > aFont( new ScVbaFont( getParent(), mxContext, mPalette, xProps ) );
                aFont->setSuperscript( aValue );
            }
        }
        return;

    }
        xCell.set( xCellRange->getCellByPosition( 0,0 ) );

    uno::Reference< beans::XPropertySet > xProps = lcl_TextProperties( xCell );
    bool bValue = false;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

        if( bValue )
    {
        nValue = SUPERSCRIPT;
            nValue2 = SUPERSCRIPTHEIGHT;
    }
    xProps->setPropertyValue("CharEscapement", uno::Any(nValue) );
     xProps->setPropertyValue("CharEscapementHeight", uno::Any(nValue2) );
}

uno::Any SAL_CALL
ScVbaFont::getSuperscript() throw ( uno::RuntimeException, std::exception )
{
        uno::Reference< table::XCell> xCell( mxFont, uno::UNO_QUERY );
        uno::Reference< table::XCellRange > xCellRange( mxFont, uno::UNO_QUERY );
    if ( !xCell.is() )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xCellRange, uno::UNO_QUERY_THROW );
        sal_Int32 nCols = xColumnRowRange->getColumns()->getCount();
        sal_Int32 nRows = xColumnRowRange->getRows()->getCount();
        uno::Any aRes;
        for ( sal_Int32 col = 0; col < nCols; ++col )
        {
            for ( sal_Int32 row = 0; row < nRows; ++row )
            {
                uno::Reference< beans::XPropertySet > xProps( xCellRange->getCellByPosition( col, row ), uno::UNO_QUERY_THROW );
                uno::Reference< ScVbaFont > aFont( new ScVbaFont( getParent(), mxContext, mPalette, xProps ) );
                if ( !col && !row )
                    aRes = aFont->getSuperscript();
                else if ( aRes != aFont->getSuperscript() )
                    return aNULL();
            }
        }
        return aRes;

    }
        xCell.set( xCellRange->getCellByPosition( 0,0 ) );
    uno::Reference< beans::XPropertySet > xProps = lcl_TextProperties( xCell );
    short nValue = 0;
    xProps->getPropertyValue("CharEscapement") >>= nValue;
    return uno::makeAny( ( nValue == SUPERSCRIPT ) );
}

void SAL_CALL
ScVbaFont::setSubscript( const uno::Any& aValue ) throw ( uno::RuntimeException, std::exception )
{
        uno::Reference< table::XCell> xCell( mxFont, uno::UNO_QUERY );
        uno::Reference< table::XCellRange > xCellRange( mxFont, uno::UNO_QUERY );
    if ( !xCell.is() )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xCellRange, uno::UNO_QUERY_THROW );
        sal_Int32 nCols = xColumnRowRange->getColumns()->getCount();
        sal_Int32 nRows = xColumnRowRange->getRows()->getCount();
        for ( sal_Int32 col = 0; col < nCols; ++col )
        {
            for ( sal_Int32 row = 0; row < nRows; ++row )
            {
                uno::Reference< beans::XPropertySet > xProps( xCellRange->getCellByPosition( col, row ) , uno::UNO_QUERY_THROW );
                uno::Reference< ScVbaFont > aFont( new ScVbaFont( getParent(), mxContext, mPalette, xProps ) );
                aFont->setSubscript( aValue );
            }
        }
        return;

    }
        xCell.set( xCellRange->getCellByPosition( 0,0 ) );
    uno::Reference< beans::XPropertySet > xProps = lcl_TextProperties( xCell );

    bool bValue = false;
    aValue >>= bValue;
    sal_Int16 nValue = NORMAL;
    sal_Int8 nValue2 = NORMALHEIGHT;

        if( bValue )
    {
        nValue= SUBSCRIPT;
            nValue2 = SUBSCRIPTHEIGHT;
    }

     xProps->setPropertyValue("CharEscapementHeight", uno::Any(nValue2) );
    xProps->setPropertyValue("CharEscapement", uno::Any(nValue) );

}

uno::Any SAL_CALL
ScVbaFont::getSubscript() throw ( uno::RuntimeException, std::exception )
{
        uno::Reference< table::XCell> xCell( mxFont, uno::UNO_QUERY );
        uno::Reference< table::XCellRange > xCellRange( mxFont, uno::UNO_QUERY );
    if ( !xCell.is() )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(xCellRange, uno::UNO_QUERY_THROW );
        sal_Int32 nCols = xColumnRowRange->getColumns()->getCount();
        sal_Int32 nRows = xColumnRowRange->getRows()->getCount();
        uno::Any aRes;
        for ( sal_Int32 col = 0; col < nCols; ++col )
        {
            for ( sal_Int32 row = 0; row < nRows; ++row )
            {
                uno::Reference< beans::XPropertySet > xProps( xCellRange->getCellByPosition( col, row ), uno::UNO_QUERY_THROW );
                uno::Reference< ScVbaFont > aFont( new ScVbaFont( getParent(), mxContext, mPalette, xProps ) );
                if ( !col && !row )
                    aRes = aFont->getSubscript();
                else if ( aRes != aFont->getSubscript() )
                    return aNULL();
            }
        }
        return aRes;

    }
        xCell.set( xCellRange->getCellByPosition( 0,0 ) );
    uno::Reference< beans::XPropertySet > xProps = lcl_TextProperties( xCell );

    short nValue = NORMAL;
    xProps->getPropertyValue("CharEscapement") >>= nValue;
    return uno::makeAny( ( nValue == SUBSCRIPT ) );
}

uno::Any SAL_CALL
ScVbaFont::getSize() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_HEIGHT) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getSize();
}

void SAL_CALL
ScVbaFont::setColorIndex( const uno::Any& _colorindex ) throw( uno::RuntimeException, std::exception )
{
    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;
    // #FIXME  xlColorIndexAutomatic & xlColorIndexNone are not really
    // handled properly here

    if ( !nIndex || ( nIndex == excel::XlColorIndex::xlColorIndexAutomatic ) )
        {
        nIndex = 1;  // check default ( assume black )
                ScVbaFont_BASE::setColorIndex( uno::makeAny( nIndex ) );
        }
        else
            ScVbaFont_BASE::setColorIndex( _colorindex );
}

uno::Any SAL_CALL
ScVbaFont::getColorIndex() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_COLOR) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getColorIndex();
}

void  SAL_CALL
ScVbaFont::setStandardFontSize( const uno::Any& /*aValue*/ ) throw( uno::RuntimeException, std::exception )
{
//XXX #TODO# #FIXME#
    //mxFont->setPropertyValue("CharSize", ( uno::Any )fValue );
    throw uno::RuntimeException(
        "setStandardFontSize not supported" );
}

uno::Any SAL_CALL
ScVbaFont::getStandardFontSize() throw ( uno::RuntimeException, std::exception )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException( "getStandardFontSize not supported" );
    // return uno::Any();
}

void  SAL_CALL
ScVbaFont::setStandardFont( const uno::Any& /*aValue*/ ) throw( uno::RuntimeException, std::exception )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException("setStandardFont not supported" );
}

uno::Any SAL_CALL
ScVbaFont::getStandardFont() throw ( uno::RuntimeException, std::exception )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException("getStandardFont not supported");
    // return uno::Any();
}

void SAL_CALL
ScVbaFont::setFontStyle( const uno::Any& aValue ) throw( uno::RuntimeException, std::exception )
{
    bool bBold = false;
    bool bItalic = false;

    OUString aStyles;
    aValue >>= aStyles;

    std::vector< OUString > aTokens;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = aStyles.getToken( 0, ' ', nIndex );
        aTokens.push_back( aToken );
    }while( nIndex >= 0 );

    std::vector< OUString >::iterator it;
    for( it = aTokens.begin(); it != aTokens.end(); ++it )
    {
        if( (*it).equalsIgnoreAsciiCase("Bold") )
            bBold = true;

        if( (*it).equalsIgnoreAsciiCase("Italic") )
            bItalic = true;
    }

    setBold( uno::makeAny( bBold ) );
    setItalic( uno::makeAny( bItalic ) );
}

uno::Any SAL_CALL
ScVbaFont::getFontStyle() throw ( uno::RuntimeException, std::exception )
{
    OUStringBuffer aStyles;
    bool bValue = false;
    getBold() >>= bValue;
    if( bValue )
        aStyles.append("Bold");

    getItalic() >>= bValue;
    if( bValue )
    {
        if( !aStyles.isEmpty() )
            aStyles.append(" ");
        aStyles.append("Italic");
    }
    return uno::makeAny( aStyles.makeStringAndClear() );
}

uno::Any SAL_CALL
ScVbaFont::getBold() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_WEIGHT) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getBold();
}

void SAL_CALL
ScVbaFont::setUnderline( const uno::Any& aValue ) throw ( uno::RuntimeException, std::exception )
{
    // default
    sal_Int32 nValue = excel::XlUnderlineStyle::xlUnderlineStyleNone;
    aValue >>= nValue;
    switch ( nValue )
    {
// NOTE:: #TODO #FIMXE
// xlUnderlineStyleDoubleAccounting & xlUnderlineStyleSingleAccounting
// don't seem to be supported in Openoffice.
// The import filter converts them to single or double underlines as appropriate
// So, here at the moment we are similarly silently converting
// xlUnderlineStyleSingleAccounting to xlUnderlineStyleSingle.

        case excel::XlUnderlineStyle::xlUnderlineStyleNone:
            nValue = awt::FontUnderline::NONE;
            break;
        case excel::XlUnderlineStyle::xlUnderlineStyleSingle:
        case excel::XlUnderlineStyle::xlUnderlineStyleSingleAccounting:
            nValue = awt::FontUnderline::SINGLE;
            break;
        case excel::XlUnderlineStyle::xlUnderlineStyleDouble:
        case excel::XlUnderlineStyle::xlUnderlineStyleDoubleAccounting:
            nValue = awt::FontUnderline::DOUBLE;
            break;
        default:
            throw uno::RuntimeException("Unknown value for Underline" );
    }

    mxFont->setPropertyValue("CharUnderline", uno::Any(nValue) );

}

uno::Any SAL_CALL
ScVbaFont::getUnderline() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_UNDERLINE) == SfxItemState::DONTCARE )
            return aNULL();

    sal_Int32 nValue = awt::FontUnderline::NONE;
    mxFont->getPropertyValue("CharUnderline") >>= nValue;
    switch ( nValue )
    {
        case  awt::FontUnderline::DOUBLE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleDouble;
            break;
        case  awt::FontUnderline::SINGLE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleSingle;
            break;
        case  awt::FontUnderline::NONE:
            nValue = excel::XlUnderlineStyle::xlUnderlineStyleNone;
            break;
        default:
            throw uno::RuntimeException("Unknown value retrieved for Underline" );

    }
    return uno::makeAny( nValue );
}

uno::Any SAL_CALL
ScVbaFont::getStrikethrough() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_CROSSEDOUT) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getStrikethrough();
}

uno::Any SAL_CALL
ScVbaFont::getShadow() throw (uno::RuntimeException, std::exception)
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_SHADOWED) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getShadow();
}

uno::Any SAL_CALL
ScVbaFont::getItalic() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_POSTURE) == SfxItemState::DONTCARE )
            return aNULL();

    return ScVbaFont_BASE::getItalic();
}

uno::Any SAL_CALL
ScVbaFont::getName() throw ( uno::RuntimeException, std::exception )
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT) == SfxItemState::DONTCARE )
            return aNULL();
    return ScVbaFont_BASE::getName();
}
uno::Any
ScVbaFont::getColor() throw (uno::RuntimeException, std::exception)
{
    // #TODO #FIXME - behave like getXXX above ( wrt. GetDataSet )
    uno::Any aAny;
    aAny = OORGBToXLRGB( mxFont->getPropertyValue("CharColor") );
    return aAny;
}

void  SAL_CALL
ScVbaFont::setOutlineFont( const uno::Any& aValue ) throw ( uno::RuntimeException, std::exception )
{
    mxFont->setPropertyValue("CharContoured", aValue );
}

uno::Any SAL_CALL
ScVbaFont::getOutlineFont() throw (uno::RuntimeException, std::exception)
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_CONTOUR) == SfxItemState::DONTCARE )
            return aNULL();
    return mxFont->getPropertyValue("CharContoured");
}

OUString
ScVbaFont::getServiceImplName()
{
    return OUString("ScVbaFont");
}

uno::Sequence< OUString >
ScVbaFont::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.Font";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
