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

#include <sfx2/basedlgs.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/measctrl.hxx>

class SdrView;

/// Dialog for changing TextAttributes
class SvxMeasurePage : public SvxTabPage
{
private:
    static const sal_uInt16 pRanges[];

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    MapUnit             eUnit;

    bool            bPositionModified;

    SvxRectCtl m_aCtlPosition;
    SvxXMeasurePreview  m_aCtlPreview;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldLineDist;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHelplineOverhang;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHelplineDist;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHelpline1Len;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHelpline2Len;
    std::unique_ptr<weld::CheckButton> m_xTsbBelowRefEdge;
    std::unique_ptr<weld::SpinButton> m_xMtrFldDecimalPlaces;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoPosV;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoPosH;
    std::unique_ptr<weld::CheckButton> m_xTsbShowUnit;
    std::unique_ptr<weld::ComboBox> m_xLbUnit;
    std::unique_ptr<weld::CheckButton> m_xTsbParallel;
    std::unique_ptr<weld::Label> m_xFtAutomatic;
    std::unique_ptr<weld::CustomWeld> m_xCtlPosition;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    void                FillUnitLB();

    DECL_LINK(ClickAutoPosHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ChangeAttrEditHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeAttrSpinHdl_Impl, weld::SpinButton&, void);
    DECL_LINK(ChangeAttrListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ChangeAttrClickHdl_Impl, weld::ToggleButton&, void);
    void ChangeAttrHdl_Impl(void const *);

public:

    SvxMeasurePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxMeasurePage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;

    virtual void        PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;

};

/* Derived from SfxSingleTabDialogController, in order to be able to be
   informed about virtual methods by the control. */
class SvxMeasureDialog : public SfxSingleTabDialogController
{
public:
    SvxMeasureDialog(weld::Window* pParent, const SfxItemSet& rAttr,
        const SdrView* pView);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
