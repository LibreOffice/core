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

#include <drawingml/themeelementscontext.hxx>
#include <drawingml/clrschemecontext.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/helper/attributelist.hxx>
#include "effectproperties.hxx"
#include <drawingml/effectpropertiescontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;

namespace oox::drawingml {

namespace {

class FillStyleListContext : public ContextHandler2
{
public:
    FillStyleListContext(ContextHandler2Helper const & rParent, FillStyleList& rFillStyleList, model::FormatScheme& rFormatScheme);
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

protected:
    FillStyleList& mrFillStyleList;
    model::FormatScheme& mrFormatScheme;
    virtual model::FillStyle* createAndAddFillStyle()
    {
        return mrFormatScheme.addFillStyle();
    }
};

}

FillStyleListContext::FillStyleListContext(ContextHandler2Helper const & rParent, FillStyleList& rFillStyleList, model::FormatScheme& rFormatScheme)
    : ContextHandler2(rParent)
    , mrFillStyleList(rFillStyleList)
    , mrFormatScheme(rFormatScheme)
{
}

ContextHandlerRef FillStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch (nElement)
    {
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( blipFill ):
        case A_TOKEN( pattFill ):
        case A_TOKEN( grpFill ):
        {
            mrFillStyleList.push_back(std::make_shared<FillProperties>());
            model::FillStyle* pFillStyle = createAndAddFillStyle();
            return FillPropertiesContext::createFillContext(*this, nElement, rAttribs, *mrFillStyleList.back(), pFillStyle);
        }
    }
    return nullptr;
}

namespace
{

class BackgroundFillStyleListContext : public FillStyleListContext
{
public:
    BackgroundFillStyleListContext(ContextHandler2Helper const & rParent, FillStyleList& rFillStyleList, model::FormatScheme& rFormatScheme)
        : FillStyleListContext(rParent, rFillStyleList, rFormatScheme)
    {}

protected:
    model::FillStyle* createAndAddFillStyle() override
    {
        return mrFormatScheme.addBackgroundFillStyle();
    }
};

} // end anonymous ns

namespace {

class LineStyleListContext : public ContextHandler2
{
public:
    LineStyleListContext(ContextHandler2Helper const & rParent, LineStyleList& rLineStyleList, model::FormatScheme& rFormatScheme);
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    model::FormatScheme& mrFormatScheme;
    LineStyleList& mrLineStyleList;
};

}

LineStyleListContext::LineStyleListContext( ContextHandler2Helper const & rParent, LineStyleList& rLineStyleList, model::FormatScheme& rFormatScheme)
    : ContextHandler2(rParent)
    , mrFormatScheme(rFormatScheme)
    , mrLineStyleList(rLineStyleList)
{
}

ContextHandlerRef LineStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch (nElement)
    {
        case A_TOKEN(ln):
        {
            mrLineStyleList.push_back( std::make_shared<LineProperties>( ) );
            model::LineStyle* pLineStyle = mrFormatScheme.addLineStyle();
            return new LinePropertiesContext(*this, rAttribs, *mrLineStyleList.back(), pLineStyle);
        }
    }
    return nullptr;
}

namespace {

class EffectStyleListContext : public ContextHandler2
{
public:
    EffectStyleListContext(ContextHandler2Helper const & rParent, EffectStyleList& rEffectStyleList, model::FormatScheme& rFormatScheme);
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    model::FormatScheme& mrFormatScheme;
    model::EffectStyle* mpEffectStyle;
    EffectStyleList& mrEffectStyleList;
};

}

EffectStyleListContext::EffectStyleListContext( ContextHandler2Helper const & rParent, EffectStyleList& rEffectStyleList, model::FormatScheme& rFormatScheme)
    : ContextHandler2(rParent)
    , mrFormatScheme(rFormatScheme)
    , mpEffectStyle(nullptr)
    , mrEffectStyleList(rEffectStyleList)
{
}

ContextHandlerRef EffectStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch( nElement )
    {
        case A_TOKEN( effectStyle ):
        {
            mpEffectStyle = mrFormatScheme.addEffectStyle();
            mrEffectStyleList.push_back( std::make_shared<EffectProperties>( ) );
            return this;
        }
        case A_TOKEN( effectLst ):  // CT_EffectList
        {
            if( mrEffectStyleList.back() )
                return new EffectPropertiesContext(*this, *mrEffectStyleList.back(), mpEffectStyle);
        }
        break;
    }
    return nullptr;
}

namespace
{

class FontSchemeContext : public ContextHandler2
{
public:
    FontSchemeContext(ContextHandler2Helper const & rParent, const AttributeList& rAttribs, FontScheme& rFontScheme,
                      std::map<sal_Int32, std::vector<std::pair<OUString, OUString>>>& rSupplementalFontMap,
                      model::Theme& rTheme);
    ~FontSchemeContext();
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void onEndElement() override;

private:
    FontScheme& mrFontScheme;
    TextCharacterPropertiesPtr mxCharProps;
    std::map<sal_Int32, std::vector<std::pair<OUString, OUString>>>& mrSupplementalFontMap;
    sal_Int32 maCurrentFont = 0;
    model::Theme& mrTheme;
    model::FontScheme maFontScheme;
};

} // end anonymous ns

FontSchemeContext::FontSchemeContext(ContextHandler2Helper const & rParent, const AttributeList& rAttribs, FontScheme& rFontScheme,
                                     std::map<sal_Int32, std::vector<std::pair<OUString, OUString>>>& rSupplementalFontMap,
                                     model::Theme& rTheme)
    : ContextHandler2(rParent)
    , mrFontScheme(rFontScheme)
    , mrSupplementalFontMap(rSupplementalFontMap)
    , mrTheme(rTheme)
    , maFontScheme(rAttribs.getStringDefaulted(XML_name))
{
}

FontSchemeContext::~FontSchemeContext()
{
    mrTheme.setFontScheme(maFontScheme);
}

namespace
{

void fillThemeFont(model::ThemeFont& rThemeFont, const AttributeList& rAttribs)
{
    rThemeFont.maTypeface = rAttribs.getStringDefaulted(XML_typeface);
    rThemeFont.maPanose = rAttribs.getStringDefaulted(XML_panose);
    rThemeFont.maCharset = rAttribs.getInteger(XML_charset, WINDOWS_CHARSET_DEFAULT);
    sal_Int32 nPitchFamily = rAttribs.getInteger(XML_pitchFamily, 0);
    TextFont::resolvePitch(nPitchFamily, rThemeFont.maPitch, rThemeFont.maFamily);
}

} // end anonymous ns

ContextHandlerRef FontSchemeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( majorFont ):
            mxCharProps = std::make_shared<TextCharacterProperties>();
            mrFontScheme[ XML_major ] = mxCharProps;
            maCurrentFont = XML_major;
            return this;
        case A_TOKEN( minorFont ):
            mxCharProps = std::make_shared<TextCharacterProperties>();
            mrFontScheme[ XML_minor ] = mxCharProps;
            maCurrentFont = XML_minor;
            return this;
        case A_TOKEN( latin ):
        {
            if (mxCharProps)
                mxCharProps->maLatinFont.setAttributes(rAttribs);

            model::ThemeFont aThemeFont;
            fillThemeFont(aThemeFont, rAttribs);
            if (maCurrentFont == XML_major)
                maFontScheme.setMajorLatin(aThemeFont);
            else if (maCurrentFont == XML_minor)
                maFontScheme.setMinorLatin(aThemeFont);
        }
        break;
        case A_TOKEN( ea ):
        {
            if( mxCharProps )
                mxCharProps->maAsianFont.setAttributes( rAttribs );
            model::ThemeFont aThemeFont;
            fillThemeFont(aThemeFont, rAttribs);
            if (maCurrentFont == XML_major)
                maFontScheme.setMajorAsian(aThemeFont);
            else if (maCurrentFont == XML_minor)
                maFontScheme.setMinorAsian(aThemeFont);
        }
        break;
        case A_TOKEN( cs ):
        {
            if( mxCharProps )
                mxCharProps->maComplexFont.setAttributes( rAttribs );
            model::ThemeFont aThemeFont;
            fillThemeFont(aThemeFont, rAttribs);
            if (maCurrentFont == XML_major)
                maFontScheme.setMajorComplex(aThemeFont);
            else if (maCurrentFont == XML_minor)
                maFontScheme.setMinorComplex(aThemeFont);
        }
        break;
        case A_TOKEN(font):
        {
            OUString aScript = rAttribs.getStringDefaulted(XML_script);
            OUString aTypeface = rAttribs.getStringDefaulted(XML_typeface);
            mrSupplementalFontMap[maCurrentFont].emplace_back(std::pair<OUString, OUString>{aScript, aTypeface});
            if (maCurrentFont == XML_major)
                maFontScheme.addMajorSupplementalFont({aScript, aTypeface});
            else if (maCurrentFont == XML_minor)
                maFontScheme.addMinorSupplementalFont({aScript, aTypeface});
        }
        break;
    }
    return nullptr;
}

void FontSchemeContext::onEndElement()
{
    switch( getCurrentElement() )
    {
        case A_TOKEN( majorFont ):
        case A_TOKEN( minorFont ):
            mxCharProps.reset();
        break;
    }
}

ThemeElementsContext::ThemeElementsContext(ContextHandler2Helper const & rParent, Theme& rOoxTheme, model::Theme& rTheme)
    : ContextHandler2(rParent)
    , mrOoxTheme(rOoxTheme)
    , mrTheme(rTheme)
{
}

ContextHandlerRef ThemeElementsContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    // CT_BaseStyles
    switch (nElement)
    {
        case A_TOKEN( clrScheme ):  // CT_ColorScheme
        {
            OUString aColorSchemeName = rAttribs.getStringDefaulted(XML_name);
            mrTheme.setColorSet(std::make_shared<model::ColorSet>(aColorSchemeName));
            if (rAttribs.hasAttribute(XML_name))
                mrOoxTheme.getClrScheme().SetName(rAttribs.getStringDefaulted(XML_name));
            return new clrSchemeContext(*this, mrOoxTheme.getClrScheme(), *mrTheme.getColorSet());
        }
        case A_TOKEN( fontScheme ): // CT_FontScheme
        {
            if (rAttribs.hasAttribute(XML_name))
                mrOoxTheme.setFontSchemeName(rAttribs.getStringDefaulted(XML_name));

            return new FontSchemeContext(*this, rAttribs, mrOoxTheme.getFontScheme(), mrOoxTheme.getSupplementalFontMap(), mrTheme);
        }

        case A_TOKEN( fmtScheme ):  // CT_StyleMatrix
        {
            if (rAttribs.hasAttribute(XML_name))
                mrOoxTheme.setFormatSchemeName(rAttribs.getStringDefaulted(XML_name));
            return this;
        }

        case A_TOKEN( fillStyleLst ):   // CT_FillStyleList
            return new FillStyleListContext(*this, mrOoxTheme.getFillStyleList(), mrTheme.getFormatScheme());
        case A_TOKEN( lnStyleLst ):    // CT_LineStyleList
            return new LineStyleListContext(*this, mrOoxTheme.getLineStyleList(), mrTheme.getFormatScheme());
        case A_TOKEN( effectStyleLst ): // CT_EffectStyleList
            return new EffectStyleListContext(*this, mrOoxTheme.getEffectStyleList(), mrTheme.getFormatScheme());
        case A_TOKEN( bgFillStyleLst ): // CT_BackgroundFillStyleList
            return new BackgroundFillStyleListContext( *this, mrOoxTheme.getBgFillStyleList(), mrTheme.getFormatScheme());
    }
    return nullptr;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
