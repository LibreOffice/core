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
#include <svx/colorbox.hxx>

class ScViewOptions;

class ScTpContentOptions : public SfxTabPage
{
    std::unique_ptr<ScViewOptions> m_xLocalOptions;

    std::unique_ptr<weld::ComboBox> m_xGridLB;
    std::unique_ptr<weld::Label> m_xColorFT;
    std::unique_ptr<ColorListBox> m_xColorLB;
    std::unique_ptr<weld::CheckButton> m_xBreakCB;
    std::unique_ptr<weld::CheckButton> m_xGuideLineCB;

    std::unique_ptr<weld::CheckButton> m_xFormulaCB;
    std::unique_ptr<weld::CheckButton> m_xNilCB;
    std::unique_ptr<weld::CheckButton> m_xAnnotCB;
    std::unique_ptr<weld::CheckButton> m_xValueCB;
    std::unique_ptr<weld::CheckButton> m_xAnchorCB;
    std::unique_ptr<weld::CheckButton> m_xClipMarkCB;
    std::unique_ptr<weld::CheckButton> m_xRangeFindCB;

    std::unique_ptr<weld::ComboBox> m_xObjGrfLB;
    std::unique_ptr<weld::ComboBox> m_xDiagramLB;
    std::unique_ptr<weld::ComboBox> m_xDrawLB;

    std::unique_ptr<weld::CheckButton> m_xSyncZoomCB;

    std::unique_ptr<weld::CheckButton> m_xRowColHeaderCB;
    std::unique_ptr<weld::CheckButton> m_xHScrollCB;
    std::unique_ptr<weld::CheckButton> m_xVScrollCB;
    std::unique_ptr<weld::CheckButton> m_xTblRegCB;
    std::unique_ptr<weld::CheckButton> m_xOutlineCB;
    std::unique_ptr<weld::CheckButton> m_xSummaryCB;

    void    InitGridOpt();
    DECL_LINK( GridHdl, weld::ComboBox&, void );
    DECL_LINK( SelLbObjHdl, weld::ComboBox&, void );
    DECL_LINK( CBHdl, weld::ToggleButton&, void );

public:
    ScTpContentOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet);
    virtual ~ScTpContentOptions() override;
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    ScDocument *pDoc;

    std::unique_ptr<weld::ComboBox> m_xUnitLB;
    std::unique_ptr<weld::MetricSpinButton> m_xTabMF;

    std::unique_ptr<weld::RadioButton> m_xAlwaysRB;
    std::unique_ptr<weld::RadioButton> m_xRequestRB;
    std::unique_ptr<weld::RadioButton> m_xNeverRB;

    std::unique_ptr<weld::CheckButton> m_xAlignCB;
    std::unique_ptr<weld::ComboBox> m_xAlignLB;
    std::unique_ptr<weld::CheckButton> m_xEditModeCB;
    std::unique_ptr<weld::CheckButton> m_xFormatCB;
    std::unique_ptr<weld::CheckButton> m_xExpRefCB;
    std::unique_ptr<weld::CheckButton> m_xSortRefUpdateCB;
    std::unique_ptr<weld::CheckButton> m_xMarkHdrCB;
    std::unique_ptr<weld::CheckButton> m_xTextFmtCB;
    std::unique_ptr<weld::CheckButton> m_xReplWarnCB;
    std::unique_ptr<weld::CheckButton> m_xLegacyCellSelectionCB;
    std::unique_ptr<weld::CheckButton> m_xEnterPasteModeCB;

    DECL_LINK(MetricHdl, weld::ComboBox&, void );
    DECL_LINK( AlignHdl, weld::ToggleButton&, void );


public:
    ScTpLayoutOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&  rArgSet );
    static  std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet* rCoreSet);
    virtual ~ScTpLayoutOptions() override;
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
