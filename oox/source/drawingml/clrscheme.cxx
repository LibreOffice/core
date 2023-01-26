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

#include <algorithm>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/Color.hpp>

#include <osl/diagnose.h>
#include <oox/drawingml/clrscheme.hxx>
#include <oox/token/tokens.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

namespace oox::drawingml {

bool ClrMap::getColorMap( sal_Int32& nClrToken )
{
    sal_Int32 nMapped = 0;
    std::map < sal_Int32, sal_Int32 >::const_iterator aIter( maClrMap.find( nClrToken ) );
    if ( aIter != maClrMap.end() )
        nMapped = (*aIter).second;
    if ( nMapped )
    {
        nClrToken = nMapped;
        return true;
    }
    else
        return false;
}

void ClrMap::setColorMap( sal_Int32 nClrToken, sal_Int32 nMappedClrToken )
{
    maClrMap[ nClrToken ] = nMappedClrToken;
}

namespace {

struct find_by_token
{
    explicit find_by_token(sal_Int32 token):
        m_token(token)
    {
    }

    bool operator()(const std::pair<sal_Int32, ::Color>& r)
    {
        return r.first == m_token;
    }

private:
    sal_Int32 m_token;
};

}

bool ClrScheme::getColor( sal_Int32 nSchemeClrToken, ::Color& rColor ) const
{
    OSL_ASSERT((nSchemeClrToken & sal_Int32(0xFFFF0000))==0);
    switch( nSchemeClrToken )
    {
        case XML_bg1 : nSchemeClrToken = XML_lt1; break;
        case XML_bg2 : nSchemeClrToken = XML_lt2; break;
        case XML_tx1 : nSchemeClrToken = XML_dk1; break;
        case XML_tx2 : nSchemeClrToken = XML_dk2; break;
    }

    auto aIter = std::find_if(maClrScheme.begin(), maClrScheme.end(), find_by_token(nSchemeClrToken) );

    if ( aIter != maClrScheme.end() )
        rColor = aIter->second;

    return aIter != maClrScheme.end();
}

void ClrScheme::setColor( sal_Int32 nSchemeClrToken, ::Color nColor )
{
    const auto aIter = std::find_if(maClrScheme.begin(), maClrScheme.end(), find_by_token(nSchemeClrToken) );
    if ( aIter != maClrScheme.end() )
        aIter->second = nColor;
    else
        maClrScheme.emplace_back(nSchemeClrToken, nColor);
}

bool ClrScheme::getColorByIndex(size_t nIndex, ::Color& rColor) const
{
    if (nIndex >= maClrScheme.size())
        return false;

    rColor = maClrScheme[nIndex].second;
    return true;
}

void ClrScheme::ToAny(css::uno::Any& rVal) const
{
    std::vector<util::Color> aRet;

    for (const auto& rIndexAndColor : maClrScheme)
    {
        aRet.push_back(static_cast<sal_Int32>(rIndexAndColor.second));
    }

    rVal <<= comphelper::containerToSequence(aRet);
}

void ClrScheme::fill(model::ColorSet& rColorSet) const
{
    for (const auto& [nToken, rColor] : maClrScheme)
    {
        switch (nToken)
        {
            case XML_tx1:
            case XML_dk1: rColorSet.add(model::ThemeColorType::Dark1, rColor); break;
            case XML_bg1:
            case XML_lt1: rColorSet.add(model::ThemeColorType::Light1, rColor); break;
            case XML_tx2:
            case XML_dk2: rColorSet.add(model::ThemeColorType::Dark2, rColor); break;
            case XML_bg2:
            case XML_lt2: rColorSet.add(model::ThemeColorType::Light2, rColor); break;
            case XML_accent1: rColorSet.add(model::ThemeColorType::Accent1, rColor); break;
            case XML_accent2: rColorSet.add(model::ThemeColorType::Accent2, rColor); break;
            case XML_accent3: rColorSet.add(model::ThemeColorType::Accent3, rColor); break;
            case XML_accent4: rColorSet.add(model::ThemeColorType::Accent4, rColor); break;
            case XML_accent5: rColorSet.add(model::ThemeColorType::Accent5, rColor); break;
            case XML_accent6: rColorSet.add(model::ThemeColorType::Accent6, rColor); break;
            case XML_hlink: rColorSet.add(model::ThemeColorType::Hyperlink, rColor); break;
            case XML_folHlink: rColorSet.add(model::ThemeColorType::FollowedHyperlink, rColor); break;
            default: break;
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
