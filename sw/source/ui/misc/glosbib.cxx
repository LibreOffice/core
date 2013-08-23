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
#include <unotools/transliterationwrapper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include "svtools/treelistentry.hxx"

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

SwGlossaryGroupDlg::SwGlossaryGroupDlg(Window * pParent,
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

    long nTabs[] =
    {   2, // Number of Tabs
        0, nAppFontUnits
    };

    m_pGroupTLB->SetTabs( &nTabs[0], MAP_APPFONT );
    m_pGroupTLB->SetSelectHdl(LINK(this, SwGlossaryGroupDlg, SelectHdl));
    m_pGroupTLB->GetModel()->SetSortMode(SortAscending);
    m_pNewPB->SetClickHdl(LINK(this, SwGlossaryGroupDlg, NewHdl));
    m_pDelPB->SetClickHdl(LINK(this, SwGlossaryGroupDlg, DeleteHdl));
    m_pNameED->SetModifyHdl(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
    m_pPathLB->SetSelectHdl(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
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
        m_pPathLB->SetEntryData(i, (void*)nCaseReadonly);
    }
    m_pPathLB->SelectEntryPos(0);
    m_pPathLB->Enable(sal_True);

    const sal_uInt16 nCount = pHdl->GetGroupCnt();
    for( sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString sTitle;
        String sGroup = pHdl->GetGroupName(i, &sTitle);
        if(!sGroup.Len())
            continue;
        GlosBibUserData* pData = new GlosBibUserData;
        pData->sGroupName = sGroup;
        pData->sGroupTitle = sTitle;
        String sTemp(sTitle);
        sTemp += '\t';
        pData->sPath = m_pPathLB->GetEntry((sal_uInt16)sGroup.GetToken(1, GLOS_DELIM).ToInt32());
        sTemp += pData->sPath;
        SvTreeListEntry* pEntry = m_pGroupTLB->InsertEntry(sTemp);
        pEntry->SetUserData(pData);

    }
    m_pGroupTLB->GetModel()->Resort();
}

SwGlossaryGroupDlg::~SwGlossaryGroupDlg()
{
}

void SwGlossaryGroupDlg::Apply()
{
    if(m_pNewPB->IsEnabled())
        NewHdl(m_pNewPB);

    String aActGroup = SwGlossaryDlg::GetCurrGroup();

    for (OUVector_t::const_iterator it(m_RemovedArr.begin());
            it != m_RemovedArr.end(); ++it)
    {
        const String sDelGroup = it->getToken(0, '\t');
        if( sDelGroup == aActGroup )
        {
            //when the current group is deleted, the current group has to be relocated
            if(m_pGroupTLB->GetEntryCount())
            {
                SvTreeListEntry* pFirst = m_pGroupTLB->First();
                GlosBibUserData* pUserData = (GlosBibUserData*)pFirst->GetUserData();
                pGlosHdl->SetCurGroup(pUserData->sGroupName);
            }
        }
        String sMsg(SW_RES(STR_QUERY_DELETE_GROUP1));
        String sTitle( it->getToken(1, '\t') );
        if(sTitle.Len())
            sMsg += sTitle;
        else
            sDelGroup.GetToken(1, GLOS_DELIM);
        sMsg += SW_RESSTR(STR_QUERY_DELETE_GROUP2);
        QueryBox aQuery(this->GetParent(), WB_YES_NO|WB_DEF_NO, sMsg );
        if(RET_YES == aQuery.Execute())
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
            if(!sCreatedGroup.Len())
                sCreatedGroup = sNewGroup;
        }
    }
}

IMPL_LINK( SwGlossaryGroupDlg, SelectHdl, SvTabListBox*, EMPTYARG  )
{
    m_pNewPB->Enable(sal_False);
    SvTreeListEntry* pFirstEntry = m_pGroupTLB->FirstSelected();
    if(pFirstEntry)
    {
        GlosBibUserData* pUserData = (GlosBibUserData*)pFirstEntry->GetUserData();
        String sEntry(pUserData->sGroupName);
        String sName(m_pNameED->GetText());
        bool bExists = false;
        sal_uLong nPos = m_pGroupTLB->GetEntryPos(sName, 0);
        if( 0xffffffff > nPos)
        {
            SvTreeListEntry* pEntry = m_pGroupTLB->GetEntry(nPos);
            GlosBibUserData* pFoundData = (GlosBibUserData*)pEntry->GetUserData();
            String sGroup = pFoundData->sGroupName;
            bExists = sGroup == sEntry;
        }

        m_pRenamePB->Enable(!bExists && sName.Len());
        m_pDelPB->Enable(IsDeleteAllowed(sEntry));
    }
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, NewHdl)
{
    OUString sGroup = m_pNameED->GetText()
        + OUString(GLOS_DELIM)
        + OUString::number(m_pPathLB->GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sGroup), "group already available!");
    m_InsertedArr.push_back(sGroup);
    String sTemp(m_pNameED->GetText());
    sTemp += '\t';
    sTemp += m_pPathLB->GetSelectEntry();
    SvTreeListEntry* pEntry = m_pGroupTLB->InsertEntry(sTemp);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_pPathLB->GetSelectEntry();
    pData->sGroupName = sGroup;
    pData->sGroupTitle = m_pNameED->GetText();
    pEntry->SetUserData(pData);
    m_pGroupTLB->Select(pEntry);
    m_pGroupTLB->MakeVisible(pEntry);
    m_pGroupTLB->GetModel()->Resort();

    return 0;
}

IMPL_LINK( SwGlossaryGroupDlg, DeleteHdl, Button*, pButton  )
{
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    if(!pEntry)
    {
        pButton->Enable(sal_False);
        return 0;
    }
    GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
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
        String sGroupEntry(pUserData->sGroupName);
        sGroupEntry += '\t';
        sGroupEntry += pUserData->sGroupTitle;
        m_RemovedArr.push_back(sGroupEntry);
    }
    delete pUserData;
    m_pGroupTLB->GetModel()->Remove(pEntry);
    if(!m_pGroupTLB->First())
        pButton->Enable(sal_False);
    //the content must be deleted - otherwise the new handler would be called in Apply()
    m_pNameED->SetText(aEmptyStr);
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, RenameHdl)
{
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
    String sEntry(pUserData->sGroupName);

    const OUString sNewTitle(m_pNameED->GetText());
    OUString sNewName = sNewTitle
        + OUString(GLOS_DELIM)
        + OUString::number(m_pPathLB->GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sNewName), "group already available!");

    // if the name to be renamed is among the new ones - replace
    bool bDone = false;
    for (OUVector_t::iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (String(*it) == sEntry)
        {
            m_InsertedArr.erase(it);
            m_InsertedArr.push_back(sNewName);
            bDone = true;
            break;
        }
    }
    if(!bDone)
    {
        sEntry += RENAME_TOKEN_DELIM;
        sEntry += sNewName;
        sEntry += RENAME_TOKEN_DELIM;
        sEntry += sNewTitle;
        m_RenamedArr.push_back(sEntry);
    }
    delete (GlosBibUserData*)pEntry->GetUserData();
    m_pGroupTLB->GetModel()->Remove(pEntry);
    String sTemp(m_pNameED->GetText());
    sTemp += '\t';
    sTemp += m_pPathLB->GetSelectEntry();
    pEntry = m_pGroupTLB->InsertEntry(sTemp);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = m_pPathLB->GetSelectEntry();
    pData->sGroupName = sNewName;
    pData->sGroupTitle = sNewTitle;
    pEntry->SetUserData(pData);
    m_pGroupTLB->Select(pEntry);
    m_pGroupTLB->MakeVisible(pEntry);
    m_pGroupTLB->GetModel()->Resort();
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, ModifyHdl)
{
    String sEntry(m_pNameED->GetText());
    sal_Bool bEnableNew = sal_True;
    sal_Bool bEnableDel = sal_False;
    sal_uLong nCaseReadonly =
            (sal_uLong)m_pPathLB->GetEntryData(m_pPathLB->GetSelectEntryPos());
    bool bDirReadonly = 0 != (nCaseReadonly&PATH_READONLY);

    if(!sEntry.Len() || bDirReadonly)
        bEnableNew = sal_False;
    else if(sEntry.Len())
    {
        sal_uLong nPos = m_pGroupTLB->GetEntryPos(sEntry, 0);
        //if it's not case sensitive you have to search for yourself
        if( 0xffffffff == nPos)
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            for(sal_uInt16 i = 0; i < m_pGroupTLB->GetEntryCount(); i++)
            {
                String sTemp = m_pGroupTLB->GetEntryText( i, 0 );
                nCaseReadonly = (sal_uLong)m_pPathLB->GetEntryData(
                    m_pPathLB->GetEntryPos(m_pGroupTLB->GetEntryText(i,1)));
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
            bEnableNew = sal_False;
            m_pGroupTLB->Select(m_pGroupTLB->GetEntry( nPos ));
            m_pGroupTLB->MakeVisible(m_pGroupTLB->GetEntry( nPos ));
        }
    }
    SvTreeListEntry* pEntry = m_pGroupTLB->FirstSelected();
    if(pEntry)
    {
        GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
        bEnableDel = IsDeleteAllowed(pUserData->sGroupName);
    }

    m_pDelPB->Enable(bEnableDel);
    m_pNewPB->Enable(bEnableNew);
    m_pRenamePB->Enable(bEnableNew && pEntry);
    return 0;
}

sal_Bool SwGlossaryGroupDlg::IsDeleteAllowed(const OUString &rGroup)
{
    sal_Bool bDel = (!pGlosHdl->IsReadOnly(&rGroup));

    // OM: if the name is among the new region name, it is deletable
    // as well! Because for non existing region names ReadOnly issues
    // sal_True.

    for (OUVector_t::const_iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (*it == rGroup)
        {
            bDel = sal_True;
            break;
        }
    }

    return bDel;
}

void FEdit::KeyInput( const KeyEvent& rKEvent )
{
    KeyCode aCode = rKEvent.GetKeyCode();
    if( KEYGROUP_CURSOR == aCode.GetGroup() ||
        ( KEYGROUP_MISC == aCode.GetGroup() &&
          KEY_DELETE >= aCode.GetCode() ) ||
        SVT_SEARCHPATH_DELIMITER != rKEvent.GetCharCode() )
        Edit::KeyInput( rKEvent );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeFEdit(Window *pParent, VclBuilder::stringmap &)
{
    return new FEdit(pParent);
}

void    SwGlossaryGroupTLB::RequestHelp( const HelpEvent& rHEvt )
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
            String sMsg;
            GlosBibUserData* pData = (GlosBibUserData*)pEntry->GetUserData();
            sMsg = pData->sPath;
            sMsg += INET_PATH_TOKEN;
            sMsg += pData->sGroupName.GetToken(0, GLOS_DELIM);
            sMsg += SwGlossaries::GetExtension();

            Help::ShowQuickHelp( this, aItemRect, sMsg,
                        QUICKHELP_LEFT|QUICKHELP_VCENTER );
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwGlossaryGroupTLB(Window *pParent, VclBuilder::stringmap &)
{
    return new SwGlossaryGroupTLB(pParent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
