/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <sfx2/autoredactdialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/idle.hxx>
#include <vcl/gdimtf.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <sfx2/strings.hrc>
#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <preview.hxx>
#include <sfx2/printer.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/weld.hxx>

SfxAutoRedactDialog::SfxAutoRedactDialog(weld::Window* pParent)
    : SfxDialogController(pParent, "sfx/ui/autoredactdialog.ui", "AutoRedactDialog")
    , m_xRedactionTargetsLabel(m_xBuilder->weld_label("labelRedactionTargets"))
    , m_xLoadBtn(m_xBuilder->weld_button("btnLoadTargets"))
    , m_xSaveBtn(m_xBuilder->weld_button("btnSaveTargets"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xEditBtn(m_xBuilder->weld_button("edit"))
    , m_xDeleteBtn(m_xBuilder->weld_button("delete"))
{
    // Can be used to remmeber the last set of redaction targets?
    OUString sExtraData;
    SvtViewOptions aDlgOpt(EViewType::Dialog,
                           OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
        aUserItem >>= sExtraData;
    }

    // update the targets configuration if necessary
    {
        weld::WaitObject aWaitCursor(m_xDialog.get());
        //m_aTargets.Update();
    }

    // fill the targets box
    /*const sal_uInt16 nCount = m_aTemplates.GetRegionCount();
    if (nCount)
    {
        for(sal_uInt16 i = 0; i < nCount; ++i)
            m_xRegionLb->append_text(m_aTemplates.GetFullRegionName(i));
        m_xRegionLb->connect_changed(LINK(this, SfxNewFileDialog, RegionSelect));
    }*/
}

SfxAutoRedactDialog::~SfxAutoRedactDialog()
{
    // Store the view options
    /*SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(m_xMoreBt->get_expanded() ? OUString("Y") : OUString("N")));*/
}

bool SfxAutoRedactDialog::hasTargets()
{
    //TODO: Implement
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
