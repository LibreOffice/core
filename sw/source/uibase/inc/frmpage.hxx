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
struct FrameMap;
// OD 12.11.2003 #i22341#
struct SwPosition;

// frame dialog
class SwFramePage: public SfxTabPage
{
    // size
    VclPtr<FixedText>       m_pWidthFT;
    VclPtr<FixedText>       m_pWidthAutoFT;
    PercentField            m_aWidthED;
    VclPtr<CheckBox>        m_pRelWidthCB;
    VclPtr<ListBox>         m_pRelWidthRelationLB;
    VclPtr<CheckBox>        m_pAutoWidthCB;

    VclPtr<FixedText>       m_pHeightFT;
    VclPtr<FixedText>       m_pHeightAutoFT;
    PercentField            m_aHeightED;
    VclPtr<CheckBox>        m_pRelHeightCB;
    VclPtr<ListBox>         m_pRelHeightRelationLB;
    VclPtr<CheckBox>        m_pAutoHeightCB;

    VclPtr<CheckBox>        m_pFixedRatioCB;
    VclPtr<PushButton>      m_pRealSizeBT;

    // anchor
    VclPtr<VclContainer>    m_pAnchorFrame;
    VclPtr<RadioButton>     m_pAnchorAtPageRB;
    VclPtr<RadioButton>     m_pAnchorAtParaRB;
    VclPtr<RadioButton>     m_pAnchorAtCharRB;
    VclPtr<RadioButton>     m_pAnchorAsCharRB;
    VclPtr<RadioButton>     m_pAnchorAtFrameRB;

    // position
    VclPtr<FixedText>       m_pHorizontalFT;
    VclPtr<ListBox>         m_pHorizontalDLB;
    VclPtr<FixedText>       m_pAtHorzPosFT;
    VclPtr<MetricField>     m_pAtHorzPosED;
    VclPtr<FixedText>       m_pHoriRelationFT;
    VclPtr<ListBox>         m_pHoriRelationLB;

    VclPtr<CheckBox>        m_pMirrorPagesCB;

    VclPtr<FixedText>       m_pVerticalFT;
    VclPtr<ListBox>         m_pVerticalDLB;
    VclPtr<FixedText>       m_pAtVertPosFT;
    VclPtr<MetricField>     m_pAtVertPosED;
    VclPtr<FixedText>       m_pVertRelationFT;
    VclPtr<ListBox>         m_pVertRelationLB;
    // #i18732# - check box for new option 'FollowTextFlow'
    VclPtr<CheckBox>        m_pFollowTextFlowCB;

    // example
    VclPtr<SvxSwFrameExample>  m_pExampleWN;

    //'string provider'
    SvxSwFramePosString m_aFramePosString;

    bool            m_bAtHorzPosModified;
    bool            m_bAtVertPosModified;

    bool            m_bFormat;
    bool            m_bNew;
    bool            m_bNoModifyHdl;
    bool            m_bIsVerticalFrame;  //current frame is in vertical environment - strings are exchanged
    // --> OD 2009-08-31 #mongolianlayou#
    bool            m_bIsVerticalL2R;
    bool            m_bIsInRightToLeft; // current frame is in right-to-left environment - strings are exchanged
    bool            m_bHtmlMode;
    sal_uInt16          m_nHtmlMode;
    OUString        m_sDlgType;
    Size            m_aGrfSize;
    SwTwips         m_nUpperBorder;
    SwTwips         m_nLowerBorder;
    double          m_fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button

    // OD 12.11.2003 #i22341# - keep content position of character for
    // to character anchored objects.
    const SwPosition* mpToCharContentPos;

    // old alignment
    sal_Int16 m_nOldH;
    sal_Int16 m_nOldHRel;
    sal_Int16 m_nOldV;
    sal_Int16 m_nOldVRel;

    FrameMap* m_pVMap;
    FrameMap* m_pHMap;

    bool    m_bAllowVertPositioning;
    bool    m_bIsMathOLE;
    bool    m_bIsMathBaselineAlignment;

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual sfxpg   DeactivatePage(SfxItemSet *pSet) override;

    DECL_LINK_TYPED(RangeModifyLoseFocusHdl, Control&, void);
    DECL_LINK_TYPED(RangeModifyClickHdl, Button*, void);
    void RangeModifyHdl();
    DECL_LINK_TYPED(AnchorTypeHdl, Button*, void);
    DECL_LINK_TYPED( PosHdl, ListBox&, void );
    DECL_LINK_TYPED( RelHdl, ListBox&, void );
    void            InitPos(RndStdIds eId, sal_Int16 nH, sal_Int16 nHRel,
                            sal_Int16 nV, sal_Int16 nVRel,
                            long   nX,  long   nY);

    DECL_LINK_TYPED(RealSizeHdl, Button *, void);
    DECL_LINK_TYPED( RelSizeClickHdl, Button *, void );
    DECL_LINK_TYPED(MirrorHdl, Button *, void);

    DECL_LINK_TYPED( AutoWidthClickHdl, Button *, void);
    DECL_LINK_TYPED( AutoHeightClickHdl, Button *, void);

    // update example
    void            UpdateExample();
    DECL_LINK_TYPED( ModifyHdl, Edit&, void );

    void            Init(const SfxItemSet& rSet, bool bReset = false);
    // OD 12.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in the alignment.
    sal_Int32       FillPosLB( const FrameMap* _pMap,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB );
    // OD 14.11.2003 #i22341# - adjustment to handle maps, that are ambigous
    //                          in their string entries.
    sal_uLong       FillRelLB( const FrameMap* _pMap,
                               const sal_uInt16 _nLBSelPos,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB,
                               FixedText& _rFT );
    sal_Int32       GetMapPos( const FrameMap *pMap, ListBox &rAlignLB );
    static sal_Int16 GetAlignment(FrameMap *pMap, sal_Int32 nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    static sal_Int16 GetRelation(FrameMap *pMap, ListBox &rRelationLB);
    RndStdIds       GetAnchor();

    void setOptimalFrameWidth();
    void setOptimalRelWidth();

    void            EnableGraficMode();   // hides auto check boxes and re-org controls for "Real Size" button

    SwWrtShell *getFrameDlgParentShell();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    static const sal_uInt16 aPageRg[];

public:
    SwFramePage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFramePage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

    void            SetNewFrame(bool bNewFrame) { m_bNew      = bNewFrame; }
    void            SetFormatUsed(bool bFormat);
    void            SetFrameType(const OUString &rType) { m_sDlgType  = rType; }
    inline bool     IsInGraficMode() { return m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"; }
    void            EnableVerticalPositioning( bool bEnable );
};

class SwGrfExtPage: public SfxTabPage
{
    // mirror
    VclPtr<VclContainer>   m_pMirror;
    VclPtr<CheckBox>       m_pMirrorVertBox;
    VclPtr<CheckBox>       m_pMirrorHorzBox;
    VclPtr<RadioButton>    m_pAllPagesRB;
    VclPtr<RadioButton>    m_pLeftPagesRB;
    VclPtr<RadioButton>    m_pRightPagesRB;
    VclPtr<BmpWindow>      m_pBmpWin;

    VclPtr<Edit>           m_pConnectED;
    VclPtr<PushButton>     m_pBrowseBT;

    OUString        aFilterName;
    OUString        aGrfName, aNewGrfName;

    ::sfx2::FileDialogHelper*     pGrfDlg;

    bool            bHtmlMode;

    // handler for mirroring
    DECL_LINK_TYPED(MirrorHdl, Button*, void);
    DECL_LINK_TYPED(BrowseHdl, Button*, void);

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual ~SwGrfExtPage();
    virtual void dispose() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwGrfExtPage(vcl::Window *pParent, const SfxItemSet &rSet);

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;
    virtual sfxpg DeactivatePage(SfxItemSet *pSet) override;
};

class SwFrameURLPage : public SfxTabPage
{
    // hyperlink
    VclPtr<Edit>            pURLED;
    VclPtr<PushButton>      pSearchPB;
    VclPtr<Edit>            pNameED;
    VclPtr<ComboBox>        pFrameCB;

    // image map
    VclPtr<CheckBox>        pServerCB;
    VclPtr<CheckBox>        pClientCB;

    DECL_LINK_TYPED(InsertFileHdl, Button*, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwFrameURLPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFrameURLPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;
};

class SwFrameAddPage : public SfxTabPage
{
    VclPtr<VclContainer> m_pNameFrame;
    VclPtr<FixedText>    m_pNameFT;
    VclPtr<Edit>         m_pNameED;
    VclPtr<FixedText>    m_pAltNameFT;
    VclPtr<Edit>         m_pAltNameED;
    VclPtr<FixedText>    m_pPrevFT;
    VclPtr<ListBox>      m_pPrevLB;
    VclPtr<FixedText>    m_pNextFT;
    VclPtr<ListBox>      m_pNextLB;

    VclPtr<VclContainer> m_pProtectFrame;
    VclPtr<CheckBox>     m_pProtectContentCB;
    VclPtr<CheckBox>     m_pProtectFrameCB;
    VclPtr<CheckBox>     m_pProtectSizeCB;

    VclPtr<VclContainer> m_pContentAlignFrame;
    VclPtr<ListBox>      m_pVertAlignLB;

    VclPtr<VclContainer> m_pPropertiesFrame;
    VclPtr<CheckBox>     m_pEditInReadonlyCB;
    VclPtr<CheckBox>     m_pPrintFrameCB;
    VclPtr<FixedText>    m_pTextFlowFT;
    VclPtr<ListBox>      m_pTextFlowLB;

    SwWrtShell*   m_pWrtSh;

    OUString      m_sDlgType;
    bool      m_bHtmlMode;
    bool      m_bFormat;
    bool      m_bNew;

    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);
    DECL_LINK_TYPED(ChainModifyHdl, ListBox&, void);

    static const sal_uInt16 aAddPgRg[];

public:
    SwFrameAddPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwFrameAddPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16*  GetRanges() { return aAddPgRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

    void            SetFormatUsed(bool bFormat);
    void            SetFrameType(const OUString &rType) { m_sDlgType = rType; }
    void            SetNewFrame(bool bNewFrame) { m_bNew  = bNewFrame; }
    void            SetShell(SwWrtShell* pSh) { m_pWrtSh  = pSh; }

};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_FRMPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
