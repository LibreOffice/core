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

#include "drawingml/themeelementscontext.hxx"
#include "drawingml/clrschemecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "drawingml/fillpropertiesgroupcontext.hxx"
#include "drawingml/textcharacterproperties.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/helper/attributelist.hxx"
#include "effectproperties.hxx"
#include "effectpropertiescontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {

class FillStyleListContext : public ContextHandler2
{
public:
    FillStyleListContext( ContextHandler2Helper& rParent, FillStyleList& rFillStyleList );
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    FillStyleList& mrFillStyleList;
};

FillStyleListContext::FillStyleListContext( ContextHandler2Helper& rParent, FillStyleList& rFillStyleList ) :
    ContextHandler2( rParent ),
    mrFillStyleList( rFillStyleList )
{
}

ContextHandlerRef FillStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( blipFill ):
        case A_TOKEN( pattFill ):
        case A_TOKEN( grpFill ):
            mrFillStyleList.push_back( FillPropertiesPtr( new FillProperties ) );
            return FillPropertiesContext::createFillContext( *this, nElement, rAttribs, *mrFillStyleList.back() );
    }
    return nullptr;
}

class LineStyleListContext : public ContextHandler2
{
public:
    LineStyleListContext( ContextHandler2Helper& rParent, LineStyleList& rLineStyleList );
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    LineStyleList& mrLineStyleList;
};

LineStyleListContext::LineStyleListContext( ContextHandler2Helper& rParent, LineStyleList& rLineStyleList ) :
    ContextHandler2( rParent ),
    mrLineStyleList( rLineStyleList )
{
}

ContextHandlerRef LineStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( ln ):
            mrLineStyleList.push_back( LinePropertiesPtr( new LineProperties ) );
            return new LinePropertiesContext( *this, rAttribs, *mrLineStyleList.back() );
    }
    return nullptr;
}

class EffectStyleListContext : public ContextHandler2
{
public:
    EffectStyleListContext( ContextHandler2Helper& rParent, EffectStyleList& rEffectStyleList );
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    EffectStyleList& mrEffectStyleList;
};

EffectStyleListContext::EffectStyleListContext( ContextHandler2Helper& rParent, EffectStyleList& rEffectStyleList ) :
    ContextHandler2( rParent ),
    mrEffectStyleList( rEffectStyleList )
{
}

ContextHandlerRef EffectStyleListContext::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch( nElement )
    {
        case A_TOKEN( effectStyle ):
            mrEffectStyleList.push_back( EffectPropertiesPtr( new EffectProperties ) );
            return this;

        case A_TOKEN( effectLst ):  // CT_EffectList
            if( mrEffectStyleList.back() )
                return new EffectPropertiesContext( *this, *mrEffectStyleList.back() );
            break;
    }
    return nullptr;
}

class FontSchemeContext : public ContextHandler2
{
public:
    FontSchemeContext( ContextHandler2Helper& rParent, FontScheme& rFontScheme );
    virtual ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void onEndElement() override;

private:
    FontScheme& mrFontScheme;
    TextCharacterPropertiesPtr mxCharProps;
};

FontSchemeContext::FontSchemeContext( ContextHandler2Helper& rParent, FontScheme& rFontScheme ) :
    ContextHandler2( rParent ),
    mrFontScheme( rFontScheme )
{
}

ContextHandlerRef FontSchemeContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( majorFont ):
            mxCharProps.reset( new TextCharacterProperties );
            mrFontScheme[ XML_major ] = mxCharProps;
            return this;
        case A_TOKEN( minorFont ):
            mxCharProps.reset( new TextCharacterProperties );
            mrFontScheme[ XML_minor ] = mxCharProps;
            return this;

        case A_TOKEN( latin ):
            if( mxCharProps.get() )
                mxCharProps->maLatinFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( ea ):
            if( mxCharProps.get() )
                mxCharProps->maAsianFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( cs ):
            if( mxCharProps.get() )
                mxCharProps->maComplexFont.setAttributes( rAttribs );
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

ThemeElementsContext::ThemeElementsContext( ContextHandler2Helper& rParent, Theme& rTheme ) :
    ContextHandler2( rParent ),
    mrTheme( rTheme )
{
}

ContextHandlerRef ThemeElementsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    // CT_BaseStyles
    switch( nElement )
    {
        case A_TOKEN( clrScheme ):  // CT_ColorScheme
            return new clrSchemeContext( *this, mrTheme.getClrScheme() );
        case A_TOKEN( fontScheme ): // CT_FontScheme
            return new FontSchemeContext( *this, mrTheme.getFontScheme() );

        case A_TOKEN( fmtScheme ):  // CT_StyleMatrix
            mrTheme.setStyleName( rAttribs.getString( XML_name ).get() );
            return this;

        case A_TOKEN( fillStyleLst ):   // CT_FillStyleList
            return new FillStyleListContext( *this, mrTheme.getFillStyleList() );
        case A_TOKEN( lnStyleLst ):    // CT_LineStyleList
            return new LineStyleListContext( *this, mrTheme.getLineStyleList() );
        case A_TOKEN( effectStyleLst ): // CT_EffectStyleList
            return new EffectStyleListContext( *this, mrTheme.getEffectStyleList() );
        case A_TOKEN( bgFillStyleLst ): // CT_BackgroundFillStyleList
            return new FillStyleListContext( *this, mrTheme.getBgFillStyleList() );
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
