/*************************************************************************
 *
 *  $RCSfile: pvlaydlg.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:59:37 $
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

#include "pvlaydlg.hxx"

#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include "uiitems.hxx"
#include "rangeutl.hxx"
#include "document.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "pvglob.hxx"
//CHINA001 #include "pvfundlg.hxx"
#include "globstr.hrc"
#include "pivot.hrc"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "scmod.hxx"

#include "sc.hrc" //CHINA001
#include "scabstdlg.hxx" //CHINA001
using namespace com::sun::star;

//----------------------------------------------------------------------------

#define FSTR(index) aFuncNameArr[index-1]
#define STD_FORMAT   SCA_VALID | SCA_TAB_3D \
                    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE

long PivotGlobal::nObjHeight = 0;    // initialized with resource data
long PivotGlobal::nObjWidth  = 0;
long PivotGlobal::nSelSpace  = 0;


//============================================================================

void lcl_FillToPivotField( PivotField& rPivotField, const ScDPFuncData& rFuncData )
{
    rPivotField.nCol = rFuncData.mnCol;
    rPivotField.nFuncMask = rFuncData.mnFuncMask;
    rPivotField.maFieldRef = rFuncData.maFieldRef;
}

//============================================================================

//----------------------------------------------------------------------------

ScDPLayoutDlg::ScDPLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const ScDPObject& rDPObject )
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
        aBtnFilter      ( this, ScResId( BTN_FILTER ) ),
        aBtnDrillDown   ( this, ScResId( BTN_DRILLDOWN ) ),

        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aFtOutArea      ( this, ScResId( FT_OUTAREA ) ),
        aEdOutPos       ( this, ScResId( ED_OUTAREA ) ),
        aRbOutPos       ( this, ScResId( RB_OUTAREA ), &aEdOutPos ),
        aFlAreas        ( this, ScResId( FL_OUTPUT ) ),

        aFtPage         ( this, ScResId( FT_PAGE ) ),
        aWndPage        ( this, ScResId( WND_PAGE ),   TYPE_PAGE,   &aFtPage ),
        aFtCol          ( this, ScResId( FT_COL ) ),
        aWndCol         ( this, ScResId( WND_COL ),    TYPE_COL,    &aFtCol ),
        aFtRow          ( this, ScResId( FT_ROW ) ),
        aWndRow         ( this, ScResId( WND_ROW ),    TYPE_ROW,    &aFtRow ),
        aFtData         ( this, ScResId( FT_DATA ) ),
        aWndData        ( this, ScResId( WND_DATA ),   TYPE_DATA,   &aFtData ),
        aWndSelect      ( this, ScResId( WND_SELECT ), TYPE_SELECT, String(ScResId(STR_SELECT)) ),

        aSlider         ( this, ScResId( WND_HSCROLL ) ),
        aFlLayout       ( this, ScResId( FL_LAYOUT ) ),
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrNewTable    ( ScResId( SCSTR_NEWTABLE ) ),

        bIsDrag         ( FALSE ),

        eLastActiveType ( TYPE_SELECT ),
        nOffset         ( 0 ),
        //
        xDlgDPObject    ( new ScDPObject( rDPObject ) ),
        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        bRefInputMode   ( FALSE )
{
    xDlgDPObject->SetAlive( TRUE );     // needed to get structure information
    xDlgDPObject->FillOldParam( thePivotData, FALSE );
    xDlgDPObject->FillLabelData( thePivotData );

    Init();
    FreeResource();
}


//----------------------------------------------------------------------------

ScDPLayoutDlg::~ScDPLayoutDlg()
{
    USHORT nEntries = aLbOutPos.GetEntryCount();
    USHORT i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)aLbOutPos.GetEntryData( i );
}


//----------------------------------------------------------------------------

ScDPFieldWindow& ScDPLayoutDlg::GetFieldWindow( ScDPFieldType eType )
{
    switch( eType )
    {
        case TYPE_PAGE: return aWndPage;
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

    aFuncNameArr.reserve( FUNC_COUNT );
    for ( USHORT i = 0; i < FUNC_COUNT; ++i )
        aFuncNameArr.push_back( String( ScResId( i + 1 ) ) );

    aBtnMore.AddWindow( &aFtOutArea );
    aBtnMore.AddWindow( &aLbOutPos );
    aBtnMore.AddWindow( &aEdOutPos );
    aBtnMore.AddWindow( &aRbOutPos );
    aBtnMore.AddWindow( &aBtnIgnEmptyRows );
    aBtnMore.AddWindow( &aBtnDetectCat );
    aBtnMore.AddWindow( &aBtnTotalCol );
    aBtnMore.AddWindow( &aBtnTotalRow );
    aBtnMore.AddWindow( &aBtnFilter );
    aBtnMore.AddWindow( &aBtnDrillDown );
    aBtnMore.AddWindow( &aFlAreas );
    aBtnMore.SetClickHdl( LINK( this, ScDPLayoutDlg, MoreClickHdl ) );

    {
        Size aSize( Window( this, ScResId( WND_FIELD ) ).GetSizePixel() );
        OHEIGHT = aSize.Height();
        OWIDTH  = aSize.Width();
    }
    SSPACE = Window( this, ScResId( WND_FIELD_SPACE ) ).GetSizePixel().Width();

    CalcWndSizes();

    aSelectArr.resize( MAX_LABELS );
    aPageArr.resize( MAX_PAGEFIELDS );
    aColArr.resize( MAX_FIELDS );
    aRowArr.resize( MAX_FIELDS );
    aDataArr.resize( MAX_FIELDS );

    InitWndSelect( thePivotData.ppLabelArr, static_cast<long>(thePivotData.nLabels) );
    InitWnd( thePivotData.aPageArr, static_cast<long>(thePivotData.nPageCount), TYPE_PAGE );
    InitWnd( thePivotData.aColArr,  static_cast<long>(thePivotData.nColCount),  TYPE_COL );
    InitWnd( thePivotData.aRowArr,  static_cast<long>(thePivotData.nRowCount),  TYPE_ROW );
    InitWnd( thePivotData.aDataArr, static_cast<long>(thePivotData.nDataCount), TYPE_DATA );

    aSlider.SetPageSize( PAGE_SIZE );
    aSlider.SetVisibleSize( PAGE_SIZE );
    aSlider.SetLineSize( LINE_SIZE );
    aSlider.SetRange( Range( 0, static_cast<long>(((thePivotData.nLabels+LINE_SIZE-1)/LINE_SIZE)*LINE_SIZE) ) );

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

    if( const ScDPSaveData* pSaveData = xDlgDPObject->GetSaveData() )
    {
        aBtnFilter.Check( pSaveData->GetFilterButton() );
        aBtnDrillDown.Check( pSaveData->GetDrillDown() );
    }
    else
    {
        aBtnFilter.Check();
        aBtnDrillDown.Check();
    }

    aWndPage.SetHelpId( HID_SC_DPLAY_PAGE );
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
        size_t nLabelCount = static_cast< size_t >( (nLabels > MAX_LABELS) ? MAX_LABELS : nLabels );
        size_t nLast = (nLabelCount > PAGE_SIZE) ? (PAGE_SIZE - 1) : (nLabelCount - 1);

        aLabelDataArr.clear();
        aLabelDataArr.reserve( nLabelCount );

        for ( size_t i=0; i < nLabelCount; i++ )
        {
            aLabelDataArr.push_back( *ppLabelArr[i] );

            if ( i <= nLast )
            {
                aWndSelect.AddField( aLabelDataArr[i].maName, i );
                aSelectArr[i].reset( new ScDPFuncData( aLabelDataArr[i].mnCol, aLabelDataArr[i].mnFuncMask ) );
            }
        }
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWnd( PivotField* pArr, long nCount, ScDPFieldType eType )
{
    if ( pArr && (eType != TYPE_SELECT) )
    {
        ScDPFuncDataVec*    pInitArr = NULL;
        ScDPFieldWindow*    pInitWnd = NULL;
        BOOL                bDataArr = FALSE;

        switch ( eType )
        {
            case TYPE_PAGE:
                pInitArr = &aPageArr;
                pInitWnd = &aWndPage;
                break;

            case TYPE_COL:
                pInitArr = &aColArr;
                pInitWnd = &aWndCol;
                break;

            case TYPE_ROW:
                pInitArr = &aRowArr;
                pInitWnd = &aWndRow;
                break;

            case TYPE_DATA:
                pInitArr = &aDataArr;
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
                SCCOL nCol = pArr[i].nCol;
                USHORT nMask = pArr[i].nFuncMask;

                if ( nCol != PIVOT_DATA_FIELD )
                {
                    (*pInitArr)[j].reset( new ScDPFuncData( nCol, nMask, pArr[i].maFieldRef ) );

                    if ( !bDataArr )
                    {
                        pInitWnd->AddField( GetLabelString( nCol ), j );
                    }
                    else
                    {
                        ScDPLabelData* pData = GetLabelData( nCol );
                        DBG_ASSERT( pData, "ScDPLabelData not found" );
                        if (pData)
                        {
                            String aStr( GetFuncString( (*pInitArr)[j]->mnFuncMask,
                                                         pData->mbIsValue ) );

                            aStr += GetLabelString( nCol );
                            pInitWnd->AddField( aStr, j );

                            pData->mnFuncMask = nMask;
                        }
                    }
                    ++j;
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

void ScDPLayoutDlg::AddField( size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    ScDPFuncData        fData( *(aSelectArr[nFromIndex]) );
    size_t              nAt   = 0;
    ScDPFieldWindow*    toWnd = NULL;
    ScDPFieldWindow*    rmWnd1 = NULL;
    ScDPFieldWindow*    rmWnd2 = NULL;
    ScDPFuncDataVec*    toArr = NULL;
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    BOOL                bDataArr = FALSE;

    switch ( eToType )
    {
        case TYPE_PAGE:
            toWnd  = &aWndPage;
            rmWnd1 = &aWndRow;
            rmWnd2 = &aWndCol;
            toArr  = &aPageArr;
            rmArr1 = &aRowArr;
            rmArr2 = &aColArr;
            break;

        case TYPE_COL:
            toWnd  = &aWndCol;
            rmWnd1 = &aWndPage;
            rmWnd2 = &aWndRow;
            toArr  = &aColArr;
            rmArr1 = &aPageArr;
            rmArr2 = &aRowArr;
            break;

        case TYPE_ROW:
            toWnd  = &aWndRow;
            rmWnd1 = &aWndPage;
            rmWnd2 = &aWndCol;
            toArr  = &aRowArr;
            rmArr1 = &aPageArr;
            rmArr2 = &aColArr;
            break;

        case TYPE_DATA:
            toWnd = &aWndData;
            toArr = &aDataArr;
            bDataArr = TRUE;
            break;
    }

    if (   (toArr->back().get() == NULL)
        && (!Contains( toArr, fData.mnCol, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, fData.mnCol, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, fData.mnCol, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        ScDPLabelData&  rData = aLabelDataArr[nFromIndex+nOffset];
        size_t      nAddedAt = 0;

        if ( !bDataArr )
        {
            if ( toWnd->AddField( rData.maName,
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
        else
        {
            USHORT nMask = fData.mnFuncMask;
            String aStr( GetFuncString( nMask, rData.mbIsValue ) );

            aStr += rData.maName;

            if ( toWnd->AddField( aStr,
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                fData.mnFuncMask = nMask;
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }

    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::MoveField( ScDPFieldType eFromType, size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    if ( eFromType == TYPE_SELECT )
        AddField( nFromIndex, eToType, rAtPos );
    else if ( eFromType != eToType )
    {
        ScDPFieldWindow*    fromWnd  = NULL;
        ScDPFieldWindow*    toWnd    = NULL;
        ScDPFieldWindow*    rmWnd1   = NULL;
        ScDPFieldWindow*    rmWnd2   = NULL;
        ScDPFuncDataVec*    fromArr  = NULL;
        ScDPFuncDataVec*    toArr    = NULL;
        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        size_t              nAt      = 0;
        BOOL                bDataArr = FALSE;

        switch ( eFromType )
        {
            case TYPE_PAGE:
                fromWnd = &aWndPage;
                fromArr = &aPageArr;
                break;

            case TYPE_COL:
                fromWnd = &aWndCol;
                fromArr = &aColArr;
                break;

            case TYPE_ROW:
                fromWnd = &aWndRow;
                fromArr = &aRowArr;
                break;

            case TYPE_DATA:
                fromWnd = &aWndData;
                fromArr = &aDataArr;
                break;
        }

        switch ( eToType )
        {
            case TYPE_PAGE:
                toWnd  = &aWndPage;
                toArr  = &aPageArr;
                rmWnd1 = &aWndCol;
                rmWnd2 = &aWndRow;
                rmArr1 = &aColArr;
                rmArr2 = &aRowArr;
                break;

            case TYPE_COL:
                toWnd  = &aWndCol;
                toArr  = &aColArr;
                rmWnd1 = &aWndPage;
                rmWnd2 = &aWndRow;
                rmArr1 = &aPageArr;
                rmArr2 = &aRowArr;
                break;

            case TYPE_ROW:
                toWnd  = &aWndRow;
                toArr  = &aRowArr;
                rmWnd1 = &aWndPage;
                rmWnd2 = &aWndCol;
                rmArr1 = &aPageArr;
                rmArr2 = &aColArr;
                break;

            case TYPE_DATA:
                toWnd = &aWndData;
                toArr = &aDataArr;
                bDataArr = TRUE;
                break;
        }

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScDPFuncData fData( *((*fromArr)[nFromIndex]) );

            if ( Contains( fromArr, fData.mnCol, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (   (toArr->back().get() == NULL)
                    && (!Contains( toArr, fData.mnCol, nAt )) )
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData.mnCol, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData.mnCol, nAt ) )
                            {
                                rmWnd2->DelField( nAt );
                                Remove( rmArr2, nAt );
                            }
                        }

                        if ( toWnd->AddField( GetLabelString( fData.mnCol ),
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
                        USHORT nMask = fData.mnFuncMask;
                        aStr  = GetFuncString( nMask );
                        aStr += GetLabelString( fData.mnCol );

                        if ( toWnd->AddField( aStr,
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            fData.mnFuncMask = nMask;
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
        ScDPFuncDataVec*    theArr   = NULL;
        size_t              nAt      = 0;
        size_t              nToIndex = 0;
        Point               aToPos;
        BOOL                bDataArr = FALSE;

        switch ( eFromType )
        {
            case TYPE_PAGE:
                theWnd = &aWndPage;
                theArr = &aPageArr;
                break;

            case TYPE_COL:
                theWnd = &aWndCol;
                theArr = &aColArr;
                break;

            case TYPE_ROW:
                theWnd = &aWndRow;
                theArr = &aRowArr;
                break;

            case TYPE_DATA:
                theWnd = &aWndData;
                theArr = &aDataArr;
                bDataArr = TRUE;
                break;
        }

        ScDPFuncData fData( *((*theArr)[nFromIndex]) );

        if ( Contains( theArr, fData.mnCol, nAt ) )
        {
            aToPos = DlgPos2WndPos( rAtPos, *theWnd );
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;

                theWnd->DelField( nAt );
                Remove( theArr, nAt );

                if ( !bDataArr )
                {
                    if ( theWnd->AddField( GetLabelString( fData.mnCol ),
                                           aToPos,
                                           nAddedAt ) )
                    {
                        Insert( theArr, fData, nAddedAt );
                    }
                }
                else
                {
                    String aStr;
                    USHORT nMask = fData.mnFuncMask;
                    aStr  = GetFuncString( nMask );
                    aStr += GetLabelString( fData.mnCol );

                    if ( theWnd->AddField( aStr,
                                           DlgPos2WndPos( rAtPos, *theWnd ),
                                           nAddedAt ) )
                    {
                        fData.mnFuncMask = nMask;
                        Insert( theArr, fData, nAddedAt );
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::RemoveField( ScDPFieldType eFromType, size_t nIndex )
{
    ScDPFuncDataVec* pArr = NULL;
    switch( eFromType )
    {
        case TYPE_PAGE: pArr = &aPageArr;    break;
        case TYPE_COL:  pArr = &aColArr;     break;
        case TYPE_ROW:  pArr = &aRowArr;     break;
        case TYPE_DATA: pArr = &aDataArr;    break;
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

        ScDPFieldType   eDnDToType = TYPE_SELECT;
        Point           aPos = ScreenToOutputPixel( rAt );
        BOOL            bDel = FALSE;

        if ( aRectPage.IsInside( aPos ) )
        {
            eDnDToType = TYPE_PAGE;
            bDel = FALSE;
        }
        else if ( aRectCol.IsInside( aPos ) )
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

        if ( aRectPage.IsInside( aPos ) )
            ePtr = POINTER_PIVOT_FIELD;
        else if ( aRectCol.IsInside( aPos ) )
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

PointerStyle ScDPLayoutDlg::NotifyMouseButtonDown( ScDPFieldType eType, size_t nFieldIndex )
{
    PointerStyle ePtr = POINTER_ARROW;

    bIsDrag       = TRUE;
    eDnDFromType  = eType;
    nDnDFromIndex = nFieldIndex;

    if ( eType == TYPE_PAGE )
        ePtr = POINTER_PIVOT_FIELD;
    else if ( eType == TYPE_COL )
        ePtr = POINTER_PIVOT_COL;
    else if ( eType == TYPE_ROW )
        ePtr = POINTER_PIVOT_ROW;

    return ePtr;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyDoubleClick( ScDPFieldType eType, size_t nFieldIndex )
{
    ScDPFuncDataVec* pArr = NULL;
    switch ( eType )
    {
        case TYPE_PAGE:     pArr = &aPageArr;   break;
        case TYPE_COL:      pArr = &aColArr;    break;
        case TYPE_ROW:      pArr = &aRowArr;    break;
        case TYPE_DATA:     pArr = &aDataArr;   break;
    }

    if ( pArr )
    {
        size_t nArrPos = 0;
        if( ScDPLabelData* pData = GetLabelData( (*pArr)[nFieldIndex]->mnCol, &nArrPos ) )
        {
            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

            switch ( eType )
            {
                case TYPE_PAGE:
                case TYPE_COL:
                case TYPE_ROW:
                {
                    // list of names of all data fields
                    std::vector< String > aDataFieldNames;
                    for( ScDPFuncDataVec::const_iterator aIt = aDataArr.begin(), aEnd = aDataArr.end();
                            (aIt != aEnd) && aIt->get(); ++aIt )
                    {
                        String aName( GetLabelString( (*aIt)->mnCol ) );
                        if( aName.Len() )
                            aDataFieldNames.push_back( aName );
                    }

                    bool bLayout = (eType == TYPE_ROW) &&
                        ((aDataFieldNames.size() > 1) || ((nFieldIndex + 1 < pArr->size()) && (*pArr)[nFieldIndex+1].get()));

                    AbstractScDPSubtotalDlg* pDlg = pFact->CreateScDPSubtotalDlg(
                        this, ScResId( RID_SCDLG_PIVOTSUBT ),
                        *xDlgDPObject, *pData, *(*pArr)[nFieldIndex], aDataFieldNames, bLayout );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        pDlg->FillLabelData( *pData );
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask;
                    }
                    delete pDlg;
                }
                break;

                case TYPE_DATA:
                {
                    AbstractScDPFunctionDlg* pDlg = pFact->CreateScDPFunctionDlg(
                        this, ScResId( RID_SCDLG_DPDATAFIELD ),
                        aLabelDataArr, *pData, *(*pArr)[nFieldIndex] );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask = pDlg->GetFuncMask();
                        (*pArr)[nFieldIndex]->maFieldRef = pDlg->GetFieldRef();

                        String aStr( GetFuncString ( aDataArr[nFieldIndex]->mnFuncMask ) );
                        aStr += GetLabelString( aDataArr[nFieldIndex]->mnCol );
                        aWndData.SetFieldText( aStr, nFieldIndex );
                    }
                    delete pDlg;
                }
            }
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

void ScDPLayoutDlg::NotifyRemoveField( ScDPFieldType eType, size_t nFieldIndex )
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

BOOL ScDPLayoutDlg::Contains( ScDPFuncDataVec* pArr, SCsCOL nCol, size_t& nAt )
{
    if ( !pArr )
        return FALSE;

    BOOL    bFound  = FALSE;
    size_t  i       = 0;

    while ( (i<pArr->size()) && ((*pArr)[i].get() != NULL) && !bFound )
    {
        bFound = ((*pArr)[i]->mnCol == nCol);
        if ( bFound )
            nAt = i;
        i++;
    }

    return bFound;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Remove( ScDPFuncDataVec* pArr, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    pArr->erase( pArr->begin() + nAt );
    pArr->push_back( ScDPFuncDataRef() );
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Insert( ScDPFuncDataVec* pArr, const ScDPFuncData& rFData, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    if ( (*pArr)[nAt].get() == NULL )
    {
        (*pArr)[nAt].reset( new ScDPFuncData( rFData ) );
    }
    else
    {
        if ( pArr->back().get() == NULL ) // mind. ein Slot frei?
        {
            pArr->insert( pArr->begin() + nAt, ScDPFuncDataRef( new ScDPFuncData( rFData ) ) );
            pArr->erase( pArr->end() - 1 );
        }
    }
}


//----------------------------------------------------------------------------

ScDPLabelData* ScDPLayoutDlg::GetLabelData( SCsCOL nCol, size_t* pnPos )
{
    ScDPLabelData* pData = 0;
    for( ScDPLabelDataVec::iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); !pData && (aIt != aEnd); ++aIt )
    {
        if( aIt->mnCol == nCol )
        {
            pData = &*aIt;
            if( pnPos ) *pnPos = aIt - aLabelDataArr.begin();
        }
    }
    return pData;
}


//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetLabelString( SCsCOL nCol )
{
    ScDPLabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData)
        return pData->maName;
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
    aWndPage.SetSizePixel( Size( MAX_PAGEFIELDS * OWIDTH / 2, 2 * OHEIGHT ) );
    aWndRow.SetSizePixel( Size( OWIDTH, MAX_FIELDS * OHEIGHT ) );
    aWndCol.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, 2 * OHEIGHT ) );
    aWndData.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, MAX_FIELDS * OHEIGHT ) );

    // #i29203# align right border of page window with data window
    long nDataPosX = aWndData.GetPosPixel().X() + aWndData.GetSizePixel().Width();
    aWndPage.SetPosPixel( Point( nDataPosX - aWndPage.GetSizePixel().Width(), aWndPage.GetPosPixel().Y() ) );

    // selection area
    aWndSelect.SetSizePixel( Size(
        2 * OWIDTH + SSPACE, LINE_SIZE * OHEIGHT + (LINE_SIZE - 1) * SSPACE ) );

    // scroll bar
    Point aSliderPos( aWndSelect.GetPosPixel() );
    Size aSliderSize( aWndSelect.GetSizePixel() );
    aSliderPos.Y() += aSliderSize.Height() + SSPACE;
    aSliderSize.Height() = GetSettings().GetStyleSettings().GetScrollBarSize();
    aSlider.SetPosSizePixel( aSliderPos, aSliderSize );

    aRectPage   = Rectangle( aWndPage.GetPosPixel(),    aWndPage.GetSizePixel() );
    aRectRow    = Rectangle( aWndRow.GetPosPixel(),     aWndRow.GetSizePixel() );
    aRectCol    = Rectangle( aWndCol.GetPosPixel(),     aWndCol.GetSizePixel() );
    aRectData   = Rectangle( aWndData.GetPosPixel(),    aWndData.GetSizePixel() );
    aRectSelect = Rectangle( aWndSelect.GetPosPixel(),  aWndSelect.GetSizePixel() );
}


//----------------------------------------------------------------------------

BOOL ScDPLayoutDlg::GetPivotArrays(    PivotField*  pPageArr,
                                       PivotField*  pColArr,
                                       PivotField*  pRowArr,
                                       PivotField*  pDataArr,
                                       USHORT&      rPageCount,
                                       USHORT&      rColCount,
                                       USHORT&      rRowCount,
                                       USHORT&      rDataCount )
{
    BOOL bFit = TRUE;
    USHORT i=0;

    for ( i=0; (i<aDataArr.size()) && (aDataArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pDataArr[i], *aDataArr[i] );
    rDataCount = i;

    for ( i=0; (i<aPageArr.size()) && (aPageArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pPageArr[i], *aPageArr[i] );
    rPageCount = i;

    for ( i=0; (i<aColArr.size()) && (aColArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pColArr[i], *aColArr[i] );
    rColCount = i;

    for ( i=0; (i<aRowArr.size()) && (aRowArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pRowArr[i], *aRowArr[i] );
    rRowCount = i;

    if ( rRowCount < aRowArr.size() )
        pRowArr[rRowCount++].nCol = PIVOT_DATA_FIELD;
    else if ( rColCount < aColArr.size() )
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
        PivotPageFieldArr aPageArr;
        PivotFieldArr   aColArr;
        PivotFieldArr   aRowArr;
        PivotFieldArr   aDataArr;
        USHORT          nPageCount;
        USHORT          nColCount;
        USHORT          nRowCount;
        USHORT          nDataCount;

        BOOL bFit = GetPivotArrays( aPageArr,   aColArr,   aRowArr,   aDataArr,
                                    nPageCount, nColCount, nRowCount, nDataCount );
        if ( bFit )
        {
            ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

            ScDPSaveData aSaveData;
            aSaveData.SetIgnoreEmptyRows( aBtnIgnEmptyRows.IsChecked() );
            aSaveData.SetRepeatIfEmpty( aBtnDetectCat.IsChecked() );
            aSaveData.SetColumnGrand( aBtnTotalCol.IsChecked() );
            aSaveData.SetRowGrand( aBtnTotalRow.IsChecked() );
            aSaveData.SetFilterButton( aBtnFilter.IsChecked() );
            aSaveData.SetDrillDown( aBtnDrillDown.IsChecked() );

            uno::Reference<sheet::XDimensionsSupplier> xSource = xDlgDPObject->GetSource();

            ScDPObject::ConvertOrientation( aSaveData, aPageArr, nPageCount,
                            sheet::DataPilotFieldOrientation_PAGE,   NULL, 0, 0, xSource, FALSE );
            ScDPObject::ConvertOrientation( aSaveData, aColArr,  nColCount,
                            sheet::DataPilotFieldOrientation_COLUMN, NULL, 0, 0, xSource, FALSE );
            ScDPObject::ConvertOrientation( aSaveData, aRowArr,  nRowCount,
                            sheet::DataPilotFieldOrientation_ROW,    NULL, 0, 0, xSource, FALSE );
            ScDPObject::ConvertOrientation( aSaveData, aDataArr, nDataCount,
                            sheet::DataPilotFieldOrientation_DATA,   NULL, 0, 0, xSource, FALSE,
                            aColArr, nColCount, aRowArr, nRowCount, aPageArr, nPageCount );

            //  "show all" property
            //! init from xDlgDPObject, set only changed values
            for( ScDPLabelDataVec::const_iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); aIt != aEnd; ++aIt )
            {
                if( ScDPSaveDimension* pDim = aSaveData.GetExistingDimensionByName( aIt->maName ) )
                {
                    pDim->SetUsedHierarchy( aIt->mnUsedHier );
                    pDim->SetShowEmpty( aIt->mbShowAll );
                    pDim->SetSortInfo( &aIt->maSortInfo );
                    pDim->SetLayoutInfo( &aIt->maLayoutInfo );
                    pDim->SetAutoShowInfo( &aIt->maShowInfo );

                    // visibility of members
                    if( const rtl::OUString* pItem = aIt->maMembers.getConstArray() )
                    {
                        sal_Int32 nVisIdx = 0, nVisSize = aIt->maVisible.getLength();
                        for( const rtl::OUString* pEnd = pItem + aIt->maMembers.getLength(); pItem != pEnd; ++pItem, ++nVisIdx )
                        {
                            bool bVis = (nVisIdx >= nVisSize) || aIt->maVisible[ nVisIdx ];
                            pDim->GetMemberByName( *pItem )->SetIsVisible( bVis );
                        }
                    }
                }
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

    size_t nFields = std::min< size_t >( aLabelDataArr.size() - nOffset, PAGE_SIZE );

    aWndSelect.ClearFields();

    size_t i=0;
    for ( i=0; i<nFields; i++ )
    {
        const ScDPLabelData& rData = aLabelDataArr[nOffset+i];
        aWndSelect.AddField( rData.maName, i );
        aSelectArr[i].reset( new ScDPFuncData( rData.mnCol, rData.mnFuncMask ) );
    }
    for ( ; i<aSelectArr.size(); i++ )
        aSelectArr[i].reset();

    aWndSelect.ModifySelectionOffset( nOffsetDiff );    // adjusts selection & redraws
    return 0;
}


