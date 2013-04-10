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


#ifndef _CHART2_VIEWELEMENTLISTPROVIDER_HXX
#define _CHART2_VIEWELEMENTLISTPROVIDER_HXX

#include <svx/xtable.hxx>
#include <svx/svdpage.hxx>

class FontList;

//.............................................................................
namespace chart
{
//.............................................................................

class DrawModelWrapper;

class ViewElementListProvider
{
public:
    ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper );
    virtual ~ViewElementListProvider();

    XColorList*    GetColorTable() const;
    XDashList*      GetDashList() const;
    XLineEndList*   GetLineEndList() const;
    XGradientList*  GetGradientList() const;
    XHatchList*     GetHatchList() const;
    XBitmapList*    GetBitmapList() const;

    //create chartspecific symbols for linecharts
    SdrObjList*     GetSymbolList() const;
    Graphic         GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const;

    FontList*       getFontList() const;
    //SfxPrinter*   getPrinter();

private:
    DrawModelWrapper*   m_pDrawModelWrapper;
    mutable FontList*   m_pFontList;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
