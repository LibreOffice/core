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

#include <sfx2/tabdlg.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/frmdirlbox.hxx>
#include <svx/swframeexample.hxx>
#include <swtypes.hxx>
#include "bmpwin.hxx"
#include "prcntfld.hxx"

namespace sfx2{class FileDialogHelper;}
class SwWrtShell;
struct FrameMap;
// OD 12.11.2003 #i22341#
struct SwPosition;

// frame dialog
class SwFramePage: public SfxTabPage
{
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

    SwFrameExample m_aExampleWN;

    // size
    std::unique_ptr<weld::Label> m_xWidthFT;
    std::unique_ptr<weld::Label> m_xWidthAutoFT;
    std::unique_ptr<weld::CheckButton> m_xRelWidthCB;
    std::unique_ptr<weld::ComboBox> m_xRelWidthRelationLB;
    std::unique_ptr<weld::CheckButton> m_xAutoWidthCB;

    std::unique_ptr<weld::Label> m_xHeightFT;
    std::unique_ptr<weld::Label> m_xHeightAutoFT;
    std::unique_ptr<weld::CheckButton> m_xRelHeightCB;
    std::unique_ptr<weld::ComboBox> m_xRelHeightRelationLB;
    std::unique_ptr<weld::CheckButton> m_xAutoHeightCB;

    std::unique_ptr<weld::CheckButton> m_xFixedRatioCB;
    std::unique_ptr<weld::Button> m_xRealSizeBT;

    // anchor
    std::unique_ptr<weld::Widget> m_xAnchorFrame;
    std::unique_ptr<weld::RadioButton> m_xAnchorAtPageRB;
    std::unique_ptr<weld::RadioButton> m_xAnchorAtParaRB;
    std::unique_ptr<weld::RadioButton> m_xAnchorAtCharRB;
    std::unique_ptr<weld::RadioButton> m_xAnchorAsCharRB;
    std::unique_ptr<weld::RadioButton> m_xAnchorAtFrameRB;

    // position
    std::unique_ptr<weld::Label> m_xHorizontalFT;
    std::unique_ptr<weld::ComboBox> m_xHorizontalDLB;
    std::unique_ptr<weld::Label> m_xAtHorzPosFT;
    std::unique_ptr<weld::MetricSpinButton> m_xAtHorzPosED;
    std::unique_ptr<weld::Label> m_xHoriRelationFT;
    std::unique_ptr<weld::ComboBox> m_xHoriRelationLB;

    std::unique_ptr<weld::CheckButton> m_xMirrorPagesCB;

    std::unique_ptr<weld::Label> m_xVerticalFT;
    std::unique_ptr<weld::ComboBox> m_xVerticalDLB;
    std::unique_ptr<weld::Label> m_xAtVertPosFT;
    std::unique_ptr<weld::MetricSpinButton> m_xAtVertPosED;
    std::unique_ptr<weld::Label> m_xVertRelationFT;
    std::unique_ptr<weld::ComboBox> m_xVertRelationLB;
    // #i18732# - check box for new option 'FollowTextFlow'
    std::unique_ptr<weld::CheckButton> m_xFollowTextFlowCB;

    // example
    std::unique_ptr<weld::CustomWeld> m_xExampleWN;

    std::unique_ptr<SwPercentField> m_xWidthED;
    std::unique_ptr<SwPercentField> m_xHeightED;

    virtual void    ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    DECL_LINK(RangeModifyClickHdl, weld::ToggleButton&, void);
    void RangeModifyHdl();
    DECL_LINK(AnchorTypeHdl, weld::ToggleButton&, void);
    DECL_LINK(PosHdl, weld::ComboBox&, void);
    DECL_LINK(RelHdl, weld::ComboBox&, void);
    void            InitPos(RndStdIds eId, sal_Int16 nH, sal_Int16 nHRel,
                            sal_Int16 nV, sal_Int16 nVRel,
                            tools::Long   nX,  tools::Long   nY);

    DECL_LINK(RealSizeHdl, weld::Button&, void);
    DECL_LINK(RelSizeClickHdl, weld::ToggleButton&, void);
    DECL_LINK(MirrorHdl, weld::ToggleButton&, void);

    DECL_LINK(AutoWidthClickHdl, weld::ToggleButton&, void);
    DECL_LINK(AutoHeightClickHdl, weld::ToggleButton&, void);

    // update example
    void            UpdateExample();
    DECL_LINK(ModifyHdl, weld::MetricSpinButton&, void);

    void            Init(const SfxItemSet& rSet);
    // OD 12.11.2003 #i22341# - adjustment to handle maps, that are ambiguous
    //                          in the alignment.
    sal_Int32       FillPosLB( const FrameMap* _pMap,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               weld::ComboBox& _rLB );
    // OD 14.11.2003 #i22341# - adjustment to handle maps, that are ambiguous
    //                          in their string entries.
    void            FillRelLB( const FrameMap* _pMap,
                               const sal_uInt16 _nLBSelPos,
                               const sal_Int16 _nAlign,
                               const sal_Int16 _nRel,
                               weld::ComboBox& _rLB,
                               weld::Label& _rFT );
    static sal_Int32 GetMapPos(const FrameMap *pMap, const weld::ComboBox& rAlignLB);
    static sal_Int16 GetAlignment(FrameMap const *pMap, sal_Int32 nMapPos, const weld::ComboBox& rRelationLB);
    static sal_Int16 GetRelation(const weld::ComboBox& rRelationLB);
    RndStdIds       GetAnchor() const;

    void setOptimalFrameWidth();
    void setOptimalRelWidth();

    void            EnableGraficMode();   // hides auto check boxes and re-org controls for "Real Size" button

    SwWrtShell *getFrameDlgParentShell();

    static const sal_uInt16 aPageRg[];

public:
    SwFramePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwFramePage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

    void            SetNewFrame(bool bNewFrame) { m_bNew      = bNewFrame; }
    void            SetFormatUsed(bool bFormat);
    void            SetFrameType(const OUString &rType) { m_sDlgType  = rType; }
    bool            IsInGraficMode() const { return m_sDlgType == "PictureDialog" || m_sDlgType == "ObjectDialog"; }
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
    std::unique_ptr<weld::MetricSpinButton> m_xNfAngle;
    std::unique_ptr<svx::DialControl> m_xCtlAngle;
    std::unique_ptr<weld::CustomWeld> m_xCtlAngleWin;
    std::unique_ptr<weld::CustomWeld> m_xBmpWin;

    // tdf#138843 add place holder for the graphic type
    std::unique_ptr<weld::Label> m_xLabelGraphicType;

    // handler for mirroring
    DECL_LINK(MirrorHdl, weld::ToggleButton&, void);
    DECL_LINK(BrowseHdl, weld::Button&, void);

    virtual void    ActivatePage(const SfxItemSet& rSet) override;

public:
    SwGrfExtPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    virtual ~SwGrfExtPage() override;

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

public:
    SwFrameURLPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwFrameURLPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);

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
    std::unique_ptr<weld::TextView> m_xDescriptionED;
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
    SwFrameAddPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwFrameAddPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
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
