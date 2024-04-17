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

// list box indexes
enum HorizontalAlign {
    ALIGNDLG_HORALIGN_STD = 0,
    ALIGNDLG_HORALIGN_LEFT = 1,
    ALIGNDLG_HORALIGN_CENTER = 2,
    ALIGNDLG_HORALIGN_RIGHT = 3,
    ALIGNDLG_HORALIGN_BLOCK = 4,
    ALIGNDLG_HORALIGN_FILL = 5,
    ALIGNDLG_HORALIGN_DISTRIBUTED = 6
};

enum VerticalAlign {
    ALIGNDLG_VERALIGN_STD = 0,
    ALIGNDLG_VERALIGN_TOP = 1,
    ALIGNDLG_VERALIGN_MID = 2,
    ALIGNDLG_VERALIGN_BOTTOM = 3,
    ALIGNDLG_VERALIGN_BLOCK = 4,
    ALIGNDLG_VERALIGN_DISTRIBUTED = 5
};

#include <sfx2/tabdlg.hxx>
#include <svtools/valueset.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/frmdirlbox.hxx>
#include <vcl/weld.hxx>

class SfxEnumItemInterface;

namespace svx {


class AlignmentTabPage : public SfxTabPage
{
    static const WhichRangesContainer s_pRanges;

public:
    virtual             ~AlignmentTabPage() override;
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );
    explicit            AlignmentTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet);

    static WhichRangesContainer GetRanges() { return s_pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
    void                InitVsRefEgde();
    void                UpdateEnableControls();

    bool                HasAlignmentChanged( const SfxItemSet& rNew, TypedWhichId<SfxEnumItemInterface> nWhich ) const;

    DECL_LINK(UpdateEnableHdl, weld::ComboBox&, void);
    DECL_LINK(StackedClickHdl, weld::Toggleable&, void);
    DECL_LINK(AsianModeClickHdl, weld::Toggleable&, void);
    DECL_LINK(WrapClickHdl, weld::Toggleable&, void);
    DECL_LINK(HyphenClickHdl, weld::Toggleable&, void);
    DECL_LINK(ShrinkClickHdl, weld::Toggleable&, void);

private:
    weld::TriStateEnabled m_aStackedState;
    weld::TriStateEnabled m_aAsianModeState;
    weld::TriStateEnabled m_aWrapState;
    weld::TriStateEnabled m_aHyphenState;
    weld::TriStateEnabled m_aShrinkState;

    ValueSet m_aVsRefEdge;

    std::unique_ptr<weld::ComboBox> m_xLbHorAlign;
    std::unique_ptr<weld::Label> m_xFtIndent;
    std::unique_ptr<weld::MetricSpinButton> m_xEdIndent;
    std::unique_ptr<weld::Label> m_xFtVerAlign;
    std::unique_ptr<weld::ComboBox> m_xLbVerAlign;

    std::unique_ptr<weld::Label> m_xFtRotate;
    std::unique_ptr<weld::MetricSpinButton> m_xNfRotate;
    std::unique_ptr<weld::Label> m_xFtRefEdge;
    std::unique_ptr<weld::CheckButton> m_xCbStacked;
    std::unique_ptr<weld::CheckButton> m_xCbAsianMode;

    std::unique_ptr<weld::CheckButton> m_xBtnWrap;
    std::unique_ptr<weld::CheckButton> m_xBtnHyphen;
    std::unique_ptr<weld::CheckButton> m_xBtnShrink;
    std::unique_ptr<svx::FrameDirectionListBox> m_xLbFrameDir;

    // hidden labels/string
    std::unique_ptr<weld::Label> m_xFtBotLock;
    std::unique_ptr<weld::Label> m_xFtTopLock;
    std::unique_ptr<weld::Label> m_xFtCelLock;
    std::unique_ptr<weld::Label> m_xFtABCD;

    std::unique_ptr<weld::Widget> m_xAlignmentFrame;
    std::unique_ptr<weld::Widget> m_xOrientFrame;
    std::unique_ptr<weld::Widget> m_xPropertiesFrame;

    std::unique_ptr<weld::CustomWeld> m_xVsRefEdge;
    std::unique_ptr<DialControl> m_xCtrlDial;
    std::unique_ptr<weld::CustomWeld> m_xCtrlDialWin;
};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
