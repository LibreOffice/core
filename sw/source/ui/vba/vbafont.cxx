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

#include "vbafont.hxx"
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <ooo/vba/word/WdUnderline.hpp>
#include <sal/macros.h>
#include <unordered_map>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const uno::Any aLongAnyTrue( sal_Int16(-1) );
const uno::Any aLongAnyFalse( sal_Int16( 0 ) );

namespace {

struct MapPair
{
    sal_Int32 nMSOConst;
    sal_Int32 nOOOConst;
};

}

MapPair const UnderLineTable[] = {
        { word::WdUnderline::wdUnderlineNone, css::awt::FontUnderline::NONE },
        { word::WdUnderline::wdUnderlineSingle, css::awt::FontUnderline::SINGLE },
        { word::WdUnderline::wdUnderlineWords, css::awt::FontUnderline::SINGLE },
        { word::WdUnderline::wdUnderlineDouble, css::awt::FontUnderline::DOUBLE },
        { word::WdUnderline::wdUnderlineDotted, css::awt::FontUnderline::DOTTED },
        { word::WdUnderline::wdUnderlineThick, css::awt::FontUnderline::BOLDDASH },
        { word::WdUnderline::wdUnderlineDash, css::awt::FontUnderline::DASH },
        { word::WdUnderline::wdUnderlineDotDash, css::awt::FontUnderline::DASHDOT },
        { word::WdUnderline::wdUnderlineDotDotDash, css::awt::FontUnderline::DASHDOTDOT },
        { word::WdUnderline::wdUnderlineWavy, css::awt::FontUnderline::WAVE },
        { word::WdUnderline::wdUnderlineDottedHeavy, css::awt::FontUnderline::BOLDDOTTED },
        { word::WdUnderline::wdUnderlineDashHeavy, css::awt::FontUnderline::BOLDDASH },
        { word::WdUnderline::wdUnderlineDotDashHeavy, css::awt::FontUnderline::BOLDDASHDOT },
        { word::WdUnderline::wdUnderlineDotDotDashHeavy, css::awt::FontUnderline::BOLDDASHDOTDOT },
        { word::WdUnderline::wdUnderlineWavyHeavy, css::awt::FontUnderline::BOLDWAVE },
        { word::WdUnderline::wdUnderlineDashLong, css::awt::FontUnderline::LONGDASH },
        { word::WdUnderline::wdUnderlineWavyDouble, css::awt::FontUnderline::DOUBLEWAVE },
        { word::WdUnderline::wdUnderlineDashLongHeavy, css::awt::FontUnderline::BOLDLONGDASH },
};

typedef std::unordered_map< sal_Int32, sal_Int32 > ConstToConst;

namespace {

class UnderLineMapper
{
    ConstToConst m_MSO2OOO;
    ConstToConst m_OOO2MSO;
private:
    UnderLineMapper()
    {
        for ( auto const & index: UnderLineTable )
        {
            m_MSO2OOO[ index.nMSOConst ] = index.nOOOConst;
            m_OOO2MSO[ index.nOOOConst ] = index.nMSOConst;
        }
    }
public:
    static OUString propName()
    {
        return u"CharUnderline"_ustr;
    }

    static UnderLineMapper& instance()
    {
        static  UnderLineMapper theMapper;
        return theMapper;
    }

    /// @throws lang::IllegalArgumentException
    sal_Int32 getOOOFromMSO( sal_Int32 nMSOConst )
    {
        ConstToConst::iterator it = m_MSO2OOO.find( nMSOConst );
        if ( it == m_MSO2OOO.end() )
            throw lang::IllegalArgumentException();
        return it->second;
    }
    /// @throws lang::IllegalArgumentException
    sal_Int32 getMSOFromOOO( sal_Int32 nOOOConst )
    {
        ConstToConst::iterator it = m_OOO2MSO.find( nOOOConst );
        if ( it == m_OOO2MSO.end() )
            throw lang::IllegalArgumentException();
        return it->second;
    }
};

}

SwVbaFont::SwVbaFont( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xPalette, uno::Reference< css::beans::XPropertySet > const & xPropertySet )
    : SwVbaFont_BASE( xParent, xContext, xPalette, xPropertySet, Component::WORD )
{
}

uno::Any SAL_CALL
SwVbaFont::getUnderline()
{
    sal_Int32 nOOVal = 0;
    mxFont->getPropertyValue(  UnderLineMapper::propName() ) >>= nOOVal;
    return uno::Any( UnderLineMapper::instance().getMSOFromOOO( nOOVal ) );
}

void SAL_CALL
SwVbaFont::setUnderline( const uno::Any& _underline )
{
    sal_Int32 nMSOVal = 0;

    if ( _underline >>= nMSOVal )
    {
        sal_Int32 nOOVal =  UnderLineMapper::instance().getOOOFromMSO( nMSOVal );
        mxFont->setPropertyValue(  UnderLineMapper::propName(), uno::Any( nOOVal ) );
    }
}

OUString
SwVbaFont::getServiceImplName()
{
    return u"SwVbaFont"_ustr;
}

uno::Any SAL_CALL
SwVbaFont::getColorIndex()
{
    sal_Int32 nColor = 0;

    getColor() >>= nColor;
    sal_Int32 nElems = mxPalette->getCount();
    sal_Int32 nIndex = 0;
    for ( sal_Int32 count=0; count<nElems; ++count )
    {
        sal_Int32 nPaletteColor = 0;
        mxPalette->getByIndex( count ) >>= nPaletteColor;
        if ( nPaletteColor == nColor )
        {
            nIndex = count;
            break;
        }
    }
    return uno::Any( nIndex );
}
uno::Any SAL_CALL
SwVbaFont::getSubscript()
{
    bool bRes = false;
    SwVbaFont_BASE::getSubscript() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getSuperscript()
{
    bool bRes = false;
    SwVbaFont_BASE::getSuperscript() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getBold()
{
    bool bRes = false;
    SwVbaFont_BASE::getBold() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getItalic()
{
    bool bRes = false;
    SwVbaFont_BASE::getItalic() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getStrikethrough()
{
    bool bRes = false;
    SwVbaFont_BASE::getStrikethrough() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getShadow()
{
    bool bRes = false;
    SwVbaFont_BASE::getShadow() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Sequence< OUString >
SwVbaFont::getServiceNames()
{
        static uno::Sequence< OUString > const aServiceNames
        {
            u"ooo.vba.word.Font"_ustr
        };
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
