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

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/treelistentry.hxx>
#include <osl/diagnose.h>

#include <swtypes.hxx>
#include <glosbib.hxx>
#include <gloshdl.hxx>
#include <actctrl.hxx>
#include <glossary.hxx>
#include <glosdoc.hxx>
#include <swunohelper.hxx>

#include <strings.hrc>

#define PATH_CASE_SENSITIVE 0x01
#define PATH_READONLY       0x02

#define RENAME_TOKEN_DELIM      u'\x0001'

SwGlossaryGroupDlg::SwGlossaryGroupDlg(weld::Window * pParent,
    std::vector<OUString> const& rPathArr, SwGlossaryHdl *pHdl)
    : SfxDialogController(pParent, "modules/swriter/ui/editcategories.ui",
                          "EditCategoriesDialog")
    , m_pParent(pParent)
    , pGlosHdl(pHdl)
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xPathLB(m_xBuilder->weld_combo_box("pathlb"))
    , m_xGroupTLB(m_xBuilder->weld_tree_view("group"))
    , m_xNewPB(m_xBuilder->weld_button("new"))
    , m_xDelPB(m_xBuilder->weld_button("delete"))
    , m_xRenamePB(m_xBuilder->weld_button("rename"))
{
    int nWidth = m_xGroupTLB->get_approximate_digit_width() * 34;
    m_xPathLB->set_size_request(nWidth, -1);
    //just has to be something small, real size will be available space
    m_xGroupTLB->set_size_request(nWidth, m_xGroupTLB->get_height_rows(10));

    std::vector<int> aWidths;
    aWidths.push_back(nWidth);
    m_xGroupTLB->set_column_fixed_widths(aWidths);
    m_xGroupTLB->connect_changed(LINK(this, SwGlossaryGroupDlg, SelectHdl));

    m_xNewPB->connect_clicked(LINK(this, SwGlossaryGroupDlg, NewHdl));
    m_xDelPB->connect_clicked(LINK(this, SwGlossaryGroupDlg, DeleteHdl));
    m_xNameED->connect_changed(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
    m_xNameED->connect_insert_text(LINK(this, SwGlossaryGroupDlg, EditInsertTextHdl));
    m_xPathLB->connect_changed(LINK(this, SwGlossaryGroupDlg, ModifyListBoxHdl));
    m_xRenamePB->connect_clicked(LINK(this, SwGlossaryGroupDlg, RenameHdl));

    m_xNameED->connect_size_allocate(LINK(this, SwGlossaryGroupDlg, EntrySizeAllocHdl));
    m_xPathLB->connect_size_allocate(LINK(this, SwGlossaryGroupDlg, EntrySizeAllocHdl));

    for (size_t i = 0; i < rPathArr.size(); ++i)
    {
        INetURLObject aTempURL(rPathArr[i]);
        const OUString sPath = aTempURL.GetMainURL(INetURLObject::DecodeMechanism::WithCharset );
        sal_uInt32 nCaseReadonly = 0;
        utl::TempFile aTempFile(&sPath);
        aTempFile.EnableKillingFile();
        if(!aTempFile.IsValid())
            nCaseReadonly |= PATH_READONLY;
        else if( SWUnoHelper::UCB_IsCaseSensitiveFileName( aTempFile.GetURL()))
            nCaseReadonly |= PATH_CASE_SENSITIVE;
        m_xPathLB->append(OUString::number(nCaseReadonly), sPath);
    }
    m_xPathLB->set_active(0);
    m_xPathLB->set_sensitive(true);

    const size_t nCount = pHdl->GetGroupCnt();
    /* tdf#111870 "My AutoText" comes from mytexts.bau but should be translated
       here as well, see also SwGlossaryDlg::Init */
    const OUString sMyAutoTextEnglish("My AutoText");
    for( size_t i = 0; i < nCount; ++i)
    {
        OUString sTitle;
        OUString sGroup = pHdl->GetGroupName(i, &sTitle);
        if(sGroup.isEmpty())
            continue;
        GlosBibUserData* pData = new GlosBibUserData;
        pData->sGroupName = sGroup;
        if ( sTitle == sMyAutoTextEnglish )
            pData->sGroupTitle = SwResId(STR_MY_AUTOTEXT);
        else
            pData->sGroupTitle = sTitle;
        pData->sPath = m_xPathLB->get_text(sGroup.getToken(1, GLOS_DELIM).toInt32());
        const OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData)));
        m_xGroupTLB->append(sId, pData->sGroupTitle);
        int nEntry = m_xGroupTLB->find_id(sId);
        m_xGroupTLB->set_text(nEntry, pData->sPath, 1);

    }
    m_xGroupTLB->make_sorted();
}

SwGlossaryGroupDlg::~SwGlossaryGroupDlg()
{
    int nCount = m_xGroupTLB->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(i).toInt64());
        delete pUserData;
    }
}

short SwGlossaryGroupDlg::run()
{
    short nRet = SfxDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

void SwGlossaryGroupDlg::Apply()
{
    if (m_xNewPB->get_sensitive())
        NewHdl(*m_xNewPB);

    const OUString aActGroup = SwGlossaryDlg::GetCurrGroup();

    for (const auto& removedStr : m_RemovedArr)
    {
        sal_Int32 nIdx{ 0 };
        const OUString sDelGroup = removedStr.getToken(0, '\t', nIdx);
        if( sDelGroup == aActGroup )
        {
            //when the current group is deleted, the current group has to be relocated
            if (m_xGroupTLB->n_children())
            {
                GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(0).toInt64());
                pGlosHdl->SetCurGroup(pUserData->sGroupName);
            }
        }
        const OUString sMsg(SwResId(STR_QUERY_DELETE_GROUP1)
                            + removedStr.getToken(0, '\t', nIdx)
                            + SwResId(STR_QUERY_DELETE_GROUP2));

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_pParent,
                                                       VclMessageType::Question, VclButtonsType::YesNo, sMsg));
        xQueryBox->set_default_response(RET_NO);
        if (RET_YES == xQueryBox->run())
            pGlosHdl->DelGroup( sDelGroup );
    }

    //don't rename before there was one
    for (auto it(m_RenamedArr.cbegin()); it != m_RenamedArr.cend(); ++it)
    {
        sal_Int32 nIdx{ 0 };
        OUString const sOld(it->getToken(0, RENAME_TOKEN_DELIM, nIdx));
        OUString sNew(it->getToken(0, RENAME_TOKEN_DELIM, nIdx));
        OUString const sTitle(it->getToken(0, RENAME_TOKEN_DELIM, nIdx));
        pGlosHdl->RenameGroup(sOld, sNew, sTitle);
        if (it == m_RenamedArr.begin())
        {
            sCreatedGroup = sNew;
        }
    }
    for (auto& sNewGroup : m_InsertedArr)
    {
        OUString sNewTitle = sNewGroup.getToken(0, GLOS_DELIM);
        if( sNewGroup != aActGroup )
        {
            pGlosHdl->NewGroup(sNewGroup, sNewTitle);
            if(sCreatedGroup.isEmpty())
                sCreatedGroup = sNewGroup;
        }
    }
}

IMPL_LINK_NOARG( SwGlossaryGroupDlg, SelectHdl, weld::TreeView&, void )
{
    m_xNewPB->set_sensitive(false);
    int nFirstEntry = m_xGroupTLB->get_selected_index();
    if (nFirstEntry != -1)
    {
        GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(nFirstEntry).toInt64());
        const OUString sEntry(pUserData->sGroupName);
        const OUString sName(m_xNameED->get_text());
        bool bExists = false;
        int nPos = m_xGroupTLB->find_text(sName);
        if (nPos != -1)
        {
            GlosBibUserData* pFoundData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(nPos).toInt64());
            fprintf(stderr, "comparing %s and %s\n",
                    OUStringToOString(pFoundData->sGroupName, RTL_TEXTENCODING_UTF8).getStr(),
                    OUStringToOString(sEntry, RTL_TEXTENCODING_UTF8).getStr());
            bExists = pFoundData->sGroupName == sEntry;
        }

        m_xRenamePB->set_sensitive(!bExists && !sName.isEmpty());
        fprintf(stderr, "one rename %d\n", !bExists && !sName.isEmpty());
        m_xDelPB->set_sensitive(IsDeleteAllowed(sEntry));
    }
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, NewHdl, weld::Button&, void)
{
    OUString sGroup = m_xNameED->get_text()
        + OUStringLiteral1(GLOS_DELIM)
        + OUString::number(m_xPathLB->get_active());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sGroup), "group already available!");
    m_InsertedArr.push_back(sGroup);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_xPathLB->get_active_text();
    pData->sGroupName = sGroup;
    pData->sGroupTitle = m_xNameED->get_text();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData)));
    m_xGroupTLB->append(sId, m_xNameED->get_text());
    int nEntry = m_xGroupTLB->find_id(sId);
    m_xGroupTLB->set_text(nEntry, pData->sPath, 1);
    m_xGroupTLB->select(nEntry);
    SelectHdl(*m_xGroupTLB);
    m_xGroupTLB->scroll_to_row(nEntry);
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, EntrySizeAllocHdl, const Size&, void)
{
    std::vector<int> aWidths;
    int x, y, width, height;
    if (m_xPathLB->get_extents_relative_to(*m_xGroupTLB, x, y, width, height))
    {
        aWidths.push_back(x);
        m_xGroupTLB->set_column_fixed_widths(aWidths);
    }
}

IMPL_LINK( SwGlossaryGroupDlg, DeleteHdl, weld::Button&, rButton, void )
{
    int nEntry = m_xGroupTLB->get_selected_index();
    if (nEntry == -1)
    {
        rButton.set_sensitive(false);
        return;
    }
    GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(nEntry).toInt64());
    OUString const sEntry(pUserData->sGroupName);
    // if the name to be deleted is among the new ones - get rid of it
    bool bDelete = true;
    auto it = std::find(m_InsertedArr.begin(), m_InsertedArr.end(), sEntry);
    if (it != m_InsertedArr.end())
    {
        m_InsertedArr.erase(it);
        bDelete = false;
    }
    // it should probably be renamed?
    if(bDelete)
    {
        it = std::find_if(m_RenamedArr.begin(), m_RenamedArr.end(),
            [&sEntry](OUString& s) { return s.getToken(0, RENAME_TOKEN_DELIM) == sEntry; });
        if (it != m_RenamedArr.end())
        {
            m_RenamedArr.erase(it);
            bDelete = false;
        }
    }
    if(bDelete)
    {
        m_RemovedArr.emplace_back(pUserData->sGroupName + "\t" + pUserData->sGroupTitle);
    }
    delete pUserData;
    m_xGroupTLB->remove(nEntry);
    if (!m_xGroupTLB->n_children())
        rButton.set_sensitive(false);
    //the content must be deleted - otherwise the new handler would be called in Apply()
    m_xNameED->set_text(OUString());
    ModifyHdl(*m_xNameED);
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, RenameHdl, weld::Button&, void)
{
    int nEntry = m_xGroupTLB->get_selected_index();
    GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(nEntry).toInt64());
    OUString sEntry(pUserData->sGroupName);

    const OUString sNewTitle(m_xNameED->get_text());
    OUString sNewName = sNewTitle
        + OUStringLiteral1(GLOS_DELIM)
        + OUString::number(m_xPathLB->get_active());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sNewName), "group already available!");

    // if the name to be renamed is among the new ones - replace
    bool bDone = false;
    auto it = std::find(m_InsertedArr.begin(), m_InsertedArr.end(), sEntry);
    if (it != m_InsertedArr.end())
    {
        m_InsertedArr.erase(it);
        m_InsertedArr.push_back(sNewName);
        bDone = true;
    }
    if(!bDone)
    {
        sEntry += OUStringLiteral1(RENAME_TOKEN_DELIM) + sNewName
                + OUStringLiteral1(RENAME_TOKEN_DELIM) + sNewTitle;
        m_RenamedArr.push_back(sEntry);
    }
    delete pUserData;
    m_xGroupTLB->remove(nEntry);

    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_xPathLB->get_active_text();
    pData->sGroupName = sNewName;
    pData->sGroupTitle = sNewTitle;

    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData)));
    m_xGroupTLB->append(sId, m_xNameED->get_text());
    nEntry = m_xGroupTLB->find_id(sId);
    m_xGroupTLB->set_text(nEntry, m_xPathLB->get_active_text(), 1);
    m_xGroupTLB->select(nEntry);
    SelectHdl(*m_xGroupTLB);
    m_xGroupTLB->scroll_to_row(nEntry);
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, ModifyListBoxHdl, weld::ComboBox&, void)
{
    ModifyHdl(*m_xNameED);
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, ModifyHdl, weld::Entry&, void)
{
    const OUString sEntry(m_xNameED->get_text());
    bool bEnableNew = true;
    bool bEnableDel = false;
    sal_uInt32 nCaseReadonly = m_xPathLB->get_active_id().toUInt32();
    bool bDirReadonly = 0 != (nCaseReadonly&PATH_READONLY);

    if (sEntry.isEmpty() || bDirReadonly)
        bEnableNew = false;
    else if(!sEntry.isEmpty())
    {
        int nPos = m_xGroupTLB->find_text(sEntry);
        //if it's not case sensitive you have to search for yourself
        if (nPos == -1)
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            for (int i = 0, nEntryCount = m_xGroupTLB->n_children(); i < nEntryCount; ++i)
            {
                const OUString sTemp = m_xGroupTLB->get_text(i, 0);
                nCaseReadonly = m_xPathLB->get_id(m_xPathLB->find_text(m_xGroupTLB->get_text(i,1))).toUInt32();
                bool bCase = 0 != (nCaseReadonly & PATH_CASE_SENSITIVE);

                if( !bCase && rSCmp.isEqual( sTemp, sEntry ))
                {
                    nPos = i;
                    break;
                }
            }
        }
        if (nPos != -1)
        {
            bEnableNew = false;
            m_xGroupTLB->select(nPos);
            m_xGroupTLB->scroll_to_row(nPos);
            SelectHdl(*m_xGroupTLB);
        }
    }
    int nEntry = m_xGroupTLB->get_selected_index();
    if (nEntry != -1)
    {
        GlosBibUserData* pUserData = reinterpret_cast<GlosBibUserData*>(m_xGroupTLB->get_id(nEntry).toInt64());
        bEnableDel = IsDeleteAllowed(pUserData->sGroupName);
    }

    m_xDelPB->set_sensitive(bEnableDel);
    m_xNewPB->set_sensitive(bEnableNew);
    m_xRenamePB->set_sensitive(bEnableNew && nEntry != -1);
    fprintf(stderr, "two rename %d\n", bEnableNew && nEntry != -1);
}

bool SwGlossaryGroupDlg::IsDeleteAllowed(const OUString &rGroup)
{
    bool bDel = !pGlosHdl->IsReadOnly(&rGroup);

    // OM: if the name is among the new region name, it is deletable
    // as well! Because for non existing region names ReadOnly issues
    // true.

    auto it = std::find(m_InsertedArr.cbegin(), m_InsertedArr.cend(), rGroup);
    if (it != m_InsertedArr.cend())
        bDel = true;

    return bDel;
}

IMPL_STATIC_LINK(SwGlossaryGroupDlg, EditInsertTextHdl, OUString&, rText, bool)
{
    rText = rText.replaceAll(OUStringLiteral1(SVT_SEARCHPATH_DELIMITER), "");
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
