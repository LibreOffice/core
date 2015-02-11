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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRMPAGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRMPAGE_HXX

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

// frame dialog
class SwFrmPage: public SfxTabPage
{
    // size
    FixedText*       m_pWidthFT;
    FixedText*       m_pWidthAutoFT;
    PercentField m_aWidthED;
    CheckBox*        m_pRelWidthCB;
    ListBox*         m_pRelWidthRelationLB;
    CheckBox*        m_pAutoWidthCB;

    FixedText*       m_pHeightFT;
    FixedText*       m_pHeightAutoFT;
    PercentField m_aHeightED;
    CheckBox*        m_pRelHeightCB;
    ListBox*         m_pRelHeightRelationLB;
    CheckBox*        m_pAutoHeightCB;

    CheckBox*        m_pFixedRatioCB;
    PushButton*      m_pRealSizeBT;

    // anchor
    VclContainer*    m_pAnchorFrame;
    RadioButton*     m_pAnchorAtPageRB;
    RadioButton*     m_pAnchorAtParaRB;
    RadioButton*     m_pAnchorAtCharRB;
    RadioButton*     m_pAnchorAsCharRB;
    RadioButton*     m_pAnchorAtFrameRB;

    // position
    FixedText*       m_pHorizontalFT;
    ListBox*         m_pHorizontalDLB;
    FixedText*       m_pAtHorzPosFT;
    MetricField*     m_pAtHorzPosED;
    FixedText*       m_pHoriRelationFT;
    ListBox*         m_pHoriRelationLB;

    CheckBox*        m_pMirrorPagesCB;

    FixedText*       m_pVerticalFT;
    ListBox*         m_pVerticalDLB;
    FixedText*       m_pAtVertPosFT;
    MetricField*     m_pAtVertPosED;
    FixedText*       m_pVertRelationFT;
    ListBox*         m_pVertRelationLB;
    // #i18732# - check box for new option 'FollowTextFlow'
    CheckBox*        m_pFollowTextFlowCB;

    // example
    SvxSwFrameExample*  m_pExampleWN;

    //'string provider'
    SvxSwFramePosString aFramePosString;

    bool            bAtHorzPosModified;
    bool            bAtVertPosModified;

    bool            bFormat;
    bool            bNew;
    bool            bNoModifyHdl;
    bool            bIsVerticalFrame;  //current frame is in vertical environment - strings are exchanged
    // --> OD 2009-08-31 #mongolianlayou#
    bool            bIsVerticalL2R;
    bool            bIsInRightToLeft; // current frame is in right-to-left environment - strings are exchanged
    bool            bHtmlMode;
    sal_uInt16          nHtmlMode;
    OUString        sDlgType;
    Size            aGrfSize;
    SwTwips         nUpperBorder;
    SwTwips         nLowerBorder;
    double          fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button

    // OD 12.11.2003 #i22341# - keep content position of character for
    // to character anchored objects.
    const SwPosition* mpToCharCntntPos;

    // old alignment
    sal_Int16 nOldH;
    sal_Int16 nOldHRel;
    sal_Int16 nOldV;
    sal_Int16 nOldVRel;

    FrmMap* pVMap;
    FrmMap* pHMap;

    bool    m_bAllowVertPositioning;
    bool    m_bIsMathOLE;
    bool    m_bIsMathBaselineAlignment;

    virtual void    ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int     DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;

    DECL_LINK(RangeModifyHdl, void *);
    DECL_LINK(AnchorTypeHdl, void *);
    DECL_LINK( PosHdl, ListBox * );
    DECL_LINK( RelHdl, ListBox * );
    void            InitPos(RndStdIds eId, sal_Int16 nH, sal_Int16 nHRel,
                            sal_Int16 nV, sal_Int16 nVRel,
                            long   nX,  long   nY);

    DECL_LINK(RealSizeHdl, void *);
    DECL_LINK( RelSizeClickHdl, CheckBox * );
    DECL_LINK(MirrorHdl, void *);

    DECL_LINK( AutoWidthClickHdl, void* );
    DECL_LINK( AutoHeightClickHdl, void* );

    // update example
    void            UpdateExample();
    DECL_LINK( ModifyHdl, Edit * );

    void            Init(const SfxItemSet& rSet, bool bReset = false);
    // OD 12.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in the alignment.
    sal_Int32       FillPosLB( const FrmMap* _pMap,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB );
    // OD 14.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in their string entries.
    sal_uLong       FillRelLB( const FrmMap* _pMap,
                               const sal_uInt16 _nLBSelPos,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB,
                               FixedText& _rFT );
    sal_Int32       GetMapPos( const FrmMap *pMap, ListBox &rAlignLB );
    sal_Int16       GetAlignment(FrmMap *pMap, sal_Int32 nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    sal_Int16       GetRelation(FrmMap *pMap, ListBox &rRelationLB);
    RndStdIds       GetAnchor();

    void setOptimalFrmWidth();
    void setOptimalRelWidth();

    void            EnableGraficMode( void );   // hides auto check boxes and re-org controls for "Real Size" button

    SwFrmPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFrmPage();

    SwWrtShell *getFrmDlgParentShell();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    static const sal_uInt16 aPageRg[];

public:

    static SfxTabPage *Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet *rSet) SAL_OVERRIDE;

    void            SetNewFrame(bool bNewFrame) { bNew      = bNewFrame; }
    void            SetFormatUsed(bool bFmt);
    void            SetFrmType(const OUString &rType) { sDlgType  = rType; }
    inline bool     IsInGraficMode( void ) { return sDlgType == "PictureDialog" || sDlgType == "ObjectDialog"; }
    void            EnableVerticalPositioning( bool bEnable );
};

class SwGrfExtPage: public SfxTabPage
{
    // mirror
    VclContainer*   m_pMirror;
    CheckBox*       m_pMirrorVertBox;
    CheckBox*       m_pMirrorHorzBox;
    RadioButton*    m_pAllPagesRB;
    RadioButton*    m_pLeftPagesRB;
    RadioButton*    m_pRightPagesRB;
    BmpWindow*      m_pBmpWin;

    Edit*           m_pConnectED;
    PushButton*     m_pBrowseBT;

    OUString        aFilterName;
    OUString        aGrfName, aNewGrfName;

    ::sfx2::FileDialogHelper*     pGrfDlg;

    bool            bHtmlMode;

    // handler for mirroring
    DECL_LINK(MirrorHdl, void *);
    DECL_LINK(BrowseHdl, void *);

    virtual void    ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    SwGrfExtPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwGrfExtPage();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(vcl::Window *pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet *rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
};

class SwFrmURLPage : public SfxTabPage
{
    // hyperlink
    Edit*            pURLED;
    PushButton*      pSearchPB;
    Edit*            pNameED;
    ComboBox*        pFrameCB;

    // image map
    CheckBox*        pServerCB;
    CheckBox*        pClientCB;

    DECL_LINK(InsertFileHdl, void *);

    SwFrmURLPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFrmURLPage();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(vcl::Window *pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet *rSet) SAL_OVERRIDE;
};

class SwFrmAddPage : public SfxTabPage
{
    VclContainer* pNameFrame;
    FixedText*    pNameFT;
    Edit*         pNameED;
    FixedText*    pAltNameFT;
    Edit*         pAltNameED;
    FixedText*    pPrevFT;
    ListBox*      pPrevLB;
    FixedText*    pNextFT;
    ListBox*      pNextLB;

    VclContainer* pProtectFrame;
    CheckBox*     pProtectContentCB;
    CheckBox*     pProtectFrameCB;
    CheckBox*     pProtectSizeCB;

    VclContainer* m_pContentAlignFrame;
    ListBox*      m_pVertAlignLB;

    VclContainer* pPropertiesFrame;
    CheckBox*     pEditInReadonlyCB;
    CheckBox*     pPrintFrameCB;
    FixedText*    pTextFlowFT;
    ListBox*      pTextFlowLB;

    SwWrtShell*   pWrtSh;

    OUString      sDlgType;
    bool      bHtmlMode;
    bool      bFormat;
    bool      bNew;

    DECL_LINK(EditModifyHdl, void *);
    DECL_LINK(ChainModifyHdl, ListBox*);

    SwFrmAddPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFrmAddPage();

    static const sal_uInt16 aAddPgRg[];

public:

    static SfxTabPage*  Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16*  GetRanges() { return aAddPgRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet *rSet) SAL_OVERRIDE;

    void            SetFormatUsed(bool bFmt);
    void            SetFrmType(const OUString &rType) { sDlgType = rType; }
    void            SetNewFrame(bool bNewFrame) { bNew  = bNewFrame; }
    void            SetShell(SwWrtShell* pSh) { pWrtSh  = pSh; }

};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_FRMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
