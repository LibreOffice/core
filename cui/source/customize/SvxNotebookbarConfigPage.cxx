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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <helpids.h>
#include <strings.hrc>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <SvxNotebookbarConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include <dialmgr.hxx>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <osl/file.hxx>
#include <CustomNotebookbarGenerator.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/theUICommandDescription.hpp>

namespace uno = com::sun::star::uno;
namespace frame = com::sun::star::frame;
namespace lang = com::sun::star::lang;
namespace container = com::sun::star::container;
namespace beans = com::sun::star::beans;
namespace graphic = com::sun::star::graphic;

static bool isCategoryAvailable(OUString& sClassId, OUString& sUIItemId, OUString& sActiveCategory,
                                bool& isCategory)
{
    if (sUIItemId == sActiveCategory)
        return true;
    else if ((sClassId == "GtkMenu" || sClassId == "GtkGrid") && sUIItemId != sActiveCategory)
    {
        isCategory = false;
        return false;
    }
    return false;
}

static OUString charToString(const char* cString)
{
    OUStringBuffer aString;
    for (unsigned long i = 0; i < strlen(cString); i++)
    {
        aString.append(cString[i]);
    }
    return aString.makeStringAndClear();
}

static OUString getFileName(const OUString& aFileName)
{
    if (aFileName == "notebookbar.ui")
        return "Tabbed";
    else if (aFileName == "notebookbar_compact.ui")
        return "TabbedCompact";
    else if (aFileName == "notebookbar_groupedbar_full.ui")
        return "Groupedbar";
    else if (aFileName == "notebookbar_groupedbar_compact.ui")
        return "GroupedbarCompact";
    else
        return "None";
}

static OUString getModuleId(const OUString& sModuleName)
{
    if (sModuleName == "Writer")
        return OUString("com.sun.star.text.TextDocument");
    else if (sModuleName == "Draw")
        return OUString("com.sun.star.drawing.DrawingDocument");
    else if (sModuleName == "Impress")
        return OUString("com.sun.star.presentation.PresentationDocument");
    else if (sModuleName == "Calc")
        return OUString("com.sun.star.sheet.SpreadsheetDocument");
    else
        return OUString("None");
}

SvxNotebookbarConfigPage::SvxNotebookbarConfigPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SvxConfigPage(pParent, rSet)
{
    m_xDescriptionFieldLb->set_visible(false);
    m_xSearchEdit->set_visible(false);
    m_xDescriptionField->set_visible(false);
    m_xMoveUpButton->set_visible(false);
    m_xMoveDownButton->set_visible(false);
    m_xAddCommandButton->set_visible(false);
    m_xRemoveCommandButton->set_visible(false);
    m_xLeftFunctionLabel->set_visible(false);
    m_xSearchLabel->set_visible(false);
    m_xCategoryLabel->set_visible(false);
    m_xCategoryListBox->set_visible(false);
    m_xInsertBtn->set_visible(false);
    m_xModifyBtn->set_visible(false);
    m_xResetBtn->set_visible(false);
    m_xCustomizeLabel->set_visible(false);

    weld::TreeView& rCommandCategoryBox = m_xFunctions->get_widget();
    rCommandCategoryBox.hide();

    m_xContentsListBox.reset(
        new SvxNotebookbarEntriesListBox(m_xBuilder->weld_tree_view("toolcontents"), this));
    std::vector<int> aWidths;
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    Size aSize(m_xFunctions->get_size_request());
    rTreeView.set_size_request(aSize.Width(), aSize.Height());

    int nExpectedSize = 16;

    int nStandardImageColWidth = rTreeView.get_checkbox_column_width();
    int nMargin = nStandardImageColWidth - nExpectedSize;
    if (nMargin < 16)
        nMargin = 16;

    if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_LARGE)
        nExpectedSize = 24;
    else if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_32)
        nExpectedSize = 32;

    int nImageColWidth = nExpectedSize + nMargin;

    aWidths.push_back(nStandardImageColWidth);
    aWidths.push_back(nImageColWidth);
    rTreeView.set_column_fixed_widths(aWidths);

    rTreeView.set_hexpand(true);
    rTreeView.set_vexpand(true);
    rTreeView.set_help_id(HID_SVX_CONFIG_NOTEBOOKBAR_CONTENTS);
    rTreeView.show();
}

SvxNotebookbarConfigPage::~SvxNotebookbarConfigPage() { disposeOnce(); }

void SvxNotebookbarConfigPage::DeleteSelectedTopLevel() {}

void SvxNotebookbarConfigPage::DeleteSelectedContent() {}

void SvxNotebookbarConfigPage::Init()
{
    m_xTopLevelListBox->clear();
    m_xContentsListBox->clear();
    m_xSaveInListBox->clear();
    CustomNotebookbarGenerator::createCustomizedUIFile();
    OUString sAppName, sFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sFileName);
    OUString sNotebookbarInterface = getFileName(sFileName);

    OUString sScopeName
        = utl::ConfigManager::getProductName() + " " + sAppName + " -  " + sNotebookbarInterface;
    OUString sSaveInListBoxID = notebookbarTabScope;

    m_xSaveInListBox->append(sSaveInListBoxID, sScopeName);
    m_xSaveInListBox->set_active_id(sSaveInListBoxID);

    m_xTopLevelListBox->append("NotebookBar", "All Commands");
    m_xTopLevelListBox->set_active_id("NotebookBar");
    SelectElement();
}

SaveInData* SvxNotebookbarConfigPage::CreateSaveInData(
    const css::uno::Reference<css::ui::XUIConfigurationManager>& xCfgMgr,
    const css::uno::Reference<css::ui::XUIConfigurationManager>& xParentCfgMgr,
    const OUString& aModuleId, bool bDocConfig)
{
    return static_cast<SaveInData*>(
        new ToolbarSaveInData(xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig));
}

void SvxNotebookbarConfigPage::UpdateButtonStates() {}

short SvxNotebookbarConfigPage::QueryReset()
{
    OUString msg = CuiResId(RID_SVXSTR_CONFIRM_TOOLBAR_RESET);

    OUString saveInName = m_xSaveInListBox->get_active_text();

    OUString label = SvxConfigPageHelper::replaceSaveInName(msg, saveInName);

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
        GetDialogFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo, label));
    int nValue = xQueryBox->run();
    if (nValue == RET_YES)
    {
        OUString sOriginalUIPath = CustomNotebookbarGenerator::getOriginalUIPath();
        OUString sCustomizedUIPath = CustomNotebookbarGenerator::getCustomizedUIPath();
        osl::File::copy(sOriginalUIPath, sCustomizedUIPath);
        OUString sAppName, sFileName;
        CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sFileName);
        OUString sNotebookbarInterface = getFileName(sFileName);
        Sequence<OUString> sSequenceEntries;
        CustomNotebookbarGenerator::setCustomizedUIItem(sSequenceEntries, sNotebookbarInterface);
        OUString sUIPath = "modules/s" + sAppName.toAsciiLowerCase() + "/ui/";
        sfx2::SfxNotebookBar::ReloadNotebookBar(sUIPath);
    }
    return nValue;
}

void SvxConfigPage::InsertEntryIntoNotebookbarTabUI(OUString& sClassId, OUString& sUIItemId,
                                                    OUString& sUIItemCommand, int nPos,
                                                    int nStartCol)
{
    OUString sAppName, sFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sFileName);

    css::uno::Reference<css::container::XNameAccess> m_xCommandToLabelMap,
        m_xGlobalCommandToLabelMap;
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<container::XNameAccess> xNameAccess(
        css::frame::theUICommandDescription::get(xContext));

    uno::Sequence<beans::PropertyValue> aPropSeq, aGlobalPropSeq;

    xNameAccess->getByName("com.sun.star.text.GlobalDocument") >>= m_xGlobalCommandToLabelMap;
    xNameAccess->getByName(getModuleId(sAppName)) >>= m_xCommandToLabelMap;

    try
    {
        uno::Any aModuleVal = m_xCommandToLabelMap->getByName(sUIItemCommand);

        aModuleVal >>= aPropSeq;
    }
    catch (container::NoSuchElementException&)
    {
    }

    try
    {
        uno::Any aGlobalVal = m_xGlobalCommandToLabelMap->getByName(sUIItemCommand);
        aGlobalVal >>= aGlobalPropSeq;
    }
    catch (container::NoSuchElementException&)
    {
    }

    OUString aLabel;
    for (sal_Int32 i = 0; i < aPropSeq.getLength(); ++i)
        if (aPropSeq[i].Name == "Name")
            aPropSeq[i].Value >>= aLabel;
    if (aLabel.isEmpty())
        for (sal_Int32 i = 0; i < aGlobalPropSeq.getLength(); ++i)
            if (aGlobalPropSeq[i].Name == "Name")
                aGlobalPropSeq[i].Value >>= aLabel;

    OUString aName = SvxConfigPageHelper::stripHotKey(aLabel);

    if (sClassId == "GtkSeparatorMenuItem" || sClassId == "GtkSeparator")
    {
        OUString sDataInTree = "--------------------------------------------";
        m_xContentsListBox->set_text(nPos, sDataInTree, nStartCol + 1);
    }
    else
    {
        if (aName.isEmpty())
            aName = sUIItemId;
        auto xImage = GetSaveInData()->GetImage(sUIItemCommand);
        if (xImage.is())
            m_xContentsListBox->set_image(nPos, xImage, nStartCol);
        m_xContentsListBox->set_text(nPos, aName, nStartCol + 1);
        m_xContentsListBox->set_id(nPos, sUIItemId);
    }
}

void SvxNotebookbarConfigPage::getNodeValue(xmlNode* pNodePtr, NotebookbarEntries& aNodeEntries)
{
    pNodePtr = pNodePtr->xmlChildrenNode;
    while (pNodePtr)
    {
        if (!(xmlStrcmp(pNodePtr->name, reinterpret_cast<const xmlChar*>("property"))))
        {
            xmlChar* UriValue = xmlGetProp(pNodePtr, reinterpret_cast<const xmlChar*>("name"));
            if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>("visible"))))
            {
                xmlChar* aValue = xmlNodeGetContent(pNodePtr);
                const char* cVisibleValue = reinterpret_cast<const char*>(aValue);
                OUString sVisibleValue = charToString(cVisibleValue);
                aNodeEntries.sVisibleValue = sVisibleValue;
                xmlFree(aValue);
            }
            if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>("action_name"))))
            {
                xmlChar* aValue = xmlNodeGetContent(pNodePtr);
                const char* cActionName = reinterpret_cast<const char*>(aValue);
                OUString sActionName = charToString(cActionName);
                aNodeEntries.sActionName = sActionName;
                xmlFree(aValue);
            }
            xmlFree(UriValue);
        }
        pNodePtr = pNodePtr->next;
    }
}

void SvxNotebookbarConfigPage::searchNodeandAttribute(std::vector<NotebookbarEntries>& aEntries,
                                                      std::vector<CategoriesEntries>& aCategoryList,
                                                      OUString& sActiveCategory,
                                                      CategoriesEntries& aCurItemEntry,
                                                      xmlNode* pNodePtr, bool isCategory)
{
    pNodePtr = pNodePtr->xmlChildrenNode;
    while (pNodePtr)
    {
        if (pNodePtr->type == XML_ELEMENT_NODE)
        {
            const char* cNodeName = reinterpret_cast<const char*>(pNodePtr->name);
            OUString sNodeName = charToString(cNodeName);
            OUString sSecondVal;
            if (sNodeName == "object")
            {
                xmlChar* UriValue = xmlGetProp(pNodePtr, reinterpret_cast<const xmlChar*>("id"));
                const char* cUIItemID = reinterpret_cast<const char*>(UriValue);
                OUString sUIItemId = charToString(cUIItemID);
                xmlFree(UriValue);

                UriValue = xmlGetProp(pNodePtr, reinterpret_cast<const xmlChar*>("class"));
                const char* cClassId = reinterpret_cast<const char*>(UriValue);
                OUString sClassId = charToString(cClassId);
                xmlFree(UriValue);

                CategoriesEntries aCategoryEntry;
                if (sClassId == "sfxlo-PriorityHBox")
                {
                    aCategoryEntry.sDisplayName = sUIItemId;
                    aCategoryEntry.sUIItemId = sUIItemId;
                    aCategoryEntry.sClassType = sClassId;
                    aCategoryList.push_back(aCategoryEntry);

                    aCurItemEntry = aCategoryEntry;
                }
                else if (sClassId == "sfxlo-PriorityMergedHBox")
                {
                    aCategoryEntry.sDisplayName = aCurItemEntry.sDisplayName + " | " + sUIItemId;
                    aCategoryEntry.sUIItemId = sUIItemId;
                    aCategoryEntry.sClassType = sClassId;

                    if (aCurItemEntry.sClassType == sClassId)
                    {
                        sal_Int32 rPos = 0;
                        aCategoryEntry.sDisplayName
                            = aCurItemEntry.sDisplayName.getToken(rPos, ' ', rPos) + " | "
                              + sUIItemId;
                    }
                    aCategoryList.push_back(aCategoryEntry);
                    aCurItemEntry = aCategoryEntry;
                }
                else if (sClassId == "svtlo-ManagedMenuButton")
                {
                    sal_Int32 rPos = 1;
                    sSecondVal = sUIItemId.getToken(rPos, ':', rPos);
                    if (!sSecondVal.isEmpty())
                    {
                        aCategoryEntry.sDisplayName
                            = aCurItemEntry.sDisplayName + " | " + sSecondVal;
                        aCategoryEntry.sUIItemId = sSecondVal;
                        aCategoryList.push_back(aCategoryEntry);
                    }
                }

                NotebookbarEntries nodeEntries;
                if (isCategoryAvailable(sClassId, sUIItemId, sActiveCategory, isCategory)
                    || isCategory)
                {
                    isCategory = true;
                    if (sClassId == "GtkMenuItem" || sClassId == "GtkToolButton"
                        || sClassId == "GtkMenuToolButton"
                        || (sClassId == "svtlo-ManagedMenuButton" && sSecondVal.isEmpty()))
                    {
                        nodeEntries.sClassId = sClassId;
                        nodeEntries.sUIItemId = sUIItemId;
                        nodeEntries.sDisplayName = sUIItemId;

                        getNodeValue(pNodePtr, nodeEntries);
                        aEntries.push_back(nodeEntries);
                    }
                    else if (sClassId == "GtkSeparatorMenuItem" || sClassId == "GtkSeparator")
                    {
                        nodeEntries.sClassId = sClassId;
                        nodeEntries.sUIItemId = sUIItemId;
                        nodeEntries.sDisplayName = "Null";
                        nodeEntries.sVisibleValue = "Null";
                        nodeEntries.sActionName = "Null";
                        aEntries.push_back(nodeEntries);
                    }
                    else if (sClassId == "sfxlo-PriorityHBox"
                             || sClassId == "sfxlo-PriorityMergedHBox"
                             || sClassId == "svtlo-ManagedMenuButton")
                    {
                        nodeEntries.sClassId = sClassId;
                        nodeEntries.sUIItemId = sUIItemId;
                        nodeEntries.sDisplayName
                            = aCategoryList[aCategoryList.size() - 1].sDisplayName;
                        nodeEntries.sVisibleValue = "Null";
                        nodeEntries.sActionName = "Null";
                        aEntries.push_back(nodeEntries);
                    }
                }
            }
            searchNodeandAttribute(aEntries, aCategoryList, sActiveCategory, aCurItemEntry,
                                   pNodePtr, isCategory);
        }
        pNodePtr = pNodePtr->next;
    }
}

void SvxNotebookbarConfigPage::FillFunctionsList(std::vector<NotebookbarEntries>& aEntries,
                                                 std::vector<CategoriesEntries>& aCategoryList,
                                                 OUString& sActiveCategory)
{
    xmlDocPtr pDoc;
    xmlNodePtr pNodePtr;
    OUString sUIFilePath = CustomNotebookbarGenerator::getCustomizedUIPath();
    char* cUIFileUIPath = CustomNotebookbarGenerator::convertToCharPointer(sUIFilePath);
    pDoc = xmlParseFile(cUIFileUIPath);
    pNodePtr = xmlDocGetRootElement(pDoc);

    CategoriesEntries aCurItemEntry;
    searchNodeandAttribute(aEntries, aCategoryList, sActiveCategory, aCurItemEntry, pNodePtr,
                           false);
    if (pDoc != nullptr)
    {
        xmlFreeDoc(pDoc);
    }
    delete[] cUIFileUIPath;
}

void SvxNotebookbarConfigPage::SelectElement()
{
    m_xContentsListBox->clear();
    std::vector<NotebookbarEntries> aEntries;
    std::vector<CategoriesEntries> aCategoryList;
    OUString sActiveCategory = m_xTopLevelListBox->get_active_id();
    FillFunctionsList(aEntries, aCategoryList, sActiveCategory);

    if (m_xTopLevelListBox->get_count() == 1)
    {
        for (unsigned long nIdx = 0; nIdx < aCategoryList.size(); nIdx++)
            m_xTopLevelListBox->append(aCategoryList[nIdx].sUIItemId,
                                       aCategoryList[nIdx].sDisplayName);
    }
    unsigned long nStart = 0;
    if (aEntries[nStart].sClassId == "sfxlo-PriorityHBox"
        || aEntries[nStart].sClassId == "sfxlo-PriorityMergedHBox")
        nStart = 1;

    std::vector<NotebookbarEntries> aTempEntries;
    for (unsigned long nIdx = nStart; nIdx < aEntries.size(); nIdx++)
    {
        if (aEntries[nIdx].sClassId == "svtlo-ManagedMenuButton")
        {
            aTempEntries.push_back(aEntries[nIdx]);
            std::vector<NotebookbarEntries> aGtkEntries;
            sal_Int32 rPos = 1;
            sActiveCategory = aEntries[nIdx].sUIItemId.getToken(rPos, ':', rPos);
            FillFunctionsList(aGtkEntries, aCategoryList, sActiveCategory);
            for (unsigned long Idx = 0; Idx < aGtkEntries.size(); Idx++)
                aTempEntries.push_back(aGtkEntries[Idx]);
            aGtkEntries.clear();
        }
        else
            aTempEntries.push_back(aEntries[nIdx]);
    }

    aEntries = aTempEntries;
    aTempEntries.clear();

    sal_Int64 nId = 0;
    for (unsigned long nIdx = 0; nIdx < aEntries.size(); nIdx++)
    {
        OUString sId(OUString::number(nId));
        m_xContentsListBox->insert(nIdx, sId);
        if (aEntries[nIdx].sActionName != "Null")
        {
            if (aEntries[nIdx].sVisibleValue == "True")
            {
                m_xContentsListBox->set_toggle(nIdx, TRISTATE_TRUE, 0);
            }
            else
            {
                m_xContentsListBox->set_toggle(nIdx, TRISTATE_FALSE, 0);
            }
        }
        InsertEntryIntoNotebookbarTabUI(aEntries[nIdx].sClassId, aEntries[nIdx].sDisplayName,
                                        aEntries[nIdx].sActionName, nIdx, 1);
        ++nId;
    }
    aEntries.clear();
}

SvxNotebookbarEntriesListBox::SvxNotebookbarEntriesListBox(std::unique_ptr<weld::TreeView> xParent,
                                                           SvxConfigPage* pPg)
    : SvxMenuEntriesListBox(std::move(xParent), pPg)
{
    m_xControl->connect_toggled(LINK(this, SvxNotebookbarEntriesListBox, CheckButtonHdl));
    m_xControl->connect_key_press(Link<const KeyEvent&, bool>());
    m_xControl->connect_key_press(LINK(this, SvxNotebookbarEntriesListBox, KeyInputHdl));
}

SvxNotebookbarEntriesListBox::~SvxNotebookbarEntriesListBox() {}

static void EditRegistryFile(OUString& sUIItemId, OUString& sSetEntry,
                             OUString& sNotebookbarInterface)
{
    int nFlag = 0;
    Sequence<OUString> aOldEntries
        = CustomNotebookbarGenerator::getCustomizedUIItem(sNotebookbarInterface);
    Sequence<OUString> aNewEntries(aOldEntries.getLength() + 1);
    for (int nIdx = 0; nIdx < aOldEntries.getLength(); nIdx++)
    {
        sal_Int32 rPos = 0;
        OUString sFirstValue = aOldEntries[nIdx].getToken(rPos, ',', rPos);
        if (sFirstValue == sUIItemId)
        {
            aOldEntries[nIdx] = sSetEntry;
            nFlag = 1;
            break;
        }
        aNewEntries[nIdx] = aOldEntries[nIdx];
    }

    if (nFlag == 0)
    {
        aNewEntries[aOldEntries.getLength()] = sSetEntry;
        CustomNotebookbarGenerator::setCustomizedUIItem(aNewEntries, sNotebookbarInterface);
    }
    else
    {
        CustomNotebookbarGenerator::setCustomizedUIItem(aOldEntries, sNotebookbarInterface);
    }
}

void SvxNotebookbarEntriesListBox::ChangedVisibility(int nRow)
{
    OUString sUIItemId = m_xControl->get_selected_id();
    OUString sAppName;
    OUString sFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sFileName);
    OUString sNotebookbarInterface = getFileName(sFileName);

    OUString sVisible;
    if (m_xControl->get_toggle(nRow, 0) == TRISTATE_TRUE)
        sVisible = "True";
    else
        sVisible = "False";
    OUString sSetEntries = sUIItemId + ",visible," + sVisible;
    Sequence<OUString> sSeqOfEntries(1);
    sSeqOfEntries[0] = sSetEntries;
    EditRegistryFile(sUIItemId, sSetEntries, sNotebookbarInterface);
    CustomNotebookbarGenerator::modifyCustomizedUIFile(sSeqOfEntries);
    OUString sUIPath = "modules/s" + sAppName.toAsciiLowerCase() + "/ui/";
    sfx2::SfxNotebookBar::ReloadNotebookBar(sUIPath);
}

IMPL_LINK(SvxNotebookbarEntriesListBox, CheckButtonHdl, const row_col&, rRowCol, void)
{
    ChangedVisibility(rRowCol.first);
}

IMPL_LINK(SvxNotebookbarEntriesListBox, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    if (rKeyEvent.GetKeyCode() == KEY_SPACE)
    {
        int nRow = m_xControl->get_selected_index();
        m_xControl->set_toggle(
            nRow, m_xControl->get_toggle(nRow, 0) == TRISTATE_TRUE ? TRISTATE_FALSE : TRISTATE_TRUE,
            0);
        ChangedVisibility(nRow);
        return true;
    }
    return SvxMenuEntriesListBox::KeyInputHdl(rKeyEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
