/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    ::oox::drawingml::Color&            getTextColor(){ return maTextColor; }
    ::boost::optional< sal_Bool >&      getTextBoldStyle(){ return maTextBoldStyle; }
    ::boost::optional< sal_Bool >&      getTextItalicStyle(){ return maTextItalicStyle; }
    ::oox::drawingml::TextFont&         getAsianFont(){ return maAsianFont; }
    ::oox::drawingml::TextFont&         getComplexFont(){ return maComplexFont; }
    ::oox::drawingml::TextFont&         getSymbolFont(){ return maSymbolFont; }
    ::oox::drawingml::TextFont&         getLatinFont(){ return maLatinFont; }

    ::oox::drawingml::FillPropertiesPtr&                            getFillProperties(){ return mpFillProperties; }
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >&    getLineBorders(){ return maLineBorders; }

    ::oox::drawingml::ShapeStyleRefMap& getStyleRefs(){ return maStyleRefs; }

private:

    ::oox::drawingml::Color             maTextColor;
    ::boost::optional< sal_Bool >       maTextBoldStyle;
    ::boost::optional< sal_Bool >       maTextItalicStyle;
    ::oox::drawingml::TextFont          maAsianFont;
    ::oox::drawingml::TextFont          maComplexFont;
    ::oox::drawingml::TextFont          maSymbolFont;
    ::oox::drawingml::TextFont          maLatinFont;

    ::oox::drawingml::FillPropertiesPtr mpFillProperties;
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr > maLineBorders;
    ::oox::drawingml::ShapeStyleRefMap  maStyleRefs;
};

} } }

#endif  //  OOX_DRAWINGML_TABLESTYLEPART_HXX
