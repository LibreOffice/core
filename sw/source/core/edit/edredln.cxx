/*************************************************************************
 *
 *  $RCSfile: edredln.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#include "redline.hxx"
#include "doc.hxx"
#include "swundo.hxx"
#include "editsh.hxx"
#include "edimp.hxx"
#include "frmtool.hxx"


USHORT SwEditShell::GetRedlineMode() const
{
    return GetDoc()->GetRedlineMode();
}

void SwEditShell::SetRedlineMode( USHORT eMode )
{
    if( eMode != GetDoc()->GetRedlineMode() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetRedlineMode( eMode );
        EndAllAction();
    }
}

BOOL SwEditShell::IsRedlineOn() const
{
    return GetDoc()->IsRedlineOn();
}

USHORT SwEditShell::GetRedlineCount() const
{
    return GetDoc()->GetRedlineTbl().Count();
}

const SwRedline& SwEditShell::GetRedline( USHORT nPos ) const
{
    return *GetDoc()->GetRedlineTbl()[ nPos ];
}

void lcl_InvalidateAll( ViewShell* pSh )
{
    ViewShell *pStop = pSh;
    do
    {
        if ( pSh->GetWin() )
            pSh->GetWin()->Invalidate();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != pStop );
}

BOOL SwEditShell::AcceptRedline( USHORT nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    BOOL bRet = GetDoc()->AcceptRedline( nPos );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

BOOL SwEditShell::AcceptRedline()
{
    SET_CURR_SHELL( this );
    StartAllAction();

    GetDoc()->StartUndo();
    BOOL bRet = FALSE;
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() && GetDoc()->AcceptRedline( *PCURCRSR ) )
            bRet = TRUE;
    FOREACHPAM_END()
    GetDoc()->EndUndo();
    if( !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );

    EndAllAction();
    return bRet;
}

BOOL SwEditShell::RejectRedline( USHORT nPos )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    BOOL bRet = GetDoc()->RejectRedline( nPos );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

BOOL SwEditShell::RejectRedline()
{
    SET_CURR_SHELL( this );
    StartAllAction();

    GetDoc()->StartUndo();
    BOOL bRet = FALSE;
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() && GetDoc()->RejectRedline( *PCURCRSR ) )
            bRet = TRUE;
    FOREACHPAM_END()
    GetDoc()->EndUndo();
    if( !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );

    EndAllAction();
    return bRet;
}

// Kommentar am Redline setzen
BOOL SwEditShell::SetRedlineComment( const String& rS )
{
    BOOL bRet = FALSE;
    FOREACHPAM_START(this)
        bRet = bRet || GetDoc()->SetRedlineComment( *PCURCRSR, rS );
    FOREACHPAM_END()

    return bRet;
}

const SwRedline* SwEditShell::GetCurrRedline() const
{
    return GetDoc()->GetRedline( *GetCrsr()->GetPoint() );
}

void SwEditShell::UpdateRedlineAttr()
{
    if( ( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE ) ==
        ( REDLINE_SHOW_MASK & GetDoc()->GetRedlineMode() ))
    {
        SET_CURR_SHELL( this );
        StartAllAction();

        GetDoc()->UpdateRedlineAttr();

        EndAllAction();
    }
}

    // suche das Redline zu diesem Data und returne die Pos im Array
    // USHRT_MAX wird returnt, falls nicht vorhanden
USHORT SwEditShell::FindRedlineOfData( const SwRedlineData& rData ) const
{
    const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();

    for( USHORT i = 0, nCnt = rTbl.Count(); i < nCnt; ++i )
        if( &rTbl[ i ]->GetRedlineData() == &rData )
            return i;
    return USHRT_MAX;
}



