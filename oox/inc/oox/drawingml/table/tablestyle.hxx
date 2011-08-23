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

#ifndef OOX_DRAWINGML_TABLESTYLE_HXX
#define OOX_DRAWINGML_TABLESTYLE_HXX

#include "oox/drawingml/table/tablestylepart.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/shape.hxx"

namespace oox { namespace drawingml { namespace table {

class TableStyle
{
public:

    TableStyle();
    ~TableStyle();

    rtl::OUString&	getStyleId(){ return maStyleId; }
    rtl::OUString&	getStyleName() { return maStyleName; }

    ::oox::drawingml::ShapeStyleRef&		getBackgroundFillStyleRef(){ return maFillStyleRef; }

    ::oox::drawingml::FillPropertiesPtr&	getBackgroundFillProperties(){ return mpFillProperties; }

    TableStylePart&	getWholeTbl()	{ return maWholeTbl; }
    TableStylePart&	getBand1H()		{ return maBand1H; }
    TableStylePart&	getBand2H()		{ return maBand2H; }
    TableStylePart&	getBand1V()		{ return maBand1V; }
    TableStylePart&	getBand2V()		{ return maBand2V; }
    TableStylePart&	getLastCol()	{ return maLastCol; }
    TableStylePart&	getFirstCol()	{ return maFirstCol; }
    TableStylePart&	getLastRow()	{ return maLastRow; }
    TableStylePart&	getSeCell()		{ return maSeCell; }
    TableStylePart&	getSwCell()		{ return maSwCell; }
    TableStylePart&	getFirstRow()	{ return maFirstRow; }
    TableStylePart&	getNeCell()		{ return maNeCell; }
    TableStylePart&	getNwCell()		{ return maNwCell; }

private:

    rtl::OUString			maStyleId;
    rtl::OUString			maStyleName;

    ::oox::drawingml::ShapeStyleRef maFillStyleRef;

    ::oox::drawingml::FillPropertiesPtr	mpFillProperties;

    TableStylePart	maWholeTbl;
    TableStylePart	maBand1H;
    TableStylePart	maBand2H;
    TableStylePart	maBand1V;
    TableStylePart	maBand2V;
    TableStylePart	maLastCol;
    TableStylePart	maFirstCol;
    TableStylePart	maLastRow;
    TableStylePart	maSeCell;
    TableStylePart	maSwCell;
    TableStylePart	maFirstRow;
    TableStylePart	maNeCell;
    TableStylePart	maNwCell;
};

} } }

#endif  //  OOX_DRAWINGML_TABLESTYLE_HXX
