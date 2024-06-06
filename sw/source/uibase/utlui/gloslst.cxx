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

#include <vcl/weld.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>
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

namespace {

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

    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    explicit SwGlossDecideDlg(weld::Window* pParent);

    weld::TreeView& GetTreeView() {return *m_xListLB;}
};

}

SwGlossDecideDlg::SwGlossDecideDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/swriter/ui/selectautotextdialog.ui"_ustr, u"SelectAutoTextDialog"_ustr)
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xListLB(m_xBuilder->weld_tree_view(u"treeview"_ustr))
{
    m_xListLB->set_size_request(m_xListLB->get_approximate_digit_width() * 32,
                                m_xListLB->get_height_rows(8));
    m_xListLB->connect_row_activated(LINK(this, SwGlossDecideDlg, DoubleClickHdl));
    m_xListLB->connect_changed(LINK(this, SwGlossDecideDlg, SelectHdl));
}

IMPL_LINK_NOARG(SwGlossDecideDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(SwGlossDecideDlg, SelectHdl, weld::TreeView&, void)
{
    m_xOk->set_sensitive(m_xListLB->get_selected_index() != -1);
}

SwGlossaryList::SwGlossaryList() :
    AutoTimer("SwGlossaryList"), m_bFilled(false)
{
    SvtPathOptions aPathOpt;
    m_sPath = aPathOpt.GetAutoTextPath();
    SetTimeout(GLOS_TIMEOUT);
}

SwGlossaryList::~SwGlossaryList()
{
    ClearGroups();
}

// If the GroupName is already known, then only rShortName
// will be filled. Otherwise also rGroupName will be set and
// on demand asked for the right group.

bool SwGlossaryList::GetShortName(std::u16string_view rLongName,
                                  OUString& rShortName, OUString& rGroupName )
{
    if(!m_bFilled)
        Update();

    std::vector<TripleString> aTripleStrings;

    size_t nCount = m_aGroupArr.size();
    for(size_t i = 0; i < nCount; i++ )
    {
        AutoTextGroup* pGroup = m_aGroupArr[i].get();
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
        if (!pView)
            return bRet;
        SwGlossDecideDlg aDlg(pView->GetFrameWeld());
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
    if(!m_bFilled)
        Update();
    return m_aGroupArr.size();
}

OUString SwGlossaryList::GetGroupName(size_t nPos)
{
    OSL_ENSURE(m_aGroupArr.size() > nPos, "group not available");
    if(nPos < m_aGroupArr.size())
    {
        AutoTextGroup* pGroup = m_aGroupArr[nPos].get();
        OUString sRet = pGroup->sName;
        return sRet;
    }
    return OUString();
}

OUString SwGlossaryList::GetGroupTitle(size_t nPos)
{
    OSL_ENSURE(m_aGroupArr.size() > nPos, "group not available");
    if(nPos < m_aGroupArr.size())
    {
        AutoTextGroup* pGroup = m_aGroupArr[nPos].get();
        return pGroup->sTitle;
    }
    return OUString();
}

sal_uInt16 SwGlossaryList::GetBlockCount(size_t nGroup)
{
    OSL_ENSURE(m_aGroupArr.size() > nGroup, "group not available");
    if(nGroup < m_aGroupArr.size())
    {
        AutoTextGroup* pGroup = m_aGroupArr[nGroup].get();
        return pGroup->nCount;
    }
    return 0;
}

OUString SwGlossaryList::GetBlockLongName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(m_aGroupArr.size() > nGroup, "group not available");
    if(nGroup < m_aGroupArr.size())
    {
        AutoTextGroup* pGroup = m_aGroupArr[nGroup].get();
        return pGroup->sLongNames.getToken(nBlock, STRING_DELIM);
    }
    return OUString();
}

OUString SwGlossaryList::GetBlockShortName(size_t nGroup, sal_uInt16 nBlock)
{
    OSL_ENSURE(m_aGroupArr.size() > nGroup, "group not available");
    if(nGroup < m_aGroupArr.size())
    {
        AutoTextGroup* pGroup = m_aGroupArr[nGroup].get();
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
    if(sTemp != m_sPath)
    {
        m_sPath = sTemp;
        m_bFilled = false;
        ClearGroups();
    }
    SwGlossaries* pGlossaries = ::GetGlossaries();
    const std::vector<OUString> & rPathArr = pGlossaries->GetPathArray();
    const OUString sExt( SwGlossaries::GetExtension() );
    if(!m_bFilled)
    {
        const size_t nGroupCount = pGlossaries->GetGroupCnt();
        for(size_t i = 0; i < nGroupCount; ++i)
        {
            OUString sGrpName = pGlossaries->GetGroupName(i);
            const size_t nPath = static_cast<size_t>(
                o3tl::toInt32(o3tl::getToken(sGrpName, 1, GLOS_DELIM)));
            if( nPath < rPathArr.size() )
            {
                std::unique_ptr<AutoTextGroup> pGroup(new AutoTextGroup);
                pGroup->sName = sGrpName;

                FillGroup(pGroup.get(), pGlossaries);
                OUString sName = rPathArr[nPath] + "/" +
                    o3tl::getToken(pGroup->sName, 0, GLOS_DELIM) + sExt;
                FStatHelper::GetModifiedDateTimeOfFile( sName,
                                                &pGroup->aDateModified,
                                                &pGroup->aDateModified );

                m_aGroupArr.insert( m_aGroupArr.begin(), std::move(pGroup) );
            }
        }
        m_bFilled = true;
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
                sName += OUStringChar(GLOS_DELIM) + OUString::number( o3tl::narrowing<sal_uInt16>(nPath) );
                AutoTextGroup* pFound = FindGroup( sName );
                if( !pFound )
                {
                    pFound = new AutoTextGroup;
                    pFound->sName = sName;
                    FillGroup( pFound, pGlossaries );
                    pFound->aDateModified = rDT;

                    m_aGroupArr.push_back(std::unique_ptr<AutoTextGroup>(pFound));
                }
                else if( pFound->aDateModified != rDT )
                {
                    FillGroup(pFound, pGlossaries);
                    pFound->aDateModified = rDT;
                }
            }

            for( size_t i = m_aGroupArr.size(); i>0; )
            {
                --i;
                // maybe remove deleted groups
                AutoTextGroup* pGroup = m_aGroupArr[i].get();
                const size_t nGroupPath = static_cast<size_t>(
                    o3tl::toInt32(o3tl::getToken(pGroup->sName, 1, GLOS_DELIM)));
                // Only the groups will be checked which are registered
                // for the current subpath.
                if( nGroupPath == nPath )
                {
                    std::u16string_view sCompareGroup = o3tl::getToken(pGroup->sName, 0, GLOS_DELIM);
                    bool bFound = std::any_of(aFoundGroupNames.begin(), aFoundGroupNames.end(),
                        [&sCompareGroup](const OUString& rGroupName) { return sCompareGroup == rGroupName; });

                    if(!bFound)
                    {
                        m_aGroupArr.erase(m_aGroupArr.begin() + i);
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

AutoTextGroup* SwGlossaryList::FindGroup(std::u16string_view rGroupName)
{
    for(const auto & pRet : m_aGroupArr)
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
            + OUStringChar(STRING_DELIM);
        pGroup->sShortNames += pBlock->GetShortName(j)
            + OUStringChar(STRING_DELIM);
    }
}

// Give back all (not exceeding FIND_MAX_GLOS) found modules
// with matching beginning.

void SwGlossaryList::HasLongName(const std::vector<OUString>& rBeginCandidates,
                                 std::vector<std::pair<OUString, sal_uInt16>>& rLongNames)
{
    if(!m_bFilled)
        Update();
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    // We store results for all candidate words in separate lists, so that later
    // we can sort them according to the candidate position
    std::vector<std::vector<OUString>> aResults(rBeginCandidates.size());

    // We can't break after FIND_MAX_GLOS items found, since first items may have ended up in
    // lower-priority lists, and those from higher-priority lists are yet to come. So process all.
    for (const auto& pGroup : m_aGroupArr)
    {
        sal_Int32 nIdx{ 0 };
        for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
        {
            OUString sBlock = pGroup->sLongNames.getToken(0, STRING_DELIM, nIdx);
            for (size_t i = 0; i < rBeginCandidates.size(); ++i)
            {
                const OUString& s = rBeginCandidates[i];
                if (s.getLength() + 1 < sBlock.getLength()
                    && rSCmp.isEqual(sBlock.copy(0, s.getLength()), s))
                {
                    aResults[i].push_back(sBlock);
                }
            }
        }
    }

    std::vector<std::pair<OUString, sal_uInt16>> aAllResults;
    // Sort and concatenate all result lists. See QuickHelpData::SortAndFilter
    for (size_t i = 0; i < rBeginCandidates.size(); ++i)
    {
        std::sort(aResults[i].begin(), aResults[i].end(),
                  [origWord = rBeginCandidates[i]](const OUString& s1, const OUString& s2) {
                      int nRet = s1.compareToIgnoreAsciiCase(s2);
                      if (nRet == 0)
                      {
                          // fdo#61251 sort stuff that starts with the exact rOrigWord before
                          // another ignore-case candidate
                          int n1StartsWithOrig = s1.startsWith(origWord) ? 0 : 1;
                          int n2StartsWithOrig = s2.startsWith(origWord) ? 0 : 1;
                          return n1StartsWithOrig < n2StartsWithOrig;
                      }
                      return nRet < 0;
                  });
        // All suggestions must be accompanied with length of the text they would replace
        std::transform(aResults[i].begin(), aResults[i].end(), std::back_inserter(aAllResults),
                       [nLen = sal_uInt16(rBeginCandidates[i].getLength())](const OUString& s) {
                           return std::make_pair(s, nLen);
                       });
    }

    const auto& it = std::unique(
        aAllResults.begin(), aAllResults.end(),
        [](const std::pair<OUString, sal_uInt16>& s1, const std::pair<OUString, sal_uInt16>& s2) {
            return s1.first.equalsIgnoreAsciiCase(s2.first);
        });
    if (const auto nCount = std::min<size_t>(std::distance(aAllResults.begin(), it), FIND_MAX_GLOS))
    {
        rLongNames.insert(rLongNames.end(), aAllResults.begin(), aAllResults.begin() + nCount);
    }
}

void    SwGlossaryList::ClearGroups()
{
    m_aGroupArr.clear();
    m_bFilled = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
