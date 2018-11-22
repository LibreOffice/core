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
#include "bmpwin.hxx"
#include <svx/swframeexample.hxx>
#include "prcntfld.hxx"
#include <globals.hrc>
#include <svx/dialcontrol.hxx>
#include <svx/frmdirlbox.hxx>

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

    bool            m_bAtHorzPosModified;
    bool            m_bAtVertPosModified;

    bool            m_bFormat;
    bool            m_bNew;
    bool            m_bNoModifyHdl;
    bool            m_bIsVerticalFrame;  //current frame is in vertical environment - strings are exchanged
    // #mongolianlayout#
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

    FrameMap const * m_pVMap;
    FrameMap const * m_pHMap;

    bool    m_bAllowVertPositioning;
    bool    m_bIsMathOLE;
    bool    m_bIsMathBaselineAlignment;

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    DECL_LINK(RangeModifyLoseFocusHdl, Control&, void);
    DECL_LINK(RangeModifyClickHdl, Button*, void);
    void RangeModifyHdl();
    DECL_LINK(AnchorTypeHdl, Button*, void);
    DECL_LINK( PosHdl, ListBox&, void );
    DECL_LINK( RelHdl, ListBox&, void );
    void            InitPos(RndStdIds eId, sal_Int16 nH, sal_Int16 nHRel,
                            sal_Int16 nV, sal_Int16 nVRel,
                            long   nX,  long   nY);

    DECL_LINK(RealSizeHdl, Button *, void);
    DECL_LINK( RelSizeClickHdl, Button *, void );
    DECL_LINK(MirrorHdl, Button *, void);

    DECL_LINK( AutoWidthClickHdl, Button *, void);
    DECL_LINK( AutoHeightClickHdl, Button *, void);

    // update example
    void            UpdateExample();
    DECL_LINK( ModifyHdl, Edit&, void );

    void            Init(const SfxItemSet& rSet, bool bReset = false);
    // OD 12.11.2003 #i22341# - adjustment to handle maps, that are ambiguous
    //                          in the alignment.
    sal_Int32       FillPosLB( const FrameMap* _pMap,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB );
    // OD 14.11.2003 #i22341# - adjustment to handle maps, that are ambiguous
    //                          in their string entries.
    void            FillRelLB( const FrameMap* _pMap,
                               const sal_uInt16 _nLBSelPos,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               ListBox& _rLB,
                               FixedText& _rFT );
    static sal_Int32 GetMapPos(const FrameMap *pMap, ListBox const &rAlignLB);
    static sal_Int16 GetAlignment(FrameMap const *pMap, sal_Int32 nMapPos, ListBox const &rRelationLB);
    static sal_Int16 GetRelation(ListBox const &rRelationLB);
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
    virtual ~SwFramePage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

    void            SetNewFrame(bool bNewFrame) { m_bNew      = bNewFrame; }
    void            SetFormatUsed(bool bFormat);
    void            SetFrameType(const OUString &rType) { m_sDlgType  = rType; }
    bool     IsInGraficMode() { return m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"; }
    void            EnableVerticalPositioning( bool bEnable );
};

class SwGrfExtPage : public SfxTabPage
{
    OUString        aFilterName;
    OUString        aGrfName, aNewGrfName;

    std::unique_ptr<::sfx2::FileDialogHelper> m_xGrfDlg;

    bool            m_bHtmlMode;

    // mirror
    BmpWindow m_aBmpWin;
    svx::SvxDialControl m_aCtlAngle;
    std::unique_ptr<weld::Widget> m_xMirror;
    std::unique_ptr<weld::CheckButton> m_xMirrorVertBox;
    std::unique_ptr<weld::CheckButton> m_xMirrorHorzBox;
    std::unique_ptr<weld::RadioButton> m_xAllPagesRB;
    std::unique_ptr<weld::RadioButton> m_xLeftPagesRB;
    std::unique_ptr<weld::RadioButton> m_xRightPagesRB;

    std::unique_ptr<weld::Entry> m_xConnectED;
    std::unique_ptr<weld::Button> m_xBrowseBT;
    std::unique_ptr<weld::Frame> m_xLinkFrame;

    // RotGrfFlyFrame: Need Angle and RotateControls now
    std::unique_ptr<weld::Frame> m_xFlAngle;
    std::unique_ptr<weld::SpinButton> m_xNfAngle;
    std::unique_ptr<weld::CustomWeld> m_xCtlAngle;
    std::unique_ptr<weld::CustomWeld> m_xBmpWin;


    // handler for mirroring
    DECL_LINK(MirrorHdl, weld::ToggleButton&, void);
    DECL_LINK(BrowseHdl, weld::Button&, void);

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual ~SwGrfExtPage() override;
    virtual void dispose() override;

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwGrfExtPage(TabPageParent pParent, const SfxItemSet &rSet);

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet *pSet) override;
};

class SwFrameURLPage : public SfxTabPage
{
    // hyperlink
    std::unique_ptr<weld::Entry> m_xURLED;
    std::unique_ptr<weld::Button> m_xSearchPB;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::ComboBox> m_xFrameCB;

    // image map
    std::unique_ptr<weld::CheckButton> m_xServerCB;
    std::unique_ptr<weld::CheckButton> m_xClientCB;

    DECL_LINK(InsertFileHdl, weld::Button&, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwFrameURLPage(TabPageParent pParent, const SfxItemSet &rSet);
    virtual ~SwFrameURLPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;
};

class SwFrameAddPage : public SfxTabPage
{
    SwWrtShell*   m_pWrtSh;

    OUString      m_sDlgType;
    bool      m_bHtmlMode;
    bool      m_bFormat;
    bool      m_bNew;

    std::unique_ptr<weld::Widget> m_xNameFrame;
    std::unique_ptr<weld::Label> m_xNameFT;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Label> m_xAltNameFT;
    std::unique_ptr<weld::Entry> m_xAltNameED;
    std::unique_ptr<weld::Label> m_xDescriptionFT;
    std::unique_ptr<weld::Entry> m_xDescriptionED;
    std::unique_ptr<weld::Label> m_xPrevFT;
    std::unique_ptr<weld::ComboBox> m_xPrevLB;
    std::unique_ptr<weld::Label> m_xNextFT;
    std::unique_ptr<weld::ComboBox> m_xNextLB;

    std::unique_ptr<weld::Widget> m_xProtectFrame;
    std::unique_ptr<weld::CheckButton> m_xProtectContentCB;
    std::unique_ptr<weld::CheckButton> m_xProtectFrameCB;
    std::unique_ptr<weld::CheckButton> m_xProtectSizeCB;

    std::unique_ptr<weld::Widget> m_xContentAlignFrame;
    std::unique_ptr<weld::ComboBox> m_xVertAlignLB;

    std::unique_ptr<weld::Widget> m_xPropertiesFrame;
    std::unique_ptr<weld::CheckButton> m_xEditInReadonlyCB;
    std::unique_ptr<weld::CheckButton> m_xPrintFrameCB;
    std::unique_ptr<weld::Label> m_xTextFlowFT;
    std::unique_ptr<svx::FrameDirectionListBox> m_xTextFlowLB;

    DECL_LINK(EditModifyHdl, weld::Entry&, void);
    DECL_LINK(ChainModifyHdl, weld::ComboBox&, void);

    static const sal_uInt16 aAddPgRg[];

public:
    SwFrameAddPage(TabPageParent pParent, const SfxItemSet &rSet);
    virtual ~SwFrameAddPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet *rSet);
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
