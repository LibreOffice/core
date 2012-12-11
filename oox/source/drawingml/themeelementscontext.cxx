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

#include "oox/drawingml/themeelementscontext.hxx"
#include "oox/drawingml/clrschemecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/effectproperties.hxx"
#include "oox/drawingml/effectpropertiescontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {

// ============================================================================

class FillStyleListContext : public ContextHandler
{
public:
    FillStyleListContext( ContextHandler& rParent, FillStyleList& rFillStyleList );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);

private:
    FillStyleList& mrFillStyleList;
};

FillStyleListContext::FillStyleListContext( ContextHandler& rParent, FillStyleList& rFillStyleList ) :
    ContextHandler( rParent ),
    mrFillStyleList( rFillStyleList )
{
}

Reference< XFastContextHandler > FillStyleListContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
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
            return FillPropertiesContext::createFillContext( *this, nElement, xAttribs, *mrFillStyleList.back() );
    }
    return 0;
}

// ============================================================================

class LineStyleListContext : public ContextHandler
{
public:
    LineStyleListContext( ContextHandler& rParent, LineStyleList& rLineStyleList );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);

private:
    LineStyleList& mrLineStyleList;
};

LineStyleListContext::LineStyleListContext( ContextHandler& rParent, LineStyleList& rLineStyleList ) :
    ContextHandler( rParent ),
    mrLineStyleList( rLineStyleList )
{
}

Reference< XFastContextHandler > LineStyleListContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttribs )
    throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case A_TOKEN( ln ):
            mrLineStyleList.push_back( LinePropertiesPtr( new LineProperties ) );
            return new LinePropertiesContext( *this, xAttribs, *mrLineStyleList.back() );
    }
    return 0;
}

// ============================================================================

class EffectStyleListContext : public ContextHandler
{
public:
    EffectStyleListContext( ContextHandler& rParent, EffectStyleList& rEffectStyleList );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);

private:
    EffectStyleList& mrEffectStyleList;
};

EffectStyleListContext::EffectStyleListContext( ContextHandler& rParent, EffectStyleList& rEffectStyleList ) :
    ContextHandler( rParent ),
    mrEffectStyleList( rEffectStyleList )
{
}

Reference< XFastContextHandler > EffectStyleListContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& /*xAttribs*/ ) throw (SAXException, RuntimeException)
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
    return 0;
}

// ============================================================================

class FontSchemeContext : public ContextHandler
{
public:
    FontSchemeContext( ContextHandler& rParent, FontScheme& rFontScheme );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) throw (SAXException, RuntimeException);

private:
    FontScheme& mrFontScheme;
    TextCharacterPropertiesPtr mxCharProps;
};

FontSchemeContext::FontSchemeContext( ContextHandler& rParent, FontScheme& rFontScheme ) :
    ContextHandler( rParent ),
    mrFontScheme( rFontScheme )
{
}

Reference< XFastContextHandler > FontSchemeContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs )
    throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rxAttribs );
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
                mxCharProps->maLatinFont.setAttributes( aAttribs );
        break;
        case A_TOKEN( ea ):
            if( mxCharProps.get() )
                mxCharProps->maAsianFont.setAttributes( aAttribs );
        break;
        case A_TOKEN( cs ):
            if( mxCharProps.get() )
                mxCharProps->maComplexFont.setAttributes( aAttribs );
        break;
    }
    return 0;
}

void FontSchemeContext::endFastElement( sal_Int32 nElement ) throw (SAXException, RuntimeException)
{
    switch( nElement )
    {
        case A_TOKEN( majorFont ):
        case A_TOKEN( minorFont ):
            mxCharProps.reset();
        break;
    }
}

// ============================================================================

ThemeElementsContext::ThemeElementsContext( ContextHandler& rParent, Theme& rTheme ) :
    ContextHandler( rParent ),
    mrTheme( rTheme )
{
}

Reference< XFastContextHandler > ThemeElementsContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    // CT_BaseStyles
    Reference< XFastContextHandler > xRet;
    switch( nElement )
    {
        case A_TOKEN( clrScheme ):  // CT_ColorScheme
            return new clrSchemeContext( *this, mrTheme.getClrScheme() );
        case A_TOKEN( fontScheme ): // CT_FontScheme
            return new FontSchemeContext( *this, mrTheme.getFontScheme() );

        case A_TOKEN( fmtScheme ):  // CT_StyleMatrix
            mrTheme.setStyleName( xAttribs->getOptionalValue( XML_name ) );
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
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
