/*************************************************************************
 *
 *  $RCSfile: tabvwsh3.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-22 18:13:02 $
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

#include "scitems.hxx"
#include <svx/zoom.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/request.hxx>
#include <sfx2/topfrm.hxx>
#include <svtools/ptitem.hxx>
#include <svtools/stritem.hxx>

#ifdef VCL /* Comp.- Mode */
#include <vcl/msgbox.hxx>
#include <vcl/fonttype.hxx>
#else
#include <vcl/msgbox.hxx>
#include <vcl/gdiobj.hxx>
#endif

#include "globstr.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "inputwin.hxx"
#include "scresid.hxx"
#include "printfun.hxx"
#include "docsh.hxx"
#include "rangelst.hxx"
#include "prevwsh.hxx"
#include "rangeutl.hxx"
#include "reffact.hxx"
#include "uiitems.hxx"
#include "cell.hxx"
#include "inputhdl.hxx"
#include "scendlg.hxx"
#include "mtrindlg.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "dwfunctr.hxx"

#define IS_EDITMODE() GetViewData()->HasEditView( GetViewData()->GetActivePart() )
#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)
#define GET_STRING(nid) ((const SfxStringItem&)pReqArgs->Get(nid)).GetValue()
#define GET_UINT16(nid) ((const SfxUInt16Item&)pReqArgs->Get(nid)).GetValue()
#define RECALC_PAGE(pDocSh) ScPrintFunc( pDocSh, GetPrinter(), nCurTab ).UpdatePages()

//------------------------------------------------------------------

void ScTabViewShell::Execute( SfxRequest& rReq )
{
    SfxViewFrame*       pThisFrame  = GetViewFrame();
    SfxBindings&        rBindings   = pThisFrame->GetBindings();
    SfxApplication*     pSfxApp     = SFX_APP();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // der kommt beim MouseButtonUp
        HideListBox();                  // Autofilter-DropDown-Listbox

    switch ( nSlot )
    {
        case FID_INSERT_FILE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(FID_INSERT_FILE,TRUE,&pItem) == SFX_ITEM_SET )
                {
                    String aFileName = ((const SfxStringItem*)pItem)->GetValue();

                        // Einfuege-Position

                    Point aInsertPos;
                    if ( pReqArgs->GetItemState(FN_PARAM_1,TRUE,&pItem) == SFX_ITEM_SET )
                        aInsertPos = ((const SfxPointItem*)pItem)->GetValue();
                    else
                        aInsertPos = GetInsertPos();

                        //  als Link?

                    BOOL bAsLink = FALSE;
                    if ( pReqArgs->GetItemState(FN_PARAM_2,TRUE,&pItem) == SFX_ITEM_SET )
                        bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

                        // ausfuehren

                    PasteFile( aInsertPos, aFileName, bAsLink );
                }
            }
            break;

        case SID_OPENDLG_EDIT_PRINTAREA:
            {
                USHORT          nId  = ScPrintAreasDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case SID_CHANGE_PRINTAREA:
            {
                if ( pReqArgs )         // OK aus Dialog
                {
                    String aPrintStr = GET_STRING( SID_CHANGE_PRINTAREA );
                    String aRowStr   = GET_STRING( FN_PARAM_2 );
                    String aColStr   = GET_STRING( FN_PARAM_3 );

                    SetPrintRanges( &aPrintStr, &aColStr, &aRowStr, FALSE );

                    rReq.Done();
                }
            }
            break;

        case SID_ADD_PRINTAREA:
        case SID_DEFINE_PRINTAREA:      // Menue oder Basic
            {
                BOOL bAdd = ( nSlot == SID_ADD_PRINTAREA );
                if ( pReqArgs )
                {
                    String aPrintStr = GET_STRING( SID_DEFINE_PRINTAREA );
                    SetPrintRanges( &aPrintStr, NULL, NULL, bAdd );
                }
                else
                {
                    SetPrintRanges( NULL, NULL, NULL, bAdd );       // aus Selektion
                    rReq.Done();
                }
            }
            break;

        case SID_DELETE_PRINTAREA:
            {
                String aEmpty;
                SetPrintRanges( &aEmpty, NULL, NULL, FALSE );       // Druckbereich loeschen
                rReq.Done();
            }
            break;

        case FID_DEL_MANUALBREAKS:
            RemoveManualBreaks();
            rReq.Done();
            break;

        case FID_ADJUST_PRINTZOOM:
            AdjustPrintZoom();
            rReq.Done();
            break;

        case FID_RESET_PRINTZOOM:
            SetPrintZoom( 100, 0 );     // 100%, nicht auf Seiten
            rReq.Done();
            break;

        case SID_FORMATPAGE:
        case SID_STATUS_PAGESTYLE:
        case SID_HFEDIT:
            GetViewData()->GetDocShell()->
                ExecutePageStyle( *this, rReq, GetViewData()->GetTabNo() );
            break;

        case SID_JUMPTOMARK:
        case SID_CURRENTCELL:
            if ( pReqArgs )
            {
                String aAddress;
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET )
                    aAddress = ((const SfxStringItem*)pItem)->GetValue();
                else if ( nSlot == SID_JUMPTOMARK && pReqArgs->GetItemState(
                                            ID_VAL_DUMMY0, TRUE, &pItem ) == SFX_ITEM_SET )
                    aAddress = ((const SfxStringItem*)pItem)->GetValue();
                //  SID_JUMPTOMARK steht in der IDL mit ID_VAL_DUMMY0 als Parameter

                BOOL bUnmark = FALSE;
                if ( pReqArgs->GetItemState( FN_PARAM_1, TRUE, &pItem ) == SFX_ITEM_SET )
                    bUnmark = ((const SfxBoolItem*)pItem)->GetValue();

                BOOL bFound = FALSE;
                ScViewData* pViewData = GetViewData();
                ScDocument* pDoc      = pViewData->GetDocument();
                ScMarkData& rMark     = pViewData->GetMarkData();
                ScRange     aScRange;
                ScAddress   aScAddress;
                USHORT      nResult = aScRange.Parse( aAddress, pDoc );
                USHORT      nTab = pViewData->GetTabNo();
                BOOL        bMark = TRUE;

                // Ist es ein Bereich ?
                if( nResult & SCA_VALID )
                {
                    if ( nResult & SCA_TAB_3D )
                    {
                        if( aScRange.aStart.Tab() != nTab )
                            SetTabNo( nTab = aScRange.aStart.Tab() );
                    }
                    else
                    {
                        aScRange.aStart.SetTab( nTab );
                        aScRange.aEnd.SetTab( nTab );
                    }
                }
                // Ist es eine Zelle ?
                else if( (nResult=aScAddress.Parse( aAddress, pDoc )) & SCA_VALID )
                {
                    if ( nResult & SCA_TAB_3D )
                    {
                        if( aScAddress.Tab() != nTab )
                            SetTabNo( nTab = aScAddress.Tab() );
                    }
                    else
                        aScAddress.SetTab( nTab );

                    aScRange = ScRange( aScAddress, aScAddress );
                    // Zellen sollen nicht markiert werden
                    bMark = FALSE;
                }
                // Ist es benahmster Bereich (erst Namen dann DBBereiche) ?
                else
                {
                    ScRangeUtil     aRangeUtil;
                    if( aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_NAMES ) ||
                        aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_DBASE ) )
                    {
                        nResult |= SCA_VALID;
                        if( aScRange.aStart.Tab() != nTab )
                            SetTabNo( nTab = aScRange.aStart.Tab() );
                    }
                }

                if ( !(nResult & SCA_VALID) &&
                        ByteString(aAddress, RTL_TEXTENCODING_ASCII_US).IsNumericAscii() )
                {
                    sal_Int32 nNumeric = aAddress.ToInt32();
                    if ( nNumeric > 0 && nNumeric <= MAXROW+1 )
                    {
                        //  1-basierte Zeilennummer

                        aScAddress.SetRow( (USHORT)(nNumeric - 1) );
                        aScAddress.SetCol( pViewData->GetCurX() );
                        aScAddress.SetTab( nTab );
                        aScRange = ScRange( aScAddress, aScAddress );
                        bMark    = FALSE;
                        nResult  = SCA_VALID;
                    }
                }

                if ( aScRange.aStart.Row() > MAXROW || aScRange.aEnd.Row() > MAXROW )
                    nResult = 0;

                // wir haben was gefunden
                if( nResult & SCA_VALID )
                {
                    bFound = TRUE;
                    USHORT nCol = aScRange.aStart.Col();
                    USHORT nRow = aScRange.aStart.Row();
                    BOOL bNothing = ( pViewData->GetCurX()==nCol && pViewData->GetCurY()==nRow );

                    // markieren
                    if( bMark )
                    {
                        if (rMark.IsMarked())           // ist derselbe Bereich schon markiert?
                        {
                            ScRange aOldMark;
                            rMark.GetMarkArea( aOldMark );
                            aOldMark.Justify();
                            ScRange aCurrent = aScRange;
                            aCurrent.Justify();
                            bNothing = ( aCurrent == aOldMark );
                        }
                        else
                            bNothing = FALSE;

                        if (!bNothing)
                            MarkRange( aScRange, FALSE );   // Cursor kommt hinterher...
                    }
                    else
                    {
                        //  aus dem Navigator die Selektion nur aufheben, wenn Parameter da
                        if( rReq.IsAPI() || bUnmark )
                        {
                            MoveCursorAbs( nCol, nRow,
                                SC_FOLLOW_NONE, FALSE, FALSE );
                        }
                    }

                    // und Cursor setzen

                    // zusammengefasste Zellen beruecksichtigen:
                    while ( pDoc->IsHorOverlapped( nCol, nRow, nTab ) )     //! ViewData !!!
                        --nCol;
                    while ( pDoc->IsVerOverlapped( nCol, nRow, nTab ) )
                        --nRow;

                    //  Navigator-Aufrufe sind nicht API!!!

                    if( bNothing )
                    {
                        if (rReq.IsAPI())
                            rReq.Ignore();      // wenn Makro, dann gar nix
                        else
                            rReq.Done();        // sonst wenigstens aufzeichnen
                    }
                    else
                    {
                        pViewData->ResetOldCursor();
                        SetCursor( nCol, nRow );
                        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
                        rBindings.Invalidate( SID_CURRENTCELL );
                        rBindings.Update( nSlot );

                        if (!rReq.IsAPI())
                            rReq.Done();
                    }

                    rReq.SetReturnValue( SfxStringItem( SID_CURRENTCELL, aAddress ) );
                }

                if (!bFound)    // kein gueltiger Bereich
                {
                    //  wenn es ein Tabellenname ist, umschalten (fuer Navigator/URL's)

                    USHORT nNameTab;
                    if ( pDoc->GetTable( aAddress, nNameTab ) )
                    {
                        bFound = TRUE;
                        if ( nNameTab != nTab )
                            SetTabNo( nNameTab );
                    }
                }

                if ( !bFound && nSlot == SID_JUMPTOMARK )
                {
                    //  Grafik-Objekte probieren (nur bei URL's)

                    bFound = SelectObject( aAddress );
                }

                if (!bFound && !rReq.IsAPI())
                    ErrorMessage( STR_ERR_INVALID_AREA );
            }
            break;

        case SID_CURRENTOBJECT:
            if ( pReqArgs )
            {
                String aName = ((const SfxStringItem&)pReqArgs->Get(nSlot)).GetValue();
                SelectObject( aName );
            }
            break;

        case SID_CURRENTTAB:
            if ( pReqArgs )
            {
                //  Tabelle fuer Basic ist 1-basiert
                USHORT nTab = ((const SfxUInt16Item&)pReqArgs->Get(nSlot)).GetValue() - 1;
                ScDocument* pDoc = GetViewData()->GetDocument();
                if ( nTab < pDoc->GetTableCount() )
                {
                    SetTabNo( nTab );
                    rBindings.Update( nSlot );

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                //! sonst Fehler ?
            }
            break;

        case SID_CURRENTDOC:
            if ( pReqArgs )
            {
                String aStrDocName( ((const SfxStringItem&)pReqArgs->
                                        Get(nSlot)).GetValue() );

                SfxViewFrame*   pFrame = NULL;
                ScDocShell*     pDocSh = (ScDocShell*)SfxObjectShell::GetFirst();
                BOOL            bFound = FALSE;

                // zu aktivierenden ViewFrame suchen

                while ( pDocSh && !bFound )
                {
                    if ( pDocSh->GetTitle() == aStrDocName )
                    {
                        pFrame = SfxViewFrame::GetFirst( pDocSh, TYPE(SfxTopViewFrame) );
                        bFound = ( NULL != pFrame );
                    }

                    pDocSh = (ScDocShell*)SfxObjectShell::GetNext( *pDocSh );
                }

                if ( bFound )
                    if ( pFrame->ISA(SfxTopViewFrame) )
                        pFrame->GetFrame()->Appear();

                rReq.Ignore();//XXX wird von SFX erledigt
            }

        case SID_ATTR_SIZE://XXX ???
            break;


        case SID_PRINTPREVIEW:
            {
                if ( !pThisFrame->ISA( SfxInPlaceFrame ) )          // nicht bei OLE
                {
                    //  print preview is now always in the same frame as the tab view
                    //  -> always switch this frame back to normal view
                    //  (ScPreviewShell ctor reads view data)

                    pThisFrame->GetBindings().Execute( SID_VIEWSHELL1,
                                                NULL, 0, SFX_CALLMODE_ASYNCHRON );
                }
                //  else Fehler (z.B. Ole)
            }
            break;

        case SID_DETECTIVE_DEL_ALL:
            DetectiveDelAll();
            rReq.Done();
            break;

        //  SID_TABLE_ACTIVATE und SID_MARKAREA werden von Basic aus an der versteckten
        //  View aufgerufen, um auf der sichtbaren View zu markieren/umzuschalten:

        case SID_TABLE_ACTIVATE:
            DBG_ERROR("old slot SID_TABLE_ACTIVATE");
            break;

        case SID_REPAINT:
            PaintGrid();
            PaintTop();
            PaintLeft();
            PaintExtras();
            rReq.Done();
            break;

        case FID_PAGEBREAKMODE:
            {
                BOOL bSet = !GetViewData()->IsPagebreakMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->
                        GetItemState(FID_PAGEBREAKMODE, TRUE, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                SetPagebreakMode( bSet );
                UpdatePageBreakData();
                SetCurSubShell( GetCurObjectSelectionType(), TRUE );
                PaintGrid();
                PaintTop();
                PaintLeft();
                rBindings.Invalidate( FID_PAGEBREAKMODE );
                rReq.Done();
            }
            break;

        case FID_FUNCTION_BOX:
            {
                if ( rReq.GetArgs() )
                    pThisFrame->SetChildWindow(ScFunctionChildWindow::GetChildWindowId(),
                                        ((const SfxBoolItem&) (rReq.GetArgs()->
                                        Get(FID_FUNCTION_BOX))).GetValue());
                else
                    pThisFrame->ToggleChildWindow(ScFunctionChildWindow::GetChildWindowId() );

                GetViewFrame()->GetBindings().Invalidate(FID_FUNCTION_BOX);
                rReq.Done ();
            }
            break;


        case FID_TOGGLESYNTAX:
            {
                BOOL bSet = !GetViewData()->IsSyntaxMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                GetViewData()->SetSyntaxMode( bSet );
                PaintGrid();
                rBindings.Invalidate( FID_TOGGLESYNTAX );
                rReq.Done();
            }
            break;
        case FID_TOGGLEHEADERS:
            {
                BOOL bSet = !GetViewData()->IsHeaderMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                GetViewData()->SetHeaderMode( bSet );
                RepeatResize();
                rBindings.Invalidate( FID_TOGGLEHEADERS );
                rReq.Done();
            }
            break;
        case FID_TOGGLEINPUTLINE:
            {
                USHORT          nId  = ScInputWindowWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                BOOL bSet = ( pWnd == NULL );
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();

                pThisFrame->SetChildWindow( nId, bSet );

                rBindings.Invalidate( FID_TOGGLEINPUTLINE );
            }
            break;

        case SID_ATTR_ZOOM: // Statuszeile
        case FID_SCALE:
            {
                SvxZoomType eOldZoomType = GetZoomType();
                SvxZoomType eZoomType = eOldZoomType;
                const Fraction& rOldY = GetViewData()->GetZoomY();  // Y wird angezeigt
                USHORT nOldZoom = (USHORT)(( rOldY.GetNumerator() * 100 )
                                            / rOldY.GetDenominator());
                USHORT nZoom = nOldZoom;
                BOOL bCancel = FALSE;

                if ( pReqArgs )
                {
                    const SvxZoomItem& rZoomItem = (const SvxZoomItem&)
                                                   pReqArgs->Get(SID_ATTR_ZOOM);

                    eZoomType = rZoomItem.GetType();
                    nZoom     = rZoomItem.GetValue();
                }
                else
                {
                    SfxItemSet      aSet     ( GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
                    SvxZoomItem     aZoomItem( eOldZoomType, nOldZoom, SID_ATTR_ZOOM );
                    SvxZoomDialog*  pDlg = NULL;
                    ScMarkData&     rMark = GetViewData()->GetMarkData();
                    USHORT          nBtnFlags =   SVX_ZOOM_ENABLE_50
                                                | SVX_ZOOM_ENABLE_75
                                                | SVX_ZOOM_ENABLE_100
                                                | SVX_ZOOM_ENABLE_150
                                                | SVX_ZOOM_ENABLE_200
                                                | SVX_ZOOM_ENABLE_WHOLEPAGE
                                                | SVX_ZOOM_ENABLE_PAGEWIDTH;

                    if ( rMark.IsMarked() )
                        nBtnFlags = nBtnFlags | SVX_ZOOM_ENABLE_OPTIMAL;

                    aZoomItem.SetValueSet( nBtnFlags );
                    aSet.Put( aZoomItem );
                    pDlg = new SvxZoomDialog( GetDialogParent(), aSet );
                    pDlg->SetLimits( MINZOOM, MAXZOOM );

                    bCancel = ( RET_CANCEL == pDlg->Execute() );

                    if ( !bCancel )
                    {
                        const SvxZoomItem&  rZoomItem = (const SvxZoomItem&)
                                                pDlg->GetOutputItemSet()->
                                                    Get( SID_ATTR_ZOOM );

                        eZoomType = rZoomItem.GetType();
                        nZoom     = rZoomItem.GetValue();
                    }

                    delete pDlg;
                }

                if ( !bCancel )
                {
                    if ( eZoomType == SVX_ZOOM_PERCENT )
                    {
                        if ( nZoom < MINZOOM )  nZoom = MINZOOM;
                        if ( nZoom > MAXZOOM )  nZoom = MAXZOOM;
                    }
                    else
                    {
                        nZoom = CalcZoom( eZoomType, nOldZoom );
                        bCancel = nZoom == 0;
                    }

                    switch ( eZoomType )
                    {
                        case SVX_ZOOM_WHOLEPAGE:
                        case SVX_ZOOM_PAGEWIDTH:
                            SetZoomType( eZoomType );
                            break;

                        default:
                            SetZoomType( SVX_ZOOM_PERCENT );
                    }
                }

                if ( nZoom != nOldZoom && !bCancel )
                {
                    if (!GetViewData()->IsPagebreakMode())
                    {
                        ScModule* pScMod = SC_MOD();
                        ScAppOptions aNewOpt = pScMod->GetAppOptions();
                        aNewOpt.SetZoom( nZoom );
                        aNewOpt.SetZoomType( GetZoomType() );
                        pScMod->SetAppOptions( aNewOpt );
                    }
                    Fraction aFract( nZoom, 100 );
                    SetZoom( aFract, aFract );
                    PaintGrid();
                    PaintTop();
                    PaintLeft();
                    rBindings.Invalidate( SID_ATTR_ZOOM );
                    rReq.Done();
                }
            }
            break;

        case SID_OPENDLG_MODCHART:
            {
                bChartDlgIsEdit = ( nSlot == SID_OPENDLG_MODCHART );
                if (bChartDlgIsEdit)
                {
                    aEditChartName = GetSelectedChartName();
                    DrawDeselectAll();      // flackert sonst bei Ref-Input
                }

                ResetChartArea();

                USHORT          nId  = ScChartDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;



        //----------------------------------------------------------------

        case FID_TAB_SELECTALL:
            SelectAllTables();
            rReq.Done();
            break;



        case SID_OUTLINE_DELETEALL:
            RemoveAllOutlines();
            rReq.Done();
            break;

        case SID_AUTO_OUTLINE:
            AutoOutline();
            rReq.Done();
            break;


        case SID_WINDOW_SPLIT:
            {
                ScSplitMode eHSplit = GetViewData()->GetHSplitMode();
                ScSplitMode eVSplit = GetViewData()->GetVSplitMode();
                if ( eHSplit == SC_SPLIT_NORMAL || eVSplit == SC_SPLIT_NORMAL )     // aufheben
                    RemoveSplit();
                else if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )      // normal
                    FreezeSplitters( FALSE );
                else                                                                // erzeugen
                    SplitAtCursor();
                rReq.Done();

                InvalidateSplit();
            }
            break;

        case SID_WINDOW_FIX:
            {
                ScSplitMode eHSplit = GetViewData()->GetHSplitMode();
                ScSplitMode eVSplit = GetViewData()->GetVSplitMode();
                if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )           // aufheben
                    RemoveSplit();
                else
                    FreezeSplitters( TRUE );        // erzeugen oder fixieren
                rReq.Done();

                InvalidateSplit();
            }
            break;

        //  ----------------------------------------------------------------

        case FID_CHG_SHOW:
            {
                USHORT          nId  = ScHighlightChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case FID_CHG_ACCEPT:
            {
                pThisFrame->ToggleChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId());
                GetViewFrame()->GetBindings().Invalidate(FID_CHG_ACCEPT);
                rReq.Done ();

                /*
                USHORT          nId  = ScAcceptChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
                */
            }
            break;

        case FID_CHG_COMMENT:
            {
                ScViewData* pData = GetViewData();
                ScAddress aCursorPos( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
                ScDocShell* pDocSh = pData->GetDocShell();

                ScChangeAction* pAction = pDocSh->GetChangeAction( aCursorPos );
                if ( pAction )
                {
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                         pReqArgs->GetItemState( nSlot, TRUE, &pItem ) == SFX_ITEM_SET &&
                         pItem->ISA( SfxStringItem ) )
                    {
                        String aComment = ((const SfxStringItem*)pItem)->GetValue();
                        pDocSh->SetChangeComment( pAction, aComment );
                        rReq.Done();
                    }
                    else
                    {
                        pDocSh->ExecuteChangeCommentDialog( pAction, GetDialogParent() );
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_CREATE_SW_DRAWVIEW:
            //  wird von den Forms gerufen, wenn die DrawView mit allem Zubehoer
            //  angelegt werden muss
            if (!GetScDrawView())
            {
                GetViewData()->GetDocShell()->MakeDrawLayer();
                rBindings.InvalidateAll(FALSE);
            }
            break;

        case SID_STATUS_DOCPOS:
            {
                //! Navigator an-/ausschalten (wie im Writer) ???
                //!GetViewData()->GetDispatcher().Execute( SID_NAVIGATOR,
                //!                       SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
            }
            break;

        case FID_PROTECT_DOC:
            {
                ScDocument*         pDoc = GetViewData()->GetDocument();
                SfxPasswordDialog*  pDlg;

                if (pDoc->IsDocProtected())
                {
                    BOOL    bCancel = FALSE;
                    String  aPassword;

                    if (pDoc->GetDocPassword().getLength())
                    {
                        String  aText( ScResId(SCSTR_PASSWORD) );

                        pDlg = new SfxPasswordDialog(   GetDialogParent(), &aText );
                        pDlg->SetText( ScResId(SCSTR_UNPROTECTDOC) );
                        pDlg->SetMinLen( 0 );
                        pDlg->SetHelpId( FID_PROTECT_DOC );
                        pDlg->SetEditHelpId( HID_PASSWD_DOC );

                        if (pDlg->Execute() == RET_OK)
                            aPassword = pDlg->GetPassword();
                        else
                            bCancel = TRUE;
                        delete pDlg;
                    }
                    if (!bCancel)
                    {
                        Unprotect( TABLEID_DOC, aPassword );
                        rReq.Done();
                    }
                }
                else
                {
                    String aText( ScResId(SCSTR_PASSWORDOPT) );

                    pDlg = new SfxPasswordDialog(   GetDialogParent(), &aText );
                    pDlg->SetText( ScResId(SCSTR_PROTECTDOC) );
                    pDlg->SetMinLen( 0 );
                    pDlg->SetHelpId( FID_PROTECT_DOC );
                    pDlg->SetEditHelpId( HID_PASSWD_DOC );
                    pDlg->ShowExtras( SHOWEXTRAS_CONFIRM );

                    if (pDlg->Execute() == RET_OK)
                    {
                        String aPassword = pDlg->GetPassword();
                        BOOL bOk = FALSE;

                        if ( aPassword.Len() )
                        {
                            //  compare strings
                            if ( aPassword == pDlg->GetConfirm() )
                                bOk = TRUE;
                            else
                            {
                                InfoBox aBox( GetDialogParent(),
                                                String( ScResId( SCSTR_WRONGPASSWORD ) ) );
                                aBox.Execute();
                            }
                        }
                        else
                            bOk = TRUE;     // ohne Passwort

                        if (bOk)
                        {
                            Protect( TABLEID_DOC, aPassword );
                            rReq.Done();
                        }
                    }

                    delete pDlg;
                }
                rBindings.Invalidate( FID_PROTECT_DOC );
            }
            break;


        case FID_PROTECT_TABLE:
            {
                ScDocument*         pDoc = GetViewData()->GetDocument();
                USHORT              nTab = GetViewData()->GetTabNo();
                SfxPasswordDialog*  pDlg;
                String              aPassword;
                BOOL                bCancel = FALSE;
                BOOL                bOldProtection = pDoc->IsTabProtected(nTab);
                BOOL                bNewProtection = ! bOldProtection;

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                        aPassword = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FID_PROTECT_TABLE, &pItem ) )
                        bNewProtection = ((const SfxBoolItem*)pItem)->GetValue();
                }

                if( ! rReq.IsAPI() )
                {
                    if (bOldProtection)
                    {
                        if (pDoc->GetTabPassword(nTab).getLength())
                        {
                            String  aText( ScResId(SCSTR_PASSWORD) );

                            pDlg = new SfxPasswordDialog( GetDialogParent(), &aText );
                            pDlg->SetText( ScResId(SCSTR_UNPROTECTTAB) );
                            pDlg->SetMinLen( 0 );
                            pDlg->SetHelpId( FID_PROTECT_TABLE );
                            pDlg->SetEditHelpId( HID_PASSWD_TABLE );

                            if (pDlg->Execute() == RET_OK)
                                aPassword = pDlg->GetPassword();
                            else
                                bCancel = TRUE;

                            delete pDlg;
                        }
                    }
                    else
                    {
                        String aText( ScResId(SCSTR_PASSWORDOPT) );

                        pDlg = new SfxPasswordDialog( GetDialogParent(), &aText );
                        pDlg->SetText( ScResId(SCSTR_PROTECTTAB) );
                        pDlg->SetMinLen( 0 );
                        pDlg->SetHelpId( FID_PROTECT_TABLE );
                        pDlg->SetEditHelpId( HID_PASSWD_TABLE );
                        pDlg->ShowExtras( SHOWEXTRAS_CONFIRM );

                        if (pDlg->Execute() == RET_OK)
                        {
                            aPassword = pDlg->GetPassword();
                            if ( aPassword.Len() )
                            {
                                //  compare strings
                                if ( aPassword != pDlg->GetConfirm() )
                                {
                                    InfoBox aBox( GetDialogParent(),
                                                    String( ScResId( SCSTR_WRONGPASSWORD ) ) );
                                    aBox.Execute();
                                    bCancel = TRUE;
                                }
                            }
                        }
                        else
                            bCancel = TRUE;

                        delete pDlg;
                    }
                }

                if( ! bCancel && bNewProtection!=bOldProtection )
                {
                    if ( bOldProtection )
                        Unprotect( nTab, aPassword );
                    else
                        Protect( nTab, aPassword );

                    if( ! rReq.IsAPI() )
                    {
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_TABLE, bNewProtection ) );
                        rReq.AppendItem( SfxStringItem( FN_PARAM_1, aPassword ) );
                        rReq.Done();
                    }
                }

                TabChanged();
                UpdateInputHandler(TRUE);   // damit sofort wieder eingegeben werden kann
                SelectionChanged();
            }
            break;

        default:
            DBG_ERROR("Unbekannter Slot bei ScTabViewShell::Execute");
            break;
    }
}



