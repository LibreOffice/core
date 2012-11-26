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


#ifndef _SVX_OPTGRID_HXX
#define _SVX_OPTGRID_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>
#include <svl/eitem.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include "svx/svxdllapi.h"

class SvxGridTabPage;

// class SvxOptionsGrid --------------------------------------------------

class SVX_DLLPUBLIC SvxOptionsGrid
{
protected:
    sal_uInt32          mnFldDrawX;
    sal_uInt32          mnFldDivisionX;
    sal_uInt32          mnFldDrawY;
    sal_uInt32          mnFldDivisionY;
    sal_uInt32          mnFldSnapX;
    sal_uInt32          mnFldSnapY;

    bool                mbUseGridsnap : 1;
    bool                mbSynchronize : 1;
    bool                mbGridVisible : 1;
    bool                mbEqualGrid : 1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void SetFldDrawX(sal_uInt32 nSet) { mnFldDrawX = nSet; }
    void SetFldDivisionX(sal_uInt32 nSet) { mnFldDivisionX = nSet; }
    void SetFldDrawY(sal_uInt32 nSet) { mnFldDrawY = nSet; }
    void SetFldDivisionY(sal_uInt32 nSet) { mnFldDivisionY = nSet; }
    void SetFldSnapX(sal_uInt32 nSet) { mnFldSnapX = nSet; }
    void SetFldSnapY(sal_uInt32 nSet) { mnFldSnapY = nSet; }

    void SetUseGridSnap(bool bSet) { mbUseGridsnap = bSet; }
    void SetSynchronize(bool bSet) { mbSynchronize = bSet; }
    void SetGridVisible(bool bSet) { mbGridVisible = bSet; }
    void SetEqualGrid(bool bSet) { mbEqualGrid = bSet; }

    sal_uInt32 GetFldDrawX() const { return mnFldDrawX; }
    sal_uInt32 GetFldDivisionX() const { return mnFldDivisionX; }
    sal_uInt32 GetFldDrawY() const { return mnFldDrawY; }
    sal_uInt32 GetFldDivisionY() const { return mnFldDivisionY; }
    sal_uInt32 GetFldSnapX() const { return mnFldSnapX; }
    sal_uInt32 GetFldSnapY() const { return mnFldSnapY; }

    bool GetUseGridSnap() const { return mbUseGridsnap; }
    bool GetSynchronize() const { return mbSynchronize; }
    bool GetGridVisible() const { return mbGridVisible; }
    bool GetEqualGrid() const { return mbEqualGrid; }
};

// class SvxGridItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    // #i9076#
    friend class SvxGridTabPage;

public:
    SvxGridItem( sal_uInt16 _nWhich) : SfxPoolItem(_nWhich){};
    SvxGridItem( const SvxGridItem& pTestItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

};

// class SvxGridTabPage --------------------------------------------------

class SVX_DLLPUBLIC SvxGridTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SvxGridTabPage( Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );

private:
    CheckBox            aCbxUseGridsnap;
    CheckBox            aCbxGridVisible;

    FixedLine           aFlResolution;
    FixedText           aFtDrawX;
    MetricField         aMtrFldDrawX;
    FixedText           aFtDrawY;
    MetricField         aMtrFldDrawY;

    FixedLine           aFlDivision;
    FixedText           aFtDivisionX;
    NumericField        aNumFldDivisionX;
    FixedText           aDivisionPointX;

    FixedText           aFtDivisionY;
    NumericField        aNumFldDivisionY;
    FixedText           aDivisionPointY;

    CheckBox            aCbxSynchronize;
    FixedLine           aGrpDrawGrid; // Neu

protected:
    //these controls are used in draw and impress
    FixedLine       aGrpSnap;
    CheckBox        aCbxSnapHelplines;
    CheckBox        aCbxSnapBorder;
    CheckBox        aCbxSnapFrame;
    CheckBox        aCbxSnapPoints;
    FixedText       aFtSnapArea;
    MetricField     aMtrFldSnapArea;

    FixedLine       aSeparatorFL;

    FixedLine       aGrpOrtho;
    CheckBox        aCbxOrtho;
    CheckBox        aCbxBigOrtho;
    CheckBox        aCbxRotate;
    MetricField     aMtrFldAngle;
    FixedText       aFtBezAngle;
    MetricField     aMtrFldBezAngle;

private:
    bool                mbAttrModified;

#ifdef _SVX_OPTGRID_CXX
    DECL_LINK( ClickRotateHdl_Impl, void * );
    DECL_LINK( ChangeDrawHdl_Impl, MetricField * );
    DECL_LINK( ChangeGridsnapHdl_Impl, void * );
    DECL_LINK( ChangeDivisionHdl_Impl, NumericField * );

#endif
};


#endif

