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
#pragma once

#include <sfx2/tabdlg.hxx>
#include <svx/swframeexample.hxx>
#include <vcl/weld.hxx>

// SvxSwPosSizeTabPage - position and size page for Writer drawing objects
struct FrmMap;
class SdrView;
struct SvxSwFrameValidation;
enum class SvxAnchorIds;

class SvxSwPosSizeTabPage : public SfxTabPage
{
    Link<SvxSwFrameValidation&,void> m_aValidateLink;

    ::tools::Rectangle           m_aRect; //size of all selected objects
    ::tools::Rectangle           m_aWorkArea;
    Point               m_aAnchorPos;

    FrmMap const * m_pVMap;
    FrmMap const * m_pHMap;
    const SdrView*      m_pSdrView;

    // initial values
    short   m_nOldH;
    short   m_nOldHRel;
    short   m_nOldV;
    short   m_nOldVRel;

    double  m_fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button
    bool    m_bHtmlMode;
    bool    m_bIsVerticalFrame;
    bool    m_bPositioningDisabled;
    bool    m_bIsMultiSelection;
    bool    m_bIsInRightToLeft;
    bool m_bDoNotMirrorRtlDrawObjs = false;
    TriState    m_nProtectSizeState;

    SwFrameExample m_aExampleWN;

    std::unique_ptr<weld::MetricSpinButton> m_xWidthMF;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightMF;
    std::unique_ptr<weld::CheckButton> m_xKeepRatioCB;
    std::unique_ptr<weld::RadioButton> m_xToPageRB;
    std::unique_ptr<weld::RadioButton> m_xToParaRB;
    std::unique_ptr<weld::RadioButton> m_xToCharRB;
    std::unique_ptr<weld::RadioButton> m_xAsCharRB;
    std::unique_ptr<weld::RadioButton> m_xToFrameRB;
    std::unique_ptr<weld::CheckButton> m_xPositionCB;
    std::unique_ptr<weld::CheckButton> m_xSizeCB;
    std::unique_ptr<weld::Widget> m_xPosFrame;
    std::unique_ptr<weld::Label> m_xHoriFT;
    std::unique_ptr<weld::ComboBox> m_xHoriLB;
    std::unique_ptr<weld::Label> m_xHoriByFT;
    std::unique_ptr<weld::MetricSpinButton> m_xHoriByMF;
    std::unique_ptr<weld::Label> m_xHoriToFT;
    std::unique_ptr<weld::ComboBox> m_xHoriToLB;
    std::unique_ptr<weld::CheckButton> m_xHoriMirrorCB;
    std::unique_ptr<weld::Label> m_xVertFT;
    std::unique_ptr<weld::ComboBox> m_xVertLB;
    std::unique_ptr<weld::Label> m_xVertByFT;
    std::unique_ptr<weld::MetricSpinButton> m_xVertByMF;
    std::unique_ptr<weld::Label> m_xVertToFT;
    std::unique_ptr<weld::ComboBox> m_xVertToLB;
    std::unique_ptr<weld::CheckButton> m_xFollowCB;
    std::unique_ptr<weld::CustomWeld> m_xExampleWN;

    DECL_LINK(RangeModifyHdl, weld::Widget&, void);
    DECL_LINK(RangeModifyClickHdl, weld::Toggleable&, void);
    DECL_LINK(AnchorTypeHdl, weld::Toggleable&, void);
    DECL_LINK(PosHdl, weld::ComboBox&, void);
    DECL_LINK(RelHdl, weld::ComboBox&, void);
    DECL_LINK(MirrorHdl, weld::Toggleable&, void);
    DECL_LINK(ModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ProtectHdl, weld::Toggleable&, void);

    void            InitPos(RndStdIds nAnchorType, sal_uInt16 nH, sal_uInt16 nHRel,
                            sal_uInt16 nV,  sal_uInt16 nVRel,
                            tools::Long   nX,  tools::Long   nY);
    static sal_uInt16   GetMapPos(FrmMap const *pMap, const weld::ComboBox& rAlignLB);
    static short        GetAlignment(FrmMap const *pMap, sal_uInt16 nMapPos, const weld::ComboBox& rRelationLB);
    static short        GetRelation(const weld::ComboBox& rRelationLB);
    RndStdIds           GetAnchorType(bool* pbHasChanged = nullptr);
    void                FillRelLB(FrmMap const *pMap, sal_uInt16 nLBSelPos, sal_uInt16 nAlign, sal_uInt16 nRel, weld::ComboBox& rLB, weld::Label& rFT);
    sal_uInt16          FillPosLB(FrmMap const *pMap, sal_uInt16 nAlign, const sal_uInt16 _nRel, weld::ComboBox& rLB);

    void            UpdateExample();

    void setOptimalFrmWidth();
    void setOptimalRelWidth();

public:
    SvxSwPosSizeTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    virtual ~SvxSwPosSizeTabPage() override;

    static WhichRangesContainer GetRanges();

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    EnableAnchorTypes(SvxAnchorIds nAnchorEnable);

    void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
            {m_aValidateLink = rLink;}

    void SetView( const SdrView* pSdrView );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
