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
#include <vcl/weld.hxx>
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
#include <view.hxx>

#include <vector>

#define STRING_DELIM char(0x0A)
#define GLOS_TIMEOUT 30000   // update every 30 seconds
#define FIND_MAX_GLOS 20

struct TripleString
{
    OUString sGroup;
    OUString sBlock;
    OUString sShort;
};

class SwGlossDecideDlg : public weld::GenericDialogController
{
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::TreeView> m_xListLB;

    DECL_LINK(DoubleClickHdl, weld::TreeView&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    explicit SwGlossDecideDlg(weld::Window* pParent);

    weld::TreeView& GetTreeView() {return *m_xListLB;}
};

SwGlossDecideDlg::SwGlossDecideDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/selectautotextdialog.ui", "SelectAutoTextDialog")
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xListLB(m_xBuilder->weld_tree_view("treeview"))
{
    m_xListLB->set_size_request(m_xListLB->get_approximate_digit_width() * 32,
                                m_xListLB->get_height_rows(8));
    m_xListLB->connect_row_activated(LINK(this, SwGlossDecideDlg, DoubleClickHdl));
    m_xListLB->connect_changed(LINK(this, SwGlossDecideDlg, SelectHdl));
}

IMPL_LINK_NOARG(SwGlossDecideDlg, DoubleClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwGlossDecideDlg, SelectHdl, weld::TreeView&, void)
{
    m_xOk->set_sensitive(m_xListLB->get_selected_index() != -1);
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
        AutoTextGroup* pGroup = aGroupArr[i].get();
        if(!rGroupName.isEmpty() && rGroupName != pGroup->sName)
            continue;

        sal_Int32 nPosLong = 0;
        for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
        {
            const OUString sLong = pGroup->sLongNames.getToken(0, STRING_DELIM, nPosLong);
            if(rLongName != sLong)
                continue;

            TripleString aTriple;
            aTriple.sGroup = pGroup->sName;
            aTriple.sBlock = sLong;
            aTriple.sShort = pGroup->sShortNames.getToken(j, STRING_DELIM);
            aTripleStrings.push_back(aTriple);
        }
    }

    bool bRet = false;
    nCount = aTripleStrings.size();
    if(1 == nCount)
    {
        const TripleString& rTriple(aTripleStrings.front());
        rShortName = rTriple.sShort;
        rGroupName = rTriple.sGroup;
        bRet = true;
    }
    else if(1 < nCount)
    {
        SwView *pView  = ::GetActiveView();
        SwGlossDecideDlg aDlg(pView ? pView->GetFrameWeld() : nullptr);
        OUString sTitle = aDlg.get_title() + " " + aTripleStrings.front().sBlock;
        aDlg.set_title(sTitle);

        weld::TreeView& rLB = aDlg.GetTreeView();
        for (const auto& rTriple : aTripleStrings)
            rLB.append_text(rTriple.sGroup.getToken(0, GLOS_DELIM));

        rLB.select(0);
        if (aDlg.run() == RET_OK && rLB.get_selected_index() != -1)
        {
            const TripleString& rTriple(aTripleStrings[rLB.get_selected_index()]);
            rShortName = rTriple.sShort;
            rGroupName = rTriple.sGroup;
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

OUString SwGlossaryList::GetGroupName(size_t nPos)
{
    OSL_ENSURE(aGroupArr.size() > nPos, "group not available");
    if(nPos < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nPos].get();
        OUString sRet = pGroup->sName;
        return sRet;
    }
    return OUString();
}

OUString SwGlossaryList::GetGroupTitle(size_t nPos)
{
    OSL_ENSURE(aGroupArr.size() > nPos, "group not available");
    if(nPos < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nPos].get();
        return pGroup->sTitle;
    }
    return OUString();
}

sal_uInt16 SwGlossaryList::GetBlockCount(size_t nGroup)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup].get();
        return pGroup->nCount;
    }
    return 0;
}

OUString SwGlossaryList::GetBlockLongName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup].get();
        return pGroup->sLongNames.getToken(nBlock, STRING_DELIM);
    }
    return OUString();
}

OUString SwGlossaryList::GetBlockShortName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(aGroupArr.size() > nGroup, "group not available");
    if(nGroup < aGroupArr.size())
    {
        AutoTextGroup* pGroup = aGroupArr[nGroup].get();
        return pGroup->sShortNames.getToken(nBlock, STRING_DELIM);
    }
    return OUString();
}

void SwGlossaryList::Update()
{
    if(!IsActive())
        Start();

    SvtPathOptions aPathOpt;
    const OUString& sTemp( aPathOpt.GetAutoTextPath() );
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
                std::unique_ptr<AutoTextGroup> pGroup(new AutoTextGroup);
                pGroup->sName = sGrpName;

                FillGroup(pGroup.get(), pGlossaries);
                OUString sName = rPathArr[nPath] + "/" +
                    pGroup->sName.getToken(0, GLOS_DELIM) + sExt;
                FStatHelper::GetModifiedDateTimeOfFile( sName,
                                                &pGroup->aDateModified,
                                                &pGroup->aDateModified );

                aGroupArr.insert( aGroupArr.begin(), std::move(pGroup) );
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
            std::vector<DateTime> aDateTimeArr;

            SWUnoHelper::UCB_GetFileListOfFolder( rPathArr[nPath], aFiles,
                                                    &sExt, &aDateTimeArr );
            for( size_t nFiles = 0; nFiles < aFiles.size(); ++nFiles )
            {
                const OUString aTitle = aFiles[ nFiles ];
                ::DateTime& rDT = aDateTimeArr[ nFiles ];

                OUString sName( aTitle.copy( 0, aTitle.getLength() - sExt.getLength() ));

                aFoundGroupNames.push_back(sName);
                sName += OUStringLiteral1(GLOS_DELIM) + OUString::number( static_cast<sal_uInt16>(nPath) );
                AutoTextGroup* pFound = FindGroup( sName );
                if( !pFound )
                {
                    pFound = new AutoTextGroup;
                    pFound->sName = sName;
                    FillGroup( pFound, pGlossaries );
                    pFound->aDateModified = rDT;

                    aGroupArr.push_back(std::unique_ptr<AutoTextGroup>(pFound));
                }
                else if( pFound->aDateModified < rDT )
                {
                    FillGroup(pFound, pGlossaries);
                    pFound->aDateModified = rDT;
                }
            }

            for( size_t i = aGroupArr.size(); i>0; )
            {
                --i;
                // maybe remove deleted groups
                AutoTextGroup* pGroup = aGroupArr[i].get();
                const size_t nGroupPath = static_cast<size_t>(
                    pGroup->sName.getToken( 1, GLOS_DELIM).toInt32());
                // Only the groups will be checked which are registered
                // for the current subpath.
                if( nGroupPath == nPath )
                {
                    OUString sCompareGroup = pGroup->sName.getToken(0, GLOS_DELIM);
                    bool bFound = std::any_of(aFoundGroupNames.begin(), aFoundGroupNames.end(),
                        [&sCompareGroup](const OUString& rGroupName) { return sCompareGroup == rGroupName; });

                    if(!bFound)
                    {
                        aGroupArr.erase(aGroupArr.begin() + i);
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
    for(auto & pRet : aGroupArr)
    {
        if(pRet->sName == rGroupName)
            return pRet.get();
    }
    return nullptr;
}

void SwGlossaryList::FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGlossaries)
{
    std::unique_ptr<SwTextBlocks> pBlock = pGlossaries->GetGroupDoc(pGroup->sName);
    pGroup->nCount = pBlock ? pBlock->GetCount() : 0;
    pGroup->sLongNames.clear();
    pGroup->sShortNames.clear();
    if(pBlock)
        pGroup->sTitle = pBlock->GetName();

    for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
    {
        pGroup->sLongNames += pBlock->GetLongName(j)
            + OUStringLiteral1(STRING_DELIM);
        pGroup->sShortNames += pBlock->GetShortName(j)
            + OUStringLiteral1(STRING_DELIM);
    }
}

// Give back all (not exceeding FIND_MAX_GLOS) found modules
// with matching beginning.

void SwGlossaryList::HasLongName(const OUString& rBegin, std::vector<OUString> *pLongNames)
{
    if(!bFilled)
        Update();
    sal_uInt16 nFound = 0;
    const size_t nCount = aGroupArr.size();
    sal_Int32 nBeginLen = rBegin.getLength();
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    for(size_t i = 0; i < nCount; ++i)
    {
        AutoTextGroup* pGroup = aGroupArr[i].get();
        sal_Int32 nIdx{ 0 };
        for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
        {
            OUString sBlock = pGroup->sLongNames.getToken(0, STRING_DELIM, nIdx);
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
}

void    SwGlossaryList::ClearGroups()
{
    aGroupArr.clear();
    bFilled = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
