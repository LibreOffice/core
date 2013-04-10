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



#ifndef _SVX_OPTCHART_HXX
#define _SVX_OPTCHART_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
// header for ValueSet
#include <svtools/valueset.hxx>
// header for ColorLB
#include <svx/dlgctrl.hxx>
// header for PushButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for XColorList
#include <svx/xtable.hxx>

#include "cfgchart.hxx"

class ChartColorLB : public ColorLB
{
public:
    ChartColorLB( Window* pParent, ResId Id ) : ColorLB( pParent, Id ) {}
    ChartColorLB( Window* pParent, WinBits aWB ) : ColorLB( pParent, aWB ) {}

    void FillBox( const SvxChartColorTable & rTab );
};


class SvxDefaultColorOptPage : public SfxTabPage
{
private:
    FixedLine               aGbChartColors;
    ChartColorLB            aLbChartColors;
    FixedLine               aGbColorBox;
    ValueSet                aValSetColorBox;
    PushButton              aPBDefault;

    SvxChartOptions*        pChartOptions;
    SvxChartColorTableItem* pColorConfig;
    XColorList*         pColorTab;

    DECL_LINK( ResetToDefaults, void * );
    DECL_LINK( ListClickedHdl, ChartColorLB * );
    DECL_LINK( BoxClickedHdl, ValueSet * );

    void FillColorBox();
    long GetColorIndex( const Color& rCol );

public:
    SvxDefaultColorOptPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxDefaultColorOptPage();

    void    Construct();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool        FillItemSet( SfxItemSet& rOutAttrs );
    virtual void        Reset( const SfxItemSet& rInAttrs );
};

#endif  // _SVX_OPTCHART_HXX

