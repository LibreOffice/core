/*************************************************************************
 *
 *  $RCSfile: cellsh1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 10:18:10 $
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

//------------------------------------------------------------------

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SI_NOSBXCONTROLS
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS

//------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>

#define _ZFORLIST_DECLARE_TABLE
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <vcl/msgbox.hxx>
#include <so3/pastedlg.hxx>
#include <sot/formats.hxx>
#include <vcl/clip.hxx>
#include <svx/postattr.hxx>
#include <svx/fontitem.hxx>
#include <svx/charmap.hxx>
#include <sfx2/passwd.hxx>
#include <svx/hlnkitem.hxx>
#include <svtools/sbxcore.hxx>
#include <svtools/useroptions.hxx>
#include <vcl/waitobj.hxx>

#include "cellsh.hxx"
#include "sc.hrc"
#include "document.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "inscldlg.hxx"
#include "inscodlg.hxx"
#include "delcldlg.hxx"
#include "delcodlg.hxx"
#include "filldlg.hxx"
#include "groupdlg.hxx"
#include "impex.hxx"
#include "reffind.hxx"
#include "namecrea.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "namepast.hxx"
#include "inputhdl.hxx"

#include "globstr.hrc"


#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

//------------------------------------------------------------------
void ScCellShell::ExecuteEdit( SfxRequest& rReq )
{
    ScModule*           pScMod      = SC_MOD();
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SfxBindings&        rBindings   = pTabViewShell->GetViewFrame()->GetBindings();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    USHORT              nSlot       = rReq.GetSlot();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

                                    // Eingabe beenden
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        switch ( nSlot )
        {
            case FID_DEFINE_NAME:
            case FID_USE_NAME:
            case FID_INSERT_NAME:
            case SID_SPELLING:

            pScMod->InputEnterHandler();
            pTabViewShell->UpdateInputHandler();
            break;

            default:
            break;
        }
    }

    switch ( nSlot )
    {
        //
        //  Einfuegen / Loeschen von Zellen / Zeilen / Spalten
        //

        case FID_INS_ROW:
            pTabViewShell->InsertCells(INS_INSROWS);
            rReq.Done();
            break;

        case FID_INS_COLUMN:
            pTabViewShell->InsertCells(INS_INSCOLS);
            rReq.Done();
            break;

        case FID_INS_CELLSDOWN:
            pTabViewShell->InsertCells(INS_CELLSDOWN);
            rReq.Done();
            break;

        case FID_INS_CELLSRIGHT:
            pTabViewShell->InsertCells(INS_CELLSRIGHT);
            rReq.Done();
            break;

        case SID_DEL_ROWS:
            pTabViewShell->DeleteCells( DEL_DELROWS );
            rReq.Done();
            break;

        case SID_DEL_COLS:
            pTabViewShell->DeleteCells( DEL_DELCOLS );
            rReq.Done();
            break;

        case FID_INS_CELL:
            {
                InsCellCmd eCmd=INS_NONE;

                if ( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    String aFlags;

                    if( IS_AVAILABLE( FID_INS_CELL, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();
                    if( aFlags.Len() )
                    {
                        switch( aFlags.GetChar(0) )
                        {
                            case 'V': eCmd = INS_CELLSDOWN ;break;
                            case '>': eCmd = INS_CELLSRIGHT ;break;
                            case 'R': eCmd = INS_INSROWS ;break;
                            case 'C': eCmd = INS_INSCOLS ;break;
                        }
                    }
                }
                else
                {
                    if ( GetViewData()->SimpleColMarked() )
                        eCmd = INS_INSCOLS;
                    else if ( GetViewData()->SimpleRowMarked() )
                        eCmd = INS_INSROWS;
                    else
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        BOOL bTheFlag=(pDoc->GetChangeTrack()!=NULL);

                        ScInsertCellDlg* pDlg = new ScInsertCellDlg( pTabViewShell->GetDialogParent(),
                                                            bTheFlag);
                        if (pDlg->Execute() == RET_OK)
                            eCmd = pDlg->GetInsCellCmd();
                        delete pDlg;
                    }
                }

                if (eCmd!=INS_NONE)
                {
                    pTabViewShell->InsertCells( eCmd );

                    if( ! rReq.IsAPI() )
                    {
                        String aParam;

                        switch( eCmd )
                        {
                            case INS_CELLSDOWN: aParam='V'; break;
                            case INS_CELLSRIGHT: aParam='>'; break;
                            case INS_INSROWS: aParam='R'; break;
                            case INS_INSCOLS: aParam='C'; break;
                        }
                        rReq.AppendItem( SfxStringItem( FID_INS_CELL, aParam ) );
                        rReq.Done();
                    }
                }
            }
            break;

        case FID_DELETE_CELL:
            {
                DelCellCmd eCmd = DEL_NONE;

                if ( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    String aFlags;

                    if( IS_AVAILABLE( FID_DELETE_CELL, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();
                    if( aFlags.Len() )
                    {
                        switch( aFlags.GetChar(0) )
                        {
                            case 'U': eCmd = DEL_CELLSUP ;break;
                            case 'L': eCmd = DEL_CELLSLEFT ;break;
                            case 'R': eCmd = DEL_DELROWS ;break;
                            case 'C': eCmd = DEL_DELCOLS ;break;
                        }
                    }
                }
                else
                {
                    if ( GetViewData()->SimpleColMarked() )
                        eCmd = DEL_DELCOLS;
                    else if ( GetViewData()->SimpleRowMarked() )
                        eCmd = DEL_DELROWS;
                    else
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        BOOL bTheFlag=GetViewData()->GetMarkData().IsMultiMarked() ||
                                    (pDoc->GetChangeTrack()!=NULL);

                        ScDeleteCellDlg* pDlg = new ScDeleteCellDlg(
                                pTabViewShell->GetDialogParent(),bTheFlag);

                        if (pDlg->Execute() == RET_OK)
                            eCmd = pDlg->GetDelCellCmd();
                        delete pDlg;
                    }
                }

                if (eCmd != DEL_NONE )
                {
                    pTabViewShell->DeleteCells( eCmd );

                    if( ! rReq.IsAPI() )
                    {
                        String aParam;

                        switch( eCmd )
                        {
                            case DEL_CELLSUP: aParam='U'; break;
                            case DEL_CELLSLEFT: aParam='L'; break;
                            case DEL_DELROWS: aParam='R'; break;
                            case DEL_DELCOLS: aParam='C'; break;
                        }
                        rReq.AppendItem( SfxStringItem( FID_DELETE_CELL, aParam ) );
                        rReq.Done();
                    }
                }
            }
            break;

        //
        //  Inhalte von Zellen loeschen
        //

        case SID_DELETE_CONTENTS:
            pTabViewShell->DeleteContents( IDF_CONTENTS );
            rReq.Done();
            break;

        case SID_DELETE:
            {
                USHORT  nFlags = IDF_NONE;

                if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    String  aFlags = 'A';

                    if( IS_AVAILABLE( SID_DELETE, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();

                    aFlags.ToUpperAscii();
                    BOOL    bCont = TRUE;

                    for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                    {
                        switch( aFlags.GetChar(i) )
                        {
                            case 'A': // Alle
                            nFlags |= IDF_ALL;
                            bCont = FALSE; // nicht mehr weitermachen!
                            break;
                            case 'S': nFlags |= IDF_STRING; break;
                            case 'V': nFlags |= IDF_VALUE; break;
                            case 'D': nFlags |= IDF_DATETIME; break;
                            case 'F': nFlags |= IDF_FORMULA; break;
                            case 'N': nFlags |= IDF_NOTE; break;
                            case 'T': nFlags |= IDF_ATTRIB; break;
                            case 'O': nFlags |= IDF_OBJECTS; break;
                        }
                    }
                }
                else
                {
                    if (pTabViewShell->SelectionEditable())
                    {
                        ScDeleteContentsDlg* pDlg = new ScDeleteContentsDlg(    pTabViewShell->GetDialogParent() );
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        USHORT nTab = GetViewData()->GetTabNo();
                        if ( pDoc->IsTabProtected(nTab) )
                            pDlg->DisableObjects();
                        if (pDlg->Execute() == RET_OK)
                        {
                            nFlags = pDlg->GetDelContentsCmdBits();
                        }
                        delete pDlg;
                    }
                    else
                        pTabViewShell->ErrorMessage(STR_PROTECTIONERR);
                }

                if( nFlags != IDF_NONE )
                {
                    pTabViewShell->DeleteContents( nFlags );

                    if( ! rReq.IsAPI() )
                    {
                        String  aFlags;

                        if( nFlags == IDF_ALL )
                        {
                            aFlags += 'A';
                        }
                        else
                        {
                            if( nFlags & IDF_STRING ) aFlags += 'S';
                            if( nFlags & IDF_VALUE ) aFlags += 'V';
                            if( nFlags & IDF_DATETIME ) aFlags += 'D';
                            if( nFlags & IDF_FORMULA ) aFlags += 'F';
                            if( nFlags & IDF_NOTE ) aFlags += 'N';
                            if( nFlags & IDF_ATTRIB ) aFlags += 'T';
                            if( nFlags & IDF_OBJECTS ) aFlags += 'O';
                        }

                        rReq.AppendItem( SfxStringItem( SID_DELETE, aFlags ) );
                        rReq.Done();
                    }
                }
            }
            break;

        //
        //  Ausfuellen...
        //

        case FID_FILL_TO_BOTTOM:
            pTabViewShell->FillSimple( FILL_TO_BOTTOM );
            rReq.Done();
            break;

        case FID_FILL_TO_RIGHT:
            pTabViewShell->FillSimple( FILL_TO_RIGHT );
            rReq.Done();
            break;

        case FID_FILL_TO_TOP:
            pTabViewShell->FillSimple( FILL_TO_TOP );
            rReq.Done();
            break;

        case FID_FILL_TO_LEFT:
            pTabViewShell->FillSimple( FILL_TO_LEFT );
            rReq.Done();
            break;

        case FID_FILL_TAB:
            {
                USHORT nFlags = IDF_NONE;
                USHORT nFunction = PASTE_NOFUNC;
                BOOL bSkipEmpty = FALSE;
                BOOL bAsLink    = FALSE;

                if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    String  aFlags = 'A';

                    if( IS_AVAILABLE( FID_FILL_TAB, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();

                    aFlags.ToUpperAscii();
                    BOOL    bCont = TRUE;

                    for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                    {
                        switch( aFlags.GetChar(i) )
                        {
                            case 'A': // Alle
                            nFlags |= IDF_ALL;
                            bCont = FALSE; // nicht mehr weitermachen!
                            break;
                            case 'S': nFlags |= IDF_STRING; break;
                            case 'V': nFlags |= IDF_VALUE; break;
                            case 'D': nFlags |= IDF_DATETIME; break;
                            case 'F': nFlags |= IDF_FORMULA; break;
                            case 'N': nFlags |= IDF_NOTE; break;
                            case 'T': nFlags |= IDF_ATTRIB; break;
                        }
                    }
                }
                else
                {
                    ScInsertContentsDlg* pDlg =
                        new ScInsertContentsDlg(pTabViewShell->GetDialogParent(),
                                                 0, /* nCheckDefaults */
                                                 &ScGlobal::GetRscString(STR_FILL_TAB) );
                    pDlg->SetFillMode(TRUE);

                    if (pDlg->Execute() == RET_OK)
                    {
                        nFlags     = pDlg->GetInsContentsCmdBits();
                        nFunction  = pDlg->GetFormulaCmdBits();
                        bSkipEmpty = pDlg->IsSkipEmptyCells();
                        bAsLink    = pDlg->IsLink();
                        //  MoveMode gibt's bei Tabelle fuellen nicht
                    }
                    delete pDlg;
                }

                if( nFlags != IDF_NONE )
                {
                    pTabViewShell->FillTab( nFlags, nFunction, bSkipEmpty, bAsLink );

                    if( ! rReq.IsAPI() )
                    {
                        String  aFlags;

                        if( nFlags == IDF_ALL )
                        {
                            aFlags += 'A';
                        }
                        else
                        {
                            if( nFlags & IDF_STRING ) aFlags += 'S';
                            if( nFlags & IDF_VALUE ) aFlags += 'V';
                            if( nFlags & IDF_DATETIME ) aFlags += 'D';
                            if( nFlags & IDF_FORMULA ) aFlags += 'F';
                            if( nFlags & IDF_NOTE ) aFlags += 'N';
                            if( nFlags & IDF_ATTRIB ) aFlags += 'T';
                        }

                        rReq.AppendItem( SfxStringItem( FID_FILL_TAB, aFlags ) );
                        rReq.Done();
                    }
                }
            }
            break;

        case FID_FILL_SERIES:
            {
                USHORT nStartCol;
                USHORT nStartRow;
                USHORT nStartTab;
                USHORT nEndCol;
                USHORT nEndRow;
                USHORT nEndTab;
                USHORT nPossDir = FDS_OPT_NONE;
                FillDir     eFillDir     = FILL_TO_BOTTOM;
                FillCmd     eFillCmd     = FILL_LINEAR;
                FillDateCmd eFillDateCmd = FILL_DAY;
                double fStartVal = MAXDOUBLE;
                double fIncVal   = 1;
                double fMaxVal   = MAXDOUBLE;
                BOOL   bDoIt     = FALSE;

                GetViewData()->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                              nEndCol, nEndRow, nEndTab );

                if( nStartCol!=nEndCol )
                {
                    nPossDir |= FDS_OPT_HORZ;
                    eFillDir=FILL_TO_RIGHT;
                }

                if( nStartRow!=nEndRow )
                {
                    nPossDir |= FDS_OPT_VERT;
                    eFillDir=FILL_TO_BOTTOM;
                }

                ScDocument*      pDoc = GetViewData()->GetDocument();
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    String  aFillDir, aFillCmd, aFillDateCmd;
                    String  aFillStep, aFillStart, aFillMax;
                    ULONG   nKey;
                    double  fTmpVal;

                    bDoIt=FALSE;

                    if( IS_AVAILABLE( FID_FILL_SERIES, &pItem ) )
                        aFillDir = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                        aFillCmd = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
                        aFillDateCmd = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_3, &pItem ) )
                        aFillStep = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_4, &pItem ) )
                        aFillStart = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_5, &pItem ) )
                        aFillMax = ((const SfxStringItem*)pItem)->GetValue();

                    if( aFillDir.Len() )
                        switch( aFillDir.GetChar(0) )
                        {
                            case 'B': case 'b': eFillDir=FILL_TO_BOTTOM; break;
                            case 'R': case 'r': eFillDir=FILL_TO_RIGHT; break;
                            case 'T': case 't': eFillDir=FILL_TO_TOP; break;
                            case 'L': case 'l': eFillDir=FILL_TO_LEFT; break;
                        }

                    if( aFillCmd.Len() )
                        switch( aFillCmd.GetChar(0) )
                        {
                            case 'S': case 's': eFillCmd=FILL_SIMPLE; break;
                            case 'L': case 'l': eFillCmd=FILL_LINEAR; break;
                            case 'G': case 'g': eFillCmd=FILL_GROWTH; break;
                            case 'D': case 'd': eFillCmd=FILL_DATE; break;
                            case 'A': case 'a': eFillCmd=FILL_AUTO; break;
                        }

                    if( aFillDateCmd.Len() )
                        switch( aFillDateCmd.GetChar(0) )
                        {
                            case 'D': case 'd': eFillDateCmd=FILL_DAY; break;
                            case 'W': case 'w': eFillDateCmd=FILL_WEEKDAY; break;
                            case 'M': case 'm': eFillDateCmd=FILL_MONTH; break;
                            case 'Y': case 'y': eFillDateCmd=FILL_YEAR; break;
                        }

                    nKey = 0;
                    if( pFormatter->IsNumberFormat( aFillStart, nKey, fTmpVal ))
                        fStartVal = fTmpVal;

                    nKey = 0;
                    if( pFormatter->IsNumberFormat( aFillStep, nKey, fTmpVal ))
                        fIncVal = fTmpVal;

                    nKey = 0;
                    if( pFormatter->IsNumberFormat( aFillMax, nKey, fTmpVal ))
                        fMaxVal = fTmpVal;

                    bDoIt   = TRUE;

                }
                else // (pReqArgs == NULL) => Dialog hochziehen
                {
                    //
                    ULONG nPrivFormat;
                    CellType eCellType;
                    pDoc->GetNumberFormat( nStartCol, nStartRow, nStartTab, nPrivFormat );
                    pDoc->GetCellType( nStartCol, nStartRow, nStartTab,eCellType );
                    const SvNumberformat* pPrivEntry = pFormatter->GetEntry( nPrivFormat );
                    if (!pPrivEntry)
                    {
                        DBG_ERROR("Zahlformat nicht gefunden !!!");
                    }
                    else
                    {
                        short nPrivType = pPrivEntry->GetType();
                        if ( ( nPrivType & NUMBERFORMAT_DATE)>0)
                        {
                           eFillCmd=FILL_DATE;
                        }
                        else if(eCellType==CELLTYPE_STRING)
                        {
                           eFillCmd=FILL_AUTO;
                        }
                    }

                    //
                    String aStartStr;

                    //  Startwert nur vorbelegen, wenn nur 1 Zeile oder Spalte:
                    if ( nStartCol == nEndCol || nStartRow == nEndRow )
                    {
                        double fInputEndVal;
                        String aEndStr;

                        pDoc->GetInputString( nStartCol, nStartRow, nStartTab, aStartStr);
                        pDoc->GetValue( nStartCol, nStartRow, nStartTab, fStartVal );


                        if(eFillDir==FILL_TO_BOTTOM && nStartRow < nEndRow )
                        {
                            pDoc->GetInputString( nStartCol, nStartRow+1, nStartTab, aEndStr);
                            if(aEndStr.Len()>0)
                            {
                                pDoc->GetValue( nStartCol, nStartRow+1, nStartTab, fInputEndVal);
                                fIncVal=fInputEndVal-fStartVal;
                            }
                        }
                        else
                        {
                            if(nStartCol < nEndCol)
                            {
                                pDoc->GetInputString( nStartCol+1, nStartRow, nStartTab, aEndStr);
                                if(aEndStr.Len()>0)
                                {
                                    pDoc->GetValue( nStartCol+1, nStartRow, nStartTab, fInputEndVal);
                                    fIncVal=fInputEndVal-fStartVal;
                                }
                            }
                        }
                        if(eFillCmd==FILL_DATE)
                        {
                            Date aNullDate = *pDoc->GetFormatTable()->GetNullDate();
                            Date aStartDate = aNullDate;
                            aStartDate+= (long)fStartVal;
                            Date aEndDate = aNullDate;
                            aEndDate+= (long)fInputEndVal;
                            double fTempDate=0;

                            if(aStartDate.GetYear()!=aEndDate.GetYear())
                            {
                                eFillDateCmd = FILL_YEAR;
                                fTempDate=aEndDate.GetYear()-aStartDate.GetYear();
                            }
                            if(aStartDate.GetMonth()!=aEndDate.GetMonth())
                            {
                                eFillDateCmd = FILL_MONTH;
                                fTempDate=fTempDate*12+aEndDate.GetMonth()-aStartDate.GetMonth();
                            }
                            if(aStartDate.GetDay()==aEndDate.GetDay())
                            {
                                fIncVal=fTempDate;
                            }
                        }
                    }
                    ScFillSeriesDlg* pDlg = new ScFillSeriesDlg(
                                    pTabViewShell->GetDialogParent(), *pDoc,
                                eFillDir, eFillCmd, eFillDateCmd,
                                aStartStr, fIncVal, fMaxVal,
                                nPossDir);

                    if ( nStartCol != nEndCol && nStartRow != nEndRow )
                    {
                        pDlg->SetEdStartValEnabled(FALSE);
                    }

                    if ( pDlg->Execute() == RET_OK )
                    {
                        eFillDir        = pDlg->GetFillDir();
                        eFillCmd        = pDlg->GetFillCmd();
                        eFillDateCmd    = pDlg->GetFillDateCmd();

                        if(eFillCmd==FILL_AUTO)
                        {
                            String aStr=pDlg->GetStartStr();
                            if(aStr.Len()>0)
                                pTabViewShell->EnterData( nStartCol, nStartRow, nStartTab, aStr );
                        }
                        fStartVal       = pDlg->GetStart();
                        fIncVal         = pDlg->GetStep();
                        fMaxVal         = pDlg->GetMax();
                        bDoIt           = TRUE;
                    }
                    delete pDlg;
                }

                if( bDoIt )
                {
                    //nScFillModeMouseModifier = 0; // kein Ctrl/Copy
                    pTabViewShell->FillSeries( eFillDir, eFillCmd, eFillDateCmd, fStartVal, fIncVal, fMaxVal );

                    if( ! rReq.IsAPI() )
                    {
                        String  aPara;
                        Color*  pColor=0;

                        switch( eFillDir )
                        {
                            case FILL_TO_BOTTOM:    aPara = 'B'; break;
                            case FILL_TO_RIGHT:     aPara = 'R'; break;
                            case FILL_TO_TOP:       aPara = 'T'; break;
                            case FILL_TO_LEFT:      aPara = 'L'; break;
                            default: aPara.Erase(); break;
                        }
                        rReq.AppendItem( SfxStringItem( FID_FILL_SERIES, aPara ) );

                        switch( eFillCmd )
                        {
                            case FILL_SIMPLE:       aPara = 'S'; break;
                            case FILL_LINEAR:       aPara = 'L'; break;
                            case FILL_GROWTH:       aPara = 'G'; break;
                            case FILL_DATE:         aPara = 'D'; break;
                            case FILL_AUTO:         aPara = 'A'; break;
                            default: aPara.Erase(); break;
                        }
                        rReq.AppendItem( SfxStringItem( FN_PARAM_1, aPara ) );

                        switch( eFillDateCmd )
                        {
                            case FILL_DAY:          aPara = 'D'; break;
                            case FILL_WEEKDAY:      aPara = 'W'; break;
                            case FILL_MONTH:        aPara = 'M'; break;
                            case FILL_YEAR:         aPara = 'Y'; break;
                            default: aPara.Erase(); break;
                        }
                        rReq.AppendItem( SfxStringItem( FN_PARAM_2, aPara ) );

                        ULONG nFormatKey = pFormatter->GetStandardFormat(NUMBERFORMAT_NUMBER,
                                    ScGlobal::eLnge );

                        pFormatter->GetOutputString( fIncVal, nFormatKey, aPara, &pColor );
                        rReq.AppendItem( SfxStringItem( FN_PARAM_3, aPara ) );

                        pFormatter->GetOutputString( fStartVal, nFormatKey, aPara, &pColor );
                        rReq.AppendItem( SfxStringItem( FN_PARAM_4, aPara ) );

                        pFormatter->GetOutputString( fMaxVal, nFormatKey, aPara, &pColor );
                        rReq.AppendItem( SfxStringItem( FN_PARAM_5, aPara ) );

                        rReq.Done();
                    }
                }
            }
            break;

        case FID_FILL_AUTO:
            {
                USHORT nStartCol;
                USHORT nStartRow;
                USHORT nEndCol;
                USHORT nEndRow;
                USHORT nStartTab, nEndTab;

                GetViewData()->GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
                USHORT nFillCol = GetViewData()->GetRefEndX();
                USHORT nFillRow = GetViewData()->GetRefEndY();

                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;

                    if( IS_AVAILABLE( FID_FILL_AUTO, &pItem ) )
                    {
                        ScAddress aScAddress;
                        String aArg = ((const SfxStringItem*)pItem)->GetValue();

                        if( aScAddress.Parse( aArg ) & SCA_VALID )
                        {
                            nFillRow = aScAddress.Row();
                            nFillCol = aScAddress.Col();
                        }
                    }

                    GetViewData()->GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );
                }
                else    // Aufruf per Maus
                {
                    //  #55284# nicht innerhalb einer zusammengefassten Zelle

                    if ( nStartCol == nEndCol && nStartRow == nEndRow )
                    {
                        USHORT nMergeCol = nStartCol;
                        USHORT nMergeRow = nStartRow;
                        if ( GetViewData()->GetDocument()->ExtendMerge(
                                nStartCol, nStartRow, nMergeCol, nMergeRow,
                                GetViewData()->GetTabNo() ) )
                        {
                            if ( nFillCol <= nMergeCol && nFillRow == nStartRow )
                                nFillCol = nStartCol;
                            if ( nFillRow <= nMergeRow && nFillCol == nStartCol )
                                nFillRow = nStartRow;
                        }
                    }
                }

                if ( nFillCol != nEndCol || nFillRow != nEndRow )
                {
                    if ( nFillCol==nEndCol || nFillRow==nEndRow )
                    {
                        FillDir eDir;
                        USHORT nCount = 0;

                        if ( nFillCol==nEndCol )
                        {
                            if ( nFillRow > nEndRow )
                            {
                                eDir = FILL_TO_BOTTOM;
                                nCount = nFillRow - nEndRow;
                            }
                            else if ( nFillRow < nStartRow )
                            {
                                eDir = FILL_TO_TOP;
                                nCount = nStartRow - nFillRow;
                            }
                        }
                        else
                        {
                            if ( nFillCol > nEndCol )
                            {
                                eDir = FILL_TO_RIGHT;
                                nCount = nFillCol - nEndCol;
                            }
                            else if ( nFillCol < nStartCol )
                            {
                                eDir = FILL_TO_LEFT;
                                nCount = nStartCol - nFillCol;
                            }
                        }

                        if ( nCount )
                        {
                            pTabViewShell->FillAuto( eDir, nStartCol, nStartRow, nEndCol, nEndRow, nCount );

                            if( ! rReq.IsAPI() )
                            {
                                String  aAdrStr;
                                ScAddress aAdr( nFillCol, nFillRow, 0 );
                                aAdr.Format( aAdrStr, SCR_ABS, GetViewData()->GetDocument() );

                                rReq.AppendItem( SfxStringItem( FID_FILL_AUTO, aAdrStr ) );
                                rReq.Done();
                            }
                        }

                    }
                    else
                        DBG_ERROR( "Richtung nicht eindeutig fuer AutoFill" );
                }
            }
            break;

        //
        //  Gliederung (Outlines)
        //  SID_AUTO_OUTLINE, SID_OUTLINE_DELETEALL in Execute (in docsh.idl)
        //

        case SID_OUTLINE_HIDE:
            pTabViewShell->HideMarkedOutlines();
            rReq.Done();
            break;

        case SID_OUTLINE_SHOW:
            pTabViewShell->ShowMarkedOutlines();
            rReq.Done();
            break;

        case SID_OUTLINE_MAKE:
            {
                BOOL bColumns;
                BOOL bOk = TRUE;

                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    bOk = FALSE;

                    if( IS_AVAILABLE( SID_OUTLINE_MAKE, &pItem ) )
                    {
                        String aCol = ((const SfxStringItem*)pItem)->GetValue();
                        aCol.ToUpperAscii();

                        switch( aCol.GetChar(0) )
                        {
                            case 'R': bColumns=FALSE; bOk = TRUE;break;
                            case 'C': bColumns=TRUE; bOk = TRUE;break;
                        }
                    }
                }
                else            // Dialog, wenn nicht ganze Zeilen/Spalten markiert
                {
                    if ( GetViewData()->SimpleColMarked() && !GetViewData()->SimpleRowMarked() )
                        bColumns = TRUE;
                    else if ( !GetViewData()->SimpleColMarked() && GetViewData()->SimpleRowMarked() )
                        bColumns = FALSE;
                    else
                    {
                        ScGroupDlg* pDlg = new ScGroupDlg(pTabViewShell->GetDialogParent(),
                                                            RID_SCDLG_GRP_MAKE, FALSE );
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = FALSE;
                        delete pDlg;
                    }
                }
                if (bOk)
                {
                    pTabViewShell->MakeOutline( bColumns );

                    if( ! rReq.IsAPI() )
                    {
                        String aCol = bColumns ? 'C' : 'R';
                        rReq.AppendItem( SfxStringItem( SID_OUTLINE_MAKE, aCol ) );
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_OUTLINE_REMOVE:
            {
                BOOL bColumns;
                BOOL bOk = TRUE;

                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    bOk = FALSE;

                    if( IS_AVAILABLE( SID_OUTLINE_REMOVE, &pItem ) )
                    {
                        String aCol = ((const SfxStringItem*)pItem)->GetValue();
                        aCol.ToUpperAscii();

                        switch( aCol.GetChar(0) )
                        {
                            case 'R': bColumns=FALSE; bOk = TRUE;break;
                            case 'C': bColumns=TRUE; bOk = TRUE;break;
                        }
                    }
                }
                else            // Dialog nur, wenn Aufheben fuer Zeilen und Spalten moeglich
                {
                    BOOL bColPoss, bRowPoss;
                    pTabViewShell->TestRemoveOutline( bColPoss, bRowPoss );
                    if ( bColPoss && bRowPoss )
                    {
                        ScGroupDlg* pDlg = new ScGroupDlg(  pTabViewShell->GetDialogParent(),
                                                            RID_SCDLG_GRP_KILL, TRUE );
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = FALSE;
                        delete pDlg;
                    }
                    else if ( bColPoss )
                        bColumns = TRUE;
                    else if ( bRowPoss )
                        bColumns = FALSE;
                    else
                        bOk = FALSE;
                }
                if (bOk)
                {
                    pTabViewShell->RemoveOutline( bColumns );

                    if( ! rReq.IsAPI() )
                    {
                        String aCol = bColumns ? 'C' : 'R';
                        rReq.AppendItem( SfxStringItem( SID_OUTLINE_REMOVE, aCol ) );
                        rReq.Done();
                    }
                }
            }
            break;

        //
        //  Clipboard
        //

        case SID_COPY:              // fuer Grafiken in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CopyToClip();
                rReq.Done();
            }
            break;

        case SID_CUT:               // fuer Grafiken in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CutToClip();
                rReq.Done();
            }
            break;

        case SID_PASTE:
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->PasteFromSystem();
                rReq.Done();
            }
            pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
            break;

        case FID_INS_CELL_CONTENTS:
            {
                USHORT nFlags = IDF_NONE;
                USHORT nFunction = PASTE_NOFUNC;
                BOOL bSkipEmpty = FALSE;
                BOOL bTranspose = FALSE;
                BOOL bAsLink    = FALSE;
                InsCellCmd eMoveMode = INS_NONE;

                ScDocument* pDoc = GetViewData()->GetDocument();
                BOOL bOtherDoc = !pDoc->IsClipboardSource();

                if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    String  aFlags = 'A';

                    if( IS_AVAILABLE( FID_INS_CELL_CONTENTS, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();

                    aFlags.ToUpperAscii();
                    BOOL    bCont = TRUE;

                    for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                    {
                        switch( aFlags.GetChar(i) )
                        {
                            case 'A': // Alle
                            nFlags |= IDF_ALL;
                            bCont = FALSE; // nicht mehr weitermachen!
                            break;
                            case 'S': nFlags |= IDF_STRING; break;
                            case 'V': nFlags |= IDF_VALUE; break;
                            case 'D': nFlags |= IDF_DATETIME; break;
                            case 'F': nFlags |= IDF_FORMULA; break;
                            case 'N': nFlags |= IDF_NOTE; break;
                            case 'T': nFlags |= IDF_ATTRIB; break;
                        }
                    }
                }
                else
                {
                    if (pTabViewShell->SelectionEditable())
                    {
                        ScInsertContentsDlg* pDlg = new ScInsertContentsDlg(    pTabViewShell->GetDialogParent() );
                        pDlg->SetOtherDoc( bOtherDoc );
                        // #53661# bei ChangeTrack MoveMode disablen
                        pDlg->SetChangeTrack( pDoc->GetChangeTrack() != NULL );
                        // #72930# cut/move references may disable shift
                        // directions if source and destination ranges intersect
                        if ( !bOtherDoc )
                        {
                            ScDocument* pClipDoc = ScGlobal::GetClipDoc();
                            if ( pClipDoc->IsCutMode() )
                            {
                                ScViewData* pViewData = GetViewData();
                                if ( pViewData->GetMarkData().GetTableSelect(
                                        pViewData->GetTabNo() ) )
                                {
                                    USHORT nPosX = pViewData->GetCurX();
                                    USHORT nPosY = pViewData->GetCurY();
                                    USHORT nClipStartX, nClipStartY, nClipSizeX, nClipSizeY;
                                    pClipDoc->GetClipStart( nClipStartX, nClipStartY );
                                    pClipDoc->GetClipArea( nClipSizeX, nClipSizeY );
                                    int nDisableShift = 0;
                                    if ( nClipStartX <= nPosX + nClipSizeX &&
                                            nPosX <= nClipStartX + nClipSizeX )
                                        nDisableShift |= SC_CELL_SHIFT_DISABLE_DOWN;
                                    if ( nClipStartY <= nPosY + nClipSizeY &&
                                            nPosY <= nClipStartY + nClipSizeY )
                                        nDisableShift |= SC_CELL_SHIFT_DISABLE_RIGHT;
                                    if ( nDisableShift )
                                        pDlg->SetCellShiftDisabled( nDisableShift );
                                }
                            }
                        }
                        if (pDlg->Execute() == RET_OK)
                        {
                            nFlags     = pDlg->GetInsContentsCmdBits();
                            nFunction  = pDlg->GetFormulaCmdBits();
                            bSkipEmpty = pDlg->IsSkipEmptyCells();
                            bTranspose = pDlg->IsTranspose();
                            bAsLink    = pDlg->IsLink();
                            eMoveMode  = pDlg->GetMoveMode();
                        }
                        delete pDlg;
                    }
                    else
                        pTabViewShell->ErrorMessage(STR_PROTECTIONERR);
                }

                if( nFlags != IDF_NONE )
                {
                    {
                        WaitObject aWait( GetViewData()->GetDialogParent() );
                        if ( bAsLink && bOtherDoc )
                            pTabViewShell->PasteFromSystem(SOT_FORMATSTR_ID_LINK);  // DDE einfuegen
                        else
                            pTabViewShell->PasteFromClip( nFlags, NULL,
                                nFunction, bSkipEmpty, bTranspose, bAsLink,
                                eMoveMode );
                    }

                    if( ! rReq.IsAPI() )
                    {
                        String  aFlags;

                        if( nFlags == IDF_ALL )
                        {
                            aFlags += 'A';
                        }
                        else
                        {
                            if( nFlags & IDF_STRING ) aFlags += 'S';
                            if( nFlags & IDF_VALUE ) aFlags += 'V';
                            if( nFlags & IDF_DATETIME ) aFlags += 'D';
                            if( nFlags & IDF_FORMULA ) aFlags += 'F';
                            if( nFlags & IDF_NOTE ) aFlags += 'N';
                            if( nFlags & IDF_ATTRIB ) aFlags += 'T';
                        }

                        rReq.AppendItem( SfxStringItem( FID_INS_CELL_CONTENTS, aFlags ) );
                        rReq.Done();
                    }
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromXXX ???
            break;

        case FID_PASTE_CONTENTS:
            // Unterscheidung, ob eigene oder fremde Daten,
            // dadurch FID_INS_CELL_CONTENTS ueberfluessig
            {
                //  Clipboard-ID als Parameter angegeben? Basic "PasteSpecial(Format)"
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    ULONG nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                    BOOL bRet;
                    {
                        WaitObject aWait( GetViewData()->GetDialogParent() );
                        bRet = pTabViewShell->PasteFromSystem(nFormat, TRUE);       // TRUE: keine Fehlermeldungen
                    }
                    rReq.SetReturnValue(SfxInt16Item(nSlot, bRet)); // 1 = Erfolg, 0 = Fehler
                    rReq.Done();
                }
                else if ( ScGlobal::IsClipCaptured() )  // eigene Tabellen-Daten
                {
                    rReq.SetSlot( FID_INS_CELL_CONTENTS );
                    ExecuteSlot( rReq, GetInterface() );
                    rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = Erfolg
                }
                else                                    // Zeichenobjekte oder fremde Daten
                {
                    BOOL bDraw = ScGlobal::IsClipDraw();

                    SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
                    if (pClipObj.Is())
                    {
                        SvPasteObjectDialog* pDlg = new SvPasteObjectDialog;
                        //  nOle ist special, stellvertretend fuer alle vier ?!?
                        ULONG nOle  = SOT_FORMATSTR_ID_EMBED_SOURCE;
                        ULONG nSdr  = SOT_FORMATSTR_ID_DRAWING;
                        pDlg->Insert( nSdr,                 Clipboard::GetFormatName( nSdr ) );
                        pDlg->Insert( SOT_FORMATSTR_ID_SVXB,Clipboard::GetFormatName( SOT_FORMATSTR_ID_SVXB ) );
                        pDlg->Insert( FORMAT_GDIMETAFILE,   Clipboard::GetFormatName( FORMAT_GDIMETAFILE ) );
                        pDlg->Insert( FORMAT_BITMAP,        Clipboard::GetFormatName( FORMAT_BITMAP ) );
                        pDlg->Insert( nOle,                 String('*') );
                        if (!bDraw)
                        {
                            ULONG nBiff = Exchange::RegisterFormatName(
                                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff5")));
                            pDlg->Insert( SOT_FORMATSTR_ID_LINK,ScResId( SCSTR_CLIP_DDE ) );
                            pDlg->Insert( FORMAT_STRING,        ScResId( SCSTR_CLIP_STRING ) );
                            pDlg->Insert( SOT_FORMATSTR_ID_DIF, ScResId( SCSTR_CLIP_DIF ) );
                            pDlg->Insert( FORMAT_RTF,           ScResId( SCSTR_CLIP_RTF ) );
                            pDlg->Insert( SOT_FORMATSTR_ID_HTML,Clipboard::GetFormatName( SOT_FORMATSTR_ID_HTML ) );
                            pDlg->Insert( SOT_FORMATSTR_ID_HTML_SIMPLE,Clipboard::GetFormatName( SOT_FORMATSTR_ID_HTML_SIMPLE ) );
                            pDlg->Insert( nBiff,                Clipboard::GetFormatName( nBiff ) );
                        }

                        ULONG nFormat = pDlg->Execute(  pTabViewShell->GetDialogParent(), pClipObj );
                        if (nFormat > 0)
                        {
                            {
                                WaitObject aWait( GetViewData()->GetDialogParent() );
                                if ( bDraw && nFormat == nOle )
                                    pTabViewShell->PasteDraw();
                                else
                                    pTabViewShell->PasteFromSystem(nFormat);
                            }
                            rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = Erfolg
                            rReq.Done();
                        }
                        else
                            rReq.SetReturnValue(SfxInt16Item(nSlot, 0));    // 0 = Fehler
                        delete pDlg;
                    }
                    else
                        rReq.SetReturnValue(SfxInt16Item(nSlot, 0));        // 0 = Fehler
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
            break;

        //  Clipboard-Abfrage-Slots wie im Writer (da auch an der Selection)

        case SID_GET_CLPBRD_FORMAT_COUNT:
            rReq.SetReturnValue( SfxUInt16Item( nSlot, Clipboard::GetFormatCount() ) );
            break;

        case SID_GET_CLPBRD_FORMAT_BY_IDX:
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt16Item) )
                {
                    USHORT nCount = Clipboard::GetFormatCount();
                    USHORT nPos = ((const SfxUInt16Item*)pItem)->GetValue();    // 1-based
                    if ( nPos && nPos <= nCount )
                        rReq.SetReturnValue( SfxUInt32Item( nSlot, Clipboard::GetFormat(--nPos) ) );
                }
            }
            break;

        case SID_GET_CLPBRD_FORMAT_NAME:
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    String aName = Exchange::GetFormatName(
                                    ((const SfxUInt32Item*)pItem)->GetValue() );
                    rReq.SetReturnValue( SfxStringItem( nSlot, aName ) );
                }
            }
            break;

        //
        //  sonstiges
        //

        case FID_INS_ROWBRK:
            pTabViewShell->InsertPageBreak( FALSE );
            rReq.Done();
            break;

        case FID_INS_COLBRK:
            pTabViewShell->InsertPageBreak( TRUE );
            rReq.Done();
            break;

        case FID_DEL_ROWBRK:
            pTabViewShell->DeletePageBreak( FALSE );
            rReq.Done();
            break;

        case FID_DEL_COLBRK:
            pTabViewShell->DeletePageBreak( TRUE );
            rReq.Done();
            break;

        case SID_DETECTIVE_ADD_PRED:
            pTabViewShell->DetectiveAddPred();
            rReq.Done();
            break;

        case SID_DETECTIVE_DEL_PRED:
            pTabViewShell->DetectiveDelPred();
            rReq.Done();
            break;

        case SID_DETECTIVE_ADD_SUCC:
            pTabViewShell->DetectiveAddSucc();
            rReq.Done();
            break;

        case SID_DETECTIVE_DEL_SUCC:
            pTabViewShell->DetectiveDelSucc();
            rReq.Done();
            break;

        case SID_DETECTIVE_ADD_ERR:
            pTabViewShell->DetectiveAddError();
            rReq.Done();
            break;

        case SID_DETECTIVE_INVALID:
            pTabViewShell->DetectiveMarkInvalid();
            rReq.Done();
            break;

        case SID_DETECTIVE_REFRESH:
            pTabViewShell->DetectiveRefresh();
            rReq.Done();
            break;

        case SID_SPELLING:
            pTabViewShell->DoSpellingChecker();
            break;

        case SID_THESAURUS:
            pTabViewShell->DoThesaurus();
            break;

        case SID_TOGGLE_REL:
            {
                BOOL bOk = FALSE;
                USHORT nCol = GetViewData()->GetCurX();
                USHORT nRow = GetViewData()->GetCurY();
                USHORT nTab = GetViewData()->GetTabNo();
                ScDocument* pDoc = GetViewData()->GetDocument();
                CellType eType;
                pDoc->GetCellType( nCol, nRow, nTab, eType );
                if (eType == CELLTYPE_FORMULA)
                {
                    String aOld;
                    pDoc->GetFormula( nCol, nRow, nTab, aOld );
                    xub_StrLen nLen = aOld.Len();
                    ScRefFinder aFinder( aOld, pDoc );
                    aFinder.ToggleRel( 0, nLen );
                    if (aFinder.GetFound())
                    {
                        String aNew = aFinder.GetText();
                        pTabViewShell->EnterData( nCol, nRow, nTab, aNew );
                        pTabViewShell->UpdateInputHandler();
                        bOk = TRUE;
                    }
                }
                if (!bOk)
                    pTabViewShell->ErrorMessage(STR_ERR_NOREF);
            }
            break;

        case SID_DEC_INDENT:
            pTabViewShell->ChangeIndent( FALSE );
            break;
        case SID_INC_INDENT:
            pTabViewShell->ChangeIndent( TRUE );
            break;

        case FID_USE_NAME:
            {
                USHORT nFlags = pTabViewShell->GetCreateNameFlags();

                ScNameCreateDlg* pDlg = new ScNameCreateDlg(    pTabViewShell->GetDialogParent(), nFlags );

                if( pDlg->Execute() )
                {
                    nFlags = pDlg->GetFlags();
                    pTabViewShell->CreateNames(nFlags);
                    rReq.Done();
                }
                delete pDlg;
            }
            break;

        case SID_CONSOLIDATE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET ==
                        pReqArgs->GetItemState( SCITEM_CONSOLIDATEDATA, TRUE, &pItem ) )
                {
                    const ScConsolidateParam& rParam =
                            ((const ScConsolidateItem*)pItem)->GetData();

                    pTabViewShell->Consolidate( rParam );
                    GetViewData()->GetDocument()->SetConsolidateDlgData( &rParam );

                    rReq.Done();
                }
                else if (rReq.IsAPI())
                    SbxBase::SetError(SbxERR_BAD_PARAMETER);
            }
            break;

        case SID_INS_FUNCTION:
            {
                const SfxBoolItem* pOkItem = (const SfxBoolItem*)&pReqArgs->Get( SID_DLG_RETOK );

//              pScMod->SetFunctionDlg( NULL );

                if ( pOkItem->GetValue() )      // OK
                {
                    String               aFormula;
                    const SfxStringItem* pSItem      = (const SfxStringItem*)&pReqArgs->Get( SCITEM_STRING );
                    const SfxBoolItem*   pMatrixItem = (const SfxBoolItem*)  &pReqArgs->Get( SID_DLG_MATRIX );

                    aFormula += pSItem->GetValue();
                    pScMod->ActivateInputWindow( &aFormula, pMatrixItem->GetValue() );
                }
                else                            // CANCEL
                {
                    pScMod->ActivateInputWindow( NULL );
                }
                rReq.Done();
            }
            break;

        case FID_DEFINE_NAME:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                String  aName, aSymbol, aAttrib;

                if( IS_AVAILABLE( FID_DEFINE_NAME, &pItem ) )
                    aName = ((const SfxStringItem*)pItem)->GetValue();

                if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                    aSymbol = ((const SfxStringItem*)pItem)->GetValue();

                if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
                    aAttrib = ((const SfxStringItem*)pItem)->GetValue();

                if ( aName.Len() && aSymbol.Len() )
                {
                    if (pTabViewShell->InsertName( aName, aSymbol, aAttrib ))
                        rReq.Done();
                    else
                        SbxBase::SetError( SbxERR_BAD_PARAMETER );  // Basic-Fehler
                }
            }
            else
            {
                USHORT          nId  = ScNameDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
            }
            break;

        case SID_DEFINE_COLROWNAMERANGES:
            {

                USHORT          nId  = ScColRowNameRangesDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );

            }
            break;

        case SID_UPDATECHART:
            {
                BOOL bAll = FALSE;

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;

                    if( IS_AVAILABLE( SID_UPDATECHART, &pItem ) )
                        bAll = ((const SfxBoolItem*)pItem)->GetValue();
                }

                pTabViewShell->UpdateCharts( bAll );

                if( ! rReq.IsAPI() )
                {
                    rReq.AppendItem( SfxBoolItem( SID_UPDATECHART, bAll ) );
                    rReq.Done();
                }
            }
            break;


        case SID_TABOP:
            {
                const SfxItemSet*  pOutSet = rReq.GetArgs();
                const ScTabOpItem& rItem =
                        (const ScTabOpItem&)
                            pOutSet->Get( SID_TABOP );

                pTabViewShell->TabOp( rItem.GetData() );

                rReq.Done( *pOutSet );
            }
            break;

        case SID_SOLVE:
            {
                const SfxItemSet*  pOutSet = rReq.GetArgs();
                const ScSolveItem& rItem =
                        (const ScSolveItem&)
                            pOutSet->Get( SCITEM_SOLVEDATA );

                pTabViewShell->Solve( rItem.GetData() );

                rReq.Done( *pOutSet );
            }
            break;

        case FID_INSERT_NAME:
            {
                ScDocument*     pDoc = GetViewData()->GetDocument();
                ScNamePasteDlg* pDlg = new ScNamePasteDlg(  pTabViewShell->GetDialogParent(), pDoc->GetRangeName() );

                switch( pDlg->Execute() )
                {
                    case BTN_PASTE_LIST:
                        pTabViewShell->InsertNameList();
                        break;
                    case BTN_PASTE_NAME:
                        {
                            ScInputHandler* pHdl = pScMod->GetInputHdl( pTabViewShell );
                            if (pHdl)
                            {
                                //  das "=" per Key-Event, schaltet in den Eingabe-Modus
                                pScMod->InputKeyEvent( KeyEvent('=',KeyCode()) );

                                String aName = pDlg->GetSelectedName();
                                pHdl->InsertFunction( aName, FALSE );       // ohne "()"
                            }
                        }
                        break;
                }
                delete pDlg;
            }
            break;

        case SID_RANGE_NOTETEXT:
            if (pReqArgs)
            {
                SvtUserOptions aUserOpt;

                String aNoteStr = ((const SfxStringItem&)pReqArgs->
                                    Get( SID_RANGE_NOTETEXT )).GetValue();
                String aDateStr = ScGlobal::pScInternational->GetDate( Date() );
                String aAuthorStr = aUserOpt.GetID();
                ScPostIt aNote( aNoteStr, aDateStr, aAuthorStr );

                USHORT nCol, nRow, nTab;

                //  #43343# immer Cursorposition
                nCol = GetViewData()->GetCurX();
                nRow = GetViewData()->GetCurY();
                nTab = GetViewData()->GetTabNo();

                pTabViewShell->SetNote( nCol, nRow, nTab, aNote );
                rReq.Done();
            }
            break;

        case SID_INSERT_POSTIT:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                USHORT      nCol = GetViewData()->GetCurX();
                USHORT      nRow = GetViewData()->GetCurY();
                USHORT      nTab = GetViewData()->GetTabNo();
                ScPostIt    aNote;

                if ( pReqArgs )
                {
                    const SvxPostItAuthorItem&  rAuthorItem = (const SvxPostItAuthorItem&)pReqArgs->Get( SID_ATTR_POSTIT_AUTHOR );
                    const SvxPostItDateItem&    rDateItem   = (const SvxPostItDateItem&)  pReqArgs->Get( SID_ATTR_POSTIT_DATE );
                    const SvxPostItTextItem&    rTextItem   = (const SvxPostItTextItem&)  pReqArgs->Get( SID_ATTR_POSTIT_TEXT );
                    aNote.SetText( rTextItem.GetValue() );
                    aNote.SetDate( rDateItem.GetValue() );
                    aNote.SetAuthor( rAuthorItem.GetValue() );

                    pTabViewShell->SetNote( nCol, nRow, nTab, aNote );
                    rReq.Done();
                }
                else
                {
                    pTabViewShell->EditNote();                  // Zeichenobjekt zum Editieren
                }
            }
            break;

        case FID_NOTE_VISIBLE:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                USHORT      nCol = GetViewData()->GetCurX();
                USHORT      nRow = GetViewData()->GetCurY();
                USHORT      nTab = GetViewData()->GetTabNo();
                ScPostIt    aNote;

                if ( pDoc->GetNote( nCol, nRow, nTab, aNote ) )
                {
                    BOOL bShow;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs && pReqArgs->GetItemState(
                            FID_NOTE_VISIBLE, TRUE, &pItem ) == SFX_ITEM_SET )
                        bShow = ((const SfxBoolItem*) pItem)->GetValue();
                    else
                        bShow = !pDoc->HasNoteObject( nCol, nRow, nTab );

                    if ( bShow )
                        pTabViewShell->ShowNote();
                    else
                        pTabViewShell->HideNote();

                    if (!pReqArgs)
                        rReq.AppendItem( SfxBoolItem( FID_NOTE_VISIBLE, bShow ) );

                    rReq.Done();
                    rBindings.Invalidate( FID_NOTE_VISIBLE );
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_CHARMAP:
            if( pReqArgs != NULL )
            {
                const SvxFontItem&   rFontItem   = (const SvxFontItem&) rReq.GetArgs()->Get(SID_CHARMAP);
                const SfxStringItem& rStringItem = (const SfxStringItem&) rReq.GetArgs()->Get(SID_ATTR_SECIALCHAR);
                Font    aFont;

                aFont.SetName( rFontItem.GetFamilyName() );
                aFont.SetStyleName( rFontItem.GetStyleName() );
                aFont.SetFamily( rFontItem.GetFamily() );
                aFont.SetPitch( rFontItem.GetPitch() );
                aFont.SetCharSet( rFontItem.GetCharSet() );

                pTabViewShell->InsertSpecialChar( rStringItem.GetValue(), aFont );
                if( ! rReq.IsAPI() )
                    rReq.Done();
            }
            else
            {
                SvxCharacterMap* pDlg = new SvxCharacterMap(    pTabViewShell->GetDialogParent(), FALSE );
                Font             aCurFont;

                pTabViewShell->GetSelectionPattern()->GetFont( aCurFont );
                pDlg->SetCharFont( aCurFont );

                if ( pDlg->Execute() == RET_OK )
                {
                    Font aNewFont( pDlg->GetCharFont() );

                    pTabViewShell->InsertSpecialChar( pDlg->GetCharacters(), aNewFont );

                    SfxStringItem aStringItem( SID_ATTR_SECIALCHAR,
                                               pDlg->GetCharacters() );
                    SvxFontItem   aFontItem( aNewFont.GetFamily(),
                                             aNewFont.GetName(),
                                             aNewFont.GetStyleName(),
                                             aNewFont.GetPitch(),
                                             aNewFont.GetCharSet(),
                                             SID_CHARMAP );

                    rReq.AppendItem( aFontItem );
                    rReq.AppendItem( aStringItem );
                    rReq.Done();
                }
                delete pDlg;
            }
            break;

        case SID_SELECT_SCENARIO:
            {
                // Testing

                if ( pReqArgs )
                {
                    const SfxStringItem* pItem =
                        (const SfxStringItem*)&pReqArgs->Get( SID_SELECT_SCENARIO );

                    if( pItem )
                    {
                        pTabViewShell->UseScenario( pItem->GetValue() );
                        //! wofuer soll der Return-Wert gut sein?!?!
                        rReq.SetReturnValue( SfxStringItem( SID_SELECT_SCENARIO, pItem->GetValue() ) );
                        rReq.Done();
                    }
                    else
                        DBG_ERROR("NULL");
                }
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if( IS_AVAILABLE( SID_HYPERLINK_SETLINK, &pItem ) )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName   = pHyper->GetName();
                    const String& rURL    = pHyper->GetURL();
                    const String& rTarget = pHyper->GetTargetFrame();
                    USHORT nType = (USHORT) pHyper->GetInsertMode();

                    pTabViewShell->InsertURL( rName, rURL, rTarget, nType );
                }
            }
            break;

        case FID_CONDITIONAL_FORMAT:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if( IS_AVAILABLE( FID_CONDITIONAL_FORMAT, &pItem ) )
                {
                    // Wenn RefInput auf andere Tabelle als Datentabelle umgeschaltet
                    // hat wieder zurueckschalten:
                    if ( GetViewData()->GetTabNo() != GetViewData()->GetRefTabNo() )
                    {
                        pTabViewShell->SetTabNo( GetViewData()->GetRefTabNo() );
                        pTabViewShell->PaintExtras();
                    }

                    const ScCondFrmtItem* pCndFmtItem = (const ScCondFrmtItem*) pItem;
                    pTabViewShell->SetConditionalFormat( pCndFmtItem->GetData() );
                    rReq.Done();
                }
            }
            break;

        //
        //
        //

        default:
            DBG_ERROR("falscher Slot bei ExecuteEdit");
            break;
    }
}





