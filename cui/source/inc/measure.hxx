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


#ifndef _SVX_MEASURE_HXX
#define _SVX_MEASURE_HXX

// include ---------------------------------------------------------------

#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/measctrl.hxx>

class SdrView;

/*************************************************************************
|*
|* Dialog zum "Andern von TextAttributen
|*
\************************************************************************/

class SvxMeasurePage : public SvxTabPage
{
private:

    FixedLine           aFlLine;
    FixedText           aFtLineDist;
    MetricField         aMtrFldLineDist;
    FixedText           aFtHelplineOverhang;
    MetricField         aMtrFldHelplineOverhang;
    FixedText           aFtHelplineDist;
    MetricField         aMtrFldHelplineDist;
    FixedText           aFtHelpline1Len;
    MetricField         aMtrFldHelpline1Len;
    FixedText           aFtHelpline2Len;
    MetricField         aMtrFldHelpline2Len;
    TriStateBox         aTsbBelowRefEdge;
    FixedText           aFtDecimalPlaces;
    MetricField         aMtrFldDecimalPlaces;

    FixedLine           aFlLabel;
    FixedText           aFtPosition;
    SvxRectCtl          aCtlPosition;
    TriStateBox         aTsbAutoPosV;
    TriStateBox         aTsbAutoPosH;
    TriStateBox         aTsbShowUnit;
    ListBox             aLbUnit;
    TriStateBox         aTsbParallel;

    SvxXMeasurePreview  aCtlPreview;

    FixedLine           aFlVert;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

    sal_Bool                bPositionModified;

#ifdef _SVX_MEASURE_CXX
    void                FillUnitLB();

    DECL_LINK( ClickAutoPosHdl_Impl, void * );
    DECL_LINK( ChangeAttrHdl_Impl, void * );
#endif

public:

    SvxMeasurePage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxMeasurePage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001
};

/*************************************************************************
|*
|* Von SfxSingleTabDialog abgeleitet, um vom Control "uber virtuelle Methode
|* benachrichtigt werden zu k"onnen.
|*
\************************************************************************/

class SvxMeasureDialog : public SfxSingleTabDialog
{
public:
    SvxMeasureDialog( Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    ~SvxMeasureDialog();
};


#endif // _SVX_MEASURE_HXX

