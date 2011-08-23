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

#ifndef OOX_DRAWINGML_TABLESTYLEPART_HXX
#define OOX_DRAWINGML_TABLESTYLEPART_HXX

#include <rtl/ustring.hxx>
#include <boost/optional.hpp>
#include "oox/drawingml/color.hxx"
#include "oox/drawingml/textfont.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/shape.hxx"
#include <map>

namespace oox { namespace drawingml { namespace table {

class TableStylePart
{
public:

    TableStylePart();
    ~TableStylePart();

    ::oox::drawingml::Color&			getTextColor(){ return maTextColor; }
    ::boost::optional< sal_Bool >&		getTextBoldStyle(){ return maTextBoldStyle; }
    ::boost::optional< sal_Bool >&		getTextItalicStyle(){ return maTextItalicStyle; }
    ::oox::drawingml::TextFont&			getAsianFont(){ return maAsianFont; }
    ::oox::drawingml::TextFont&			getComplexFont(){ return maComplexFont; }
    ::oox::drawingml::TextFont&			getSymbolFont(){ return maSymbolFont; }
    ::oox::drawingml::TextFont&			getLatinFont(){ return maLatinFont; }

    ::oox::drawingml::FillPropertiesPtr&                            getFillProperties(){ return mpFillProperties; }
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >&	getLineBorders(){ return maLineBorders; }
    
    ::oox::drawingml::ShapeStyleRefMap& getStyleRefs(){ return maStyleRefs; }

private:

    ::oox::drawingml::Color				maTextColor;
    ::boost::optional< sal_Bool >		maTextBoldStyle;
    ::boost::optional< sal_Bool >		maTextItalicStyle;
    ::oox::drawingml::TextFont			maAsianFont;
    ::oox::drawingml::TextFont			maComplexFont;
    ::oox::drawingml::TextFont			maSymbolFont;
    ::oox::drawingml::TextFont			maLatinFont;

    ::oox::drawingml::FillPropertiesPtr	mpFillProperties;
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr > maLineBorders;
    ::oox::drawingml::ShapeStyleRefMap  maStyleRefs;
};

} } }

#endif  //  OOX_DRAWINGML_TABLESTYLEPART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
