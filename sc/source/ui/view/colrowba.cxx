/*************************************************************************
 *
 *  $RCSfile: colrowba.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:58:57 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/svdtrans.hxx>

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#include "colrowba.hxx"
#include "document.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "appoptio.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

//==================================================================

String lcl_MetricString( long nTwips, const String& rText )
{
    if ( nTwips <= 0 )
        return ScGlobal::GetRscString(STR_TIP_HIDE);
    else
    {
        FieldUnit eUserMet = SC_MOD()->GetAppOptions().GetAppMetric();

        long nUserVal = MetricField::ConvertValue( nTwips*100, 1, 2, FUNIT_TWIP, eUserMet );

        String aStr = rText;
        aStr += ' ';
        aStr += ScGlobal::pLocaleData->getNum( nUserVal, 2 );
        aStr += ' ';
        aStr += SdrFormatter::GetUnitStr(eUserMet);

        return aStr;
    }
}

//==================================================================

ScColBar::ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                    ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng ) :
            ScHeaderControl( pParent, pEng, MAXCOL+1, HDR_HORIZONTAL ),
            pViewData( pData ),
            eWhich( eWhichPos ),
            pFuncSet( pFunc ),
            pSelEngine( pEng )
{
    Show();
}

ScColBar::~ScColBar()
{
}

SCCOLROW ScColBar::GetPos()
{
    return pViewData->GetPosX(eWhich);
}

USHORT ScColBar::GetEntrySize( SCCOLROW nEntryNo )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if ( pDoc->GetColFlags( static_cast<SCCOL>(nEntryNo), nTab ) & CR_HIDDEN )
        return 0;
    else
        return (USHORT) ScViewData::ToPixel( pDoc->GetColWidth( static_cast<SCCOL>(nEntryNo), nTab ), pViewData->GetPPTX() );
}

String ScColBar::GetEntryText( SCCOLROW nEntryNo )
{
    return ColToAlpha( static_cast<SCCOL>(nEntryNo) );
}

void ScColBar::SetEntrySize( SCCOLROW nPos, USHORT nNewSize )
{
    USHORT nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize>0 && nNewSize<10) nNewSize=10;             // (Pixel)

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = STD_EXTRA_WIDTH;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (USHORT) ( nNewSize / pViewData->GetPPTX() );

    ScMarkData& rMark = pViewData->GetMarkData();
//  SCTAB nTab = pViewData->GetTabNo();

    SCCOLROW* pRanges = new SCCOLROW[MAXCOL+1];
    SCCOL nRangeCnt = 0;
    if ( rMark.IsColumnMarked( static_cast<SCCOL>(nPos) ) )
    {
        SCCOL nStart = 0;
        while (nStart<=MAXCOL)
        {
            while (nStart<MAXCOL && !rMark.IsColumnMarked(nStart))
                ++nStart;
            if (rMark.IsColumnMarked(nStart))
            {
                SCCOL nEnd = nStart;
                while (nEnd<MAXCOL && rMark.IsColumnMarked(nEnd))
                    ++nEnd;
                if (!rMark.IsColumnMarked(nEnd))
                    --nEnd;
                pRanges[static_cast<size_t>(2*nRangeCnt)  ] = nStart;
                pRanges[static_cast<size_t>(2*nRangeCnt+1)] = nEnd;
                ++nRangeCnt;
                nStart = nEnd+1;
            }
            else
                nStart = MAXCOL+1;
        }
    }
    else
    {
        pRanges[0] = nPos;
        pRanges[1] = nPos;
        nRangeCnt = 1;
    }

    pViewData->GetView()->SetWidthOrHeight( TRUE, nRangeCnt, pRanges, eMode, nSizeTwips );
    delete[] pRanges;
}

void ScColBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    SCCOLROW nRange[2];
    nRange[0] = nStart;
    nRange[1] = nEnd;
    pViewData->GetView()->SetWidthOrHeight( TRUE, 1, nRange, SC_SIZE_DIRECT, 0 );
}

void ScColBar::SetMarking( BOOL bSet )
{
    pViewData->GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pViewData->GetView()->UpdateAutoFillMark();
    }
}

void ScColBar::SelectWindow()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    pViewSh->SetActive();           // Appear und SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = pViewData->GetActivePart();
    if (eWhich==SC_SPLIT_LEFT)
    {
        if (eActive==SC_SPLIT_TOPRIGHT)     eActive=SC_SPLIT_TOPLEFT;
        if (eActive==SC_SPLIT_BOTTOMRIGHT)  eActive=SC_SPLIT_BOTTOMLEFT;
    }
    else
    {
        if (eActive==SC_SPLIT_TOPLEFT)      eActive=SC_SPLIT_TOPRIGHT;
        if (eActive==SC_SPLIT_BOTTOMLEFT)   eActive=SC_SPLIT_BOTTOMRIGHT;
    }
    pViewSh->ActivatePart( eActive );

    pFuncSet->SetColumn( TRUE );
    pFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

BOOL ScColBar::IsDisabled()
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

BOOL ScColBar::ResizeAllowed()
{
    return !pViewData->HasEditView( pViewData->GetActivePart() ) &&
            !pViewData->GetDocShell()->IsReadOnly();
}

void ScColBar::DrawInvert( long nDragPos )
{
    Rectangle aRect( nDragPos,0, nDragPos+HDR_SLIDERSIZE-1,GetOutputSizePixel().Width()-1 );
    Update();
    Invert(aRect);

    pViewData->GetView()->InvertVertical(eWhich,nDragPos);
}

String ScColBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTX() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_WIDTH) );
}

BOOL ScColBar::IsLayoutRTL()        // overloaded only for columns
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}

//==================================================================

ScRowBar::ScRowBar( Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
                    ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng ) :
            ScHeaderControl( pParent, pEng, MAXROW+1, HDR_VERTICAL ),
            pViewData( pData ),
            eWhich( eWhichPos ),
            pFuncSet( pFunc ),
            pSelEngine( pEng )
{
    Show();
}

ScRowBar::~ScRowBar()
{
}

SCCOLROW ScRowBar::GetPos()
{
    return pViewData->GetPosY(eWhich);
}

USHORT ScRowBar::GetEntrySize( SCCOLROW nEntryNo )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if ( pDoc->GetRowFlags( nEntryNo, nTab ) & CR_HIDDEN )
        return 0;
    else
        return (USHORT) ScViewData::ToPixel( pDoc->GetRowHeight( nEntryNo, nTab ), pViewData->GetPPTY() );
}

String ScRowBar::GetEntryText( SCCOLROW nEntryNo )
{
    return String::CreateFromInt32( nEntryNo + 1 );
}

void ScRowBar::SetEntrySize( SCCOLROW nPos, USHORT nNewSize )
{
    USHORT nSizeTwips;
    ScSizeMode eMode = SC_SIZE_DIRECT;
    if (nNewSize>0 && nNewSize<10) nNewSize=10;             // (Pixel)

    if ( nNewSize == HDR_SIZE_OPTIMUM )
    {
        nSizeTwips = 0;
        eMode = SC_SIZE_OPTIMAL;
    }
    else
        nSizeTwips = (USHORT) ( nNewSize / pViewData->GetPPTY() );

    ScMarkData& rMark = pViewData->GetMarkData();
//  SCTAB nTab = pViewData->GetTabNo();

    SCCOLROW* pRanges = new SCCOLROW[MAXROW+1];
    SCROW nRangeCnt = 0;
    if ( rMark.IsRowMarked( nPos ) )
    {
        SCROW nStart = 0;
        while (nStart<=MAXROW)
        {
            while (nStart<MAXROW && !rMark.IsRowMarked(nStart))
                ++nStart;
            if (rMark.IsRowMarked(nStart))
            {
                SCROW nEnd = nStart;
                while (nEnd<MAXROW && rMark.IsRowMarked(nEnd))
                    ++nEnd;
                if (!rMark.IsRowMarked(nEnd))
                    --nEnd;
                pRanges[static_cast<size_t>(2*nRangeCnt)  ] = nStart;
                pRanges[static_cast<size_t>(2*nRangeCnt+1)] = nEnd;
                ++nRangeCnt;
                nStart = nEnd+1;
            }
            else
                nStart = MAXROW+1;
        }
    }
    else
    {
        pRanges[0] = nPos;
        pRanges[1] = nPos;
        nRangeCnt = 1;
    }

    pViewData->GetView()->SetWidthOrHeight( FALSE, nRangeCnt, pRanges, eMode, nSizeTwips );
    delete[] pRanges;
}

void ScRowBar::HideEntries( SCCOLROW nStart, SCCOLROW nEnd )
{
    SCCOLROW nRange[2];
    nRange[0] = nStart;
    nRange[1] = nEnd;
    pViewData->GetView()->SetWidthOrHeight( FALSE, 1, nRange, SC_SIZE_DIRECT, 0 );
}

void ScRowBar::SetMarking( BOOL bSet )
{
    pViewData->GetMarkData().SetMarking( bSet );
    if (!bSet)
    {
        pViewData->GetView()->UpdateAutoFillMark();
    }
}

void ScRowBar::SelectWindow()
{
    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    pViewSh->SetActive();           // Appear und SetViewFrame
    pViewSh->DrawDeselectAll();

    ScSplitPos eActive = pViewData->GetActivePart();
    if (eWhich==SC_SPLIT_TOP)
    {
        if (eActive==SC_SPLIT_BOTTOMLEFT)   eActive=SC_SPLIT_TOPLEFT;
        if (eActive==SC_SPLIT_BOTTOMRIGHT)  eActive=SC_SPLIT_TOPRIGHT;
    }
    else
    {
        if (eActive==SC_SPLIT_TOPLEFT)      eActive=SC_SPLIT_BOTTOMLEFT;
        if (eActive==SC_SPLIT_TOPRIGHT)     eActive=SC_SPLIT_BOTTOMRIGHT;
    }
    pViewSh->ActivatePart( eActive );

    pFuncSet->SetColumn( FALSE );
    pFuncSet->SetWhich( eActive );

    pViewSh->ActiveGrabFocus();
}

BOOL ScRowBar::IsDisabled()
{
    ScModule* pScMod = SC_MOD();
    return pScMod->IsFormulaMode() || pScMod->IsModalMode();
}

BOOL ScRowBar::ResizeAllowed()
{
    return !pViewData->HasEditView( pViewData->GetActivePart() ) &&
            !pViewData->GetDocShell()->IsReadOnly();
}

void ScRowBar::DrawInvert( long nDragPos )
{
    Rectangle aRect( 0,nDragPos, GetOutputSizePixel().Width()-1,nDragPos+HDR_SLIDERSIZE-1 );
    Update();
    Invert(aRect);

    pViewData->GetView()->InvertHorizontal(eWhich,nDragPos);
}

String ScRowBar::GetDragHelp( long nVal )
{
    long nTwips = (long) ( nVal / pViewData->GetPPTY() );
    return lcl_MetricString( nTwips, ScGlobal::GetRscString(STR_TIP_HEIGHT) );
}

//  GetHiddenCount ist nur fuer Zeilen ueberladen

SCROW ScRowBar::GetHiddenCount( SCROW nEntryNo )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    return pDoc->GetHiddenRowCount( nEntryNo, nTab );
}

BOOL ScRowBar::IsMirrored()         // overloaded only for rows
{
    return pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );
}


