/*************************************************************************
 *
 *  $RCSfile: pvlaydlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:54 $
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

//----------------------------------------------------------------------------

#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#define _PVLAYDLG_CXX
#include "pvlaydlg.hxx"
#undef  _PVLAYDLG_CXX

#include "uiitems.hxx"
#include "rangeutl.hxx"
#include "document.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "pivot.hxx"
#include "pvglob.hxx"
#include "pvfundlg.hxx"
#include "globstr.hrc"
#include "pivot.hrc"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "scmod.hxx"

using namespace com::sun::star;

//----------------------------------------------------------------------------

#define FSTR(index) *(aFuncNameArr[index-1])
#define STD_FORMAT   SCA_VALID | SCA_TAB_3D \
                    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE

long PivotGlobal::nObjHeight = 0;    // initialized with resource data
long PivotGlobal::nObjWidth  = 0;
long PivotGlobal::nSelSpace  = 0;


//============================================================================
//  struct FuncData

struct FuncData
{
    short  nCol;
    USHORT nFuncMask;

    FuncData( short col, USHORT funcs = PIVOT_FUNC_SUM )
        : nCol(col), nFuncMask(funcs) {}
    FuncData( const FuncData& rCpy )
        : nCol(rCpy.nCol), nFuncMask(rCpy.nFuncMask) {}

    FuncData& operator=( const FuncData& r )
        { nCol=r.nCol; nFuncMask=r.nFuncMask; return *this; }

    BOOL operator==( const FuncData& r )
        { return ( (nCol==r.nCol)&&(nFuncMask==r.nFuncMask) ); }
};


//============================================================================

//----------------------------------------------------------------------------

ScDPLayoutDlg::ScDPLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const ScDPObject* pDPObject )
    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_PIVOT_LAYOUT ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnOptions     ( this, ScResId( BTN_OPTIONS ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

        aFtInfo         ( this, ScResId( FT_INFO ) ),
        aBtnIgnEmptyRows( this, ScResId( BTN_IGNEMPTYROWS ) ),
        aBtnDetectCat   ( this, ScResId( BTN_DETECTCAT ) ),
        aBtnTotalCol    ( this, ScResId( BTN_TOTALCOL ) ),
        aBtnTotalRow    ( this, ScResId( BTN_TOTALROW ) ),

        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aFtOutArea      ( this, ScResId( FT_OUTAREA ) ),
        aEdOutPos       ( this, ScResId( ED_OUTAREA ) ),
        aRbOutPos       ( this, ScResId( RB_OUTAREA ), &aEdOutPos ),
        aFlAreas        ( this, ScResId( FL_OUTPUT ) ),

        aFtRow          ( this, ScResId( FT_ROW ) ),
        aWndRow         ( this, ScResId( WND_ROW ),    TYPE_ROW,    &aFtRow ),
        aFtCol          ( this, ScResId( FT_COL ) ),
        aWndCol         ( this, ScResId( WND_COL ),    TYPE_COL,    &aFtCol ),
        aFtData         ( this, ScResId( FT_DATA ) ),
        aWndData        ( this, ScResId( WND_DATA ),   TYPE_DATA,   &aFtData ),
        aWndSelect      ( this, ScResId( WND_SELECT ), TYPE_SELECT, String(ScResId(STR_SELECT)) ),

        aSlider         ( this, ScResId( WND_HSCROLL ) ),
        aFlLayout       ( this, ScResId( FL_LAYOUT ) ),
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrNewTable    ( ScResId( SCSTR_NEWTABLE ) ),

        bIsDrag         ( FALSE ),
        aLabelDataArr   ( NULL ),
        nLabelCount     ( 0 ),

        eLastActiveType ( TYPE_SELECT ),
        nOffset         ( 0 ),
        //
        pDlgDPObject    ( NULL ),
        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        bRefInputMode   ( FALSE )
{
    if ( pDPObject )
    {
        pDlgDPObject = new ScDPObject( *pDPObject );
        pDlgDPObject->SetAlive( TRUE );     // needed to get structure information
        pDlgDPObject->FillOldParam( thePivotData, FALSE );
        pDlgDPObject->FillLabelData( thePivotData, bShowAll, MAX_LABELS );
    }

    Init();
    FreeResource();
}


//----------------------------------------------------------------------------

__EXPORT ScDPLayoutDlg::~ScDPLayoutDlg()
{
    USHORT nEntries = aLbOutPos.GetEntryCount();
    USHORT i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)aLbOutPos.GetEntryData( i );

    for ( i=0; i<nLabelCount; i++ )
        delete aLabelDataArr[i];
    delete [] aLabelDataArr;

    for ( i=0; i<FUNC_COUNT; i++ )
        delete aFuncNameArr[i];
    for ( i=0; (i<MAX_LABELS); i++ )
        delete aSelectArr[i];

    for ( i=0; (i<MAX_FIELDS); i++ )
    {
        delete aColArr  [i];
        delete aRowArr  [i];
        delete aDataArr [i];
    }

    delete pDlgDPObject;
}


//----------------------------------------------------------------------------

ScDPFieldWindow& ScDPLayoutDlg::GetFieldWindow( ScDPFieldType eType )
{
    switch( eType )
    {
        case TYPE_ROW:  return aWndRow;
        case TYPE_COL:  return aWndCol;
        case TYPE_DATA: return aWndData;
    }
    return aWndSelect;
}

void __EXPORT ScDPLayoutDlg::Init()
{
    DBG_ASSERT( pViewData && pDoc,
                "Ctor-Initialisierung fehlgeschlagen!" );

    aBtnRemove.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );
    aBtnOptions.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );

    USHORT i;
    for ( i=0; i<FUNC_COUNT; i++ )
        aFuncNameArr[i] = new String( ScResId( i+1 ) );

    aBtnMore.AddWindow( &aFtOutArea );
    aBtnMore.AddWindow( &aLbOutPos );
    aBtnMore.AddWindow( &aEdOutPos );
    aBtnMore.AddWindow( &aRbOutPos );
    aBtnMore.AddWindow( &aBtnIgnEmptyRows );
    aBtnMore.AddWindow( &aBtnDetectCat );
    aBtnMore.AddWindow( &aBtnTotalCol );
    aBtnMore.AddWindow( &aBtnTotalRow );
    aBtnMore.AddWindow( &aFlAreas );
    aBtnMore.SetClickHdl( LINK( this, ScDPLayoutDlg, MoreClickHdl ) );

    {
        Size aSize( Window( this, ScResId( WND_FIELD ) ).GetSizePixel() );
        OHEIGHT = aSize.Height();
        OWIDTH  = aSize.Width();
    }
    SSPACE = Window( this, ScResId( WND_FIELD_SPACE ) ).GetSizePixel().Width();

    CalcWndSizes();

    for ( i=0; i<MAX_LABELS; i++ )
        aSelectArr[i] = NULL;

    for ( i=0; i<MAX_FIELDS; i++ )
    {
        aColArr[i]  = NULL;
        aRowArr[i]  = NULL;
        aDataArr[i] = NULL;
    }

    InitWndSelect( thePivotData.ppLabelArr, thePivotData.nLabels );
    InitWnd( thePivotData.aColArr,  thePivotData.nColCount,  TYPE_COL );
    InitWnd( thePivotData.aRowArr,  thePivotData.nRowCount,  TYPE_ROW );
    InitWnd( thePivotData.aDataArr, thePivotData.nDataCount, TYPE_DATA );

    aSlider.SetPageSize( PAGE_SIZE );
    aSlider.SetVisibleSize( PAGE_SIZE );
    aSlider.SetLineSize( LINE_SIZE );
    aSlider.SetRange( Range( 0, ((thePivotData.nLabels+LINE_SIZE-1)/LINE_SIZE)*LINE_SIZE ) );

    if ( thePivotData.nLabels > PAGE_SIZE )
    {
        aSlider.SetEndScrollHdl( LINK( this, ScDPLayoutDlg, ScrollHdl ) );
        aSlider.Show();
    }
    else
        aSlider.Hide();

    // Ein-/Ausgabebereiche: ----------------------------------------------

    aLbOutPos .SetSelectHdl( LINK( this, ScDPLayoutDlg, SelAreaHdl ) );
    aEdOutPos .SetModifyHdl( LINK( this, ScDPLayoutDlg, EdModifyHdl ) );
    aBtnOk    .SetClickHdl ( LINK( this, ScDPLayoutDlg, OkHdl ) );
    aBtnCancel.SetClickHdl ( LINK( this, ScDPLayoutDlg, CancelHdl ) );

    if ( pViewData && pDoc )
    {
        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * in einem Zeiger-Array gemerkt, bei denen es sich
         * um sinnvolle Bereiche handelt
         */

        aLbOutPos.Clear();
        aLbOutPos.InsertEntry( aStrUndefined, 0 );
        aLbOutPos.InsertEntry( aStrNewTable,  1 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                USHORT nInsert = aLbOutPos.InsertEntry( aName );

                aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc );
                aLbOutPos.SetEntryData( nInsert, new String( aRefStr ) );
            }
        }
    }

    if ( thePivotData.nTab != MAXTAB+1 )
    {
        String aStr;
        ScAddress( thePivotData.nCol,
                   thePivotData.nRow,
                   thePivotData.nTab ).Format( aStr, STD_FORMAT, pDoc );
        aEdOutPos.SetText( aStr );
        EdModifyHdl(0);
    }
    else
    {
        aLbOutPos.SelectEntryPos( aLbOutPos.GetEntryCount()-1 );
        SelAreaHdl(NULL);
    }

    aBtnIgnEmptyRows.Check( thePivotData.bIgnoreEmptyRows );
    aBtnDetectCat   .Check( thePivotData.bDetectCategories );
    aBtnTotalCol    .Check( thePivotData.bMakeTotalCol );
    aBtnTotalRow    .Check( thePivotData.bMakeTotalRow );

    aWndCol.SetHelpId( HID_SC_DPLAY_COLUMN );
    aWndRow.SetHelpId( HID_SC_DPLAY_ROW );
    aWndData.SetHelpId( HID_SC_DPLAY_DATA );
    aWndSelect.SetHelpId( HID_SC_DPLAY_SELECT );

    InitFocus();

//  SetDispatcherLock( TRUE ); // Modal-Modus einschalten

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScDPLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWndSelect( LabelData** ppLabelArr, long nLabels )
{
    if ( ppLabelArr )
    {
        long nLast;
        nLabelCount     = (nLabels > MAX_LABELS)    ? MAX_LABELS  : nLabels;
        nLast           = (nLabelCount > PAGE_SIZE) ? PAGE_SIZE-1 : nLabelCount-1;
        aLabelDataArr   = new LabelData*[nLabelCount];

        for ( long i=0; (i<nLabelCount); i++ )
        {
            aLabelDataArr[i] = new LabelData( *ppLabelArr[i] );

            if ( i <= nLast )
            {
                aWndSelect.AddField( *(aLabelDataArr[i]->pStrColName), i );
                aSelectArr[i] = new FuncData( aLabelDataArr[i]->nCol,
                                              aLabelDataArr[i]->nFuncMask );
            }
        }
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWnd( PivotField* pArr, long nCount, ScDPFieldType eType )
{
    if ( pArr && (eType != TYPE_SELECT) )
    {
        FuncData**          pInitArr = NULL;
        ScDPFieldWindow*    pInitWnd = NULL;
        BOOL                bDataArr = FALSE;

        switch ( eType )
        {
            case TYPE_COL:
                pInitArr = aColArr;
                pInitWnd = &aWndCol;
                break;

            case TYPE_ROW:
                pInitArr = aRowArr;
                pInitWnd = &aWndRow;
                break;

            case TYPE_DATA:
                pInitArr = aDataArr;
                pInitWnd = &aWndData;
                bDataArr = TRUE;
                break;
            default:
            break;
        }

        if ( pInitArr && pInitWnd )
        {
            long j=0;
            for ( long i=0; (i<nCount); i++ )
            {
                USHORT nCol = pArr[i].nCol;
                USHORT nMask = pArr[i].nFuncMask;

                if ( nCol != PIVOT_DATA_FIELD )
                {
                    pInitArr[j] = new FuncData( nCol, nMask );

                    if ( !bDataArr )
                    {
                        pInitWnd->AddField( GetLabelString( nCol ), j );
                    }
                    else
                    {
                        LabelData* pData = GetLabelData( nCol );
                        DBG_ASSERT( pData, "LabelData not found" );
                        if (pData)
                        {
                            String aStr( GetFuncString( pInitArr[j]->nFuncMask,
                                                         pData->bIsValue ) );

                            aStr += GetLabelString( nCol );
                            pInitWnd->AddField( aStr, j );
                        }
                    }
                    j += 1;
                }
            }
// Do not redraw here -> first the FixedText has to get its mnemonic char
//            pInitWnd->Redraw();
        }
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitFocus()
{
    if( aWndSelect.IsEmpty() )
    {
        aBtnOk.GrabFocus();
        NotifyFieldFocus( TYPE_SELECT, FALSE );
    }
    else
        aWndSelect.GrabFocus();
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::AddField( long nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    FuncData            fData( *(aSelectArr[nFromIndex]) );
    long                nAt   = 0;
    ScDPFieldWindow*    toWnd = NULL;
    ScDPFieldWindow*    rmWnd = NULL;
    FuncData**          toArr = NULL;
    FuncData**          rmArr = NULL;
    BOOL                bDataArr = FALSE;

    switch ( eToType )
    {
        case TYPE_COL:
            toWnd = &aWndCol;
            rmWnd = &aWndRow;
            toArr = aColArr;
            rmArr = aRowArr;
            break;

        case TYPE_ROW:
            toWnd = &aWndRow;
            rmWnd = &aWndCol;
            toArr = aRowArr;
            rmArr = aColArr;
            break;

        case TYPE_DATA:
            toWnd = &aWndData;
            rmWnd = NULL;
            toArr = aDataArr;
            rmArr = NULL;
            bDataArr = TRUE;
            break;
    }

    if (   (toArr[MAX_FIELDS-1] == NULL)
        && (!Contains( toArr, fData.nCol, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr )
        {
            if ( Contains( rmArr, fData.nCol, nAt ) )
            {
                rmWnd->DelField( nAt );
                Remove( rmArr, nAt );
            }
        }

        LabelData*  pData = aLabelDataArr[nFromIndex+nOffset];
        long        nAddedAt = 0;

        if ( !bDataArr )
        {
            if ( toWnd->AddField( *(pData->pStrColName),
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
        else
        {
            USHORT nMask = fData.nFuncMask;
            String aStr( GetFuncString( nMask, pData->bIsValue ) );

            aStr += *(pData->pStrColName);

            if ( toWnd->AddField( aStr,
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                fData.nFuncMask = nMask;
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }

    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::MoveField( ScDPFieldType eFromType, long nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    if ( eFromType == TYPE_SELECT )
        AddField( nFromIndex, eToType, rAtPos );
    else if ( eFromType != eToType )
    {
        ScDPFieldWindow*    fromWnd  = NULL;
        ScDPFieldWindow*    toWnd    = NULL;
        ScDPFieldWindow*    rmWnd    = NULL;
        FuncData**          fromArr  = NULL;
        FuncData**          toArr    = NULL;
        FuncData**          rmArr    = NULL;
        long                nAt      = 0;
        BOOL                bDataArr = FALSE;

        switch ( eFromType )
        {
            case TYPE_COL:
                fromWnd = &aWndCol;
                fromArr = aColArr;
                break;

            case TYPE_ROW:
                fromWnd = &aWndRow;
                fromArr = aRowArr;
                break;

            case TYPE_DATA:
                fromWnd = &aWndData;
                fromArr = aDataArr;
                break;
        }

        switch ( eToType )
        {
            case TYPE_COL:
                toWnd = &aWndCol;
                toArr = aColArr;
                rmWnd = &aWndRow;
                rmArr = aRowArr;
                break;

            case TYPE_ROW:
                toWnd = &aWndRow;
                toArr = aRowArr;
                rmWnd = &aWndCol;
                rmArr = aColArr;
                break;

            case TYPE_DATA:
                toWnd = &aWndData;
                toArr = aDataArr;
                bDataArr = TRUE;
                break;
        }

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            FuncData fData( *(fromArr[nFromIndex]) );

            if ( Contains( fromArr, fData.nCol, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (   (toArr[MAX_FIELDS-1] == NULL)
                    && (!Contains( toArr, fData.nCol, nAt )) )
                {
                    long nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr )
                        {
                            if ( Contains( rmArr, fData.nCol, nAt ) )
                            {
                                rmWnd->DelField( nAt );
                                Remove( rmArr, nAt );
                            }
                        }

                        if ( toWnd->AddField( GetLabelString( fData.nCol ),
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                    else
                    {
                        String aStr;
                        USHORT nMask = fData.nFuncMask;
                        aStr  = GetFuncString( nMask );
                        aStr += GetLabelString( fData.nCol );

                        if ( toWnd->AddField( aStr,
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            fData.nFuncMask = nMask;
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldWindow*    theWnd  = NULL;
        FuncData**          theArr   = NULL;
        long                nAt      = 0;
        long                nToIndex = 0;
        Point               aToPos;
        BOOL                bDataArr = FALSE;

        switch ( eFromType )
        {
            case TYPE_COL:
                theWnd = &aWndCol;
                theArr = aColArr;
                break;

            case TYPE_ROW:
                theWnd = &aWndRow;
                theArr = aRowArr;
                break;

            case TYPE_DATA:
                theWnd = &aWndData;
                theArr = aDataArr;
                bDataArr = TRUE;
                break;
        }

        FuncData fData( *(theArr[nFromIndex]) );

        if ( Contains( theArr, fData.nCol, nAt ) )
        {
            aToPos = DlgPos2WndPos( rAtPos, *theWnd );
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                long nAddedAt = 0;

                theWnd->DelField( nAt );
                Remove( theArr, nAt );

                if ( !bDataArr )
                {
                    if ( theWnd->AddField( GetLabelString( fData.nCol ),
                                           aToPos,
                                           nAddedAt ) )
                    {
                        Insert( theArr, fData, nAddedAt );
                    }
                }
                else
                {
                    String aStr;
                    USHORT nMask = fData.nFuncMask;
                    aStr  = GetFuncString( nMask );
                    aStr += GetLabelString( fData.nCol );

                    if ( theWnd->AddField( aStr,
                                           DlgPos2WndPos( rAtPos, *theWnd ),
                                           nAddedAt ) )
                    {
                        fData.nFuncMask = nMask;
                        Insert( theArr, fData, nAddedAt );
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::RemoveField( ScDPFieldType eFromType, long nIndex )
{
    FuncData** pArr = NULL;
    switch( eFromType )
    {
        case TYPE_COL:  pArr = aColArr;     break;
        case TYPE_ROW:  pArr = aRowArr;     break;
        case TYPE_DATA: pArr = aDataArr;    break;
    }

    if( pArr )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eFromType );
        rWnd.DelField( nIndex );
        Remove( pArr, nIndex );
        if( rWnd.IsEmpty() ) InitFocus();
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMouseButtonUp( const Point& rAt )
{
    if ( bIsDrag )
    {
        bIsDrag = FALSE;

        ScDPFieldType   eDnDToType;
        Point           aPos = ScreenToOutputPixel( rAt );
        BOOL            bDel = FALSE;

        if ( aRectCol.IsInside( aPos ) )
        {
            eDnDToType = TYPE_COL;
            bDel = FALSE;
        }
        else if ( aRectRow.IsInside( aPos ) )
        {
            eDnDToType = TYPE_ROW;
            bDel = FALSE;
        }
        else if ( aRectData.IsInside( aPos ) )
        {
            eDnDToType = TYPE_DATA;
            bDel = FALSE;
        }
        else if ( aRectSelect.IsInside( aPos ) )
        {
            eDnDToType = TYPE_SELECT;
            bDel = TRUE;
        }
        else
            bDel = TRUE;

        if ( bDel )
            RemoveField( eDnDFromType, nDnDFromIndex );
        else
            MoveField( eDnDFromType, nDnDFromIndex, eDnDToType, aPos );
    }
}


//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseMove( const Point& rAt )
{
    PointerStyle ePtr = POINTER_ARROW;

    if ( bIsDrag )
    {
        Point aPos = ScreenToOutputPixel( rAt );

        if ( aRectCol.IsInside( aPos ) )
            ePtr = POINTER_PIVOT_COL;
        else if ( aRectRow.IsInside( aPos ) )
            ePtr = POINTER_PIVOT_ROW;
        else if ( aRectData.IsInside( aPos ) )
            ePtr = POINTER_PIVOT_FIELD;
        else if ( eDnDFromType != TYPE_SELECT )
            ePtr = POINTER_PIVOT_DELETE;
        else if ( aRectSelect.IsInside( aPos ) )
            ePtr = POINTER_PIVOT_FIELD;
        else
            ePtr = POINTER_NOTALLOWED;
    }

    return ePtr;
}


//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseButtonDown( ScDPFieldType eType, long nFieldIndex )
{
    PointerStyle ePtr = POINTER_ARROW;

    bIsDrag       = TRUE;
    eDnDFromType  = eType;
    nDnDFromIndex = nFieldIndex;

    if ( eType == TYPE_COL )
        ePtr = POINTER_PIVOT_COL;
    else if ( eType == TYPE_ROW )
        ePtr = POINTER_PIVOT_ROW;

    return ePtr;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyDoubleClick( ScDPFieldType eType, long nFieldIndex )
{
    FuncData** pArr = NULL;
    switch ( eType )
    {
        case TYPE_COL:      pArr = aColArr;     break;
        case TYPE_ROW:      pArr = aRowArr;     break;
        case TYPE_DATA:     pArr = aDataArr;    break;
    }

    if ( pArr )
    {
        long nArrPos = 0;
        LabelData* pData = GetLabelData( pArr[nFieldIndex]->nCol, &nArrPos );
        if ( pData )
        {
            String aFieldName = *(pData->pStrColName);
            BOOL bOldShowAll = bShowAll[nArrPos];

            ScDPFunctionDlg* pDlg =
                new ScDPFunctionDlg( this,
                                        eType != TYPE_DATA,
                                        aFieldName,
                                        pArr[nFieldIndex]->nFuncMask,
                                        bOldShowAll );

            if ( pDlg->Execute() == RET_OK )
            {
                  pArr[nFieldIndex]->nFuncMask
                = pData->nFuncMask
                = pDlg->GetFuncMask();

                if ( eType == TYPE_DATA )
                {
                    String aStr;
                    aStr  = GetFuncString ( aDataArr[nFieldIndex]->nFuncMask );
                    aStr += GetLabelString( aDataArr[nFieldIndex]->nCol );
                    aWndData.SetFieldText( aStr, nFieldIndex );
                }
                else
                {
                    BOOL bNewShowAll = pDlg->GetShowAll();
                    if (bNewShowAll != bOldShowAll)
                    {
                        //! remember which were changed?
                        bShowAll[nArrPos] = bNewShowAll;
                    }
                }
            }
            delete pDlg;
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyFieldFocus( ScDPFieldType eType, BOOL bGotFocus )
{
    /*  Enable Remove/Options buttons on GetFocus in field window.
        #107616# Enable them also, if dialog is deactivated (click into document).
        The !IsActive() condition handles the case that a LoseFocus event of a
        field window would follow the Deactivate event of this dialog. */
    BOOL bEnable = (bGotFocus || !IsActive()) && (eType != TYPE_SELECT);
    aBtnRemove.Enable( bEnable );
    aBtnOptions.Enable( bEnable );
    if( bGotFocus )
        eLastActiveType = eType;
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMoveField( ScDPFieldType eToType )
{
    ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
    if( (eToType != TYPE_SELECT) && !rWnd.IsEmpty() )
    {
        MoveField( eLastActiveType, rWnd.GetSelectedField(), eToType, GetFieldWindow( eToType ).GetLastPosition() );
        if( rWnd.IsEmpty() )
            NotifyFieldFocus( eToType, TRUE );
        else
            rWnd.GrabFocus();
        if( eLastActiveType == TYPE_SELECT )
            aWndSelect.SelectNext();
    }
    else
        InitFocus();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyRemoveField( ScDPFieldType eType, long nFieldIndex )
{
    if( eType != TYPE_SELECT )
        RemoveField( eType, nFieldIndex );
}

//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::NotifyMoveSlider( USHORT nKeyCode )
{
    long nOldPos = aSlider.GetThumbPos();
    switch( nKeyCode )
    {
        case KEY_HOME:  aSlider.DoScroll( 0 );                      break;
        case KEY_END:   aSlider.DoScroll( aSlider.GetRangeMax() );  break;
        case KEY_UP:
        case KEY_LEFT:  aSlider.DoScrollAction( SCROLL_LINEUP );    break;
        case KEY_DOWN:
        case KEY_RIGHT: aSlider.DoScrollAction( SCROLL_LINEDOWN );  break;
    }
    return nOldPos != aSlider.GetThumbPos();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::Deactivate()
{
    /*  #107616# If the dialog has been deactivated (click into document), the LoseFocus
        event from field window disables Remove/Options buttons. Re-enable them here by
        simulating a GetFocus event. Event order of LoseFocus and Deactivate is not important.
        The last event will enable the buttons in both cases (see NotifyFieldFocus). */
    NotifyFieldFocus( eLastActiveType, TRUE );
}

//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::Contains( FuncData** pArr, short nCol, long& nAt )
{
    if ( !pArr )
        return FALSE;

    BOOL    bFound  = FALSE;
    long    i       = 0;

    while ( (i<MAX_FIELDS) && (pArr[i] != NULL) && !bFound )
    {
        bFound = (pArr[i]->nCol == nCol);
        if ( bFound )
            nAt = i;
        i++;
    }

    return bFound;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Remove( FuncData** pArr, long nAt )
{
    if ( !pArr || (nAt>MAX_FIELDS-1) )
        return;

    DELETEZ( pArr[nAt] );

    if ( (nAt != MAX_FIELDS-1) && (pArr[nAt+1] != NULL) )
    {
        long i=nAt;
        while ( i<MAX_FIELDS )
        {
            pArr[i] = pArr[i+1];
            i++;
        }
        pArr[MAX_FIELDS-1] = NULL;
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Insert( FuncData** pArr, const FuncData& rFData, long nAt )
{
    if ( !pArr || (nAt>MAX_FIELDS-1) )
        return;

    if ( pArr[nAt] == NULL )
    {
        pArr[nAt] = new FuncData( rFData );
    }
    else
    {
        if ( pArr[MAX_FIELDS-1] == NULL ) // mind. ein Slot frei?
        {
            for ( long i=MAX_FIELDS-1; i>nAt; i-- )
                pArr[i] = pArr[i-1];
            pArr[nAt] = new FuncData( rFData );
        }
    }
}


//----------------------------------------------------------------------------

LabelData* ScDPLayoutDlg::GetLabelData( short nCol, long* pPos )
{
    LabelData*  pData   = NULL;
    BOOL        bFound  = FALSE;

    if ( aLabelDataArr )
    {
        for ( long i=0; (i<nLabelCount) && !bFound; i++ )
        {
            bFound = (aLabelDataArr[i]->nCol == nCol);
            if ( bFound )
            {
                pData = aLabelDataArr[i];
                if (pPos)
                    *pPos = i;
            }
        }
    }

    return pData;
}


//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetLabelString( short nCol )
{
    LabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData && pData->pStrColName)
        return *pData->pStrColName;

    return String();
}


//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetFuncString( USHORT& rFuncMask, BOOL bIsValue )
{
    String aStr;

    if (   rFuncMask == PIVOT_FUNC_NONE
        || rFuncMask == PIVOT_FUNC_AUTO )
    {
        if ( bIsValue )
        {
            aStr = FSTR(PIVOTSTR_SUM);
            rFuncMask = PIVOT_FUNC_SUM;
        }
        else
        {
            aStr = FSTR(PIVOTSTR_COUNT);
            rFuncMask = PIVOT_FUNC_COUNT;
        }
    }
    else if ( rFuncMask == PIVOT_FUNC_SUM )       aStr = FSTR(PIVOTSTR_SUM);
    else if ( rFuncMask == PIVOT_FUNC_COUNT )     aStr = FSTR(PIVOTSTR_COUNT);
    else if ( rFuncMask == PIVOT_FUNC_AVERAGE )   aStr = FSTR(PIVOTSTR_AVG);
    else if ( rFuncMask == PIVOT_FUNC_MAX )       aStr = FSTR(PIVOTSTR_MAX);
    else if ( rFuncMask == PIVOT_FUNC_MIN )       aStr = FSTR(PIVOTSTR_MIN);
    else if ( rFuncMask == PIVOT_FUNC_PRODUCT )   aStr = FSTR(PIVOTSTR_PROD);
    else if ( rFuncMask == PIVOT_FUNC_COUNT_NUM ) aStr = FSTR(PIVOTSTR_COUNT2);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEV )   aStr = FSTR(PIVOTSTR_DEV);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEVP )  aStr = FSTR(PIVOTSTR_DEV2);
    else if ( rFuncMask == PIVOT_FUNC_STD_VAR )   aStr = FSTR(PIVOTSTR_VAR);
    else if ( rFuncMask == PIVOT_FUNC_STD_VARP )  aStr = FSTR(PIVOTSTR_VAR2);
    else
    {
        aStr  = ScGlobal::GetRscString( STR_TABLE_ERGEBNIS );
        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " - " ));
    }

    return aStr;
}


//----------------------------------------------------------------------------

Point ScDPLayoutDlg::DlgPos2WndPos( const Point& rPt, Window& rWnd )
{
    Point aWndPt( rPt );
    aWndPt.X() = rPt.X()-rWnd.GetPosPixel().X();
    aWndPt.Y() = rPt.Y()-rWnd.GetPosPixel().Y();

    return aWndPt;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::CalcWndSizes()
{
    // row/column/data area sizes
    aWndRow.SetSizePixel( Size( OWIDTH, MAX_FIELDS * OHEIGHT ) );
    aWndCol.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, 2 * OHEIGHT ) );
    aWndData.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, MAX_FIELDS * OHEIGHT ) );

    // selection area
    aWndSelect.SetSizePixel( Size(
        2 * OWIDTH + SSPACE, LINE_SIZE * OHEIGHT + (LINE_SIZE - 1) * SSPACE ) );

    // scroll bar
    Point aSliderPos( aWndSelect.GetPosPixel() );
    Size aSliderSize( aWndSelect.GetSizePixel() );
    aSliderPos.Y() += aSliderSize.Height() + SSPACE;
    aSliderSize.Height() = GetSettings().GetStyleSettings().GetScrollBarSize();
    aSlider.SetPosSizePixel( aSliderPos, aSliderSize );

    aRectRow    = Rectangle( aWndRow.GetPosPixel(),     aWndRow.GetSizePixel() );
    aRectCol    = Rectangle( aWndCol.GetPosPixel(),     aWndCol.GetSizePixel() );
    aRectData   = Rectangle( aWndData.GetPosPixel(),    aWndData.GetSizePixel() );
    aRectSelect = Rectangle( aWndSelect.GetPosPixel(),  aWndSelect.GetSizePixel() );
}


//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::GetPivotArrays( PivotField*  pColArr,
                                       PivotField*  pRowArr,
                                       PivotField*  pDataArr,
                                       USHORT&      rColCount,
                                       USHORT&      rRowCount,
                                       USHORT&      rDataCount )
{
    BOOL bFit = TRUE;
    USHORT i=0;

    for ( i=0; (i<MAX_FIELDS) && (aDataArr[i] != NULL ); i++ )
    {
        pDataArr[i].nCol        = aDataArr[i]->nCol;
        pDataArr[i].nFuncMask   = aDataArr[i]->nFuncMask;
    }
    rDataCount = i;

    for ( i=0; (i<MAX_FIELDS) && (aColArr[i] != NULL ); i++ )
    {
        pColArr[i].nCol         = aColArr[i]->nCol;
        pColArr[i].nFuncMask    = aColArr[i]->nFuncMask;
    }
    rColCount = i;

    for ( i=0; (i<MAX_FIELDS) && (aRowArr[i] != NULL ); i++ )
    {
        pRowArr[i].nCol         = aRowArr[i]->nCol;
        pRowArr[i].nFuncMask    = aRowArr[i]->nFuncMask;
    }
    rRowCount = i;

    if ( rRowCount < MAX_FIELDS )
        pRowArr[rRowCount++].nCol = PIVOT_DATA_FIELD;
    else if ( rColCount < MAX_FIELDS )
        pColArr[rColCount++].nCol = PIVOT_DATA_FIELD;
    else
        bFit = FALSE;       // kein Platz fuer Datenfeld

    return bFit;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( bRefInputMode )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdOutPos );
/*
        ScAddress   aAdr( nStartCol, nStartRow, nStartTab );
        aAdr.PutInOrder( ScAddress( nEndCol, nEndRow, nEndTab ) );
*/
        String aRefStr;
        rRef.aStart.Format( aRefStr, STD_FORMAT, pDoc );
        aEdOutPos.SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetActive()
{
    if ( bRefInputMode )
    {
        aEdOutPos.GrabFocus();
        EdModifyHdl( NULL );
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, ClickHdl, PushButton *, pBtn )
{
    if( pBtn == &aBtnRemove )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
        RemoveField( eLastActiveType, rWnd.GetSelectedField() );
        if( !rWnd.IsEmpty() ) rWnd.GrabFocus();
    }
    else if( pBtn == &aBtnOptions )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
        NotifyDoubleClick( eLastActiveType, rWnd.GetSelectedField() );
        rWnd.GrabFocus();
    }
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, OkHdl, OKButton *, EMPTYARG )
{
    String      aOutPosStr( aEdOutPos.GetText() );
    ScAddress   aAdrDest;
    BOOL        bToNewTable = (aLbOutPos.GetSelectEntryPos() == 1);
    USHORT      nResult     = !bToNewTable ? aAdrDest.Parse( aOutPosStr, pDoc ) : 0;

    if (   bToNewTable
        || ( (aOutPosStr.Len() > 0) && (SCA_VALID == (nResult & SCA_VALID)) ) )
    {
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();

        ScPivotParam    theOutParam;
        PivotFieldArr   aColArr;
        PivotFieldArr   aRowArr;
        PivotFieldArr   aDataArr;
        USHORT          nColCount;
        USHORT          nRowCount;
        USHORT          nDataCount;

        BOOL bFit = GetPivotArrays( aColArr,   aRowArr,   aDataArr,
                                    nColCount, nRowCount, nDataCount );
        if ( bFit && pDlgDPObject )
        {
            ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

            ScDPSaveData aSaveData;
            aSaveData.SetIgnoreEmptyRows( aBtnIgnEmptyRows.IsChecked() );
            aSaveData.SetRepeatIfEmpty( aBtnDetectCat.IsChecked() );
            aSaveData.SetColumnGrand( aBtnTotalCol.IsChecked() );
            aSaveData.SetRowGrand( aBtnTotalRow.IsChecked() );

            uno::Reference<sheet::XDimensionsSupplier> xSource = pDlgDPObject->GetSource();

            ScDPObject::ConvertOrientation( aSaveData, aColArr,  nColCount,
                            sheet::DataPilotFieldOrientation_COLUMN, NULL, 0, 0, xSource, FALSE );
            ScDPObject::ConvertOrientation( aSaveData, aRowArr,  nRowCount,
                            sheet::DataPilotFieldOrientation_ROW,    NULL, 0, 0, xSource, FALSE );
            ScDPObject::ConvertOrientation( aSaveData, aDataArr, nDataCount,
                            sheet::DataPilotFieldOrientation_DATA,   NULL, 0, 0, xSource, FALSE,
                            aColArr, nColCount, aRowArr, nRowCount );

            //  "show all" property
            //! init from pDlgDPObject, set only changed values
            for ( long i=0; i<nLabelCount; i++ )
                if ( aLabelDataArr && aLabelDataArr[i]->pStrColName )
                {
                    ScDPSaveDimension* pDim =
                        aSaveData.GetExistingDimensionByName( *aLabelDataArr[i]->pStrColName );
                    if (pDim)
                        pDim->SetShowEmpty( bShowAll[i] );
                }

            USHORT nWhichPivot = SC_MOD()->GetPool().GetWhich( SID_PIVOT_TABLE );
            ScPivotItem aOutItem( nWhichPivot, &aSaveData, &aOutRange, bToNewTable );

            bRefInputMode = FALSE;      // to allow deselecting when switching sheets

            SetDispatcherLock( FALSE );
            SwitchToDocument();

            //  #95513# don't hide the dialog before executing the slot, instead it is used as
            //  parent for message boxes in ScTabViewShell::GetDialogParent

            GetBindings().GetDispatcher()->Execute( SID_PIVOT_TABLE,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      &aOutItem, 0L, 0L );
            Close();
        }
        else
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                     ScGlobal::GetRscString( STR_PIVOT_ERROR )
                    ).Execute();
        }
    }
    else
    {
        if ( !aBtnMore.GetState() )
            aBtnMore.SetState( TRUE );

        ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                 ScGlobal::GetRscString( STR_INVALID_TABREF )
                ).Execute();
        aEdOutPos.GrabFocus();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScDPLayoutDlg, CancelHdl, CancelButton *, EMPTYARG )
{
    Close();
    return 0;
}
IMPL_LINK_INLINE_END( ScDPLayoutDlg, CancelHdl, CancelButton *, EMPTYARG )


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, MoreClickHdl, MoreButton *, pBtn )
{
    if ( aBtnMore.GetState() )
    {
        bRefInputMode = TRUE;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        aEdOutPos.Enable();
        aEdOutPos.GrabFocus();
        aRbOutPos.Enable();
    }
    else
    {
        bRefInputMode = FALSE;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, EdModifyHdl, Edit *, EMPTYARG )
{
    String  theCurPosStr = aEdOutPos.GetText();
    USHORT  nResult = ScAddress().Parse( theCurPosStr, pDoc );

    if ( SCA_VALID == (nResult & SCA_VALID) )
    {
        String* pStr    = NULL;
        BOOL    bFound  = FALSE;
        USHORT  i       = 0;
        USHORT  nCount  = aLbOutPos.GetEntryCount();

        for ( i=2; i<nCount && !bFound; i++ )
        {
            pStr = (String*)aLbOutPos.GetEntryData( i );
            bFound = (theCurPosStr == *pStr);
        }

        if ( bFound )
            aLbOutPos.SelectEntryPos( --i );
        else
            aLbOutPos.SelectEntryPos( 0 );
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, SelAreaHdl, ListBox *, EMPTYARG )
{
    String  aString;
    USHORT  nSelPos = aLbOutPos.GetSelectEntryPos();

    if ( nSelPos > 1 )
    {
        aString = *(String*)aLbOutPos.GetEntryData( nSelPos );
    }
    else if ( nSelPos == aLbOutPos.GetEntryCount()-1 ) // auf neue Tabelle?
    {
        aEdOutPos.Disable();
        aRbOutPos.Disable();
    }
    else
    {
        aEdOutPos.Enable();
        aRbOutPos.Enable();
    }

    aEdOutPos.SetText( aString );
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, ScrollHdl, ScrollBar *, EMPTYARG )
{
    long nNewOffset = aSlider.GetThumbPos();
    long nOffsetDiff = nNewOffset - nOffset;
    nOffset = nNewOffset;

    LabelData*  pData   = NULL;
    long        nFields = Min( nLabelCount - nOffset, (long)PAGE_SIZE );

    aWndSelect.ClearFields();

    long i=0;
    for ( i=0; i<nFields; i++ )
    {
        pData = aLabelDataArr[nOffset+i];
        aWndSelect.AddField( *(pData->pStrColName), i );
        if ( aSelectArr[i] )
            *(aSelectArr[i]) = FuncData( pData->nCol, pData->nFuncMask );
        else
            aSelectArr[i] = new FuncData( pData->nCol, pData->nFuncMask );
    }
    for ( ; i<MAX_LABELS; i++ )
        DELETEZ( aSelectArr[i] );

    aWndSelect.ModifySelectionOffset( nOffsetDiff );    // adjusts selection & redraws
    return 0;
}


