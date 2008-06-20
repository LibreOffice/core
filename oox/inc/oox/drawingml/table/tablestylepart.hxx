/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablestylepart.hxx,v $
 *
 * $Revision: 1.2 $
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
#include <map>

namespace oox { namespace drawingml { namespace table {

class TableStylePart
{
public:

    enum LineType
    {
        NONE,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        INSIDEH,
        INSIDEV,
        TL2BR,
        TR2BL
    };

    TableStylePart();
    ~TableStylePart();

    ::oox::drawingml::Color&            getTextColor(){ return maTextColor; };
    rtl::OUString&                      getThemeableFontRef(){ return maThemeableFontRefIndex; };
    ::oox::drawingml::Color&            getThemeableFontColor(){ return maThemeableFontColor; };
    ::boost::optional< sal_Bool >&      getTextBoldStyle(){ return maTextBoldStyle; };
    ::boost::optional< sal_Bool >&      getTextItalicStyle(){ return maTextItalicStyle; };
    ::oox::drawingml::TextFont&         getAsianFont(){ return maAsianFont; };
    ::oox::drawingml::TextFont&         getComplexFont(){ return maComplexFont; };
    ::oox::drawingml::TextFont&         getSymbolFont(){ return maSymbolFont; };
    ::oox::drawingml::TextFont&         getLatinFont(){ return maLatinFont; };

    rtl::OUString&                      getFillStyleRef(){ return maFillStyleRef; };
    ::oox::drawingml::Color&            getFillStyleColor(){ return maFillStyleColor; };

    boost::shared_ptr< ::oox::drawingml::FillProperties >&                          getFillProperties(){ return mpFillProperties; };
    std::map < LineType, boost::shared_ptr< ::oox::drawingml::LineProperties > >&   getLineBorders(){ return maLineBorders; };
    std::map < LineType, rtl::OUString >&                                           getLineBordersStyleRef(){ return maLineBordersStyleRef; };
    std::map < LineType, oox::drawingml::Color >&                                   getLineBordersStyleColor(){ return maLineBordersStyleColor; };

private:

    ::oox::drawingml::Color             maTextColor;
    rtl::OUString                       maThemeableFontRefIndex;
    ::oox::drawingml::Color             maThemeableFontColor;
    ::boost::optional< sal_Bool >       maTextBoldStyle;
    ::boost::optional< sal_Bool >       maTextItalicStyle;
    ::oox::drawingml::TextFont          maAsianFont;
    ::oox::drawingml::TextFont          maComplexFont;
    ::oox::drawingml::TextFont          maSymbolFont;
    ::oox::drawingml::TextFont          maLatinFont;
    rtl::OUString                       maFillStyleRef;
    ::oox::drawingml::Color             maFillStyleColor;

    boost::shared_ptr< ::oox::drawingml::FillProperties >                           mpFillProperties;
    std::map < LineType, boost::shared_ptr< ::oox::drawingml::LineProperties > >    maLineBorders;
    std::map < LineType, rtl::OUString >                                            maLineBordersStyleRef;
    std::map < LineType, oox::drawingml::Color >                                    maLineBordersStyleColor;
};

} } }

#endif  //  OOX_DRAWINGML_TABLESTYLEPART_HXX
