/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <autoredactdialog.hxx>
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
    m_xControl->set_size_request(555, 250);
    std::vector<int> aWidths;
    aWidths.push_back(100);
    aWidths.push_back(50);
    aWidths.push_back(200);
    aWidths.push_back(105);
    aWidths.push_back(105);
    m_xControl->set_column_fixed_widths(aWidths);
    m_xControl->set_selection_mode(SelectionMode::Multiple);
}

namespace
{
OUString getTypeName(RedactionTargetType nType)
{
    OUString sTypeName("Unknown");

    switch (nType)
    {
        case RedactionTargetType::REDACTION_TARGET_TEXT:
            sTypeName = "Text";
            break;
        case RedactionTargetType::REDACTION_TARGET_REGEX:
            sTypeName = "Regex";
            break;
        case RedactionTargetType::REDACTION_TARGET_PREDEFINED:
            sTypeName = "Predefined";
            break;
        case RedactionTargetType::REDACTION_TARGET_UNKNOWN:
            sTypeName = "Unknown";
            break;
    }

    return sTypeName;
}
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
    m_xControl->set_text(nRow, getTypeName(pTarget->sType), 1);
    m_xControl->set_text(nRow, pTarget->sContent, 2);
    m_xControl->set_text(nRow, pTarget->bCaseSensitive ? OUString("Yes") : OUString("No"), 3);
    m_xControl->set_text(nRow, pTarget->bWholeWords ? OUString("Yes") : OUString("No"), 4);
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

/*IMPL_LINK_NOARG(SfxAutoRedactDialog, LoadHdl, weld::Button&, void)
{
    //TODO: Implement
    //Load a targets list from a previously saved file (a json file in the user profile dir?)
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, SaveHdl, weld::Button&, void)
{
    //TODO: Implement
    //Allow saving the targets into a file
}*/

IMPL_LINK_NOARG(SfxAutoRedactDialog, AddHdl, weld::Button&, void)
{
    // Open the Add Target dialog, craete a new target and insert into the targets vector and the listbox
    SfxAddTargetDialog aAddTargetDialog(getDialog(), m_xTargetsBox->GetNameProposal());

    bool bIncomplete;
    do
    {
        bIncomplete = false;

        if (aAddTargetDialog.run() != RET_OK)
            return;

        if (aAddTargetDialog.getName().isEmpty()
            || aAddTargetDialog.getType() == RedactionTargetType::REDACTION_TARGET_UNKNOWN
            || aAddTargetDialog.getContent().isEmpty())
        {
            bIncomplete = true;
            std::unique_ptr<weld::MessageDialog> xBox(
                Application::CreateMessageDialog(getDialog(), VclMessageType::Warning,
                                                 VclButtonsType::Ok, "All fields are required"));
            xBox->run();
        }
        else if (m_xTargetsBox->GetTargetByName(aAddTargetDialog.getName()))
        {
            bIncomplete = true;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
                "There is already a target with this name"));
            xBox->run();
        }

    } while (bIncomplete);

    //Alright, we now have everything we need to construct a new target
    RedactionTarget* redactiontarget = new RedactionTarget(
        { aAddTargetDialog.getName(), aAddTargetDialog.getType(), aAddTargetDialog.getContent(),
          aAddTargetDialog.isCaseSensitive(), aAddTargetDialog.isWholeWords(), 0 });

    // Only the visual/display part
    m_xTargetsBox->InsertTarget(redactiontarget);

    // Actually add to the targets vector
    if (m_xTargetsBox->GetTargetByName(redactiontarget->sName))
        m_aTableTargets.emplace_back(redactiontarget, redactiontarget->sName);
    else
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
            "An error occured while adding new target. Please report this incidence."));
        xBox->run();
        delete redactiontarget;
    }
}

/*IMPL_LINK_NOARG(SfxAutoRedactDialog, EditHdl, weld::Button&, void)
{
    //TODO: Implement
    //Reuse the Add Target dialog
}*/

IMPL_LINK_NOARG(SfxAutoRedactDialog, DeleteHdl, weld::Button&, void)
{
    std::vector<int> aSelectedRows = m_xTargetsBox->get_selected_rows();

    //No selection, so nothing to delete
    if (aSelectedRows.empty())
        return;

    if (aSelectedRows.size() > 1)
    {
        OUString sMsg("Are you sure you would like to delete "
                      + OUString::number(aSelectedRows.size()) + " targets at once?");
        //Warn the user about multiple deletions
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            getDialog(), VclMessageType::Question, VclButtonsType::OkCancel, sMsg));
        if (xBox->run() == RET_CANCEL)
            return;
    }

    // After each delete, the indexes of the following items decrease by one.
    int delta = 0;
    for (const auto& i : aSelectedRows)
    {
        m_aTableTargets.erase(m_aTableTargets.begin() + (i - delta));
        m_xTargetsBox->remove(i - delta++);
    }
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

    // TODO: fill the targets box

    // Handler connections
    //m_xLoadBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, LoadHdl));
    //m_xSaveBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, SaveHdl));
    m_xAddBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, AddHdl));
    //m_xEditBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, EditHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, DeleteHdl));
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

SfxAddTargetDialog::SfxAddTargetDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, "sfx/ui/addtargetdialog.ui", "AddTargetDialog")
    , m_xName(m_xBuilder->weld_entry("name"))
    , m_xType(m_xBuilder->weld_combo_box("type"))
    , m_xContent(m_xBuilder->weld_entry("content"))
    , m_xCaseSensitive(m_xBuilder->weld_check_button("checkboxCaseSensitive"))
    , m_xWholeWords(m_xBuilder->weld_check_button("checkboxWholeWords"))
{
    m_xName->set_text(rName);
    m_xName->select_region(0, rName.getLength());
}

RedactionTargetType SfxAddTargetDialog::getType() const
{
    OUString sTypeID = m_xType->get_active_id();

    if (sTypeID == "text")
        return RedactionTargetType::REDACTION_TARGET_TEXT;
    else if (sTypeID == "regex")
        return RedactionTargetType::REDACTION_TARGET_REGEX;
    else if (sTypeID == "predefined")
        return RedactionTargetType::REDACTION_TARGET_PREDEFINED;
    else
        return RedactionTargetType::REDACTION_TARGET_UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
