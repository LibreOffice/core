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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/boxitem.hxx>

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

sal_Bool __EXPORT ScTabViewShell::HasSelection( sal_Bool bText ) const
{
    sal_Bool bHas = sal_False;
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
        ScMarkType eMarkType = pData->GetSimpleArea( aRange );
        if ( eMarkType == SC_MARK_SIMPLE )
            bHas = ( aRange.aStart != aRange.aEnd );    // more than 1 cell
        else
            bHas = sal_True;                                // multiple selection or filtered
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


