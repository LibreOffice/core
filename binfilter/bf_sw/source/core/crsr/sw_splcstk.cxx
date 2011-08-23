/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "crsrsh.hxx"

#include "splcstk.hxx"
#include "viscrs.hxx"
namespace binfilter {



SwSpellCrsrStk::SwSpellCrsrStk( SwCrsrShell& rShell )
    : rCrsrShell( rShell ), pErrorPos( 0 )
{
    pCurCrsr =
    pCrsrStk = new SwIntrnlCrsr( rCrsrShell,
                                 *rCrsrShell.GetCrsr()->GetPoint() );
    if( rCrsrShell.GetCrsr()->HasMark() )
    {
        pCurCrsr->SetMark();
        *pCurCrsr->GetMark() = *rCrsrShell.GetCrsr()->GetMark();
    }
        // gibt es mehr als einen Cursor ??
    if( rCrsrShell.GetCrsr()->GetNext() != rCrsrShell.GetCrsr() )
    {
        rCrsrShell.HideCrsr(); // alle Cursor erstmal verstecken

        // dann verschiebe den gesamten Ring hierher, in dem aus der Shell
        // alle ausgekettet und hier eingekettet werden. Bis auf den akt. !
        Ring* pCrsr = rCrsrShell.GetCrsr();
        pCrsr->MoveRingTo( pCurCrsr );
        pCrsr->MoveTo( pCrsr );
    }
}


SwSpellCrsrStk::~SwSpellCrsrStk()
{
    register Ring* p = pCrsrStk;
    while( p->GetNext() != pCrsrStk )
        delete p->GetNext();

    delete pCrsrStk;
    delete pErrorPos;
}


    // akt. Cursor in der CrsrShell mit akt. setzen
void SwSpellCrsrStk::SetCurCrsr()
{
    *rCrsrShell.GetCrsr()->GetPoint() = *pCurCrsr->Start();
    if( pCurCrsr->HasMark() )
    {
        rCrsrShell.GetCrsr()->SetMark();
        *rCrsrShell.GetCrsr()->GetMark() = *pCurCrsr->End();
    }
    else
        rCrsrShell.GetCrsr()->DeleteMark();
}

    // gehe zum naechsten Cursor im Ring (falls am Ende returne FALSE;


BOOL SwSpellCrsrStk::SetNextCrsr()
{
    if( pCurCrsr->GetNext() == pCrsrStk )
        return FALSE;

    pCurCrsr = (SwIntrnlCrsr*)pCurCrsr->GetNext();
    SetCurCrsr();
    return TRUE;
}


    // alle Cursor aus dem CursorStack wieder in der Shell setzen
void SwSpellCrsrStk::RestoreCrsrs()
{
    // verschiebe den gesamten Ring in die Shell, in dem hier
    // alle ausgekettet und dort eingekettet werden. Bis auf den Stack !
    if( pCrsrStk->GetNext() != pCrsrStk )
    {
        // sich selbst "ausketten"
        pCrsrStk->MoveRingTo( rCrsrShell.GetCrsr() );
        pCrsrStk->MoveTo( pCrsrStk );
    }

    pCurCrsr = pCrsrStk;

    SetCurCrsr();

    rCrsrShell.ShowCrsr(); 				// Cursor wieder anzeigen
}


    // speicher die akt. Start-Position als Error. An dieser wird spaeter
    // wieder aufgesetzt
void SwSpellCrsrStk::StoreErrPos()
{
    if( !pErrorPos )
        pErrorPos = new SwPosition( *rCrsrShell.GetCrsr()->Start() );
    else
        *pErrorPos = *rCrsrShell.GetCrsr()->Start();
}


    // akt. Cursor auf die Error-Position setzen
void SwSpellCrsrStk::SetErrPos()
{
    if( !pErrorPos )
        return;

    *rCrsrShell.GetCrsr()->GetPoint() = *pErrorPos;
    if( pCurCrsr->HasMark() )
    {
        rCrsrShell.GetCrsr()->SetMark();
        *rCrsrShell.GetCrsr()->GetMark() = *pCurCrsr->End();
    }
    else
        rCrsrShell.GetCrsr()->DeleteMark();
}


#if 0

SwPaM* SwSpellCrsrStk::GetCurCrsr() const { return pCurCrsr; }
#endif



}
