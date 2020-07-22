/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <autoredactdialog.hxx>

#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include <osl/file.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/viewoptions.hxx>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <boost/property_tree/json_parser.hpp>

#define FILEDIALOG_FILTER_JSON "*.json"

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
    OUString sTypeName(SfxResId(STR_REDACTION_TARGET_TYPE_UNKNOWN));

    switch (nType)
    {
        case RedactionTargetType::REDACTION_TARGET_TEXT:
            sTypeName = SfxResId(STR_REDACTION_TARGET_TYPE_TEXT);
            break;
        case RedactionTargetType::REDACTION_TARGET_REGEX:
            sTypeName = SfxResId(STR_REDACTION_TARGET_TYPE_REGEX);
            break;
        case RedactionTargetType::REDACTION_TARGET_PREDEFINED:
            sTypeName = SfxResId(STR_REDACTION_TARGET_TYPE_PREDEF);
            break;
        case RedactionTargetType::REDACTION_TARGET_UNKNOWN:
            sTypeName = SfxResId(STR_REDACTION_TARGET_TYPE_UNKNOWN);
            break;
    }

    return sTypeName;
}

/// Returns TypeID to be used in the add/edit target dialog
OUString getTypeID(RedactionTargetType nType)
{
    OUString sTypeID("unknown");

    switch (nType)
    {
        case RedactionTargetType::REDACTION_TARGET_TEXT:
            sTypeID = "text";
            break;
        case RedactionTargetType::REDACTION_TARGET_REGEX:
            sTypeID = "regex";
            break;
        case RedactionTargetType::REDACTION_TARGET_PREDEFINED:
            sTypeID = "predefined";
            break;
        case RedactionTargetType::REDACTION_TARGET_UNKNOWN:
            sTypeID = "unknown";
            break;
    }

    return sTypeID;
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

    OUString sContent = pTarget->sContent;

    if (pTarget->sType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        //selection_num;selection_name
        sContent = sContent.getToken(1, ';');
    }

    // Add to the end
    int nRow = m_xControl->n_children();
    m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(pTarget)), pTarget->sName);
    m_xControl->set_text(nRow, getTypeName(pTarget->sType), 1);
    m_xControl->set_text(nRow, sContent, 2);
    m_xControl->set_text(
        nRow, pTarget->bCaseSensitive ? SfxResId(STR_REDACTION_YES) : SfxResId(STR_REDACTION_NO),
        3);
    m_xControl->set_text(
        nRow, pTarget->bWholeWords ? SfxResId(STR_REDACTION_YES) : SfxResId(STR_REDACTION_NO), 4);
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

OUString TargetsTable::GetNameProposal() const
{
    OUString sDefaultTargetName(SfxResId(STR_REDACTION_TARGET));
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

void TargetsTable::setRowData(int nRowIndex, const RedactionTarget* pTarget)
{
    OUString sContent = pTarget->sContent;

    if (pTarget->sType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        //selection_num;selection_name
        sContent = sContent.getToken(1, ';');
    }

    m_xControl->set_text(nRowIndex, pTarget->sName, 0);
    m_xControl->set_text(nRowIndex, getTypeName(pTarget->sType), 1);
    m_xControl->set_text(nRowIndex, sContent, 2);
    m_xControl->set_text(
        nRowIndex,
        pTarget->bCaseSensitive ? SfxResId(STR_REDACTION_YES) : SfxResId(STR_REDACTION_NO), 3);
    m_xControl->set_text(
        nRowIndex, pTarget->bWholeWords ? SfxResId(STR_REDACTION_YES) : SfxResId(STR_REDACTION_NO),
        4);
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, Load, weld::Button&, void)
{
    //Load a targets list from a previously saved file (a json file?)
    // ask for filename, where we should load the new config data from
    StartFileDialog(StartFileDialogType::Open, SfxResId(STR_REDACTION_LOAD_TARGETS));
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, Save, weld::Button&, void)
{
    //Allow saving the targets into a file
    StartFileDialog(StartFileDialogType::SaveAs, SfxResId(STR_REDACTION_SAVE_TARGETS));
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, AddHdl, weld::Button&, void)
{
    // Open the Add Target dialog, create a new target and insert into the targets vector and the listbox
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
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
                SfxResId(STR_REDACTION_FIELDS_REQUIRED)));
            xBox->run();
        }
        else if (m_xTargetsBox->GetTargetByName(aAddTargetDialog.getName()))
        {
            bIncomplete = true;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
                SfxResId(STR_REDACTION_TARGET_NAME_CLASH)));
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
            SfxResId(STR_REDACTION_TARGET_ADD_ERROR)));
        xBox->run();
        delete redactiontarget;
    }
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, EditHdl, weld::Button&, void)
{
    sal_Int32 nSelectedRow = m_xTargetsBox->get_selected_index();

    // No selection, nothing to edit
    if (nSelectedRow < 0)
        return;

    // Only one entry should be selected for editing
    if (m_xTargetsBox->get_selected_rows().size() > 1)
    {
        //Warn the user about multiple selections
        std::unique_ptr<weld::MessageDialog> xBox(
            Application::CreateMessageDialog(getDialog(), VclMessageType::Error, VclButtonsType::Ok,
                                             SfxResId(STR_REDACTION_MULTI_EDIT)));
        xBox->run();
        return;
    }

    // Get the redaction target to be edited
    RedactionTarget* pTarget
        = reinterpret_cast<RedactionTarget*>(m_xTargetsBox->get_id(nSelectedRow).toInt64());

    // Construct and run the edit target dialog
    SfxAddTargetDialog aEditTargetDialog(getDialog(), pTarget->sName, pTarget->sType,
                                         pTarget->sContent, pTarget->bCaseSensitive,
                                         pTarget->bWholeWords);

    bool bIncomplete;
    do
    {
        bIncomplete = false;

        if (aEditTargetDialog.run() != RET_OK)
            return;

        if (aEditTargetDialog.getName().isEmpty()
            || aEditTargetDialog.getType() == RedactionTargetType::REDACTION_TARGET_UNKNOWN
            || aEditTargetDialog.getContent().isEmpty())
        {
            bIncomplete = true;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
                SfxResId(STR_REDACTION_FIELDS_REQUIRED)));
            xBox->run();
        }
        else if (aEditTargetDialog.getName() != pTarget->sName
                 && m_xTargetsBox->GetTargetByName(aEditTargetDialog.getName()))
        {
            bIncomplete = true;
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
                SfxResId(STR_REDACTION_TARGET_NAME_CLASH)));
            xBox->run();
        }

    } while (bIncomplete);

    // Update the redaction target
    pTarget->sName = aEditTargetDialog.getName();
    pTarget->sType = aEditTargetDialog.getType();
    pTarget->sContent = aEditTargetDialog.getContent();
    pTarget->bCaseSensitive = aEditTargetDialog.isCaseSensitive();
    pTarget->bWholeWords = aEditTargetDialog.isWholeWords();

    // And sync the targets box row with the actual target data
    m_xTargetsBox->setRowData(nSelectedRow, pTarget);
}

IMPL_LINK_NOARG(SfxAutoRedactDialog, DeleteHdl, weld::Button&, void)
{
    std::vector<int> aSelectedRows = m_xTargetsBox->get_selected_rows();

    //No selection, so nothing to delete
    if (aSelectedRows.empty())
        return;

    if (aSelectedRows.size() > 1)
    {
        OUString sMsg(SfxResId(STR_REDACTION_MULTI_DELETE)
                          .replaceFirst("$(TARGETSCOUNT)", OUString::number(aSelectedRows.size())));
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

namespace
{
boost::property_tree::ptree redactionTargetToJSON(const RedactionTarget* pTarget)
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

    weld::WaitObject aWaitObject(getDialog());

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
            addTarget(pTarget);
        }
    }
    catch (css::uno::Exception& e)
    {
        SAL_WARN("sfx.doc",
                 "Exception caught while trying to load the targets JSON from file: " << e.Message);
        return;
        //TODO: Warn the user with a message box
    }
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

    weld::WaitObject aWaitObject(getDialog());

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
}

void SfxAutoRedactDialog::StartFileDialog(StartFileDialogType nType, const OUString& rTitle)
{
    OUString aFilterAllStr(SfxResId(STR_SFX_FILTERNAME_ALL));
    OUString aFilterJsonStr(SfxResId(STR_REDACTION_JSON_FILE_FILTER));

    bool bSave = nType == StartFileDialogType::SaveAs;
    short nDialogType = bSave ? css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION
                              : css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    m_pFileDlg.reset(new sfx2::FileDialogHelper(nDialogType, FileDialogFlags::NONE, getDialog()));

    m_pFileDlg->SetTitle(rTitle);
    m_pFileDlg->AddFilter(aFilterAllStr, FILEDIALOG_FILTER_ALL);
    m_pFileDlg->AddFilter(aFilterJsonStr, FILEDIALOG_FILTER_JSON);
    m_pFileDlg->SetCurrentFilter(aFilterJsonStr);

    Link<sfx2::FileDialogHelper*, void> aDlgClosedLink
        = bSave ? LINK(this, SfxAutoRedactDialog, SaveHdl)
                : LINK(this, SfxAutoRedactDialog, LoadHdl);
    m_pFileDlg->StartExecuteModal(aDlgClosedLink);
}

void SfxAutoRedactDialog::addTarget(RedactionTarget* pTarget)
{
    // Only the visual/display part
    m_xTargetsBox->InsertTarget(pTarget);

    // Actually add to the targets vector
    if (m_xTargetsBox->GetTargetByName(pTarget->sName))
        m_aTableTargets.emplace_back(pTarget, pTarget->sName);
    else
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            getDialog(), VclMessageType::Warning, VclButtonsType::Ok,
            SfxResId(STR_REDACTION_TARGET_ADD_ERROR)));
        xBox->run();
        delete pTarget;
    }
}

void SfxAutoRedactDialog::clearTargets()
{
    // Clear the targets box
    m_xTargetsBox->clear();

    // Clear the targets vector
    auto delTarget
        = [](const std::pair<RedactionTarget*, OUString>& targetPair) { delete targetPair.first; };

    std::for_each(m_aTableTargets.begin(), m_aTableTargets.end(), delTarget);

    m_aTableTargets.clear();
}

SfxAutoRedactDialog::SfxAutoRedactDialog(weld::Window* pParent)
    : SfxDialogController(pParent, "sfx/ui/autoredactdialog.ui", "AutoRedactDialog")
    , m_bIsValidState(true)
    , m_bTargetsCopied(false)
    , m_xRedactionTargetsLabel(m_xBuilder->weld_label("labelRedactionTargets"))
    , m_xTargetsBox(new TargetsTable(m_xBuilder->weld_tree_view("targets")))
    , m_xLoadBtn(m_xBuilder->weld_button("btnLoadTargets"))
    , m_xSaveBtn(m_xBuilder->weld_button("btnSaveTargets"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xEditBtn(m_xBuilder->weld_button("edit"))
    , m_xDeleteBtn(m_xBuilder->weld_button("delete"))
{
    // Can be used to remember the last set of redaction targets?
    OUString sExtraData;
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
    }

    // Handler connections
    m_xLoadBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, Load));
    m_xSaveBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, Save));
    m_xAddBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, AddHdl));
    m_xEditBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, EditHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SfxAutoRedactDialog, DeleteHdl));
}

SfxAutoRedactDialog::~SfxAutoRedactDialog()
{
    if (m_aTableTargets.empty())
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
    }
}

bool SfxAutoRedactDialog::hasTargets() const
{
    //TODO: Add also some validity checks?
    if (m_aTableTargets.empty())
        return false;

    return true;
}

bool SfxAutoRedactDialog::getTargets(std::vector<std::pair<RedactionTarget*, OUString>>& r_aTargets)
{
    if (m_aTableTargets.empty())
        return true;

    r_aTargets = m_aTableTargets;
    m_bTargetsCopied = true;
    return true;
}

IMPL_LINK_NOARG(SfxAddTargetDialog, SelectTypeHdl, weld::ComboBox&, void)
{
    if (m_xType->get_active_id() == "predefined")
    {
        // Hide the usual content widgets
        // We will just set the id as content
        // And handle with proper regex in the SfxRedactionHelper
        m_xLabelContent->set_sensitive(false);
        m_xLabelContent->set_visible(false);
        m_xContent->set_sensitive(false);
        m_xContent->set_visible(false);
        m_xWholeWords->set_sensitive(false);
        m_xWholeWords->set_visible(false);
        m_xCaseSensitive->set_sensitive(false);
        m_xCaseSensitive->set_visible(false);

        // And show the predefined targets
        m_xLabelPredefContent->set_sensitive(true);
        m_xLabelPredefContent->set_visible(true);
        m_xPredefContent->set_sensitive(true);
        m_xPredefContent->set_visible(true);
    }
    else
    {
        m_xLabelPredefContent->set_sensitive(false);
        m_xLabelPredefContent->set_visible(false);
        m_xPredefContent->set_sensitive(false);
        m_xPredefContent->set_visible(false);

        m_xLabelContent->set_sensitive(true);
        m_xLabelContent->set_visible(true);
        m_xContent->set_sensitive(true);
        m_xContent->set_visible(true);
        m_xWholeWords->set_sensitive(true);
        m_xWholeWords->set_visible(true);
        m_xCaseSensitive->set_sensitive(true);
        m_xCaseSensitive->set_visible(true);
    }
}

SfxAddTargetDialog::SfxAddTargetDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, "sfx/ui/addtargetdialog.ui", "AddTargetDialog")
    , m_xName(m_xBuilder->weld_entry("name"))
    , m_xType(m_xBuilder->weld_combo_box("type"))
    , m_xLabelContent(m_xBuilder->weld_label("label_content"))
    , m_xContent(m_xBuilder->weld_entry("content"))
    , m_xLabelPredefContent(m_xBuilder->weld_label("label_content_predef"))
    , m_xPredefContent(m_xBuilder->weld_combo_box("content_predef"))
    , m_xCaseSensitive(m_xBuilder->weld_check_button("checkboxCaseSensitive"))
    , m_xWholeWords(m_xBuilder->weld_check_button("checkboxWholeWords"))
{
    m_xName->set_text(rName);
    m_xName->select_region(0, rName.getLength());

    m_xType->connect_changed(LINK(this, SfxAddTargetDialog, SelectTypeHdl));
}

SfxAddTargetDialog::SfxAddTargetDialog(weld::Window* pParent, const OUString& sName,
                                       const RedactionTargetType& eTargetType,
                                       const OUString& sContent, bool bCaseSensitive,
                                       bool bWholeWords)
    : GenericDialogController(pParent, "sfx/ui/addtargetdialog.ui", "AddTargetDialog")
    , m_xName(m_xBuilder->weld_entry("name"))
    , m_xType(m_xBuilder->weld_combo_box("type"))
    , m_xLabelContent(m_xBuilder->weld_label("label_content"))
    , m_xContent(m_xBuilder->weld_entry("content"))
    , m_xLabelPredefContent(m_xBuilder->weld_label("label_content_predef"))
    , m_xPredefContent(m_xBuilder->weld_combo_box("content_predef"))
    , m_xCaseSensitive(m_xBuilder->weld_check_button("checkboxCaseSensitive"))
    , m_xWholeWords(m_xBuilder->weld_check_button("checkboxWholeWords"))
{
    m_xName->set_text(sName);
    m_xName->select_region(0, sName.getLength());

    m_xType->set_active_id(getTypeID(eTargetType));
    m_xType->connect_changed(LINK(this, SfxAddTargetDialog, SelectTypeHdl));

    if (eTargetType == RedactionTargetType::REDACTION_TARGET_PREDEFINED)
    {
        SelectTypeHdl(*m_xPredefContent);
        m_xPredefContent->set_active(sContent.getToken(0, ';').toInt32());
    }
    else
    {
        m_xContent->set_text(sContent);
    }

    m_xCaseSensitive->set_active(bCaseSensitive);
    m_xWholeWords->set_active(bWholeWords);

    set_title(SfxResId(STR_REDACTION_EDIT_TARGET));
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

OUString SfxAddTargetDialog::getContent() const
{
    if (m_xType->get_active_id() == "predefined")
    {
        return OUString(OUString::number(m_xPredefContent->get_active()) + ";"
                        + m_xPredefContent->get_active_text());
    }

    return m_xContent->get_text();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
