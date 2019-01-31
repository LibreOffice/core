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

#include <docary.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <editsh.hxx>
#include <edimp.hxx>
#include <frmtool.hxx>

RedlineFlags SwEditShell::GetRedlineFlags() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
}

void SwEditShell::SetRedlineFlags( RedlineFlags eMode )
{
    if( eMode != GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eMode );
        EndAllAction();
    }
}

bool SwEditShell::IsRedlineOn() const
{
    return GetDoc()->getIDocumentRedlineAccess().IsRedlineOn();
}

SwRedlineTable::size_type SwEditShell::GetRedlineCount() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().size();
}

const SwRangeRedline& SwEditShell::GetRedline( SwRedlineTable::size_type nPos ) const
{
    return *GetDoc()->getIDocumentRedlineAccess().GetRedlineTable()[ nPos ];
}

static void lcl_InvalidateAll( SwViewShell* pSh )
{
    for(SwViewShell& rCurrentShell : pSh->GetRingContainer())
    {
        if ( rCurrentShell.GetWin() )
            rCurrentShell.GetWin()->Invalidate();
    }
}

bool SwEditShell::AcceptRedline( SwRedlineTable::size_type nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().AcceptRedline( nPos, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

bool SwEditShell::RejectRedline( SwRedlineTable::size_type nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().RejectRedline( nPos, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

bool SwEditShell::AcceptRedlinesInSelection()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().AcceptRedline( *GetCursor(), true );
    EndAllAction();
    return bRet;
}

bool SwEditShell::RejectRedlinesInSelection()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().RejectRedline( *GetCursor(), true );
    EndAllAction();
    return bRet;
}

// Set the comment at the Redline
bool SwEditShell::SetRedlineComment( const OUString& rS )
{
    bool bRet = false;
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        bRet = bRet || GetDoc()->getIDocumentRedlineAccess().SetRedlineComment( rPaM, rS );
    }

    return bRet;
}

const SwRangeRedline* SwEditShell::GetCurrRedline() const
{
    if (const SwRangeRedline* pRed = GetDoc()->getIDocumentRedlineAccess().GetRedline( *GetCursor()->GetPoint(), nullptr ))
        return pRed;
    // check the other side of the selection to handle completely selected changes, where the Point is at the end
    return GetDoc()->getIDocumentRedlineAccess().GetRedline( *GetCursor()->GetMark(), nullptr );
}

void SwEditShell::UpdateRedlineAttr()
{
    if( IDocumentRedlineAccess::IsShowChanges(GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags()) )
    {
        SET_CURR_SHELL( this );
        StartAllAction();

        GetDoc()->getIDocumentRedlineAccess().UpdateRedlineAttr();

        EndAllAction();
    }
}

/** Search the Redline of the data given
 *
 * @return Returns the Pos of the Array, or SwRedlineTable::npos if not present
 */
SwRedlineTable::size_type SwEditShell::FindRedlineOfData( const SwRedlineData& rData ) const
{
    const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();

    for( SwRedlineTable::size_type i = 0, nCnt = rTable.size(); i < nCnt; ++i )
        if( &rTable[ i ]->GetRedlineData() == &rData )
            return i;
    return SwRedlineTable::npos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
