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

#include <svx/connctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>

class SdrView;

/// Dialog for changing connectors.
class SvxConnectionPage : public SfxTabPage
{
private:
    static const sal_uInt16 pRanges[];
    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    MapUnit             eUnit;

    SvxXConnectionPreview m_aCtlPreview;
    std::unique_ptr<weld::ComboBox> m_xLbType;
    std::unique_ptr<weld::Label> m_xFtLine1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldLine1;
    std::unique_ptr<weld::Label> m_xFtLine2;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldLine2;
    std::unique_ptr<weld::Label> m_xFtLine3;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldLine3;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHorz1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldVert1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHorz2;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldVert2;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    void                FillTypeLB();

    DECL_LINK(ChangeAttrEditHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeAttrListBoxHdl_Impl, weld::ComboBox&, void);

public:

    SvxConnectionPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxConnectionPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/* Derived from SfxSingleTabDialogController, in order to be informed about
   virtual methods by the control. */
class SvxConnectionDialog : public SfxSingleTabDialogController
{
public:
    SvxConnectionDialog(weld::Window* pParent, const SfxItemSet& rAttr,
                        const SdrView* pView);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
