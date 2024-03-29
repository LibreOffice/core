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

#include <datasettings.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{
// ODataSettings
void ODataSettings::registerPropertiesFor(ODataSettings_Base* _pItem)
{
    if ( m_bQuery )
    {
        registerProperty(PROPERTY_HAVING_CLAUSE, PROPERTY_ID_HAVING_CLAUSE, PropertyAttribute::BOUND,
                    &_pItem->m_sHavingClause, cppu::UnoType<decltype(_pItem->m_sHavingClause)>::get());

        registerProperty(PROPERTY_GROUP_BY, PROPERTY_ID_GROUP_BY, PropertyAttribute::BOUND,
                    &_pItem->m_sGroupBy, cppu::UnoType<decltype(_pItem->m_sGroupBy)>::get());
    }

    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &_pItem->m_sFilter, cppu::UnoType<decltype(_pItem->m_sFilter)>::get());

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &_pItem->m_sOrder, cppu::UnoType<decltype(_pItem->m_sOrder)>::get());

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &_pItem->m_bApplyFilter, cppu::UnoType<bool>::get());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &_pItem->m_aFont, cppu::UnoType<decltype(_pItem->m_aFont)>::get());

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aRowHeight, ::cppu::UnoType<sal_Int32>::get());

    registerProperty(PROPERTY_AUTOGROW, PROPERTY_ID_AUTOGROW, PropertyAttribute::BOUND,
                    &_pItem->m_bAutoGrow, cppu::UnoType<bool>::get());

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextColor, ::cppu::UnoType<sal_Int32>::get());

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextLineColor, ::cppu::UnoType<sal_Int32>::get());

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
        &_pItem->m_nFontEmphasis, cppu::UnoType<decltype(_pItem->m_nFontEmphasis)>::get());

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,&_pItem->m_nFontRelief, cppu::UnoType<decltype(_pItem->m_nFontRelief)>::get());

    registerProperty(PROPERTY_FONTNAME,         PROPERTY_ID_FONTNAME,        PropertyAttribute::BOUND,&_pItem->m_aFont.Name,            cppu::UnoType<decltype(_pItem->m_aFont.Name)>::get());
    registerProperty(PROPERTY_FONTHEIGHT,       PROPERTY_ID_FONTHEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Height,          cppu::UnoType<decltype(_pItem->m_aFont.Height)>::get());
    registerProperty(PROPERTY_FONTWIDTH,        PROPERTY_ID_FONTWIDTH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Width,           cppu::UnoType<decltype(_pItem->m_aFont.Width)>::get());
    registerProperty(PROPERTY_FONTSTYLENAME,    PROPERTY_ID_FONTSTYLENAME,   PropertyAttribute::BOUND,&_pItem->m_aFont.StyleName,       cppu::UnoType<decltype(_pItem->m_aFont.StyleName)>::get());
    registerProperty(PROPERTY_FONTFAMILY,       PROPERTY_ID_FONTFAMILY,      PropertyAttribute::BOUND,&_pItem->m_aFont.Family,          cppu::UnoType<decltype(_pItem->m_aFont.Family)>::get());
    registerProperty(PROPERTY_FONTCHARSET,      PROPERTY_ID_FONTCHARSET,     PropertyAttribute::BOUND,&_pItem->m_aFont.CharSet,         cppu::UnoType<decltype(_pItem->m_aFont.CharSet)>::get());
    registerProperty(PROPERTY_FONTPITCH,        PROPERTY_ID_FONTPITCH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Pitch,           cppu::UnoType<decltype(_pItem->m_aFont.Pitch)>::get());
    registerProperty(PROPERTY_FONTCHARWIDTH,    PROPERTY_ID_FONTCHARWIDTH,   PropertyAttribute::BOUND,&_pItem->m_aFont.CharacterWidth,  cppu::UnoType<decltype(_pItem->m_aFont.CharacterWidth)>::get());
    registerProperty(PROPERTY_FONTWEIGHT,       PROPERTY_ID_FONTWEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Weight,          cppu::UnoType<decltype(_pItem->m_aFont.Weight)>::get());
    registerProperty(PROPERTY_FONTSLANT,        PROPERTY_ID_FONTSLANT,       PropertyAttribute::BOUND,&_pItem->m_aFont.Slant,           cppu::UnoType<decltype(_pItem->m_aFont.Slant)>::get());
    registerProperty(PROPERTY_FONTUNDERLINE,    PROPERTY_ID_FONTUNDERLINE,   PropertyAttribute::BOUND,&_pItem->m_aFont.Underline,       cppu::UnoType<decltype(_pItem->m_aFont.Underline)>::get());
    registerProperty(PROPERTY_FONTSTRIKEOUT,    PROPERTY_ID_FONTSTRIKEOUT,   PropertyAttribute::BOUND,&_pItem->m_aFont.Strikeout,       cppu::UnoType<decltype(_pItem->m_aFont.Strikeout)>::get());
    registerProperty(PROPERTY_FONTORIENTATION,  PROPERTY_ID_FONTORIENTATION, PropertyAttribute::BOUND,&_pItem->m_aFont.Orientation,     cppu::UnoType<decltype(_pItem->m_aFont.Orientation)>::get());
    registerProperty(PROPERTY_FONTKERNING,      PROPERTY_ID_FONTKERNING,     PropertyAttribute::BOUND,&_pItem->m_aFont.Kerning,         cppu::UnoType<decltype(_pItem->m_aFont.Kerning)>::get());
    registerProperty(PROPERTY_FONTWORDLINEMODE, PROPERTY_ID_FONTWORDLINEMODE,PropertyAttribute::BOUND,&_pItem->m_aFont.WordLineMode,    cppu::UnoType<decltype(_pItem->m_aFont.WordLineMode)>::get());
    registerProperty(PROPERTY_FONTTYPE,         PROPERTY_ID_FONTTYPE,        PropertyAttribute::BOUND,&_pItem->m_aFont.Type,            cppu::UnoType<decltype(_pItem->m_aFont.Type)>::get());
}

ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper,bool _bQuery)
    :OPropertyStateContainer(_rBHelper)
    ,m_bQuery(_bQuery)
{
}

ODataSettings_Base::ODataSettings_Base()
    :m_bApplyFilter(false)
    ,m_bAutoGrow(false)
    ,m_aFont(::comphelper::getDefaultFont())
    ,m_nFontEmphasis(css::awt::FontEmphasisMark::NONE)
    ,m_nFontRelief(css::awt::FontRelief::NONE)
{
}

ODataSettings_Base::~ODataSettings_Base()
{
}

void ODataSettings::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _rDefault ) const
{
    static css::awt::FontDescriptor aFD = ::comphelper::getDefaultFont();
    switch( _nHandle )
    {
        case PROPERTY_ID_HAVING_CLAUSE:
        case PROPERTY_ID_GROUP_BY:
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
            _rDefault <<= OUString();
            break;
        case PROPERTY_ID_FONT:
            _rDefault <<= ::comphelper::getDefaultFont();
            break;
        case PROPERTY_ID_APPLYFILTER:
            _rDefault <<= false;
            break;
        case PROPERTY_ID_TEXTRELIEF:
            _rDefault <<= css::awt::FontRelief::NONE;
            break;
        case PROPERTY_ID_TEXTEMPHASIS:
            _rDefault <<= css::awt::FontEmphasisMark::NONE;
            break;
        case PROPERTY_ID_FONTNAME:
            _rDefault <<= aFD.Name;
            break;
        case PROPERTY_ID_FONTHEIGHT:
            _rDefault <<= aFD.Height;
            break;
        case PROPERTY_ID_FONTWIDTH:
            _rDefault <<= aFD.Width;
            break;
        case PROPERTY_ID_FONTSTYLENAME:
            _rDefault <<= aFD.StyleName;
            break;
        case PROPERTY_ID_FONTFAMILY:
            _rDefault <<= aFD.Family;
            break;
        case PROPERTY_ID_FONTCHARSET:
            _rDefault <<= aFD.CharSet;
            break;
        case PROPERTY_ID_FONTPITCH:
            _rDefault <<= aFD.Pitch;
            break;
        case PROPERTY_ID_FONTCHARWIDTH:
            _rDefault <<= aFD.CharacterWidth;
            break;
        case PROPERTY_ID_FONTWEIGHT:
            _rDefault <<= aFD.Weight;
            break;
        case PROPERTY_ID_FONTSLANT:
            _rDefault <<= aFD.Slant;
            break;
        case PROPERTY_ID_FONTUNDERLINE:
            _rDefault <<= aFD.Underline;
            break;
        case PROPERTY_ID_FONTSTRIKEOUT:
            _rDefault <<= aFD.Strikeout;
            break;
        case PROPERTY_ID_FONTORIENTATION:
            _rDefault <<= aFD.Orientation;
            break;
        case PROPERTY_ID_FONTKERNING:
            _rDefault <<= aFD.Kerning;
            break;
        case PROPERTY_ID_FONTWORDLINEMODE:
            _rDefault <<= aFD.WordLineMode;
            break;
        case PROPERTY_ID_FONTTYPE:
            _rDefault <<= aFD.Type;
            break;
    }
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
