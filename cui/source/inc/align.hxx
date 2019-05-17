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
#ifndef INCLUDED_CUI_SOURCE_INC_ALIGN_HXX
#define INCLUDED_CUI_SOURCE_INC_ALIGN_HXX

// list box indexes
#define ALIGNDLG_HORALIGN_STD       0
#define ALIGNDLG_HORALIGN_LEFT      1
#define ALIGNDLG_HORALIGN_CENTER    2
#define ALIGNDLG_HORALIGN_RIGHT     3
#define ALIGNDLG_HORALIGN_BLOCK     4
#define ALIGNDLG_HORALIGN_FILL      5
#define ALIGNDLG_HORALIGN_DISTRIBUTED 6

#define ALIGNDLG_VERALIGN_STD         0
#define ALIGNDLG_VERALIGN_TOP         1
#define ALIGNDLG_VERALIGN_MID         2
#define ALIGNDLG_VERALIGN_BOTTOM      3
#define ALIGNDLG_VERALIGN_BLOCK       4
#define ALIGNDLG_VERALIGN_DISTRIBUTED 5

#include <sfx2/tabdlg.hxx>
#include <svtools/valueset.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/frmdirlbox.hxx>
#include <vcl/weld.hxx>

namespace svx {


class AlignmentTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
    friend class VclPtr<AlignmentTabPage>;
    static const sal_uInt16 s_pRanges[];

public:
    virtual             ~AlignmentTabPage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* rAttrSet );
    static const sal_uInt16*  GetRanges() { return s_pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    explicit            AlignmentTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet);

    void                InitVsRefEgde();
    void                UpdateEnableControls();

    bool                HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const;

    DECL_LINK(UpdateEnableHdl, weld::ComboBox&, void);
    DECL_LINK(StackedClickHdl, weld::ToggleButton&, void);
    DECL_LINK(AsianModeClickHdl, weld::ToggleButton&, void);
    DECL_LINK(WrapClickHdl, weld::ToggleButton&, void);
    DECL_LINK(HyphenClickHdl, weld::ToggleButton&, void);
    DECL_LINK(ShrinkClickHdl, weld::ToggleButton&, void);

private:
    weld::TriStateEnabled m_aStackedState;
    weld::TriStateEnabled m_aAsianModeState;
    weld::TriStateEnabled m_aWrapState;
    weld::TriStateEnabled m_aHyphenState;
    weld::TriStateEnabled m_aShrinkState;

    SvxDialControl m_aCtrlDial;
    SvtValueSet m_aVsRefEdge;

    std::unique_ptr<weld::ComboBox> m_xLbHorAlign;
    std::unique_ptr<weld::Label> m_xFtIndent;
    std::unique_ptr<weld::MetricSpinButton> m_xEdIndent;
    std::unique_ptr<weld::Label> m_xFtVerAlign;
    std::unique_ptr<weld::ComboBox> m_xLbVerAlign;

    std::unique_ptr<weld::Label> m_xFtRotate;
    std::unique_ptr<weld::SpinButton> m_xNfRotate;
    std::unique_ptr<weld::Label> m_xFtRefEdge;
    std::unique_ptr<weld::CheckButton> m_xCbStacked;
    std::unique_ptr<weld::CheckButton> m_xCbAsianMode;

    std::unique_ptr<weld::Widget> m_xBoxDirection;
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
    std::unique_ptr<weld::CustomWeld> m_xCtrlDial;
};


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
