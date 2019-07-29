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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/colorbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

class ScViewOptions;

class ScTpContentOptions : public SfxTabPage
{
    friend class VclPtr<ScTpContentOptions>;
    VclPtr<ListBox>         pGridLB;
    VclPtr<FixedText>       pColorFT;
    VclPtr<SvxColorListBox> pColorLB;
    VclPtr<CheckBox>        pBreakCB;
    VclPtr<CheckBox>        pGuideLineCB;

    VclPtr<CheckBox>        pFormulaCB;
    VclPtr<CheckBox>        pNilCB;
    VclPtr<CheckBox>        pAnnotCB;
    VclPtr<CheckBox>        pValueCB;
    VclPtr<CheckBox>        pAnchorCB;
    VclPtr<CheckBox>        pClipMarkCB;
    VclPtr<CheckBox>        pRangeFindCB;

    VclPtr<ListBox>         pObjGrfLB;
    VclPtr<ListBox>         pDiagramLB;
    VclPtr<ListBox>         pDrawLB;

    VclPtr<CheckBox>        pSyncZoomCB;

    VclPtr<CheckBox>        pRowColHeaderCB;
    VclPtr<CheckBox>        pHScrollCB;
    VclPtr<CheckBox>        pVScrollCB;
    VclPtr<CheckBox>        pTblRegCB;
    VclPtr<CheckBox>        pOutlineCB;
    VclPtr<CheckBox>        pSummaryCB;

    std::unique_ptr<ScViewOptions> pLocalOptions;

    void    InitGridOpt();
    DECL_LINK( GridHdl, ListBox&, void );
    DECL_LINK( SelLbObjHdl, ListBox&, void );
    DECL_LINK( CBHdl, Button*, void );

            ScTpContentOptions( vcl::Window*         pParent,
                             const SfxItemSet&  rArgSet );
            virtual ~ScTpContentOptions() override;
    virtual void dispose() override;

public:
    static  VclPtr<SfxTabPage> Create          ( TabPageParent pParent,
                                          const SfxItemSet*     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    friend class VclPtrInstance<ScTpLayoutOptions>;
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

    DECL_LINK(MetricHdl, weld::ComboBox&, void );
    DECL_LINK( AlignHdl, weld::ToggleButton&, void );


    ScTpLayoutOptions(TabPageParent pParent, const SfxItemSet&  rArgSet );
public:
    virtual ~ScTpLayoutOptions() override;
    static  VclPtr<SfxTabPage> Create          ( TabPageParent pParent,
                                          const SfxItemSet*     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
