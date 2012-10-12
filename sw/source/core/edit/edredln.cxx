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
    return GetDoc()->GetRedlineMode();
}

void SwEditShell::SetRedlineMode( sal_uInt16 eMode )
{
    if( eMode != GetDoc()->GetRedlineMode() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetRedlineMode( (RedlineMode_t)eMode );
        EndAllAction();
    }
}

sal_Bool SwEditShell::IsRedlineOn() const
{
    return GetDoc()->IsRedlineOn();
}

sal_uInt16 SwEditShell::GetRedlineCount() const
{
    return GetDoc()->GetRedlineTbl().size();
}

const SwRedline& SwEditShell::GetRedline( sal_uInt16 nPos ) const
{
    return *GetDoc()->GetRedlineTbl()[ nPos ];
}

static void lcl_InvalidateAll( ViewShell* pSh )
{
    ViewShell *pStop = pSh;
    do
    {
        if ( pSh->GetWin() )
            pSh->GetWin()->Invalidate();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != pStop );
}

sal_Bool SwEditShell::AcceptRedline( sal_uInt16 nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    sal_Bool bRet = GetDoc()->AcceptRedline( nPos, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

sal_Bool SwEditShell::RejectRedline( sal_uInt16 nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    sal_Bool bRet = GetDoc()->RejectRedline( nPos, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

// Kommentar am Redline setzen
sal_Bool SwEditShell::SetRedlineComment( const String& rS )
{
    sal_Bool bRet = sal_False;
    FOREACHPAM_START(this)
        bRet = bRet || GetDoc()->SetRedlineComment( *PCURCRSR, rS );
    FOREACHPAM_END()

    return bRet;
}

const SwRedline* SwEditShell::GetCurrRedline() const
{
    return GetDoc()->GetRedline( *GetCrsr()->GetPoint(), 0 );
}

void SwEditShell::UpdateRedlineAttr()
{
    if( ( nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE ) ==
        ( nsRedlineMode_t::REDLINE_SHOW_MASK & GetDoc()->GetRedlineMode() ))
    {
        SET_CURR_SHELL( this );
        StartAllAction();

        GetDoc()->UpdateRedlineAttr();

        EndAllAction();
    }
}

    // suche das Redline zu diesem Data und returne die Pos im Array
    // USHRT_MAX wird returnt, falls nicht vorhanden
sal_uInt16 SwEditShell::FindRedlineOfData( const SwRedlineData& rData ) const
{
    const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();

    for( sal_uInt16 i = 0, nCnt = rTbl.size(); i < nCnt; ++i )
        if( &rTbl[ i ]->GetRedlineData() == &rData )
            return i;
    return USHRT_MAX;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
