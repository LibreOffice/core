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

#include <tipofthedaydlg.hxx>

#include <config_folders.h>
#include <dialmgr.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tipoftheday.hrc>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>

TipOfTheDayDialog::TipOfTheDayDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/tipofthedaydialog.ui", "TipOfTheDayDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pText(m_xBuilder->weld_label("lbText"))
    , m_pShowTip(m_xBuilder->weld_check_button("cbShowTip"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pLink(m_xBuilder->weld_link_button("btnLink"))
{
    m_pShowTip->connect_toggled(LINK(this, TipOfTheDayDialog, OnShowTipToggled));
    m_pNext->connect_clicked(LINK(this, TipOfTheDayDialog, OnNextClick));

    nNumberOfTips = SAL_N_ELEMENTS(TIPOFTHEDAY_STRINGARRAY);
    srand(time(NULL));
    nCurrentTip = rand() % nNumberOfTips;
    UpdateTip();
}

TipOfTheDayDialog::~TipOfTheDayDialog()
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    const auto t0 = std::chrono::system_clock::now().time_since_epoch();
    const sal_Int32 nDay
        = std::chrono::duration_cast<std::chrono::hours>(t0).count() / 24; // days since 1970-01-01
    officecfg::Office::Common::Misc::LastTipOfTheDayShown::set(nDay, xChanges);
    xChanges->commit();
}

static bool file_exists(const OUString& fileName)
{
    ::osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

void TipOfTheDayDialog::UpdateTip()
{
    //get string
    OUString aText;
    aText = CuiResId(TIPOFTHEDAY_STRINGARRAY[nCurrentTip].first);
    //move hyperlink into linkbutton
    sal_Int32 nPos = aText.indexOf("http");
    if (nPos > 0)
    {
        m_pLink->set_visible(true);
        if (aText.getLength() - nPos > 40)
            m_pLink->set_label(aText.copy(nPos, 40) + "...");
        else
            m_pLink->set_label(aText.copy(nPos));
        m_pLink->set_uri(aText.copy(nPos));
        aText = aText.copy(0, nPos - 1);
    }
    else
        m_pLink->set_visible(false);
    m_pText->set_label(aText);

    // import the image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/tipoftheday/");
    rtl::Bootstrap::expandMacros(aURL);
    OUString aName = TIPOFTHEDAY_STRINGARRAY[nCurrentTip].second;
    // use default image if none is available with the number
    if ((aName == "") | !file_exists(aURL + aName))
        aName = "tipoftheday.png";
    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL + aName, OUString(), aGraphic) == ERRCODE_NONE)
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev;
        m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0, 0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_STATIC_LINK(TipOfTheDayDialog, OnShowTipToggled, weld::ToggleButton&, rButton, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ShowTipOfTheDay::set(rButton.get_active(), xChanges);
    xChanges->commit();
}

IMPL_LINK_NOARG(TipOfTheDayDialog, OnNextClick, weld::Button&, void)
{
    nCurrentTip = rand() % nNumberOfTips;
    UpdateTip();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */