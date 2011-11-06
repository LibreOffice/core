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

    rtl::OUString&  getStyleId(){ return maStyleId; }
    rtl::OUString&  getStyleName() { return maStyleName; }

    ::oox::drawingml::ShapeStyleRef&        getBackgroundFillStyleRef(){ return maFillStyleRef; }

    ::oox::drawingml::FillPropertiesPtr&    getBackgroundFillProperties(){ return mpFillProperties; }

    TableStylePart& getWholeTbl()   { return maWholeTbl; }
    TableStylePart& getBand1H()     { return maBand1H; }
    TableStylePart& getBand2H()     { return maBand2H; }
    TableStylePart& getBand1V()     { return maBand1V; }
    TableStylePart& getBand2V()     { return maBand2V; }
    TableStylePart& getLastCol()    { return maLastCol; }
    TableStylePart& getFirstCol()   { return maFirstCol; }
    TableStylePart& getLastRow()    { return maLastRow; }
    TableStylePart& getSeCell()     { return maSeCell; }
    TableStylePart& getSwCell()     { return maSwCell; }
    TableStylePart& getFirstRow()   { return maFirstRow; }
    TableStylePart& getNeCell()     { return maNeCell; }
    TableStylePart& getNwCell()     { return maNwCell; }

private:

    rtl::OUString           maStyleId;
    rtl::OUString           maStyleName;

    ::oox::drawingml::ShapeStyleRef maFillStyleRef;

    ::oox::drawingml::FillPropertiesPtr mpFillProperties;

    TableStylePart  maWholeTbl;
    TableStylePart  maBand1H;
    TableStylePart  maBand2H;
    TableStylePart  maBand1V;
    TableStylePart  maBand2V;
    TableStylePart  maLastCol;
    TableStylePart  maFirstCol;
    TableStylePart  maLastRow;
    TableStylePart  maSeCell;
    TableStylePart  maSwCell;
    TableStylePart  maFirstRow;
    TableStylePart  maNeCell;
    TableStylePart  maNwCell;
};

} } }

#endif  //  OOX_DRAWINGML_TABLESTYLE_HXX
