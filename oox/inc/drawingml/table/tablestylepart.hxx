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

#ifndef INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLEPART_HXX
#define INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLEPART_HXX

#include <rtl/ustring.hxx>
#include <boost/optional.hpp>
#include <oox/drawingml/color.hxx>
#include <drawingml/textfont.hxx>
#include <oox/drawingml/shape.hxx>

#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>

#include <map>

namespace oox { namespace drawingml { namespace table {

class TableStylePart
{
public:

    TableStylePart();

    ::oox::drawingml::Color&            getTextColor(){ return maTextColor; }
    ::boost::optional< bool >&          getTextBoldStyle(){ return maTextBoldStyle; }
    ::boost::optional< bool >&          getTextItalicStyle(){ return maTextItalicStyle; }
    ::oox::drawingml::TextFont&         getAsianFont(){ return maAsianFont; }
    ::oox::drawingml::TextFont&         getComplexFont(){ return maComplexFont; }
    ::oox::drawingml::TextFont&         getSymbolFont(){ return maSymbolFont; }
    ::oox::drawingml::TextFont&         getLatinFont(){ return maLatinFont; }

    ::oox::drawingml::FillPropertiesPtr&                            getFillProperties(){ return mpFillProperties; }
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >&    getLineBorders(){ return maLineBorders; }

    ::oox::drawingml::ShapeStyleRefMap& getStyleRefs(){ return maStyleRefs; }

private:

    ::oox::drawingml::Color             maTextColor;
    ::boost::optional< bool >           maTextBoldStyle;
    ::boost::optional< bool >           maTextItalicStyle;
    ::oox::drawingml::TextFont          maAsianFont;
    ::oox::drawingml::TextFont          maComplexFont;
    ::oox::drawingml::TextFont          maSymbolFont;
    ::oox::drawingml::TextFont          maLatinFont;

    ::oox::drawingml::FillPropertiesPtr mpFillProperties;
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr > maLineBorders;
    ::oox::drawingml::ShapeStyleRefMap  maStyleRefs;
};

} } }

#endif // INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLEPART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
