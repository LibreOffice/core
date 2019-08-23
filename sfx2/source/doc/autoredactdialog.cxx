/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <autoredactdialog.hxx>

#include <preview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>

#include <osl/file.hxx>
#include <sal/log.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/idle.hxx>
#include <vcl/layout.hxx>
#include <vcl/waitobj.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>

#include <svtools/treelistentry.hxx>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <boost/property_tree/json_parser.hpp>

SvTreeListEntry* TargetsTable::GetRowByTargetName(const OUString& sName)
{
    SvTreeListEntry* pEntry = First();
    while (pEntry)
    {
        RedactionTarget* pTarget = static_cast<RedactionTarget*>(pEntry->GetUserData());
        if (pTarget->sName == sName)
        {
            return pEntry;
        }
        pEntry = Next(pEntry);
    }
    return nullptr;
}

TargetsTable::TargetsTable(SvSimpleTableContainer& rParent)
    : SvSimpleTable(rParent, 0)
{
    static long nTabs[] = { 5, 0, 100, 150, 350, 455 };

    SetTabs(nTabs, MapUnit::MapPixel);
    SetSelectionMode(SelectionMode::Multiple);
    InsertHeaderEntry("Target Name");
    InsertHeaderEntry("Type");
    InsertHeaderEntry("Content");
    InsertHeaderEntry("Case Sensitive");
    InsertHeaderEntry("Whole Words");

    rParent.SetTable(this);
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

/// Returns TypeID to be used in the add/edit target dialog
sal_Int32 getTypeID(RedactionTargetType nType)
{
    sal_Int32 nTypeID(-1);

    switch (nType)
    {
        case RedactionTargetType::REDACTION_TARGET_TEXT:
            nTypeID = 0;
            break;
        case RedactionTargetType::REDACTION_TARGET_REGEX:
            nTypeID = 1;
            break;
        case RedactionTargetType::REDACTION_TARGET_PREDEFINED:
            nTypeID = 2;
            break;
        case RedactionTargetType::REDACTION_TARGET_UNKNOWN:
            nTypeID = -1;
            break;
    }

    return nTypeID;
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
    if (pTarget->sName.isEmpty() || GetRowByTargetName(pTarget->sName) != nullptr)
    {
        SAL_WARN("sfx.doc", "Repetitive or empty target name in TargetsTable::InsertTarget()");
        pTarget->sName = GetNameProposal();
    }

    OUString sContent = pTarget->sContent;

    if (pTarget->sType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        //selection_num;selection_name
        sContent = sContent.getToken(1, ';');
    }

    OUString sColumnData = pTarget->sName + "\t" + getTypeName(pTarget->sType) + "\t" + sContent
                           + "\t" + (pTarget->bCaseSensitive ? OUString("Yes") : OUString("No"))
                           + "\t" + (pTarget->bWholeWords ? OUString("Yes") : OUString("No"));

    InsertEntryToColumn(sColumnData, TREELIST_APPEND, 0xffff, pTarget);
}

void TargetsTable::InsertTargetAtPos(RedactionTarget* pTarget, const sal_uLong& nPos)
{
    if (!pTarget)
    {
        SAL_WARN("sfx.doc", "pTarget is null in TargetsTable::InsertTarget()");
        return;
    }

    // Check if the name is empty or invalid (clashing with another entry's name)
    if (pTarget->sName.isEmpty() || GetRowByTargetName(pTarget->sName) != nullptr)
    {
        SAL_WARN("sfx.doc", "Repetitive or empty target name in TargetsTable::InsertTarget()");
        pTarget->sName = GetNameProposal();
    }

    OUString sContent = pTarget->sContent;

    if (pTarget->sType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        //selection_num;selection_name
        sContent = sContent.getToken(1, ';');
    }

    OUString sColumnData = pTarget->sName + "\t" + getTypeName(pTarget->sType) + "\t" + sContent
                           + "\t" + (pTarget->bCaseSensitive ? OUString("Yes") : OUString("No"))
                           + "\t" + (pTarget->bWholeWords ? OUString("Yes") : OUString("No"));

    InsertEntryToColumn(sColumnData, nPos, 0xffff, pTarget);
}

RedactionTarget* TargetsTable::GetTargetByName(const OUString& sName)
{
    SvTreeListEntry* pEntry = GetRowByTargetName(sName);
    if (!pEntry)
        return nullptr;

    return static_cast<RedactionTarget*>(pEntry->GetUserData());
}

OUString TargetsTable::GetNameProposal()
{
    //TODO: Define a translatable string
    OUString sDefaultTargetName("Target");
    sal_Int32 nHighestTargetId = 0;
    SvTreeListEntry* pEntry = First();

    while (pEntry)
    {
        RedactionTarget* pTarget = static_cast<RedactionTarget*>(pEntry->GetUserData());
        OUString sName = pTarget->sName;
        sal_Int32 nIndex = 0;
        if (sName.getToken(0, ' ', nIndex) == sDefaultTargetName)
        {
            sal_Int32 nCurrTargetId = sName.getToken(0, ' ', nIndex).toInt32();
            nHighestTargetId = std::max<sal_Int32>(nHighestTargetId, nCurrTargetId);
        }
        pEntry = Next(pEntry);
    }

    return sDefaultTargetName + " " + OUString::number(nHighestTargetId + 1);
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, Load, Button*, void)
{
    //Load a targets list from a previously saved file (a json file?)
    // ask for filename, where we should load the new config data from
    StartFileDialog(StartFileDialogType::Open, "Load Targets");
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, Save, Button*, void)
{
    //Allow saving the targets into a file
    StartFileDialog(StartFileDialogType::SaveAs, "Save Targets");
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, AddHdl, Button*, void)
{
    // Open the Add Target dialog, craete a new target and insert into the targets vector and the listbox
    SfxAddTargetDialog aAddTargetDialog(this, m_pTargetsBox->GetNameProposal());

    bool bIncomplete;
    do
    {
        bIncomplete = false;

        if (aAddTargetDialog.Execute() != RET_OK)
            return;

        if (aAddTargetDialog.getName().isEmpty()
            || aAddTargetDialog.getType() == RedactionTargetType::REDACTION_TARGET_UNKNOWN
            || aAddTargetDialog.getContent().isEmpty())
        {
            bIncomplete = true;
            ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::Ok, "All fields are required")
                ->Execute();
        }
        else if (m_pTargetsBox->GetTargetByName(aAddTargetDialog.getName()))
        {
            bIncomplete = true;
            ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::Ok,
                                             "There is already a target with this name")
                ->Execute();
        }

    } while (bIncomplete);

    //Alright, we now have everything we need to construct a new target
    RedactionTarget* redactiontarget = new RedactionTarget(
        { aAddTargetDialog.getName(), aAddTargetDialog.getType(), aAddTargetDialog.getContent(),
          aAddTargetDialog.isCaseSensitive(), aAddTargetDialog.isWholeWords(), 0 });

    m_pTargetsBox->InsertTarget(redactiontarget);
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, EditHdl, Button*, void)
{
    SvTreeListEntry* pEntry = m_pTargetsBox->FirstSelected();

    // No selection, nothing to edit
    if (pEntry == nullptr)
        return;

    // Only one entry should be selected for editing
    if (m_pTargetsBox->GetSelectionCount() > 1)
    {
        OUString sMsg(
            "You have selected multiple targets, but only one target can be edited at once.");
        //Warn the user about multiple selections
        ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::Ok, sMsg)->Execute();
        return;
    }

    // Get the redaction target to be edited, and its position
    RedactionTarget* pTarget = static_cast<RedactionTarget*>(pEntry->GetUserData());
    sal_uLong nPos = m_pTargetsBox->GetEntryPos(pEntry);

    // Construct and run the edit target dialog
    SfxAddTargetDialog aEditTargetDialog(this, pTarget->sName, pTarget->sType, pTarget->sContent,
                                         pTarget->bCaseSensitive, pTarget->bWholeWords);

    bool bIncomplete;
    do
    {
        bIncomplete = false;

        if (aEditTargetDialog.Execute() != RET_OK)
            return;

        if (aEditTargetDialog.getName().isEmpty()
            || aEditTargetDialog.getType() == RedactionTargetType::REDACTION_TARGET_UNKNOWN
            || aEditTargetDialog.getContent().isEmpty())
        {
            bIncomplete = true;
            ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::Ok, "All fields are required")
                ->Execute();
        }
        else if (aEditTargetDialog.getName() != pTarget->sName
                 && m_pTargetsBox->GetTargetByName(aEditTargetDialog.getName()))
        {
            bIncomplete = true;
            ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::Ok,
                                             "There is already a target with this name")
                ->Execute();
        }

    } while (bIncomplete);

    // Update the redaction target
    pTarget->sName = aEditTargetDialog.getName();
    pTarget->sType = aEditTargetDialog.getType();
    pTarget->sContent = aEditTargetDialog.getContent();
    pTarget->bCaseSensitive = aEditTargetDialog.isCaseSensitive();
    pTarget->bWholeWords = aEditTargetDialog.isWholeWords();

    // And sync the targets box row with the actual target data
    m_pTargetsBox->RemoveEntry(pEntry);
    m_pTargetsBox->InsertTargetAtPos(pTarget, nPos);

    m_pTargetsBox->SelectAll(false);
    m_pTargetsBox->SelectRow(nPos);
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, DeleteHdl, Button*, void)
{
    sal_uLong nSelectionCount = m_pTargetsBox->GetSelectionCount();

    //No selection, so nothing to delete
    if (nSelectionCount < 1)
        return;

    if (nSelectionCount > 1)
    {
        OUString sMsg("Are you sure you would like to delete " + OUString::number(nSelectionCount)
                      + " targets at once?");

        //Warn the user about multiple deletions
        if (ScopedVclPtrInstance<WarningBox>(this, MessBoxStyle::OkCancel, sMsg)->Execute()
            == RET_CANCEL)
            return;
    }

    SvTreeListEntry* pSelected = m_pTargetsBox->FirstSelected();

    for (sal_uLong i = nSelectionCount; i > 0; i--)
    {
        // remove the target
        RedactionTarget* pTarget = static_cast<RedactionTarget*>(pSelected->GetUserData());
        delete pTarget;

        // remove from the table
        SvTreeListEntry* nextSelected = m_pTargetsBox->NextSelected(pSelected);
        m_pTargetsBox->RemoveEntry(pSelected);
        pSelected = nextSelected;
    }

    m_pTargetsBox->SelectAll(false);
}

namespace
{
boost::property_tree::ptree redactionTargetToJSON(RedactionTarget* pTarget)
{
    boost::property_tree::ptree aNode;
    aNode.put("sName", pTarget->sName.toUtf8().getStr());
    aNode.put("eType", pTarget->sType);
    aNode.put("sContent", pTarget->sContent.toUtf8().getStr());
    aNode.put("bWholeWords", pTarget->bWholeWords);
    aNode.put("bCaseSensitive", pTarget->bCaseSensitive);
    aNode.put("nID", pTarget->nID);

    return aNode;
}

RedactionTarget* JSONtoRedactionTarget(const boost::property_tree::ptree::value_type& rValue)
{
    OUString sName = OUString::fromUtf8(rValue.second.get<std::string>("sName").c_str());
    RedactionTargetType eType
        = static_cast<RedactionTargetType>(atoi(rValue.second.get<std::string>("eType").c_str()));
    OUString sContent = OUString::fromUtf8(rValue.second.get<std::string>("sContent").c_str());
    bool bCaseSensitive
        = OUString::fromUtf8(rValue.second.get<std::string>("bCaseSensitive").c_str()).toBoolean();
    bool bWholeWords
        = OUString::fromUtf8(rValue.second.get<std::string>("bWholeWords").c_str()).toBoolean();
    sal_uInt32 nID = atoi(rValue.second.get<std::string>("nID").c_str());

    RedactionTarget* pTarget
        = new RedactionTarget({ sName, eType, sContent, bCaseSensitive, bWholeWords, nID });

    return pTarget;
}
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, LoadHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sTargetsFile;
    if (ERRCODE_NONE == m_pFileDlg->GetError())
        sTargetsFile = m_pFileDlg->GetPath();

    if (sTargetsFile.isEmpty())
        return;

    OUString sSysPath;
    osl::File::getSystemPathFromFileURL(sTargetsFile, sSysPath);
    sTargetsFile = sSysPath;

    EnterWait();

    try
    {
        // Create path string, and read JSON from file
        std::string sPathStr(OUStringToOString(sTargetsFile, RTL_TEXTENCODING_UTF8).getStr());

        boost::property_tree::ptree aTargetsJSON;

        boost::property_tree::read_json(sPathStr, aTargetsJSON);

        // Clear the dialog
        clearTargets();

        // Recreate & add the targets to the dialog
        for (const boost::property_tree::ptree::value_type& rValue :
             aTargetsJSON.get_child("RedactionTargets"))
        {
            RedactionTarget* pTarget = JSONtoRedactionTarget(rValue);
            m_pTargetsBox->InsertTarget(pTarget);
        }
    }
    catch (css::uno::Exception& e)
    {
        SAL_WARN("sfx.doc",
                 "Exception caught while trying to load the targets JSON from file: " << e.Message);
        return;
        //TODO: Warn the user with a message box
    }

    LeaveWait();
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, SaveHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sTargetsFile;
    if (ERRCODE_NONE == m_pFileDlg->GetError())
        sTargetsFile = m_pFileDlg->GetPath();

    if (sTargetsFile.isEmpty())
        return;

    OUString sSysPath;
    osl::File::getSystemPathFromFileURL(sTargetsFile, sSysPath);
    sTargetsFile = sSysPath;

    EnterWait();

    try
    {
        boost::property_tree::ptree aTargetsArray;
        std::vector<std::pair<RedactionTarget*, OUString>> vRedactionTargets;

        // Get the targets
        getTargets(vRedactionTargets);

        // Put the targets into a JSON array
        for (const auto& targetPair : vRedactionTargets)
        {
            aTargetsArray.push_back(std::make_pair("", redactionTargetToJSON(targetPair.first)));
        }

        // Build the JSON tree
        boost::property_tree::ptree aTargetsTree;
        aTargetsTree.add_child("RedactionTargets", aTargetsArray);

        // Create path string, and write JSON to file
        std::string sPathStr(OUStringToOString(sTargetsFile, RTL_TEXTENCODING_UTF8).getStr());

        boost::property_tree::write_json(sPathStr, aTargetsTree);
    }
    catch (css::uno::Exception& e)
    {
        SAL_WARN("sfx.doc",
                 "Exception caught while trying to save the targets JSON to file: " << e.Message);
        return;
        //TODO: Warn the user with a message box
    }

    LeaveWait();
}

void SfxAutoRedactDialog::StartFileDialog(StartFileDialogType nType, const OUString& rTitle)
{
    OUString aFilterAllStr(SfxResId(STR_SFX_FILTERNAME_ALL));
    OUString aFilterCfgStr("Target Set (*.json)");

    bool bSave = nType == StartFileDialogType::SaveAs;
    short nDialogType = bSave ? css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION
                              : css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    m_pFileDlg.reset(new sfx2::FileDialogHelper(nDialogType, FileDialogFlags::NONE, this));

    m_pFileDlg->SetTitle(rTitle);
    m_pFileDlg->AddFilter(aFilterAllStr, FILEDIALOG_FILTER_ALL);
    m_pFileDlg->AddFilter(aFilterCfgStr, "*.json");
    m_pFileDlg->SetCurrentFilter(aFilterCfgStr);

    Link<sfx2::FileDialogHelper*, void> aDlgClosedLink
        = bSave ? LINK(this, SfxAutoRedactDialog, SaveHdl)
                : LINK(this, SfxAutoRedactDialog, LoadHdl);
    m_pFileDlg->StartExecuteModal(aDlgClosedLink);
}

void SfxAutoRedactDialog::clearTargets()
{
    SvTreeListEntry* pEntry = m_pTargetsBox->First();
    while (pEntry)
    {
        RedactionTarget* pTarget = static_cast<RedactionTarget*>(pEntry->GetUserData());
        if (pTarget)
        {
            delete pTarget;
        }
        pEntry = m_pTargetsBox->Next(pEntry);
    }

    m_pTargetsBox->Clear();
}

SfxAutoRedactDialog::SfxAutoRedactDialog(vcl::Window* pParent)
    : SfxModalDialog(pParent, "AutoRedactDialog", "sfx/ui/autoredactdialog.ui")
    , m_bIsValidState(true)
    , m_bTargetsCopied(false)
{
    get(m_pTargetsContainer, "targets");
    get(m_pRedactionTargetsLabel, "labelRedactionTargets");
    get(m_pLoadBtn, "btnLoadTargets");
    get(m_pSaveBtn, "btnSaveTargets");
    get(m_pAddBtn, "add");
    get(m_pEditBtn, "edit");
    get(m_pDeleteBtn, "delete");

    m_pTargetsBox = VclPtr<TargetsTable>::Create(*m_pTargetsContainer);

    m_pLoadBtn->SetClickHdl(LINK(this, SfxAutoRedactDialog, Load));
    m_pSaveBtn->SetClickHdl(LINK(this, SfxAutoRedactDialog, Save));
    m_pDeleteBtn->SetClickHdl(LINK(this, SfxAutoRedactDialog, DeleteHdl));
    m_pAddBtn->SetClickHdl(LINK(this, SfxAutoRedactDialog, AddHdl));
    m_pEditBtn->SetClickHdl(LINK(this, SfxAutoRedactDialog, EditHdl));

    // Can be used to remmeber the last set of redaction targets?
    /*OUString sExtraData;
    SvtViewOptions aDlgOpt(EViewType::Dialog,
                           OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));

    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
        aUserItem >>= sExtraData;
    }

    // update the targets configuration if necessary
    if (!sExtraData.isEmpty())
    {
        weld::WaitObject aWaitCursor(m_xDialog.get());

        try
        {
            // Create path string, and read JSON from file
            boost::property_tree::ptree aTargetsJSON;
            std::stringstream aStream(std::string(sExtraData.toUtf8()));

            boost::property_tree::read_json(aStream, aTargetsJSON);

            // Recreate & add the targets to the dialog
            for (const boost::property_tree::ptree::value_type& rValue :
                 aTargetsJSON.get_child("RedactionTargets"))
            {
                RedactionTarget* pTarget = JSONtoRedactionTarget(rValue);
                addTarget(pTarget);
            }
        }
        catch (css::uno::Exception& e)
        {
            SAL_WARN("sfx.doc",
                     "Exception caught while trying to load the last dialog state: " << e.Message);
            return;
            //TODO: Warn the user with a message box
        }
    }*/
}

SfxAutoRedactDialog::~SfxAutoRedactDialog()
{
    if (!m_bTargetsCopied)
        clearTargets();
    /*if (m_aTableTargets.empty())
    {
        // Clear the dialog data
        SvtViewOptions aDlgOpt(EViewType::Dialog,
                               OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
        aDlgOpt.Delete();
        return;
    }

    try
    {
        // Put the targets into a JSON array
        boost::property_tree::ptree aTargetsArray;
        for (const auto& targetPair : m_aTableTargets)
        {
            aTargetsArray.push_back(std::make_pair("", redactionTargetToJSON(targetPair.first)));
        }

        // Build the JSON tree
        boost::property_tree::ptree aTargetsTree;
        aTargetsTree.add_child("RedactionTargets", aTargetsArray);
        std::stringstream aStream;

        boost::property_tree::write_json(aStream, aTargetsTree, false);

        OUString sUserDataStr(OUString::fromUtf8(aStream.str().c_str()));

        // Store the dialog data
        SvtViewOptions aDlgOpt(EViewType::Dialog,
                               OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
        aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(sUserDataStr));

        if (!m_bTargetsCopied)
            clearTargets();
    }
    catch (css::uno::Exception& e)
    {
        SAL_WARN("sfx.doc",
                 "Exception caught while trying to store the dialog state: " << e.Message);
        return;
        //TODO: Warn the user with a message box
    }*/
    disposeOnce();
}

void SfxAutoRedactDialog::dispose()
{
    m_pTargetsBox.disposeAndClear();
    m_pTargetsContainer.clear();
    m_pRedactionTargetsLabel.clear();
    m_pLoadBtn.clear();
    m_pSaveBtn.clear();
    m_pAddBtn.clear();
    m_pEditBtn.clear();
    m_pDeleteBtn.clear();
    SfxModalDialog::dispose();
}

/*bool SfxAutoRedactDialog::hasTargets() const
{
    //TODO: Add also some validity checks?
    if (m_aTableTargets.empty())
        return false;

    return true;
}*/

bool SfxAutoRedactDialog::getTargets(std::vector<std::pair<RedactionTarget*, OUString>>& r_aTargets)
{
    SvTreeListEntry* pEntry = m_pTargetsBox->First();

    if (!pEntry)
    {
        return false;
    }

    while (pEntry)
    {
        RedactionTarget* pTarget = static_cast<RedactionTarget*>(pEntry->GetUserData());
        if (pTarget && !pTarget->sName.isEmpty())
        {
            r_aTargets.emplace_back(pTarget, pTarget->sName);
            if (!m_bTargetsCopied)
                m_bTargetsCopied = true;
        }
        pEntry = m_pTargetsBox->Next(pEntry);
    }

    return true;
}

IMPL_LINK_NOARG(SfxAddTargetDialog, SelectTypeHdl, ListBox&, void)
{
    if (m_pType->GetSelectedEntryPos() == 2) //Predefined
    {
        // Hide the usual content widgets
        // We will just set the id as content
        // And handle with proper regex in the SfxRedactionHelper
        m_pLabelContent->Disable();
        m_pLabelContent->Hide();
        m_pContent->Disable();
        m_pContent->Hide();
        m_pWholeWords->Disable();
        m_pWholeWords->Hide();
        m_pCaseSensitive->Disable();
        m_pCaseSensitive->Hide();

        // And show the predefined targets
        m_pLabelPredefContent->Enable();
        m_pLabelPredefContent->Show();
        m_pPredefContent->Enable();
        m_pPredefContent->Show();
    }
    else
    {
        m_pLabelPredefContent->Disable();
        m_pLabelPredefContent->Hide();
        m_pPredefContent->Disable();
        m_pPredefContent->Hide();

        m_pLabelContent->Enable();
        m_pLabelContent->Show();
        m_pContent->Enable();
        m_pContent->Show();
        m_pWholeWords->Enable();
        m_pWholeWords->Show();
        m_pCaseSensitive->Enable();
        m_pCaseSensitive->Show();
    }
}

SfxAddTargetDialog::SfxAddTargetDialog(vcl::Window* pParent, const OUString& rName)
    : SfxModalDialog(pParent, "AddTargetDialog", "sfx/ui/addtargetdialog.ui")
{
    get(m_pName, "name");
    get(m_pType, "type");
    get(m_pLabelContent, "label_content");
    get(m_pContent, "content");
    get(m_pLabelPredefContent, "label_content_predef");
    get(m_pPredefContent, "content_predef");
    get(m_pCaseSensitive, "checkboxCaseSensitive");
    get(m_pWholeWords, "checkboxWholeWords");

    m_pName->SetText(rName);
    m_pName->SetSelection(Selection(0, rName.getLength()));

    m_pType->SetSelectHdl(LINK(this, SfxAddTargetDialog, SelectTypeHdl));
}

SfxAddTargetDialog::SfxAddTargetDialog(vcl::Window* pParent, const OUString& sName,
                                       const RedactionTargetType& eTargetType,
                                       const OUString& sContent, const bool& bCaseSensitive,
                                       const bool& bWholeWords)
    : SfxModalDialog(pParent, "AddTargetDialog", "sfx/ui/addtargetdialog.ui")
{
    get(m_pName, "name");
    get(m_pType, "type");
    get(m_pLabelContent, "label_content");
    get(m_pContent, "content");
    get(m_pLabelPredefContent, "label_content_predef");
    get(m_pPredefContent, "content_predef");
    get(m_pCaseSensitive, "checkboxCaseSensitive");
    get(m_pWholeWords, "checkboxWholeWords");

    m_pName->SetText(sName);
    m_pName->SetSelection(Selection(0, sName.getLength()));

    m_pType->SelectEntryPos(getTypeID(eTargetType));

    if (eTargetType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        SelectTypeHdl(*m_pPredefContent);
        m_pPredefContent->SelectEntryPos(sContent.getToken(0, ';').toInt32());
    }
    else
    {
        m_pContent->SetText(sContent);
    }

    if (bCaseSensitive)
        m_pCaseSensitive->SetState(TriState::TRISTATE_TRUE);
    if (bWholeWords)
        m_pWholeWords->SetState(TriState::TRISTATE_TRUE);

    SetText("Edit Target");
}

SfxAddTargetDialog::~SfxAddTargetDialog() { disposeOnce(); }

void SfxAddTargetDialog::dispose()
{
    m_pName.clear();
    m_pType.clear();
    m_pLabelContent.clear();
    m_pContent.clear();
    m_pLabelPredefContent.clear();
    m_pPredefContent.clear();
    m_pCaseSensitive.clear();
    m_pWholeWords.clear();
    SfxModalDialog::dispose();
}

RedactionTargetType SfxAddTargetDialog::getType() const
{
    sal_Int32 nTypeID = m_pType->GetSelectedEntryPos();

    switch (nTypeID)
    {
        case 0:
            return RedactionTargetType::REDACTION_TARGET_TEXT;
        case 1:
            return RedactionTargetType::REDACTION_TARGET_REGEX;
        case 2:
            return RedactionTargetType::REDACTION_TARGET_PREDEFINED;
        default:
            return RedactionTargetType::REDACTION_TARGET_UNKNOWN;
    }
}

OUString SfxAddTargetDialog::getContent() const
{
    if (m_pType->GetSelectedEntryPos() == 2) //Predefined
    {
        return OUString(OUString::number(m_pPredefContent->GetSelectedEntryPos()) + ";"
                        + m_pPredefContent->GetSelectedEntry());
    }

    return m_pContent->GetText();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
