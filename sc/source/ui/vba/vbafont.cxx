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

#include <com/sun/star/awt/FontUnderline.hpp>
#include <ooo/vba/excel/XlColorIndex.hpp>
#include <ooo/vba/excel/XlUnderlineStyle.hpp>
#include <svl/itemset.hxx>
#include <o3tl/string_view.hxx>
#include "excelvbahelper.hxx"
#include "vbafont.hxx"
#include "vbapalette.hxx"
#include <scitems.hxx>
#include <cellsuno.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaFont::ScVbaFont(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const ScVbaPalette& dPalette,
        const uno::Reference< beans::XPropertySet >& xPropertySet,
        ScCellRangeObj* pRangeObj, bool bFormControl ) :
    ScVbaFont_BASE( xParent, xContext, dPalette.getPalette(), xPropertySet, Component::EXCEL, bFormControl ),
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

uno::Any SAL_CALL
ScVbaFont::getSize()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_HEIGHT) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getSize();
}

void SAL_CALL
ScVbaFont::setColorIndex( const uno::Any& _colorindex )
{
    if(mbFormControl)
        return;

    sal_Int32 nIndex = 0;
    _colorindex >>= nIndex;
    // #FIXME  xlColorIndexAutomatic & xlColorIndexNone are not really
    // handled properly here

    if ( !nIndex || ( nIndex == excel::XlColorIndex::xlColorIndexAutomatic ) )
    {
        nIndex = 1;  // check default ( assume black )
        ScVbaFont_BASE::setColorIndex( uno::Any( nIndex ) );
    }
    else
        ScVbaFont_BASE::setColorIndex( _colorindex );
}

uno::Any SAL_CALL
ScVbaFont::getColorIndex()
{
    if(mbFormControl)
        return uno::Any( sal_Int32(0) );
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_COLOR) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getColorIndex();
}

void  SAL_CALL
ScVbaFont::setStandardFontSize( const uno::Any& /*aValue*/ )
{
//XXX #TODO# #FIXME#
    //mxFont->setPropertyValue("CharSize", ( uno::Any )fValue );
    throw uno::RuntimeException(
        u"setStandardFontSize not supported"_ustr );
}

uno::Any SAL_CALL
ScVbaFont::getStandardFontSize()
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException( u"getStandardFontSize not supported"_ustr );
    // return uno::Any();
}

void  SAL_CALL
ScVbaFont::setStandardFont( const uno::Any& /*aValue*/ )
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException(u"setStandardFont not supported"_ustr );
}

uno::Any SAL_CALL
ScVbaFont::getStandardFont()
{
//XXX #TODO# #FIXME#
    throw uno::RuntimeException(u"getStandardFont not supported"_ustr);
    // return uno::Any();
}

void SAL_CALL
ScVbaFont::setFontStyle( const uno::Any& aValue )
{
    bool bBold = false;
    bool bItalic = false;

    OUString aStyles;
    aValue >>= aStyles;

    for (sal_Int32 nIdx{ 0 }; nIdx>=0; )
    {
        const std::u16string_view aToken{ o3tl::getToken(aStyles, 0, ' ', nIdx ) };
        if (o3tl::equalsIgnoreAsciiCase(aToken, u"Bold"))
        {
            bBold = true;
            if (bItalic)
                break;
        }
        else if (o3tl::equalsIgnoreAsciiCase(aToken, u"Italic"))
        {
            bItalic = true;
            if (bBold)
                break;
        }
    }

    setBold( uno::Any( bBold ) );
    setItalic( uno::Any( bItalic ) );
}

uno::Any SAL_CALL
ScVbaFont::getFontStyle()
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
    return uno::Any( aStyles.makeStringAndClear() );
}

uno::Any SAL_CALL
ScVbaFont::getBold()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_WEIGHT) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getBold();
}

void SAL_CALL
ScVbaFont::setUnderline( const uno::Any& aValue )
{
    if(mbFormControl)
        return;

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
            throw uno::RuntimeException(u"Unknown value for Underline"_ustr );
    }

    mxFont->setPropertyValue(u"CharUnderline"_ustr, uno::Any(nValue) );

}

uno::Any SAL_CALL
ScVbaFont::getUnderline()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_UNDERLINE) == SfxItemState::INVALID )
            return aNULL();

    sal_Int32 nValue = awt::FontUnderline::NONE;

    if(mbFormControl)
        return uno::Any( nValue );

    mxFont->getPropertyValue(u"CharUnderline"_ustr) >>= nValue;
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
            throw uno::RuntimeException(u"Unknown value retrieved for Underline"_ustr );

    }
    return uno::Any( nValue );
}

uno::Any SAL_CALL
ScVbaFont::getStrikethrough()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_CROSSEDOUT) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getStrikethrough();
}

uno::Any SAL_CALL
ScVbaFont::getShadow()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_SHADOWED) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getShadow();
}

uno::Any SAL_CALL
ScVbaFont::getItalic()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_POSTURE) == SfxItemState::INVALID )
            return aNULL();

    return ScVbaFont_BASE::getItalic();
}

uno::Any SAL_CALL
ScVbaFont::getName()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT) == SfxItemState::INVALID )
            return aNULL();
    return ScVbaFont_BASE::getName();
}
uno::Any
ScVbaFont::getColor()
{
    // #TODO #FIXME - behave like getXXX above ( wrt. GetDataSet )
    uno::Any aAny = OORGBToXLRGB( mxFont->getPropertyValue(u"CharColor"_ustr) );
    return aAny;
}

void  SAL_CALL
ScVbaFont::setOutlineFont( const uno::Any& aValue )
{
    if(!mbFormControl)
        mxFont->setPropertyValue(u"CharContoured"_ustr, aValue );
}

uno::Any SAL_CALL
ScVbaFont::getOutlineFont()
{
    if ( GetDataSet() )
        if (  GetDataSet()->GetItemState( ATTR_FONT_CONTOUR) == SfxItemState::INVALID )
            return aNULL();
    return mbFormControl ? uno::Any( false ) : mxFont->getPropertyValue(u"CharContoured"_ustr);
}

OUString
ScVbaFont::getServiceImplName()
{
    return u"ScVbaFont"_ustr;
}

uno::Sequence< OUString >
ScVbaFont::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.Font"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
