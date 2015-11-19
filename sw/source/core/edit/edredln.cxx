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
#include <vcl/window.hxx>
#include "redline.hxx"
#include "doc.hxx"
#include "swundo.hxx"
#include "editsh.hxx"
#include "edimp.hxx"
#include "frmtool.hxx"

sal_uInt16 SwEditShell::GetRedlineMode() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineMode();
}

void SwEditShell::SetRedlineMode( sal_uInt16 eMode )
{
    if( eMode != GetDoc()->getIDocumentRedlineAccess().GetRedlineMode() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->getIDocumentRedlineAccess().SetRedlineMode( (RedlineMode_t)eMode );
        EndAllAction();
    }
}

bool SwEditShell::IsRedlineOn() const
{
    return GetDoc()->getIDocumentRedlineAccess().IsRedlineOn();
}

sal_uInt16 SwEditShell::GetRedlineCount() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().size();
}

const SwRangeRedline& SwEditShell::GetRedline( sal_uInt16 nPos ) const
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

bool SwEditShell::AcceptRedline( sal_uInt16 nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().AcceptRedline( nPos, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

bool SwEditShell::RejectRedline( sal_uInt16 nPos )
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
    return GetDoc()->getIDocumentRedlineAccess().GetRedline( *GetCursor()->GetPoint(), nullptr );
}

void SwEditShell::UpdateRedlineAttr()
{
    if( ( nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE ) ==
        ( nsRedlineMode_t::REDLINE_SHOW_MASK & GetDoc()->getIDocumentRedlineAccess().GetRedlineMode() ))
    {
        SET_CURR_SHELL( this );
        StartAllAction();

        GetDoc()->getIDocumentRedlineAccess().UpdateRedlineAttr();

        EndAllAction();
    }
}

/** Search the Redline of the data given
 *
 * @return Returns the Pos of the Array, or USHRT_MAX if not present
 */
sal_uInt16 SwEditShell::FindRedlineOfData( const SwRedlineData& rData ) const
{
    const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();

    for( sal_uInt16 i = 0, nCnt = rTable.size(); i < nCnt; ++i )
        if( &rTable[ i ]->GetRedlineData() == &rData )
            return i;
    return USHRT_MAX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
