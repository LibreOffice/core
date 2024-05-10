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
#ifndef INCLUDED_SVX_HDFT_HXX
#define INCLUDED_SVX_HDFT_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/weld.hxx>

#include <svx/pagectrl.hxx>
#include <svx/svxdllapi.h>
#include <memory>

namespace weld { class CustomWeld; }

namespace svx
{
    SVX_DLLPUBLIC bool ShowBorderBackgroundDlg(weld::Window* pParent, SfxItemSet* pBBSet);
}

class SVX_DLLPUBLIC SvxHFPage : public SfxTabPage
{
public:

    virtual bool    FillItemSet( SfxItemSet* rOutSet ) override;
    virtual void    Reset( const SfxItemSet* rSet ) override;

    virtual         ~SvxHFPage() override;

    void DisableDeleteQueryBox() { mbDisableQueryBox = true; }

    virtual void PageCreated(const SfxAllItemSet&) override;

    void            EnableDynamicSpacing();

protected:
    static const WhichRangesContainer pRanges;

    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    SvxHFPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet, sal_uInt16 nSetId);

    sal_uInt16       nId;
    std::unique_ptr<SfxItemSet> pBBSet;
    bool            mbDisableQueryBox : 1;
    bool            mbEnableDrawingLayerFillStyles : 1;

    SvxPageWindow m_aBspWin;
    std::unique_ptr<weld::Label> m_xPageLbl;
    std::unique_ptr<weld::CheckButton> m_xTurnOnBox;
    std::unique_ptr<weld::CheckButton> m_xCntSharedBox;
    std::unique_ptr<weld::CheckButton> m_xCntSharedFirstBox;
    std::unique_ptr<weld::Label> m_xLMLbl;
    std::unique_ptr<weld::MetricSpinButton>m_xLMEdit;
    std::unique_ptr<weld::Label> m_xRMLbl;
    std::unique_ptr<weld::MetricSpinButton> m_xRMEdit;
    std::unique_ptr<weld::Label> m_xDistFT;
    std::unique_ptr<weld::MetricSpinButton> m_xDistEdit;
    std::unique_ptr<weld::CheckButton> m_xDynSpacingCB;
    std::unique_ptr<weld::Label> m_xHeightFT;
    std::unique_ptr<weld::MetricSpinButton> m_xHeightEdit;
    std::unique_ptr<weld::CheckButton> m_xHeightDynBtn;
    std::unique_ptr<weld::Button> m_xBackgroundBtn;
    std::unique_ptr<weld::CustomWeld> m_xBspWin;

    void            InitHandler();
    void TurnOn(const weld::Toggleable* pButton);
    DECL_LINK(TurnOnHdl, weld::Toggleable&, void);
    DECL_DLLPRIVATE_LINK(BackgroundHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(ValueChangeHdl, weld::MetricSpinButton&, void);
    void RangeHdl();
    void            UpdateExample();

private:
    SVX_DLLPRIVATE void         ResetBackground_Impl( const SfxItemSet& rSet );
};

class SVX_DLLPUBLIC SvxHeaderPage final : public SvxHFPage
{
public:
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
    // returns the Which values to the range
    static WhichRangesContainer GetRanges() { return pRanges; }
    SVX_DLLPRIVATE SvxHeaderPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
};

class SVX_DLLPUBLIC SvxFooterPage final : public SvxHFPage
{
public:
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );
    static WhichRangesContainer GetRanges() { return pRanges; }
    SVX_DLLPRIVATE SvxFooterPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
};

class SVX_DLLPUBLIC DeleteHeaderDialog final : public weld::MessageDialogController
{
public:
    DeleteHeaderDialog(weld::Widget* pParent)
        : MessageDialogController(pParent, u"svx/ui/deleteheaderdialog.ui"_ustr,
                u"DeleteHeaderDialog"_ustr)
    {
    }
};

class SVX_DLLPUBLIC DeleteFooterDialog final : public weld::MessageDialogController
{
public:
    DeleteFooterDialog(weld::Widget* pParent)
        : MessageDialogController(pParent, u"svx/ui/deletefooterdialog.ui"_ustr,
                u"DeleteFooterDialog"_ustr)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
