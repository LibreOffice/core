/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *  Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

//ScRangeManagerTable
#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "namedlg.hrc"
#include "namedlg.hxx"
#include "viewdata.hxx"
#include "globalnames.hxx"

#include "sfx2/app.hxx"

#define ITEMID_NAME 1
#define ITEMID_RANGE 2
#define ITEMID_SCOPE 3

#define MINSIZE 80


String createEntryString(const ScRangeNameLine& rLine)
{
    String aRet(rLine.aName);
    aRet += '\t';
    aRet += String(rLine.aExpression);
    aRet += '\t';
    aRet += String(rLine.aScope);
    return aRet;
}

ScRangeManagerTable::ScRangeManagerTable( Window* pWindow, boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap, const ScAddress& rPos ):
    SvTabListBox( pWindow, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP ),
    maHeaderBar( pWindow, WB_BUTTONSTYLE | WB_BOTTOMBORDER ),
    maGlobalString( ScGlobal::GetRscString(STR_GLOBAL_SCOPE)),
    mrRangeMap( rRangeMap ),
    maPos( rPos )
{
    Size aBoxSize( pWindow->GetOutputSizePixel() );

    maHeaderBar.SetPosSizePixel( Point(0, 0), Size( aBoxSize.Width(), 16 ) );

    String aNameStr(ScGlobal::GetRscString(STR_HEADER_NAME));
    String aRangeStr(ScGlobal::GetRscString(STR_HEADER_RANGE));
    String aScopeStr(ScGlobal::GetRscString(STR_HEADER_SCOPE));

    long nTabSize = aBoxSize.Width()/3;
    maHeaderBar.InsertItem( ITEMID_NAME, aNameStr, nTabSize, HIB_LEFT| HIB_VCENTER );
    maHeaderBar.InsertItem( ITEMID_RANGE, aRangeStr, nTabSize, HIB_LEFT| HIB_VCENTER );
    maHeaderBar.InsertItem( ITEMID_SCOPE, aScopeStr, nTabSize, HIB_LEFT| HIB_VCENTER );

    static long nTabs[] = {3, 0, nTabSize, 2*nTabSize };
    Size aHeadSize( maHeaderBar.GetSizePixel() );

    //pParent->SetFocusControl( this );
    SetPosSizePixel( Point( 0, aHeadSize.Height() ), Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    SetTabs( &nTabs[0], MAP_PIXEL );

    maHeaderBar.SetEndDragHdl( LINK( this, ScRangeManagerTable, HeaderEndDragHdl ) );

    Init();
    Show();
    maHeaderBar.Show();
    SetSelectionMode(MULTIPLE_SELECTION);
    if (GetEntryCount())
    {
        SetCurEntry(GetEntryOnPos(0));
        CheckForFormulaString();
    }
    SetScrolledHdl( LINK( this, ScRangeManagerTable, ScrollHdl ) );
    void* pNull = NULL;
    HeaderEndDragHdl(pNull);
}

ScRangeManagerTable::~ScRangeManagerTable()
{
    Clear();
}

void ScRangeManagerTable::addEntry(const ScRangeNameLine& rLine, bool bSetCurEntry)
{
    SvTreeListEntry* pEntry = InsertEntryToColumn( createEntryString(rLine), LIST_APPEND, 0xffff);
    if (bSetCurEntry)
        SetCurEntry(pEntry);
}

void ScRangeManagerTable::GetCurrentLine(ScRangeNameLine& rLine)
{
    SvTreeListEntry* pCurrentEntry = GetCurEntry();
    GetLine(rLine, pCurrentEntry);
}

void ScRangeManagerTable::GetLine(ScRangeNameLine& rLine, SvTreeListEntry* pEntry)
{
    rLine.aName = GetEntryText( pEntry, 0);
    rLine.aExpression = GetEntryText(pEntry, 1);
    rLine.aScope = GetEntryText(pEntry, 2);
}

void ScRangeManagerTable::Init()
{
    SetUpdateMode(false);
    Clear();
    for (boost::ptr_map<rtl::OUString, ScRangeName>::const_iterator itr = mrRangeMap.begin();
            itr != mrRangeMap.end(); ++itr)
    {
        const ScRangeName* pLocalRangeName = itr->second;
        ScRangeNameLine aLine;
        if ( itr->first == STR_GLOBAL_RANGE_NAME )
            aLine.aScope = maGlobalString;
        else
            aLine.aScope = itr->first;
        for (ScRangeName::const_iterator it = pLocalRangeName->begin();
                it != pLocalRangeName->end(); ++it)
        {
            if (!it->second->HasType(RT_DATABASE) && !it->second->HasType(RT_SHARED))
            {
                aLine.aName = it->second->GetName();
                addEntry(aLine, false);
            }
        }
    }
    SetUpdateMode(true);
}

const ScRangeData* ScRangeManagerTable::findRangeData(const ScRangeNameLine& rLine)
{
    const ScRangeName* pRangeName;
    if (rLine.aScope == maGlobalString)
        pRangeName = mrRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    else
        pRangeName = mrRangeMap.find(rLine.aScope)->second;

    return pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(rLine.aName));
}



void ScRangeManagerTable::CheckForFormulaString()
{
    for (SvTreeListEntry* pEntry = GetFirstEntryInView(); pEntry ; pEntry = GetNextEntryInView(pEntry))
    {
        std::map<SvTreeListEntry*, bool>::const_iterator itr = maCalculatedFormulaEntries.find(pEntry);
        if (itr == maCalculatedFormulaEntries.end() || itr->second == false)
        {
            ScRangeNameLine aLine;
            GetLine( aLine, pEntry);
            const ScRangeData* pData = findRangeData( aLine );
            rtl::OUString aFormulaString;
            pData->GetSymbol(aFormulaString, maPos);
            SetEntryText(aFormulaString, pEntry, 1);
            maCalculatedFormulaEntries.insert( std::pair<SvTreeListEntry*, bool>(pEntry, true) );
        }

    }
}

void ScRangeManagerTable::DeleteSelectedEntries()
{
    if (GetSelectionCount())
        RemoveSelection();
}

bool ScRangeManagerTable::IsMultiSelection()
{
    return GetSelectionCount() > 1;
}

std::vector<ScRangeNameLine> ScRangeManagerTable::GetSelectedEntries()
{
    std::vector<ScRangeNameLine> aSelectedEntries;
    if (GetSelectionCount())
    {
        for (SvTreeListEntry* pEntry = FirstSelected(); pEntry != LastSelected(); pEntry = NextSelected(pEntry))
        {
            ScRangeNameLine aLine;
            GetLine( aLine, pEntry );
            aSelectedEntries.push_back(aLine);
        }
        SvTreeListEntry* pEntry = LastSelected();
        ScRangeNameLine aLine;
        GetLine( aLine, pEntry );
        aSelectedEntries.push_back(aLine);
    }
    return aSelectedEntries;
}

void ScRangeManagerTable::SetEntry(const ScRangeNameLine& rLine)
{
    for (SvTreeListEntry* pEntry = First(); pEntry; pEntry = Next(pEntry))
    {
        if (rLine.aName == rtl::OUString(GetEntryText(pEntry, 0))
                && rLine.aScope == rtl::OUString(GetEntryText(pEntry, 2)))
        {
            SetCurEntry(pEntry);
        }
    }
}

namespace {

//ensure that the minimum column size is respected
void CalculateItemSize(const long& rTableSize, long& rItemNameSize, long& rItemRangeSize)
{
    long aItemScopeSize = rTableSize - rItemNameSize - rItemRangeSize;

    if (rItemNameSize >= MINSIZE && rItemRangeSize >= MINSIZE && aItemScopeSize >= MINSIZE)
        return;

    if (rItemNameSize < MINSIZE)
    {
        long aDiffSize = MINSIZE - rItemNameSize;
        if (rItemRangeSize > aItemScopeSize)
            rItemRangeSize -= aDiffSize;
        else
            aItemScopeSize -= aDiffSize;
        rItemNameSize = MINSIZE;
    }

    if (rItemRangeSize < MINSIZE)
    {
        long aDiffSize = MINSIZE - rItemRangeSize;
        if (rItemNameSize > aItemScopeSize)
            rItemNameSize -= aDiffSize;
        else
            aItemScopeSize -= aDiffSize;
        rItemRangeSize = MINSIZE;
    }

    if (aItemScopeSize < MINSIZE)
    {
        long aDiffSize = MINSIZE - aItemScopeSize;
        if (rItemNameSize > rItemRangeSize)
            rItemNameSize -= aDiffSize;
        else
            rItemRangeSize -= aDiffSize;
    }
}

}

IMPL_LINK_NOARG(ScRangeManagerTable, HeaderEndDragHdl)
{
    long aTableSize = maHeaderBar.GetSizePixel().Width();
    long aItemNameSize = maHeaderBar.GetItemSize(ITEMID_NAME);
    long aItemRangeSize = maHeaderBar.GetItemSize(ITEMID_RANGE);

    //calculate column size based on user input and minimum size
    CalculateItemSize(aTableSize, aItemNameSize, aItemRangeSize);
    long aItemScopeSize = aTableSize - aItemNameSize - aItemRangeSize;

    Size aSz;
    aSz.Width() = aItemNameSize;
    SetTab( ITEMID_NAME, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
    maHeaderBar.SetItemSize(ITEMID_NAME, aItemNameSize);
    aSz.Width() += aItemRangeSize;
    SetTab( ITEMID_RANGE, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
    maHeaderBar.SetItemSize(ITEMID_RANGE, aItemRangeSize);
    aSz.Width() += aItemScopeSize;
    SetTab( ITEMID_SCOPE, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width(), MAP_APPFONT );
    maHeaderBar.SetItemSize(ITEMID_SCOPE, aItemScopeSize);

    return 0;
}

IMPL_LINK_NOARG(ScRangeManagerTable, ScrollHdl)
{
    CheckForFormulaString();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
