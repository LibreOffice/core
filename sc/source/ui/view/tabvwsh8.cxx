/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tabvwsh8.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/boxitem.hxx>

#include "tabvwsh.hxx"
#include "document.hxx"


//------------------------------------------------------------------

void ScTabViewShell::SetDefaultFrameLine( const SvxBorderLine* pLine )
{
    if ( pLine )
    {
        delete pCurFrameLine;
        pCurFrameLine = new SvxBorderLine( &pLine->GetColor(),
                                            pLine->GetOutWidth(),
                                            pLine->GetInWidth(),
                                            pLine->GetDistance() );
    }
    else if ( pCurFrameLine )
    {
        delete pCurFrameLine;
        pCurFrameLine = NULL;
    }
}

//------------------------------------------------------------------

BOOL __EXPORT ScTabViewShell::HasSelection( BOOL bText ) const
{
    BOOL bHas = FALSE;
    ScViewData* pData = (ScViewData*)GetViewData();     // const weggecasted
    if ( bText )
    {
        //  Text enthalten: Anzahl2 >= 1
        ScDocument* pDoc = pData->GetDocument();
        ScMarkData& rMark = pData->GetMarkData();
        ScAddress aCursor( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
        double fVal = 0.0;
        if ( pDoc->GetSelectionFunction( SUBTOTAL_FUNC_CNT2, aCursor, rMark, fVal ) )
            bHas = ( fVal > 0.5 );
    }
    else
    {
        ScRange aRange;
        if ( pData->GetSimpleArea( aRange ) )
            bHas = ( aRange.aStart != aRange.aEnd );    // mehr als 1 Zelle
        else
            bHas = TRUE;    // Mehrfachselektion
    }
    return bHas;
}

//------------------------------------------------------------------

void ScTabViewShell::UIDeactivated( SfxInPlaceClient* pClient )
{
    ClearHighlightRanges();

    //  Move an der ViewShell soll eigentlich vom Sfx gerufen werden, wenn sich
    //  das Frame-Window wegen unterschiedlicher Toolboxen o.ae. verschiebt
    //  (um nicht aus Versehen z.B. Zeichenobjekte zu verschieben, #56515#).
    //  Dieser Mechanismus funktioniert aber momentan nicht, darum hier der Aufruf
    //  per Hand (im Move wird verglichen, ob die Position wirklich geaendert ist).
    ForceMove();
    SfxViewShell::UIDeactivated( pClient );
}


