/*************************************************************************
 *
 *  $RCSfile: edatmisc.cxx,v $
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

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // fuer aNodes
#endif
#ifndef _PAM_HXX
#include <pam.hxx>      // fuer SwPaM
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>    // fuer MACROS
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>   // fuer die UndoIds
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>    // fuer Get-/ChgFmt Set-/GetAttrXXX
#endif



/*************************************
 * harte Formatierung (Attribute)
 *************************************/


void SwEditShell::ResetAttr()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    BOOL bUndoGroup = GetCrsr()->GetNext() != GetCrsr();
    if( bUndoGroup )
        GetDoc()->StartUndo(UNDO_RESETATTR);

        FOREACHPAM_START(this)
            // if ( PCURCRSR->HasMark() )
                GetDoc()->ResetAttr(*PCURCRSR);
        FOREACHPAM_END()

    if( bUndoGroup )
        GetDoc()->EndUndo(UNDO_RESETATTR);

    EndAllAction();

}



void SwEditShell::GCAttr()
{
//JP 04.02.97: wozu eine Action-Klammerung - ein Formatierung sollte nicht
//              ausgeloest werden, so dass es hier ueberfluessig ist.
//              Sonst Probleme im MouseBut.DownHdl - Bug 35562
//  StartAllAction();
    FOREACHPAM_START(this)
        SwTxtNode *pTxtNode;
        if ( !PCURCRSR->HasMark() )
        {
            if( 0 != (pTxtNode = GetDoc()->GetNodes()[
                                PCURCRSR->GetPoint()->nNode]->GetTxtNode()))
                pTxtNode->GCAttr();
        }
        else
        {
            const SwNodeIndex& rEnd = PCURCRSR->End()->nNode;
            SwNodeIndex aIdx( PCURCRSR->Start()->nNode );
            SwNode* pNd = &aIdx.GetNode();
            do {
                if( pNd->IsTxtNode() )
                    ((SwTxtNode*)pNd)->GCAttr();
            }
            while( 0 != ( pNd = GetDoc()->GetNodes().GoNext( &aIdx )) &&
                    aIdx <= rEnd );
        }
    FOREACHPAM_END()
//  EndAllAction();
}

// Setze das Attribut als neues default Attribut im Dokument.


void SwEditShell::SetDefault( const SfxPoolItem& rFmtHint )
{
    // 7502: Action-Klammerung
    StartAllAction();
    GetDoc()->SetDefault( rFmtHint );
    EndAllAction();
}

/*

void SwEditShell::SetDefault( const SfxItemSet& rSet )
{
    // 7502: Action-Klammerung
    StartAllAction();
    GetDoc()->SetDefault( rSet );
    EndAllAction();
}
*/

// Erfrage das Default Attribut in diesem Dokument.

const SfxPoolItem& SwEditShell::GetDefault( USHORT nFmtHint ) const
{
    return GetDoc()->GetDefault( nFmtHint );

}


void SwEditShell::SetAttr( const SfxPoolItem& rHint, USHORT nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        FASTBOOL bIsTblMode = IsTableMode();
        GetDoc()->StartUndo(UNDO_INSATTR);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && ( bIsTblMode ||
                *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() ))
                GetDoc()->Insert(*PCURCRSR, rHint, nFlags );
        FOREACHPAM_END()

        GetDoc()->EndUndo(UNDO_INSATTR);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->Insert( *pCrsr, rHint, nFlags );
    }
    EndAllAction();
}


void SwEditShell::SetAttr( const SfxItemSet& rSet, USHORT nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        FASTBOOL bIsTblMode = IsTableMode();
        GetDoc()->StartUndo(UNDO_INSATTR);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && ( bIsTblMode ||
                *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() ))
                GetDoc()->Insert(*PCURCRSR, rSet, nFlags );
        FOREACHPAM_END()

        GetDoc()->EndUndo(UNDO_INSATTR);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->Insert( *pCrsr, rSet, nFlags );
    }
    EndAllAction();
}




