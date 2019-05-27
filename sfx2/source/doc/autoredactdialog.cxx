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

#include <sal/log.hxx>

int TargetsTable::GetRowByTargetName(const OUString& sName)
{
    for (int i = 0, nCount = m_xControl->n_children(); i < nCount; ++i)
    {
        RedactionTarget* pTarget
            = reinterpret_cast<RedactionTarget*>(m_xControl->get_id(i).toInt64());
        if (pTarget->sName == sName)
        {
            return i;
        }
    }
    return -1;
}

TargetsTable::TargetsTable(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
{
    m_xControl->set_size_request(550, 250);
    std::vector<int> aWidths;
    aWidths.push_back(100);
    aWidths.push_back(45);
    aWidths.push_back(110);
    aWidths.push_back(105);
    aWidths.push_back(150);
    m_xControl->set_column_fixed_widths(aWidths);
    m_xControl->set_selection_mode(SelectionMode::Multiple);
}

void TargetsTable::InsertTarget(RedactionTarget* pTarget)
{
    if (!pTarget)
    {
        SAL_WARN("sfx.doc", "pTarget is null in TargetsTable::InsertTarget()");
        return;
    }

    // Check if the name is empty or invalid (clashing with another entry's name)
    if (pTarget->sName.isEmpty() || GetRowByTargetName(pTarget->sName) != -1)
    {
        pTarget->sName = GetNameProposal();
    }

    // Add to the end
    int nRow = m_xControl->n_children();
    m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(pTarget)), pTarget->sName);
    m_xControl->set_text(nRow, pTarget->sType, 1);
    m_xControl->set_text(nRow, pTarget->bCaseSensitive ? OUString("Yes") : OUString("No"), 2);
    m_xControl->set_text(nRow, pTarget->bWholeWords ? OUString("Yes") : OUString("No"), 3);
    m_xControl->set_text(nRow, pTarget->sDescription, 4);
}

void TargetsTable::SelectByName(const OUString& sName)
{
    int nEntry = GetRowByTargetName(sName);
    if (nEntry == -1)
        return;
    select(nEntry);
}

RedactionTarget* TargetsTable::GetTargetByName(const OUString& sName)
{
    int nEntry = GetRowByTargetName(sName);
    if (nEntry == -1)
        return nullptr;

    return reinterpret_cast<RedactionTarget*>(m_xControl->get_id(nEntry).toInt64());
}

OUString TargetsTable::GetNameProposal()
{
    //TODO: Define a translatable string
    OUString sDefaultTargetName("Target");
    sal_Int32 nHighestTargetId = 0;
    for (int i = 0, nCount = m_xControl->n_children(); i < nCount; ++i)
    {
        RedactionTarget* pTarget
            = reinterpret_cast<RedactionTarget*>(m_xControl->get_id(i).toInt64());
        const OUString& sName = pTarget->sName;
        sal_Int32 nIndex = 0;
        if (sName.getToken(0, ' ', nIndex) == sDefaultTargetName)
        {
            sal_Int32 nCurrTargetId = sName.getToken(0, ' ', nIndex).toInt32();
            nHighestTargetId = std::max<sal_Int32>(nHighestTargetId, nCurrTargetId);
        }
    }
    return sDefaultTargetName + " " + OUString::number(nHighestTargetId + 1);
}

SfxAutoRedactDialog::SfxAutoRedactDialog(weld::Window* pParent)
    : SfxDialogController(pParent, "sfx/ui/autoredactdialog.ui", "AutoRedactDialog")
    , m_xRedactionTargetsLabel(m_xBuilder->weld_label("labelRedactionTargets"))
    , m_xTargetsBox(new TargetsTable(m_xBuilder->weld_tree_view("targets")))
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

    /*RedactionTarget* redactiontarget
        = new RedactionTarget({ 0, "Target 1", "String", true, false, "Some description" });

    m_xTargetsBox->InsertTarget(redactiontarget);*/
}

SfxAutoRedactDialog::~SfxAutoRedactDialog()
{
    // Store the view options
    /*SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(m_xMoreBt->get_expanded() ? OUString("Y") : OUString("N")));*/
}

bool SfxAutoRedactDialog::hasTargets() const
{
    //TODO: Add also some validity checks?
    if (m_aTableTargets.empty())
        return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
