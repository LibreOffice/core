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



String createEntryString(const ScRangeNameLine& rLine)
{
    String aRet(rLine.aName);
    aRet += '\t';
    aRet += String(rLine.aExpression);
    aRet += '\t';
    aRet += String(rLine.aScope);
    return aRet;
}

ScRangeManagerTable::ScRangeManagerTable( Window* pWindow, boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap ):
    SvTabListBox( pWindow, WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP ),
    maHeaderBar( pWindow, WB_BUTTONSTYLE | WB_BOTTOMBORDER ),
    maGlobalString( ResId::toString(ScResId(STR_GLOBAL_SCOPE)))
{
    Size aBoxSize( pWindow->GetOutputSizePixel() );

    maHeaderBar.SetPosSizePixel( Point(0, 0), Size( aBoxSize.Width(), 16 ) );

    String aNameStr(ScResId(STR_HEADER_NAME));
    String aRangeStr(ScResId(STR_HEADER_RANGE));
    String aScopeStr(ScResId(STR_HEADER_SCOPE));

    long nTabSize = aBoxSize.Width()/3;
    maHeaderBar.InsertItem( ITEMID_NAME, aNameStr, nTabSize, HIB_LEFT| HIB_VCENTER );
    maHeaderBar.InsertItem( ITEMID_RANGE, aRangeStr, nTabSize, HIB_LEFT| HIB_VCENTER );
    maHeaderBar.InsertItem( ITEMID_SCOPE, aScopeStr, nTabSize, HIB_LEFT| HIB_VCENTER );

    static long nTabs[] = {3, 0, nTabSize, 2*nTabSize };
    Size aHeadSize( maHeaderBar.GetSizePixel() );

    //pParent->SetFocusControl( this );
    SetPosSizePixel( Point( 0, aHeadSize.Height() ), Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    SetTabs( &nTabs[0], MAP_PIXEL );

    Show();
    maHeaderBar.Show();
    SetSelectionMode(MULTIPLE_SELECTION);
    Init(rRangeMap);
}

void ScRangeManagerTable::addEntry(const ScRangeNameLine& rLine)
{
    SvLBoxEntry* pEntry = InsertEntryToColumn( createEntryString(rLine), LIST_APPEND, 0xffff);
    SetCurEntry(pEntry);
}

void ScRangeManagerTable::GetCurrentLine(ScRangeNameLine& rLine)
{
    SvLBoxEntry* pCurrentEntry = GetCurEntry();
    GetLine(rLine, pCurrentEntry);
}

void ScRangeManagerTable::GetLine(ScRangeNameLine& rLine, SvLBoxEntry* pEntry)
{
    rLine.aName = GetEntryText( pEntry, 0);
    rLine.aExpression = GetEntryText(pEntry, 1);
    rLine.aScope = GetEntryText(pEntry, 2);
}

void ScRangeManagerTable::Init(const boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap)
{
    Clear();
    for (boost::ptr_map<rtl::OUString, ScRangeName>::const_iterator itr = rRangeMap.begin();
            itr != rRangeMap.end(); ++itr)
    {
        const ScRangeName* pLocalRangeName = itr->second;
        ScRangeNameLine aLine;
        if (itr->first == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))
            aLine.aScope = maGlobalString;
        else
            aLine.aScope = itr->first;
        for (ScRangeName::iterator it = pLocalRangeName->begin();
                it != pLocalRangeName->end(); ++it)
        {
            if (!it->HasType(RT_DATABASE) && !it->HasType(RT_SHARED))
            {
                aLine.aName = it->GetName();
                it->GetSymbol(aLine.aExpression);
                addEntry(aLine);
            }
        }
    }
}

void ScRangeManagerTable::DeleteSelectedEntries()
{
    RemoveSelection();
}

bool ScRangeManagerTable::IsMultiSelection()
{
    return GetSelectionCount() > 1;
}

std::vector<ScRangeNameLine> ScRangeManagerTable::GetSelectedEntries()
{
    std::vector<ScRangeNameLine> aSelectedEntries;
    for (SvLBoxEntry* pEntry = FirstSelected(); pEntry != LastSelected(); pEntry = NextSelected(pEntry))
    {
        ScRangeNameLine aLine;
        GetLine( aLine, pEntry );
        aSelectedEntries.push_back(aLine);
    }
    SvLBoxEntry* pEntry = LastSelected();
    ScRangeNameLine aLine;
    GetLine( aLine, pEntry );
    aSelectedEntries.push_back(aLine);
    return aSelectedEntries;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
