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

#ifndef _FRMPAGE_HXX
#define _FRMPAGE_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/swframeposstrings.hxx>
#include <swtypes.hxx>
#include <bmpwin.hxx>
#include <svx/swframeexample.hxx>
#include <prcntfld.hxx>
#include <globals.hrc>

namespace sfx2{class FileDialogHelper;}
class SwWrtShell;
struct FrmMap;
// OD 12.11.2003 #i22341#
struct SwPosition;

/*--------------------------------------------------------------------
     Description:   frame dialog
 --------------------------------------------------------------------*/
class SwFrmPage: public SfxTabPage
{
    // size
    FixedLine       aSizeFL;
    FixedText       aWidthFT;
    FixedText       aWidthAutoFT;
    PercentField    aWidthED;
    CheckBox        aRelWidthCB;
    CheckBox        aAutoWidthCB;
    FixedText       aHeightFT;
    FixedText       aHeightAutoFT;
    PercentField    aHeightED;
    CheckBox        aRelHeightCB;
    CheckBox        aAutoHeightCB;
    CheckBox        aFixedRatioCB;
    PushButton      aRealSizeBT;

    // anchor
    FixedLine       aTypeSepFL;
    FixedLine       aTypeFL;
    RadioButton     aAnchorAtPageRB;
    RadioButton     aAnchorAtParaRB;
    RadioButton     aAnchorAtCharRB;
    RadioButton     aAnchorAsCharRB;
    RadioButton     aAnchorAtFrameRB;

    // position
    FixedLine       aPositionFL;
    FixedText       aHorizontalFT;
    ListBox         aHorizontalDLB;
    FixedText       aAtHorzPosFT;
    MetricField     aAtHorzPosED;
    FixedText       aHoriRelationFT;
    ListBox         aHoriRelationLB;
    CheckBox        aMirrorPagesCB;
    FixedText       aVerticalFT;
    ListBox         aVerticalDLB;
    FixedText       aAtVertPosFT;
    MetricField     aAtVertPosED;
    FixedText       aVertRelationFT;
    ListBox         aVertRelationLB;
    // OD 02.10.2003 #i18732# - check box for new option 'FollowTextFlow'
    CheckBox        aFollowTextFlowCB;

    // example
    SvxSwFrameExample   aExampleWN;

    //'string provider'
    SvxSwFramePosString aFramePosString;

    sal_Bool            bAtHorzPosModified;
    sal_Bool            bAtVertPosModified;

    sal_Bool            bFormat;
    sal_Bool            bNew;
    sal_Bool            bNoModifyHdl;
    sal_Bool            bIsVerticalFrame;  //current frame is in vertical environment - strings are exchanged
    // --> OD 2009-08-31 #mongolianlayou#
    sal_Bool            bIsVerticalL2R;
    sal_Bool            bIsInRightToLeft; // current frame is in right-to-left environment - strings are exchanged
    sal_Bool            bHtmlMode;
    sal_uInt16          nHtmlMode;
    sal_uInt16          nDlgType;
    Size            aGrfSize;
    Size            aWrap;
    SwTwips         nUpperBorder;
    SwTwips         nLowerBorder;
    double          fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button

    // OD 12.11.2003 #i22341# - keep content position of character for
    // to character anchored objects.
    const SwPosition* mpToCharCntntPos;

    // old alignment
    short        nOldH;
    short    nOldHRel;
    short        nOldV;
    short    nOldVRel;

    FrmMap* pVMap;
    FrmMap* pHMap;

    bool    m_bAllowVertPositioning;
    bool    m_bIsMathOLE;
    bool    m_bIsMathBaselineAlignment;

    virtual void    ActivatePage(const SfxItemSet& rSet);
    virtual int     DeactivatePage(SfxItemSet *pSet);


    DECL_LINK(RangeModifyHdl, void *);
    DECL_LINK(AnchorTypeHdl, void *);
    DECL_LINK( PosHdl, ListBox * );
    DECL_LINK( RelHdl, ListBox * );
    void            InitPos(RndStdIds eId, sal_uInt16 nH, sal_uInt16 nHRel,
                            sal_uInt16 nV,  sal_uInt16 nVRel,
                            long   nX,  long   nY);

    DECL_LINK(RealSizeHdl, void *);
    DECL_LINK( RelSizeClickHdl, CheckBox * );
    DECL_LINK(MirrorHdl, void *);

    DECL_LINK( AutoWidthClickHdl, void* );
    DECL_LINK( AutoHeightClickHdl, void* );

    // update example
    void            UpdateExample();
    DECL_LINK( ModifyHdl, Edit * );

    void            Init(const SfxItemSet& rSet, sal_Bool bReset = sal_False);
    // OD 12.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in the alignment.
    sal_uInt16          FillPosLB( const FrmMap* _pMap,
                               const sal_uInt16 _nAlign,
                               const sal_uInt16 _nRel,
                               ListBox& _rLB );
    // OD 14.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in their string entries.
    sal_uLong           FillRelLB( const FrmMap* _pMap,
                               const sal_uInt16 _nLBSelPos,
                               const sal_uInt16 _nAlign,
                               sal_uInt16 _nRel,
                               ListBox& _rLB,
                               FixedText& _rFT );
    sal_uInt16          GetMapPos( const FrmMap *pMap, ListBox &rAlignLB );
    short           GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    short           GetRelation(FrmMap *pMap, ListBox &rRelationLB);
    RndStdIds       GetAnchor();

    void            EnableGraficMode( void );   // hides auto check boxes and re-org controls for "Real Size" button

    SwFrmPage(Window *pParent, const SfxItemSet &rSet);
    ~SwFrmPage();

    SwWrtShell *getFrmDlgParentShell();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);

    void            SetNewFrame(sal_Bool bNewFrame) { bNew      = bNewFrame; }
    void            SetFormatUsed(sal_Bool bFmt);
    void            SetFrmType(sal_uInt16 nType)    { nDlgType  = nType;     }
    inline sal_Bool     IsInGraficMode( void )      { return nDlgType == DLG_FRM_GRF || nDlgType == DLG_FRM_OLE; }
    void            EnableVerticalPositioning( bool bEnable );
};

class SwGrfExtPage: public SfxTabPage
{
    // mirror
    FixedLine       aMirrorFL;
    CheckBox        aMirrorVertBox;
    CheckBox        aMirrorHorzBox;
    RadioButton     aAllPagesRB;
    RadioButton     aLeftPagesRB;
    RadioButton     aRightPagesRB;
    BmpWindow       aBmpWin;

    FixedLine       aConnectFL;
    FixedText       aConnectFT;
    Edit            aConnectED;
    PushButton      aBrowseBT;

    String          aFilterName;
    String          aGrfName, aNewGrfName;

    ::sfx2::FileDialogHelper*     pGrfDlg;

    sal_Bool            bHtmlMode;

    // handler for mirroring
    DECL_LINK(MirrorHdl, void *);
    DECL_LINK(BrowseHdl, void *);

    virtual void    ActivatePage(const SfxItemSet& rSet);
    SwGrfExtPage(Window *pParent, const SfxItemSet &rSet);
    ~SwGrfExtPage();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);
    virtual int  DeactivatePage(SfxItemSet *pSet);
};

class SwFrmURLPage : public SfxTabPage
{
    // hyperlink
    FixedLine       aHyperLinkFL;
    FixedText       aURLFT;
    Edit            aURLED;
    PushButton      aSearchPB;
    FixedText       aNameFT;
    Edit            aNameED;
    FixedText       aFrameFT;
    ComboBox        aFrameCB;

    // image map
    FixedLine       aImageFL;
    CheckBox        aServerCB;
    CheckBox        aClientCB;

    DECL_LINK(InsertFileHdl, void *);


    SwFrmURLPage(Window *pParent, const SfxItemSet &rSet);
    ~SwFrmURLPage();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);
};

class SwFrmAddPage : public SfxTabPage
{
      FixedLine   aNamesFL;
    FixedText   aNameFT;
    Edit        aNameED;
    FixedText   aAltNameFT;
    Edit        aAltNameED;
    FixedText   aPrevFT;
    ListBox     aPrevLB;
    FixedText   aNextFT;
    ListBox     aNextLB;
        FixedLine   aProtectFL;

    CheckBox    aProtectContentCB;
    CheckBox    aProtectFrameCB;
    CheckBox    aProtectSizeCB;
    FixedLine   aExtFL;

    CheckBox    aEditInReadonlyCB;
    CheckBox    aPrintFrameCB;
    FixedText   aTextFlowFT;
    ListBox     aTextFlowLB;


    SwWrtShell* pWrtSh;

    sal_uInt16      nDlgType;
    sal_Bool        bHtmlMode;
    sal_Bool        bFormat;
    sal_Bool        bNew;

    DECL_LINK(EditModifyHdl, void *);
    DECL_LINK(ChainModifyHdl, ListBox*);

    SwFrmAddPage(Window *pParent, const SfxItemSet &rSet);
    ~SwFrmAddPage();

public:

    static SfxTabPage*  Create(Window *pParent, const SfxItemSet &rSet);
    static sal_uInt16*      GetRanges();

    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset(const SfxItemSet &rSet);

    void            SetFormatUsed(sal_Bool bFmt);
    void            SetFrmType(sal_uInt16 nType) { nDlgType = nType; }
    void            SetNewFrame(sal_Bool bNewFrame) { bNew  = bNewFrame; }
    void            SetShell(SwWrtShell* pSh) { pWrtSh  = pSh; }

};

#endif // _FRMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
