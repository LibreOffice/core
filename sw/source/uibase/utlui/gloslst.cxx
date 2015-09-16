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
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>
#include <swunohelper.hxx>

#include <vector>

#include <utlui.hrc>

#define STRING_DELIM (char)0x0A
#define GLOS_TIMEOUT 30000   // update every 30 seconds
#define FIND_MAX_GLOS 20

struct TripleString
{
    OUString sGroup;
    OUString sBlock;
    OUString sShort;
};

class SwGlossDecideDlg : public ModalDialog
{
    VclPtr<OKButton> m_pOk;
    VclPtr<ListBox>  m_pListLB;

    DECL_LINK_TYPED(DoubleClickHdl, ListBox&, void);
    DECL_LINK(SelectHdl, void*);

public:
    explicit SwGlossDecideDlg(vcl::Window* pParent);
    virtual ~SwGlossDecideDlg();
    virtual void dispose() SAL_OVERRIDE;

    ListBox&    GetListBox() {return *m_pListLB;}
};

SwGlossDecideDlg::SwGlossDecideDlg(vcl::Window* pParent)
    : ModalDialog(pParent, "SelectAutoTextDialog",
        "modules/swriter/ui/selectautotextdialog.ui")
{
    get(m_pOk, "ok");
    get(m_pListLB, "treeview");
    m_pListLB->set_height_request(m_pListLB->GetTextHeight() * 10);
    m_pListLB->SetDoubleClickHdl(LINK(this, SwGlossDecideDlg, DoubleClickHdl));
    m_pListLB->SetSelectHdl(LINK(this, SwGlossDecideDlg, SelectHdl));
}

SwGlossDecideDlg::~SwGlossDecideDlg()
{
    disposeOnce();
}

void SwGlossDecideDlg::dispose()
{
    m_pOk.clear();
    m_pListLB.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwGlossDecideDlg, DoubleClickHdl, ListBox&, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SwGlossDecideDlg, SelectHdl)
{
    m_pOk->Enable(LISTBOX_ENTRY_NOTFOUND != m_pListLB->GetSelectEntryPos());
    return 0;
}

SwGlossaryList::SwGlossaryList() :
    bFilled(false)
{
    SvtPathOptions aPathOpt;
    sPath = aPathOpt.GetAutoTextPath();
    SetTimeout(GLOS_TIMEOUT);
}

SwGlossaryList::~SwGlossaryList()
{
    ClearGroups();
}

// If the GroupName is already known, then only rShortName
// will be filled. Otherwise also rGroupName will be set and
// on demand asked for the right group.

bool SwGlossaryList::GetShortName(const OUString& rLongName,
                                  OUString& rShortName, OUString& rGroupName )
{
    if(!bFilled)
        Update();

    std::vector<TripleString> aTripleStrings;

    size_t nCount = aGroupArr.size();
    for(size_t i = 0; i < nCount; i++ )
    {
        AutoTextGroup* pGroup = aGroupArr[i];
        if(!rGroupName.isEmpty() && rGroupName != pGroup->sName)
            continue;

        sal_Int32 nPosLong = 0;
        for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
        {
            const OUString sLong = pGroup->sLongNames.getToken(0, STRING_DELIM, nPosLong);
            if(rLongName != sLong)
                continue;

            TripleString pTriple;
            pTriple.sGroup = pGroup->sName;
            pTriple.sBlock = sLong;
            pTriple.sShort = pGroup->sShortNames.getToken(j, STRING_DELIM);
            aTripleStrings.push_back(pTriple);
        }
    }

    bool bRet = false;
    nCount = aTripleStrings.size();
    if(1 == nCount)
    {
        const TripleString& pTriple(aTripleStrings.front());
        rShortName = pTriple.sShort;
        rGroupName = pTriple.sGroup;
        bRet = true;
    }
    else if(1 < nCount)
    {
        ScopedVclPtrInstance< SwGlossDecideDlg > aDlg(nullptr);
        OUString sTitle = aDlg->GetText() + " " + aTripleStrings.front().sBlock;
        aDlg->SetText(sTitle);

        ListBox& rLB = aDlg->GetListBox();
        for(std::vector<TripleString>::const_iterator i = aTripleStrings.begin(); i != aTripleStrings.end(); ++i)
            rLB.InsertEntry(i->sGroup.getToken(0, GLOS_DELIM));

        rLB.SelectEntryPos(0);
        if(RET_OK == aDlg->Execute() &&
            LISTBOX_ENTRY_NOTFOUND != rLB.GetSelectEntryPos())
        {
            const TripleString& pTriple(aTripleStrings[rLB.GetSelectEntryPos()]);
            rShortName = pTriple.sShort;
            rGroupName = pTriple.sGroup;
            bRet = true;
        }
        else
            bRet = false;
    }
    return bRet;
}

size_t SwGlossaryList::GetGroupCount()
{
    if(!bFilled)
        Update();
    return aGroupArr.size();
}

OUString SwGlossaryList::GetGroupName(size_t nPos, bool bNoPath)
{
    OSL_ENSURE(aGroupArr.size() > nPos, "group not available");
    if(nPos < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nPos];
        OUString sRet = pGroup->sName;
        if(bNoPath)
            sRet = sRet.getToken(0, GLOS_DELIM);
        return sRet;
    }
    return OUString();
}

OUString SwGlossaryList::GetGroupTitle(size_t nPos)
{
    OSL_ENSURE(aGroupArr.size() > nPos, "group not available");
    if(nPos < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nPos];
        return pGroup->sTitle;
    }
    return OUString();
}

sal_uInt16 SwGlossaryList::GetBlockCount(size_t nGroup)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup];
        return pGroup->nCount;
    }
    return 0;
}

OUString SwGlossaryList::GetBlockLongName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup];
        return pGroup->sLongNames.getToken(nBlock, STRING_DELIM);
    }
    return OUString();
}

OUString SwGlossaryList::GetBlockShortName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup];
        return pGroup->sShortNames.getToken(nBlock, STRING_DELIM);
    }
    return OUString();
}

void SwGlossaryList::Update()
{
    if(!IsActive())
        Start();

    SvtPathOptions aPathOpt;
    OUString sTemp( aPathOpt.GetAutoTextPath() );
    if(sTemp != sPath)
    {
        sPath = sTemp;
        bFilled = false;
        ClearGroups();
    }
    SwGlossaries* pGlossaries = ::GetGlossaries();
    const std::vector<OUString> & rPathArr = pGlossaries->GetPathArray();
    const OUString sExt( SwGlossaries::GetExtension() );
    if(!bFilled)
    {
        const size_t nGroupCount = pGlossaries->GetGroupCnt();
        for(size_t i = 0; i < nGroupCount; ++i)
        {
            OUString sGrpName = pGlossaries->GetGroupName(i);
            const size_t nPath = static_cast<size_t>(
                sGrpName.getToken(1, GLOS_DELIM).toInt32());
            if( nPath < rPathArr.size() )
            {
                AutoTextGroup* pGroup = new AutoTextGroup;
                pGroup->sName = sGrpName;

                FillGroup(pGroup, pGlossaries);
                OUString sName = rPathArr[nPath] + "/" +
                    pGroup->sName.getToken(0, GLOS_DELIM) + sExt;
                FStatHelper::GetModifiedDateTimeOfFile( sName,
                                                &pGroup->aDateModified,
                                                &pGroup->aDateModified );

                aGroupArr.insert( aGroupArr.begin(), pGroup );
            }
        }
        bFilled = true;
    }
    else
    {
        for( size_t nPath = 0; nPath < rPathArr.size(); nPath++ )
        {
            std::vector<OUString> aFoundGroupNames;
            std::vector<OUString> aFiles;
            std::vector<DateTime*> aDateTimeArr;

            SWUnoHelper::UCB_GetFileListOfFolder( rPathArr[nPath], aFiles,
                                                    &sExt, &aDateTimeArr );
            for( size_t nFiles = 0; nFiles < aFiles.size(); ++nFiles )
            {
                const OUString aTitle = aFiles[ nFiles ];
                ::DateTime* pDT = aDateTimeArr[ nFiles ];

                OUString sName( aTitle.copy( 0, aTitle.getLength() - sExt.getLength() ));

                aFoundGroupNames.push_back(sName);
                sName += OUStringLiteral1<GLOS_DELIM>() + OUString::number( static_cast<sal_uInt16>(nPath) );
                AutoTextGroup* pFound = FindGroup( sName );
                if( !pFound )
                {
                    pFound = new AutoTextGroup;
                    pFound->sName = sName;
                    FillGroup( pFound, pGlossaries );
                    pFound->aDateModified = *pDT;

                    aGroupArr.push_back(pFound);
                }
                else if( pFound->aDateModified < *pDT )
                {
                    FillGroup(pFound, pGlossaries);
                    pFound->aDateModified = *pDT;
                }

                // don't need any more these pointers
                delete pDT;
            }

            for( size_t i = aGroupArr.size(); i>0; )
            {
                --i;
                // maybe remove deleted groups
                AutoTextGroup* pGroup = aGroupArr[i];
                const size_t nGroupPath = static_cast<size_t>(
                    pGroup->sName.getToken( 1, GLOS_DELIM).toInt32());
                // Only the groups will be checked which are registered
                // for the current subpath.
                if( nGroupPath == nPath )
                {
                    bool bFound = false;
                    OUString sCompareGroup = pGroup->sName.getToken(0, GLOS_DELIM);
                    for(std::vector<OUString>::const_iterator j = aFoundGroupNames.begin(); j != aFoundGroupNames.end() && !bFound; ++j)
                        bFound = (sCompareGroup == *j);

                    if(!bFound)
                    {
                        aGroupArr.erase(aGroupArr.begin() + i);
                        delete pGroup;
                    }
                }
            }
        }
    }
}

void SwGlossaryList::Invoke()
{
    // Only update automatically if a SwView has the focus.
    if(::GetActiveView())
        Update();
}

AutoTextGroup* SwGlossaryList::FindGroup(const OUString& rGroupName)
{
    for(size_t i = 0; i < aGroupArr.size(); ++i)
    {
        AutoTextGroup* pRet = aGroupArr[i];
        if(pRet->sName == rGroupName)
            return pRet;
    }
    return 0;
}

void SwGlossaryList::FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGlossaries)
{
    SwTextBlocks*   pBlock = pGlossaries->GetGroupDoc(pGroup->sName);
    pGroup->nCount = pBlock ? pBlock->GetCount() : 0;
    (pGroup->sLongNames).clear();
    (pGroup->sShortNames).clear();
    if(pBlock)
        pGroup->sTitle = pBlock->GetName();

    for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
    {
        pGroup->sLongNames += pBlock->GetLongName(j)
            + OUStringLiteral1<STRING_DELIM>();
        pGroup->sShortNames += pBlock->GetShortName(j)
            + OUStringLiteral1<STRING_DELIM>();
    }
    delete pBlock;
}

// Give back all (not exceeding FIND_MAX_GLOS) found modules
// with matching beginning.

bool SwGlossaryList::HasLongName(const OUString& rBegin, std::vector<OUString> *pLongNames)
{
    if(!bFilled)
        Update();
    sal_uInt16 nFound = 0;
    const size_t nCount = aGroupArr.size();
    sal_Int32 nBeginLen = rBegin.getLength();
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    for(size_t i = 0; i < nCount; ++i)
    {
        AutoTextGroup* pGroup = aGroupArr[i];
        for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
        {
            OUString sBlock = pGroup->sLongNames.getToken(j, STRING_DELIM);
            if( nBeginLen + 1 < sBlock.getLength() &&
                rSCmp.isEqual( sBlock.copy(0, nBeginLen), rBegin ))
            {
                pLongNames->push_back( sBlock );
                nFound++;
                if(FIND_MAX_GLOS == nFound)
                    break;
            }
        }
    }
    return nFound > 0;
}

void    SwGlossaryList::ClearGroups()
{
    const size_t nCount = aGroupArr.size();
    for( size_t i = 0; i < nCount; ++i )
        delete aGroupArr[ i ];

    aGroupArr.clear();
    bFilled = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
