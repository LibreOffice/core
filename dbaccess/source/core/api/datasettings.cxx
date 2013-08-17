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

#include "datasettings.hxx"
#include "apitools.hxx"
#include "dbastrings.hrc"
#include <osl/diagnose.h>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>

#include <com/sun/star/awt/FontWidth.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
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
                    &_pItem->m_sHavingClause, ::getCppuType(&_pItem->m_sHavingClause));

        registerProperty(PROPERTY_GROUP_BY, PROPERTY_ID_GROUP_BY, PropertyAttribute::BOUND,
                    &_pItem->m_sGroupBy, ::getCppuType(&_pItem->m_sGroupBy));
    }

    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &_pItem->m_sFilter, ::getCppuType(&_pItem->m_sFilter));

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &_pItem->m_sOrder, ::getCppuType(&_pItem->m_sOrder));

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &_pItem->m_bApplyFilter, ::getBooleanCppuType());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &_pItem->m_aFont, ::getCppuType(&_pItem->m_aFont));

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &_pItem->m_aTextLineColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
        &_pItem->m_nFontEmphasis, ::getCppuType(&_pItem->m_nFontEmphasis));

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,&_pItem->m_nFontRelief, ::getCppuType(&_pItem->m_nFontRelief));

    registerProperty(PROPERTY_FONTNAME,         PROPERTY_ID_FONTNAME,        PropertyAttribute::BOUND,&_pItem->m_aFont.Name,            ::getCppuType(&_pItem->m_aFont.Name));
    registerProperty(PROPERTY_FONTHEIGHT,       PROPERTY_ID_FONTHEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Height,          ::getCppuType(&_pItem->m_aFont.Height));
    registerProperty(PROPERTY_FONTWIDTH,        PROPERTY_ID_FONTWIDTH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Width,           ::getCppuType(&_pItem->m_aFont.Width));
    registerProperty(PROPERTY_FONTSTYLENAME,    PROPERTY_ID_FONTSTYLENAME,   PropertyAttribute::BOUND,&_pItem->m_aFont.StyleName,       ::getCppuType(&_pItem->m_aFont.StyleName));
    registerProperty(PROPERTY_FONTFAMILY,       PROPERTY_ID_FONTFAMILY,      PropertyAttribute::BOUND,&_pItem->m_aFont.Family,          ::getCppuType(&_pItem->m_aFont.Family));
    registerProperty(PROPERTY_FONTCHARSET,      PROPERTY_ID_FONTCHARSET,     PropertyAttribute::BOUND,&_pItem->m_aFont.CharSet,         ::getCppuType(&_pItem->m_aFont.CharSet));
    registerProperty(PROPERTY_FONTPITCH,        PROPERTY_ID_FONTPITCH,       PropertyAttribute::BOUND,&_pItem->m_aFont.Pitch,           ::getCppuType(&_pItem->m_aFont.Pitch));
    registerProperty(PROPERTY_FONTCHARWIDTH,    PROPERTY_ID_FONTCHARWIDTH,   PropertyAttribute::BOUND,&_pItem->m_aFont.CharacterWidth,  ::getCppuType(&_pItem->m_aFont.CharacterWidth));
    registerProperty(PROPERTY_FONTWEIGHT,       PROPERTY_ID_FONTWEIGHT,      PropertyAttribute::BOUND,&_pItem->m_aFont.Weight,          ::getCppuType(&_pItem->m_aFont.Weight));
    registerProperty(PROPERTY_FONTSLANT,        PROPERTY_ID_FONTSLANT,       PropertyAttribute::BOUND,&_pItem->m_aFont.Slant,           ::getCppuType(&_pItem->m_aFont.Slant));
    registerProperty(PROPERTY_FONTUNDERLINE,    PROPERTY_ID_FONTUNDERLINE,   PropertyAttribute::BOUND,&_pItem->m_aFont.Underline,       ::getCppuType(&_pItem->m_aFont.Underline));
    registerProperty(PROPERTY_FONTSTRIKEOUT,    PROPERTY_ID_FONTSTRIKEOUT,   PropertyAttribute::BOUND,&_pItem->m_aFont.Strikeout,       ::getCppuType(&_pItem->m_aFont.Strikeout));
    registerProperty(PROPERTY_FONTORIENTATION,  PROPERTY_ID_FONTORIENTATION, PropertyAttribute::BOUND,&_pItem->m_aFont.Orientation,     ::getCppuType(&_pItem->m_aFont.Orientation));
    registerProperty(PROPERTY_FONTKERNING,      PROPERTY_ID_FONTKERNING,     PropertyAttribute::BOUND,&_pItem->m_aFont.Kerning,         ::getCppuType(&_pItem->m_aFont.Kerning));
    registerProperty(PROPERTY_FONTWORDLINEMODE, PROPERTY_ID_FONTWORDLINEMODE,PropertyAttribute::BOUND,&_pItem->m_aFont.WordLineMode,    ::getCppuType(&_pItem->m_aFont.WordLineMode));
    registerProperty(PROPERTY_FONTTYPE,         PROPERTY_ID_FONTTYPE,        PropertyAttribute::BOUND,&_pItem->m_aFont.Type,            ::getCppuType(&_pItem->m_aFont.Type));
}

ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper,sal_Bool _bQuery)
    :OPropertyStateContainer(_rBHelper)
    ,ODataSettings_Base()
    ,m_bQuery(_bQuery)
{
}

ODataSettings_Base::ODataSettings_Base()
    :m_bApplyFilter(sal_False)
    ,m_aFont(::comphelper::getDefaultFont())
    ,m_nFontEmphasis(::com::sun::star::awt::FontEmphasisMark::NONE)
    ,m_nFontRelief(::com::sun::star::awt::FontRelief::NONE)
{
}

ODataSettings_Base::ODataSettings_Base(const ODataSettings_Base& _rSource)
    :m_sFilter( _rSource.m_sFilter )
    ,m_sHavingClause( _rSource.m_sHavingClause )
    ,m_sGroupBy( _rSource.m_sGroupBy )
    ,m_sOrder( _rSource.m_sOrder )
    ,m_bApplyFilter( _rSource.m_bApplyFilter )
    ,m_aFont( _rSource.m_aFont )
    ,m_aRowHeight( _rSource.m_aRowHeight )
    ,m_aTextColor( _rSource.m_aTextColor )
    ,m_aTextLineColor( _rSource.m_aTextLineColor )
    ,m_nFontEmphasis( _rSource.m_nFontEmphasis )
    ,m_nFontRelief( _rSource.m_nFontRelief )
{
}

ODataSettings_Base::~ODataSettings_Base()
{
}

void ODataSettings::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _rDefault ) const
{
    static ::com::sun::star::awt::FontDescriptor aFD = ::comphelper::getDefaultFont();
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
            _rDefault <<= sal_False;
            break;
        case PROPERTY_ID_TEXTRELIEF:
            _rDefault <<= ::com::sun::star::awt::FontRelief::NONE;
            break;
        case PROPERTY_ID_TEXTEMPHASIS:
            _rDefault <<= ::com::sun::star::awt::FontEmphasisMark::NONE;
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
