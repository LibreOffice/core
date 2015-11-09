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
#include <ooo/vba/word/WdUnderline.hpp>
#include <sal/macros.h>
#include <ooo/vba/word/WdColorIndex.hpp>
#include <unordered_map>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const uno::Any aLongAnyTrue( sal_Int16(-1) );
const uno::Any aLongAnyFalse( sal_Int16( 0 ) );

struct MapPair
{
    sal_Int32 nMSOConst;
    sal_Int32 nOOOConst;
};

static MapPair UnderLineTable[] = {
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
class UnderLineMapper
{
    ConstToConst MSO2OOO;
    ConstToConst OOO2MSO;
private:
    UnderLineMapper()
    {
        sal_Int32 nLen = SAL_N_ELEMENTS( UnderLineTable );

        for ( sal_Int32 index=0; index<nLen; ++index )
        {
            MSO2OOO[ UnderLineTable[ index ].nMSOConst ] = UnderLineTable[ index ].nOOOConst;
            OOO2MSO[ UnderLineTable[ index ].nOOOConst ] = UnderLineTable[ index ].nMSOConst;
        }
    }
public:
    static OUString propName()
    {
        return OUString("CharUnderline");
    }

    static UnderLineMapper& instance()
    {
        static  UnderLineMapper theMapper;
        return theMapper;
    }

    sal_Int32 getOOOFromMSO( sal_Int32 nMSOConst ) throw( lang::IllegalArgumentException )
    {
        ConstToConst::iterator it = MSO2OOO.find( nMSOConst );
        if ( it == MSO2OOO.end() )
            throw lang::IllegalArgumentException();
        return it->second;
    }
    sal_Int32 getMSOFromOOO( sal_Int32 nOOOConst ) throw( lang::IllegalArgumentException )
    {
        ConstToConst::iterator it = OOO2MSO.find( nOOOConst );
        if ( it == OOO2MSO.end() )
            throw lang::IllegalArgumentException();
        return it->second;
    }
};

SwVbaFont::SwVbaFont( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xPalette, uno::Reference< css::beans::XPropertySet > xPropertySet ) throw ( css::uno::RuntimeException ) : SwVbaFont_BASE( xParent, xContext, xPalette, xPropertySet )
{
}

uno::Any SAL_CALL
SwVbaFont::getUnderline() throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nOOVal = 0;
    mxFont->getPropertyValue(  UnderLineMapper::propName() ) >>= nOOVal;
    return uno::makeAny( UnderLineMapper::instance().getMSOFromOOO( nOOVal ) );
}

void SAL_CALL
SwVbaFont::setUnderline( const uno::Any& _underline ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nMSOVal = 0;

    if ( _underline >>= nMSOVal )
    {
        sal_Int32 nOOVal =  UnderLineMapper::instance().getOOOFromMSO( nMSOVal );
        mxFont->setPropertyValue(  UnderLineMapper::propName(), uno::makeAny( nOOVal ) );
    }
}

OUString
SwVbaFont::getServiceImplName()
{
    return OUString("SwVbaFont");
}

void SAL_CALL
SwVbaFont::setColorIndex( const uno::Any& _colorindex ) throw( uno::RuntimeException, std::exception )
{
        sal_Int32 nIndex = 0;
        _colorindex >>= nIndex;
        return setColor( OORGBToXLRGB(mxPalette->getByIndex( nIndex )) );
}

uno::Any SAL_CALL
SwVbaFont::getColorIndex() throw ( uno::RuntimeException, std::exception )
{
        sal_Int32 nColor = 0;

    XLRGBToOORGB( getColor() ) >>= nColor;
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
    return uno::makeAny( nIndex );
}
uno::Any SAL_CALL
SwVbaFont::getSubscript() throw ( uno::RuntimeException, std::exception )
{
    bool bRes = false;
    SwVbaFont_BASE::getSubscript() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getSuperscript() throw ( uno::RuntimeException, std::exception )
{
    bool bRes = false;
    SwVbaFont_BASE::getSuperscript() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getBold() throw (uno::RuntimeException, std::exception)
{
    bool bRes = false;
    SwVbaFont_BASE::getBold() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getItalic() throw (uno::RuntimeException, std::exception)
{
    bool bRes = false;
    SwVbaFont_BASE::getItalic() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getStrikethrough() throw (css::uno::RuntimeException, std::exception)
{
    bool bRes = false;
    SwVbaFont_BASE::getStrikethrough() >>= bRes;
    if ( bRes )
        return aLongAnyTrue;
    return aLongAnyFalse;
}

uno::Any SAL_CALL
SwVbaFont::getShadow() throw (uno::RuntimeException, std::exception)
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
        static uno::Sequence< OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = "ooo.vba.word.Font";
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
