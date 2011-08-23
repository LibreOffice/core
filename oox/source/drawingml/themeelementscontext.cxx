/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/themeelementscontext.hxx"
#include "oox/drawingml/clrschemecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"

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
        case NMSP_DRAWINGML|XML_noFill:
        case NMSP_DRAWINGML|XML_solidFill:
        case NMSP_DRAWINGML|XML_gradFill:
        case NMSP_DRAWINGML|XML_blipFill:
        case NMSP_DRAWINGML|XML_pattFill:
        case NMSP_DRAWINGML|XML_grpFill:
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
        case NMSP_DRAWINGML|XML_ln:
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
        case NMSP_DRAWINGML|XML_effectStyle:
            mrEffectStyleList.push_back( EffectStyleList::value_type( new PropertyMap ) );
            // TODO: import effect styles
            return 0;
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
        case NMSP_DRAWINGML|XML_majorFont:
            mxCharProps.reset( new TextCharacterProperties );
            mrFontScheme[ XML_major ] = mxCharProps;
            return this;
        case NMSP_DRAWINGML|XML_minorFont:
            mxCharProps.reset( new TextCharacterProperties );
            mrFontScheme[ XML_minor ] = mxCharProps;
            return this;

        case NMSP_DRAWINGML|XML_latin:
            if( mxCharProps.get() )
                mxCharProps->maLatinFont.setAttributes( aAttribs );
        break;
        case NMSP_DRAWINGML|XML_ea:
            if( mxCharProps.get() )
                mxCharProps->maAsianFont.setAttributes( aAttribs );
        break;
        case NMSP_DRAWINGML|XML_cs:
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
        case NMSP_DRAWINGML|XML_majorFont:
        case NMSP_DRAWINGML|XML_minorFont:
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
        case NMSP_DRAWINGML|XML_clrScheme:	// CT_ColorScheme
            return new clrSchemeContext( *this, mrTheme.getClrScheme() );
        case NMSP_DRAWINGML|XML_fontScheme:	// CT_FontScheme
            return new FontSchemeContext( *this, mrTheme.getFontScheme() );

        case NMSP_DRAWINGML|XML_fmtScheme:  // CT_StyleMatrix
            mrTheme.setStyleName( xAttribs->getOptionalValue( XML_name ) );
            return this;

        case NMSP_DRAWINGML|XML_fillStyleLst:   // CT_FillStyleList
            return new FillStyleListContext( *this, mrTheme.getFillStyleList() );
        case NMSP_DRAWINGML|XML_lnStyleLst:    // CT_LineStyleList
            return new LineStyleListContext( *this, mrTheme.getLineStyleList() );
        case NMSP_DRAWINGML|XML_effectStyleLst: // CT_EffectStyleList
            return new EffectStyleListContext( *this, mrTheme.getEffectStyleList() );
        case NMSP_DRAWINGML|XML_bgFillStyleLst: // CT_BackgroundFillStyleList
            return new FillStyleListContext( *this, mrTheme.getBgFillStyleList() );
    }
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
