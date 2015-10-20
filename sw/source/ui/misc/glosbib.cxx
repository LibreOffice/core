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
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/treelistentry.hxx>

#include <swtypes.hxx>
#include <glosbib.hxx>
#include <gloshdl.hxx>
#include <actctrl.hxx>
#include <glossary.hxx>
#include <glosdoc.hxx>
#include <swunohelper.hxx>

#include <misc.hrc>

#define PATH_CASE_SENSITIVE 0x01
#define PATH_READONLY       0x02

#define RENAME_TOKEN_DELIM      (sal_Unicode)1

SwGlossaryGroupDlg::SwGlossaryGroupDlg(vcl::Window * pParent,
    std::vector<OUString> const& rPathArr, SwGlossaryHdl *pHdl)
    : SvxStandardDialog(pParent, "EditCategoriesDialog",
        "modules/swriter/ui/editcategories.ui")
    , pGlosHdl(pHdl)
{
    get(m_pPathLB, "pathlb");
    get(m_pNewPB, "new");
    get(m_pDelPB, "delete");
    get(m_pRenamePB, "rename");
    get(m_pNameED, "name");
    get(m_pGroupTLB, "group");

    const int nAppFontUnits = 130;
    long nWidth = LogicToPixel(Size(nAppFontUnits, 0), MAP_APPFONT).Width();
    m_pPathLB->set_width_request(nWidth);
    //just has to be something small, real size will be available space
    m_pGroupTLB->set_width_request(nWidth);
    m_pGroupTLB->set_height_request(GetTextHeight() * 10);

    long nTabs[] =
    {   2, // Number of Tabs
        0, nAppFontUnits
    };

    m_pGroupTLB->SetTabs( &nTabs[0] );
    m_pGroupTLB->SetSelectHdl(LINK(this, SwGlossaryGroupDlg, SelectHdl));
    m_pGroupTLB->GetModel()->SetSortMode(SortAscending);
    m_pNewPB->SetClickHdl(LINK(this, SwGlossaryGroupDlg, NewHdl));
    m_pDelPB->SetClickHdl(LINK(this, SwGlossaryGroupDlg, DeleteHdl));
    m_pNameED->SetModifyHdl(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
    m_pPathLB->SetSelectHdl(LINK(this, SwGlossaryGroupDlg, ModifyListBoxHdl));
    m_pRenamePB->SetClickHdl(LINK(this, SwGlossaryGroupDlg, RenameHdl));

    for (size_t i = 0; i < rPathArr.size(); ++i)
    {
        INetURLObject aTempURL(rPathArr[i]);
        const OUString sPath = aTempURL.GetMainURL(INetURLObject::DECODE_WITH_CHARSET );
        m_pPathLB->InsertEntry(sPath);
        sal_uLong nCaseReadonly = 0;
        utl::TempFile aTempFile(&sPath);
        aTempFile.EnableKillingFile();
        if(!aTempFile.IsValid())
            nCaseReadonly |= PATH_READONLY;
        else if( SWUnoHelper::UCB_IsCaseSensitiveFileName( aTempFile.GetURL()))
            nCaseReadonly |= PATH_CASE_SENSITIVE;
        m_pPathLB->SetEntryData(i, reinterpret_cast<void*>(nCaseReadonly));
    }
    m_pPathLB->SelectEntryPos(0);
    m_pPathLB->Enable();

    const size_t nCount = pHdl->GetGroupCnt();
    for( size_t i = 0; i < nCount; ++i)
    {
        OUString sTitle;
        OUString sGroup = pHdl->GetGroupName(i, &sTitle);
        if(sGroup.isEmpty())
            continue;
        GlosBibUserData* pData = new GlosBibUserData;
        pData->sGroupName = sGroup;
        pData->sGroupTitle = sTitle;
        pData->sPath = m_pPathLB->GetEntry(sGroup.getToken(1, GLOS_DELIM).toInt32());
        SvTreeListEntry* pEntry = m_pGroupTLB->InsertEntry(sTitle + "\t" + pData->sPath);
        pEntry->SetUserData(pData);

    }
    m_pGroupTLB->GetModel()->Resort();
}

SwGlossaryGroupDlg::~SwGlossaryGroupDlg()
{
    disposeOnce();
}

void SwGlossaryGroupDlg::dispose()
{
    m_pNameED.clear();
    m_pPathLB.clear();
    m_pGroupTLB.clear();
    m_pNewPB.clear();
    m_pDelPB.clear();
    m_pRenamePB.clear();
    SvxStandardDialog::dispose();
}

void SwGlossaryGroupDlg::Apply()
{
    if(m_pNewPB->IsEnabled())
        NewHdl(m_pNewPB);

    OUString aActGroup = SwGlossaryDlg::GetCurrGroup();

    for (OUVector_t::const_iterator it(m_RemovedArr.begin());
            it != m_RemovedArr.end(); ++it)
    {
        const OUString sDelGroup = it->getToken(0, '\t');
        if( sDelGroup == aActGroup )
        {
            //when the current group is deleted, the current group has to be relocated
            if(m_pGroupTLB->GetEntryCount())
            {
                SvTreeListEntry* pFirst = m_pGroupTLB->First();
                GlosBibUserData* pUserData = static_cast<GlosBibUserData*>(pFirst->GetUserData());
                pGlosHdl->SetCurGroup(pUserData->sGroupName);
            }
        }
        OUString sTitle( it->getToken(1, '\t') );
        const OUString sMsg(SW_RESSTR(STR_QUERY_DELETE_GROUP1)
                            + sTitle
                            + SW_RESSTR(STR_QUERY_DELETE_GROUP2));
        ScopedVclPtrInstance< QueryBox > aQuery(this->GetParent(), WB_YES_NO|WB_DEF_NO, sMsg );
        if(RET_YES == aQuery->Execute())
            pGlosHdl->DelGroup( sDelGroup );
    }

    //don't rename before there was one
    for (OUVector_t::const_iterator it(m_RenamedArr.begin());
            it != m_RenamedArr.end(); ++it)
    {
        OUString const sOld(it->getToken(0, RENAME_TOKEN_DELIM));
        OUString sNew(it->getToken(1, RENAME_TOKEN_DELIM));
        OUString const sTitle(it->getToken(2, RENAME_TOKEN_DELIM));
        pGlosHdl->RenameGroup(sOld, sNew, sTitle);
        if (it == m_RenamedArr.begin())
        {
            sCreatedGroup = sNew;
        }
    }
    for (OUVector_t::const_iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        OUString sNewGroup = *it;
        OUString sNewTitle = sNewGroup.getToken(0, GLOS_DELIM);
        if( sNewGroup != aActGroup )
        {
            pGlosHdl->NewGroup(sNewGroup, sNewTitle);
            if(sCreatedGroup.isEmpty())
                sCreatedGroup = sNewGroup;
        }
    }
}

IMPL_LINK_NOARG_TYPED( SwGlossaryGroupDlg, SelectHdl, SvTreeListBox*, void )
{
    m_pNewPB->Enable(false);
    SvTreeListEntry* pFirstEntry = m_pGroupTLB->FirstSelected();
    if(pFirstEntry)
    {
        GlosBibUserData* pUserData = static_cast<GlosBibUserData*>(pFirstEntry->GetUserData());
        OUString sEntry(pUserData->sGroupName);
        OUString sName(m_pNameED->GetText());
        bool bExists = false;
        sal_uLong nPos = m_pGroupTLB->GetEntryPos(sName, 0);
        if( 0xffffffff > nPos)
        {
            SvTreeListEntry* pEntry = m_pGroupTLB->GetEntry(nPos);
            GlosBibUserData* pFoundData = static_cast<GlosBibUserData*>(pEntry->GetUserData());
            bExists = pFoundData->sGroupName == sEntry;
        }

        m_pRenamePB->Enable(!bExists && !sName.isEmpty());
        m_pDelPB->Enable(IsDeleteAllowed(sEntry));
    }
}

IMPL_LINK_NOARG_TYPED(SwGlossaryGroupDlg, NewHdl, Button*, void)
{
    OUString sGroup = m_pNameED->GetText()
        + OUStringLiteral1<GLOS_DELIM>()
        + OUString::number(m_pPathLB->GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sGroup), "group already available!");
    m_InsertedArr.push_back(sGroup);
    const OUString sTemp(m_pNameED->GetText() + "\t" + m_pPathLB->GetSelectEntry());
    SvTreeListEntry* pEntry = m_pGroupTLB->InsertEntry(sTemp);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_pPathLB->GetSelectEntry();
    pData->sGroupName = sGroup;
    pData->sGroupTitle = m_pNameED->GetText();
    pEntry->SetUserData(pData);
    m_pGroupTLB->Select(pEntry);
    m_pGroupTLB->MakeVisible(pEntry);
    m_pGroupTLB->GetModel()->Resort();
}

IMPL_LINK_TYPED( SwGlossaryGroupDlg, DeleteHdl, Button*, pButton, void )
{
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    if(!pEntry)
    {
        pButton->Enable(false);
        return;
    }
    GlosBibUserData* pUserData = static_cast<GlosBibUserData*>(pEntry->GetUserData());
    OUString const sEntry(pUserData->sGroupName);
    // if the name to be deleted is among the new ones - get rid of it
    bool bDelete = true;
    for (OUVector_t::iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (*it == sEntry)
        {
            m_InsertedArr.erase(it);
            bDelete = false;
            break;
        }

    }
    // it should probably be renamed?
    if(bDelete)
    {
        for (OUVector_t::iterator it(m_RenamedArr.begin());
                it != m_RenamedArr.end(); ++it)
        {
            if (it->getToken(0, RENAME_TOKEN_DELIM) == sEntry)
            {
                m_RenamedArr.erase(it);
                bDelete = false;
                break;
            }
        }
    }
    if(bDelete)
    {
        m_RemovedArr.push_back(pUserData->sGroupName + "\t" + pUserData->sGroupTitle);
    }
    delete pUserData;
    m_pGroupTLB->GetModel()->Remove(pEntry);
    if(!m_pGroupTLB->First())
        pButton->Enable(false);
    //the content must be deleted - otherwise the new handler would be called in Apply()
    m_pNameED->SetText(aEmptyOUStr);
}

IMPL_LINK_NOARG_TYPED(SwGlossaryGroupDlg, RenameHdl, Button*, void)
{
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    GlosBibUserData* pUserData = static_cast<GlosBibUserData*>(pEntry->GetUserData());
    OUString sEntry(pUserData->sGroupName);

    const OUString sNewTitle(m_pNameED->GetText());
    OUString sNewName = sNewTitle
        + OUStringLiteral1<GLOS_DELIM>()
        + OUString::number(m_pPathLB->GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sNewName), "group already available!");

    // if the name to be renamed is among the new ones - replace
    bool bDone = false;
    for (OUVector_t::iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (*it == sEntry)
        {
            m_InsertedArr.erase(it);
            m_InsertedArr.push_back(sNewName);
            bDone = true;
            break;
        }
    }
    if(!bDone)
    {
        sEntry += OUStringLiteral1<RENAME_TOKEN_DELIM>() + sNewName
                + OUStringLiteral1<RENAME_TOKEN_DELIM>() + sNewTitle;
        m_RenamedArr.push_back(sEntry);
    }
    delete static_cast<GlosBibUserData*>(pEntry->GetUserData());
    m_pGroupTLB->GetModel()->Remove(pEntry);
    pEntry = m_pGroupTLB->InsertEntry(m_pNameED->GetText() + "\t"
                                      + m_pPathLB->GetSelectEntry());
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_pPathLB->GetSelectEntry();
    pData->sGroupName = sNewName;
    pData->sGroupTitle = sNewTitle;
    pEntry->SetUserData(pData);
    m_pGroupTLB->Select(pEntry);
    m_pGroupTLB->MakeVisible(pEntry);
    m_pGroupTLB->GetModel()->Resort();
}

IMPL_LINK_NOARG_TYPED(SwGlossaryGroupDlg, ModifyListBoxHdl, ListBox&, void)
{
    ModifyHdl(*m_pNameED);
}
IMPL_LINK_NOARG_TYPED(SwGlossaryGroupDlg, ModifyHdl, Edit&, void)
{
    OUString sEntry(m_pNameED->GetText());
    bool bEnableNew = true;
    bool bEnableDel = false;
    sal_uLong nCaseReadonly =
            reinterpret_cast<sal_uLong>(m_pPathLB->GetSelectEntryData());
    bool bDirReadonly = 0 != (nCaseReadonly&PATH_READONLY);

    if(sEntry.isEmpty() || bDirReadonly)
        bEnableNew = false;
    else if(!sEntry.isEmpty())
    {
        sal_uLong nPos = m_pGroupTLB->GetEntryPos(sEntry, 0);
        //if it's not case sensitive you have to search for yourself
        if( 0xffffffff == nPos)
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            for(sal_uLong i = 0; i < m_pGroupTLB->GetEntryCount(); i++)
            {
                OUString sTemp = m_pGroupTLB->GetEntryText( i, 0 );
                nCaseReadonly = reinterpret_cast<sal_uLong>(m_pPathLB->GetEntryData(
                    m_pPathLB->GetEntryPos(m_pGroupTLB->GetEntryText(i,1))));
                bool bCase = 0 != (nCaseReadonly & PATH_CASE_SENSITIVE);

                if( !bCase && rSCmp.isEqual( sTemp, sEntry ))
                {
                    nPos = i;
                    break;
                }
            }
        }
        if( 0xffffffff > nPos)
        {
            bEnableNew = false;
            m_pGroupTLB->Select(m_pGroupTLB->GetEntry( nPos ));
            m_pGroupTLB->MakeVisible(m_pGroupTLB->GetEntry( nPos ));
        }
    }
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    if(pEntry)
    {
        GlosBibUserData* pUserData = static_cast<GlosBibUserData*>(pEntry->GetUserData());
        bEnableDel = IsDeleteAllowed(pUserData->sGroupName);
    }

    m_pDelPB->Enable(bEnableDel);
    m_pNewPB->Enable(bEnableNew);
    m_pRenamePB->Enable(bEnableNew && pEntry);
}

bool SwGlossaryGroupDlg::IsDeleteAllowed(const OUString &rGroup)
{
    bool bDel = (!pGlosHdl->IsReadOnly(&rGroup));

    // OM: if the name is among the new region name, it is deletable
    // as well! Because for non existing region names ReadOnly issues
    // true.

    for (OUVector_t::const_iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (*it == rGroup)
        {
            bDel = true;
            break;
        }
    }

    return bDel;
}

void FEdit::KeyInput( const KeyEvent& rKEvent )
{
    vcl::KeyCode aCode = rKEvent.GetKeyCode();
    if( KEYGROUP_CURSOR == aCode.GetGroup() ||
        ( KEYGROUP_MISC == aCode.GetGroup() &&
          KEY_DELETE >= aCode.GetCode() ) ||
        SVT_SEARCHPATH_DELIMITER != rKEvent.GetCharCode() )
        Edit::KeyInput( rKEvent );
}

VCL_BUILDER_FACTORY(FEdit)

void SwGlossaryGroupTLB::RequestHelp( const HelpEvent& rHEvt )
{
    Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
    SvTreeListEntry* pEntry = GetEntry( aPos );
    if(pEntry)
    {
        SvLBoxTab* pTab;
        SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
        if(pItem)
        {
            aPos = GetEntryPosition( pEntry );
            Size aSize(pItem->GetSize( this, pEntry ));
            aPos.X() = GetTabPos( pEntry, pTab );

            if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                aSize.Width() = GetSizePixel().Width() - aPos.X();
            aPos = OutputToScreenPixel(aPos);
            Rectangle aItemRect( aPos, aSize );
            GlosBibUserData* pData = static_cast<GlosBibUserData*>(pEntry->GetUserData());
            const OUString sMsg = pData->sPath + "/"
                                + pData->sGroupName.getToken(0, GLOS_DELIM)
                                + SwGlossaries::GetExtension();

            Help::ShowQuickHelp( this, aItemRect, sMsg,
                        QuickHelpFlags::Left|QuickHelpFlags::VCenter );
        }
    }
}

VCL_BUILDER_FACTORY(SwGlossaryGroupTLB)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
