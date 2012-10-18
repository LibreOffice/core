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

#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/tempfile.hxx>

#include <unotools/pathoptions.hxx>
#include <swtypes.hxx>
#include <glosbib.hxx>
#include <gloshdl.hxx>
#include <actctrl.hxx>
#include <glossary.hxx>
#include <glosdoc.hxx>
#include <swunohelper.hxx>

#include <glosbib.hrc>
#include <misc.hrc>
#include <helpid.h>


#define PATH_CASE_SENSITIVE 0x01
#define PATH_READONLY       0x02

#define RENAME_TOKEN_DELIM      (sal_Unicode)1

SwGlossaryGroupDlg::SwGlossaryGroupDlg(Window * pParent,
                        std::vector<String> const& rPathArr,
                        SwGlossaryHdl *pHdl) :
    SvxStandardDialog(pParent, SW_RES(DLG_BIB_BASE)),
    aBibFT(     this, SW_RES(FT_BIB)),
    aNameED(    this, SW_RES(ED_NAME)),
    aPathFT(     this, SW_RES(FT_PATH)),
    aPathLB(    this, SW_RES(LB_PATH)),
    aSelectFT(   this, SW_RES(FT_SELECT)),
    aGroupTLB(  this, SW_RES(TLB_GROUPS)),

    aOkPB(      this, SW_RES(BT_OK)),
    aCancelPB(  this, SW_RES(BT_CANCEL)),
    aHelpPB(    this, SW_RES(BT_HELP)),
    aNewPB(     this, SW_RES(PB_NEW)),
    aDelPB(     this, SW_RES(PB_DELETE)),
    aRenamePB(  this, SW_RES(PB_RENAME)),

    pGlosHdl(pHdl)
{
    FreeResource();

    long nTabs[] =
    {   2, // Number of Tabs
        0, 160
    };

    aGroupTLB.SetHelpId(HID_GLOS_GROUP_TREE);
    aGroupTLB.SetTabs( &nTabs[0], MAP_APPFONT );
    aGroupTLB.SetStyle(aGroupTLB.GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN|WB_SORT);
    aGroupTLB.SetSelectHdl(LINK(this, SwGlossaryGroupDlg, SelectHdl));
    aGroupTLB.GetModel()->SetSortMode(SortAscending);
    aNewPB.SetClickHdl(LINK(this, SwGlossaryGroupDlg, NewHdl));
    aDelPB.SetClickHdl(LINK(this, SwGlossaryGroupDlg, DeleteHdl));
    aNameED.SetModifyHdl(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
    aPathLB.SetSelectHdl(LINK(this, SwGlossaryGroupDlg, ModifyHdl));
    aRenamePB.SetClickHdl(LINK(this, SwGlossaryGroupDlg, RenameHdl));

    for (size_t i = 0; i < rPathArr.size(); ++i)
    {
        String sPath(rPathArr[i]);
        INetURLObject aTempURL(sPath);
        sPath = aTempURL.GetMainURL(INetURLObject::DECODE_WITH_CHARSET );
        aPathLB.InsertEntry(sPath);
        sal_uLong nCaseReadonly = 0;
        utl::TempFile aTempFile(&sPath);
        aTempFile.EnableKillingFile();
        if(!aTempFile.IsValid())
            nCaseReadonly |= PATH_READONLY;
        else if( SWUnoHelper::UCB_IsCaseSensitiveFileName( aTempFile.GetURL()))
            nCaseReadonly |= PATH_CASE_SENSITIVE;
        aPathLB.SetEntryData(i, (void*)nCaseReadonly);
    }
    aPathLB.SelectEntryPos(0);
    aPathLB.Enable(sal_True);

    const sal_uInt16 nCount = pHdl->GetGroupCnt();
    for( sal_uInt16 i = 0; i < nCount; ++i)
    {
        String sTitle;
        String sGroup = pHdl->GetGroupName(i, &sTitle);
        if(!sGroup.Len())
            continue;
        GlosBibUserData* pData = new GlosBibUserData;
        pData->sGroupName = sGroup;
        pData->sGroupTitle = sTitle;
        String sTemp(sTitle);
        sTemp += '\t';
        pData->sPath = aPathLB.GetEntry((sal_uInt16)sGroup.GetToken(1, GLOS_DELIM).ToInt32());
        sTemp += pData->sPath;
        SvTreeListEntry* pEntry = aGroupTLB.InsertEntry(sTemp);
        pEntry->SetUserData(pData);

    }
    aGroupTLB.GetModel()->Resort();
}

SwGlossaryGroupDlg::~SwGlossaryGroupDlg()
{
}

void SwGlossaryGroupDlg::Apply()
{
    if(aNewPB.IsEnabled())
        NewHdl(&aNewPB);

    String aActGroup = SwGlossaryDlg::GetCurrGroup();

    for (OUVector_t::const_iterator it(m_RemovedArr.begin());
            it != m_RemovedArr.end(); ++it)
    {
        const String sDelGroup =
            ::comphelper::string::getToken(*it, 0, '\t');
        if( sDelGroup == aActGroup )
        {
            //when the current group is deleted, the current group has to be relocated
            if(aGroupTLB.GetEntryCount())
            {
                SvTreeListEntry* pFirst = aGroupTLB.First();
                GlosBibUserData* pUserData = (GlosBibUserData*)pFirst->GetUserData();
                pGlosHdl->SetCurGroup(pUserData->sGroupName);
            }
        }
        String sMsg(SW_RES(STR_QUERY_DELETE_GROUP1));
        String sTitle( ::comphelper::string::getToken(*it, 1, '\t') );
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
        ::rtl::OUString const sOld(
                ::comphelper::string::getToken(*it, 0, RENAME_TOKEN_DELIM));
        String sNew(
                ::comphelper::string::getToken(*it, 1, RENAME_TOKEN_DELIM));
        ::rtl::OUString const sTitle(
                ::comphelper::string::getToken(*it, 2, RENAME_TOKEN_DELIM));
        pGlosHdl->RenameGroup(sOld, sNew, sTitle);
        if (it == m_RenamedArr.begin())
        {
            sCreatedGroup = sNew;
        }
    }
    for (OUVector_t::const_iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        String sNewGroup = *it;
        String sNewTitle = sNewGroup.GetToken(0, GLOS_DELIM);
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
    aNewPB.Enable(sal_False);
    SvTreeListEntry* pFirstEntry = aGroupTLB.FirstSelected();
    if(pFirstEntry)
    {
        GlosBibUserData* pUserData = (GlosBibUserData*)pFirstEntry->GetUserData();
        String sEntry(pUserData->sGroupName);
        String sName(aNameED.GetText());
        sal_Bool bExists = sal_False;
        sal_uLong nPos = aGroupTLB.GetEntryPos(sName, 0);
        if( 0xffffffff > nPos)
        {
            SvTreeListEntry* pEntry = aGroupTLB.GetEntry(nPos);
            GlosBibUserData* pFoundData = (GlosBibUserData*)pEntry->GetUserData();
            String sGroup = pFoundData->sGroupName;
            bExists = sGroup == sEntry;
        }

        aRenamePB.Enable(!bExists && sName.Len());
        aDelPB.Enable(IsDeleteAllowed(sEntry));
    }
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, NewHdl)
{
    String sGroup(aNameED.GetText());
    sGroup += GLOS_DELIM;
    sGroup += String::CreateFromInt32(aPathLB.GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sGroup), "group already available!");
    m_InsertedArr.push_back(sGroup);
    String sTemp(aNameED.GetText());
    sTemp += '\t';
    sTemp += aPathLB.GetSelectEntry();
    SvTreeListEntry* pEntry = aGroupTLB.InsertEntry(sTemp);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = aPathLB.GetSelectEntry();
    pData->sGroupName = sGroup;
    pData->sGroupTitle = aNameED.GetText();
    pEntry->SetUserData(pData);
    aGroupTLB.Select(pEntry);
    aGroupTLB.MakeVisible(pEntry);
    aGroupTLB.GetModel()->Resort();

    return 0;
}

IMPL_LINK( SwGlossaryGroupDlg, DeleteHdl, Button*, pButton  )
{
    SvTreeListEntry* pEntry = aGroupTLB.FirstSelected();
    if(!pEntry)
    {
        pButton->Enable(sal_False);
        return 0;
    }
    GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
    ::rtl::OUString const sEntry(pUserData->sGroupName);
    // if the name to be deleted is among the new ones - get rid of it
    sal_Bool bDelete = sal_True;
    for (OUVector_t::iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (*it == sEntry)
        {
            m_InsertedArr.erase(it);
            bDelete = sal_False;
            break;
        }

    }
    // it should probably be renamed?
    if(bDelete)
    {
        for (OUVector_t::iterator it(m_RenamedArr.begin());
                it != m_RenamedArr.end(); ++it)
        {
            if (::comphelper::string::getToken(*it, 0, RENAME_TOKEN_DELIM)
                    == sEntry)
            {
                m_RenamedArr.erase(it);
                bDelete = sal_False;
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
    aGroupTLB.GetModel()->Remove(pEntry);
    if(!aGroupTLB.First())
        pButton->Enable(sal_False);
    //the content must be deleted - otherwise the new handler would be called in Apply()
    aNameED.SetText(aEmptyStr);
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, RenameHdl)
{
    SvTreeListEntry* pEntry = aGroupTLB.FirstSelected();
    GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
    String sEntryText(aGroupTLB.GetEntryText(pEntry));
    String sEntry(pUserData->sGroupName);

    String sNewName(aNameED.GetText());
    String sNewTitle(sNewName);

    sNewName += GLOS_DELIM;
    sNewName += String::CreateFromInt32(aPathLB.GetSelectEntryPos());
    OSL_ENSURE(!pGlosHdl->FindGroupName(sNewName), "group already available!");

    // if the name to be renamed is among the new ones - replace
    sal_Bool bDone = sal_False;
    for (OUVector_t::iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (String(*it) == sEntry)
        {
            m_InsertedArr.erase(it);
            m_InsertedArr.push_back(sNewName);
            bDone = sal_True;
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
    aGroupTLB.GetModel()->Remove(pEntry);
    String sTemp(aNameED.GetText());
    sTemp += '\t';
    sTemp += aPathLB.GetSelectEntry();
    pEntry = aGroupTLB.InsertEntry(sTemp);
    GlosBibUserData* pData = new GlosBibUserData;
    pData->sPath = aPathLB.GetSelectEntry();
    pData->sGroupName = sNewName;
    pData->sGroupTitle = sNewTitle;
    pEntry->SetUserData(pData);
    aGroupTLB.Select(pEntry);
    aGroupTLB.MakeVisible(pEntry);
    aGroupTLB.GetModel()->Resort();
    return 0;
}

IMPL_LINK_NOARG(SwGlossaryGroupDlg, ModifyHdl)
{
    String sEntry(aNameED.GetText());
    sal_Bool bEnableNew = sal_True;
    sal_Bool bEnableDel = sal_False;
    sal_uLong nCaseReadonly =
            (sal_uLong)aPathLB.GetEntryData(aPathLB.GetSelectEntryPos());
    sal_Bool bDirReadonly = 0 != (nCaseReadonly&PATH_READONLY);

    if(!sEntry.Len() || bDirReadonly)
        bEnableNew = sal_False;
    else if(sEntry.Len())
    {
        sal_uLong nPos = 0xffffffff;


        nPos = aGroupTLB.GetEntryPos(sEntry, 0);
        //if it's not case sensitive you have to search for yourself
        if( 0xffffffff == nPos)
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            for(sal_uInt16 i = 0; i < aGroupTLB.GetEntryCount(); i++)
            {
                String sTemp = aGroupTLB.GetEntryText( i, 0 );
                nCaseReadonly = (sal_uLong)aPathLB.GetEntryData(
                    aPathLB.GetEntryPos(aGroupTLB.GetEntryText(i,1)));
                sal_Bool bCase = 0 != (nCaseReadonly & PATH_CASE_SENSITIVE);

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
            aGroupTLB.Select(aGroupTLB.GetEntry( nPos ));
            aGroupTLB.MakeVisible(aGroupTLB.GetEntry( nPos ));
        }
    }
    SvTreeListEntry* pEntry = aGroupTLB.FirstSelected();
    if(pEntry)
    {
        GlosBibUserData* pUserData = (GlosBibUserData*)pEntry->GetUserData();
        bEnableDel = IsDeleteAllowed(pUserData->sGroupName);
    }

    aDelPB.Enable(bEnableDel);
    aNewPB.Enable(bEnableNew);
    aRenamePB.Enable(bEnableNew && pEntry);
    return 0;
}

sal_Bool SwGlossaryGroupDlg::IsDeleteAllowed(const String &rGroup)
{
    sal_Bool bDel = (!pGlosHdl->IsReadOnly(&rGroup));

    // OM: if the name is among the new region name, it is deletable
    // as well! Because for non existing region names ReadOnly issues
    // sal_True.

    for (OUVector_t::const_iterator it(m_InsertedArr.begin());
            it != m_InsertedArr.end(); ++it)
    {
        if (String(*it) == rGroup)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
