/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>

#define _ZFORLIST_DECLARE_TABLE
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svxdlg.hxx>
#include <sot/formats.hxx>
#include <svx/postattr.hxx>
#include <editeng/fontitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <sfx2/passwd.hxx>
#include <svx/hlnkitem.hxx>
#include <basic/sbxcore.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/localedatawrapper.hxx>

#include "cellsh.hxx"
#include "sc.hrc"
#include "document.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "reffind.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "inputhdl.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "docfunc.hxx"
#include "editable.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpgroup.hxx"      // for ScDPNumGroupInfo
#include "spellparam.hxx"
#include "postit.hxx"
#include "clipparam.hxx"
#include "pivot.hxx"
#include "dpobject.hxx"
#include "dpsdbtab.hxx"     // ScImportSourceDesc
#include "dpshttab.hxx"     // ScSheetSourceDesc

#include "globstr.hrc"
#include "scui_def.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "scabstdlg.hxx"
#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/bootstrap.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

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
            case SID_SPELL_DIALOG:
            case SID_HANGUL_HANJA_CONVERSION:

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

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScInsertCellDlg* pDlg = pFact->CreateScInsertCellDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_INSCELL, bTheFlag);
                        DBG_ASSERT(pDlg, "Dialog create fail!");
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
                            default:
                            {
                                // added to avoid warnings
                            }
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
                        ScRange aRange;
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        BOOL bTheFlag=GetViewData()->IsMultiMarked() ||
                            (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE_FILTERED) ||
                            (pDoc->GetChangeTrack() != NULL);

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScDeleteCellDlg* pDlg = pFact->CreateScDeleteCellDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_DELCELL, bTheFlag );
                        DBG_ASSERT(pDlg, "Dialog create fail!");

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
                            default:
                            {
                                // added to avoid warnings
                            }
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
                    ScEditableTester aTester( pTabViewShell );
                    if (aTester.IsEditable())
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScDeleteContentsDlg* pDlg = pFact->CreateScDeleteContentsDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_DELCONT );
                        DBG_ASSERT(pDlg, "Dialog create fail!");
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        SCTAB nTab = GetViewData()->GetTabNo();
                        if ( pDoc->IsTabProtected(nTab) )
                            pDlg->DisableObjects();
                        if (pDlg->Execute() == RET_OK)
                        {
                            nFlags = pDlg->GetDelContentsCmdBits();
                        }
                        delete pDlg;
                    }
                    else
                        pTabViewShell->ErrorMessage(aTester.GetMessageId());
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
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                    AbstractScInsertContentsDlg* pDlg = pFact->CreateScInsertContentsDlg( pTabViewShell->GetDialogParent(),
                                                                                            RID_SCDLG_INSCONT, 0, /* nCheckDefaults */
                                                                                            &ScGlobal::GetRscString(STR_FILL_TAB));
                    DBG_ASSERT(pDlg, "Dialog create fail!");
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
                SCCOL nStartCol;
                SCROW nStartRow;
                SCTAB nStartTab;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nEndTab;
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
                    sal_uInt32 nKey;
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
                    sal_uInt32 nPrivFormat;
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
                        double fInputEndVal = 0.0;
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
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                    AbstractScFillSeriesDlg* pDlg = pFact->CreateScFillSeriesDlg( pTabViewShell->GetDialogParent(),
                                                            *pDoc,
                                                            eFillDir, eFillCmd, eFillDateCmd,
                                                            aStartStr, fIncVal, fMaxVal,
                                                            nPossDir,
                                                            RID_SCDLG_FILLSERIES);
                    DBG_ASSERT(pDlg, "Dialog create fail!");

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
                SCCOL nStartCol;
                SCROW nStartRow;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nStartTab, nEndTab;

                GetViewData()->GetFillData( nStartCol, nStartRow, nEndCol, nEndRow );
                SCCOL nFillCol = GetViewData()->GetRefEndX();
                SCROW nFillRow = GetViewData()->GetRefEndY();
                ScDocument* pDoc = GetViewData()->GetDocument();

                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;

                    if( IS_AVAILABLE( FID_FILL_AUTO, &pItem ) )
                    {
                        ScAddress aScAddress;
                        String aArg = ((const SfxStringItem*)pItem)->GetValue();

                        if( aScAddress.Parse( aArg, pDoc, pDoc->GetAddressConvention() ) & SCA_VALID )
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
                        SCCOL nMergeCol = nStartCol;
                        SCROW nMergeRow = nStartRow;
                        if ( GetViewData()->GetDocument()->ExtendMerge(
                                nStartCol, nStartRow, nMergeCol, nMergeRow,
                                GetViewData()->GetTabNo() ) )
                        {
                            if ( nFillCol >= nStartCol && nFillCol <= nMergeCol && nFillRow == nStartRow )
                                nFillCol = nStartCol;
                            if ( nFillRow >= nStartRow && nFillRow <= nMergeRow && nFillCol == nStartCol )
                                nFillRow = nStartRow;
                        }
                    }
                }

                if ( nFillCol != nEndCol || nFillRow != nEndRow )
                {
                    if ( nFillCol==nEndCol || nFillRow==nEndRow )
                    {
                        FillDir eDir = FILL_TO_BOTTOM;
                        SCCOLROW nCount = 0;

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

                        if ( nCount != 0)
                        {
                            pTabViewShell->FillAuto( eDir, nStartCol, nStartRow, nEndCol, nEndRow, nCount );

                            if( ! rReq.IsAPI() )
                            {
                                String  aAdrStr;
                                ScAddress aAdr( nFillCol, nFillRow, 0 );
                                aAdr.Format( aAdrStr, SCR_ABS, pDoc, pDoc->GetAddressConvention() );

                                rReq.AppendItem( SfxStringItem( FID_FILL_AUTO, aAdrStr ) );
                                rReq.Done();
                            }
                        }

                    }
                    else
                    {
                        DBG_ERROR( "Richtung nicht eindeutig fuer AutoFill" );
                    }
                }
            }
            break;

        //
        //  Gliederung (Outlines)
        //  SID_AUTO_OUTLINE, SID_OUTLINE_DELETEALL in Execute (in docsh.idl)
        //

        case SID_OUTLINE_HIDE:
            if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                    GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                pTabViewShell->SetDataPilotDetails( FALSE );
            else
                pTabViewShell->HideMarkedOutlines();
            rReq.Done();
            break;

        case SID_OUTLINE_SHOW:
            {
                ScDPObject* pDPObj = GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                    GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                if ( pDPObj )
                {
                    Sequence<sheet::DataPilotFieldFilter> aFilters;
                    USHORT nOrientation;
                    if ( pTabViewShell->HasSelectionForDrillDown( nOrientation ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScDPShowDetailDlg* pDlg = pFact->CreateScDPShowDetailDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPSHOWDETAIL, *pDPObj, nOrientation );
                        DBG_ASSERT(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                        {
                            String aNewDimName( pDlg->GetDimensionName() );
                            pTabViewShell->SetDataPilotDetails( TRUE, &aNewDimName );
                        }
                    }
                    else if ( !pDPObj->IsServiceData() &&
                               pDPObj->GetDataFieldPositionData(
                                   ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() ),
                                   aFilters ) )
                        pTabViewShell->ShowDataPilotSourceData( *pDPObj, aFilters );
                    else
                        pTabViewShell->SetDataPilotDetails( TRUE );
                }
                else
                    pTabViewShell->ShowMarkedOutlines();
                rReq.Done();
            }
            break;

        case SID_OUTLINE_MAKE:
            {
                BOOL bColumns = FALSE;
                BOOL bOk = TRUE;

                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    ScDPNumGroupInfo aNumInfo;
                    aNumInfo.Enable    = sal_True;
                    aNumInfo.AutoStart = sal_True;
                    aNumInfo.AutoEnd   = sal_True;
                    sal_Int32 nParts = 0;
                    if ( pTabViewShell->HasSelectionForDateGroup( aNumInfo, nParts ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT( pFact, "ScAbstractFactory create fail!" );
                        Date aNullDate( *GetViewData()->GetDocument()->GetFormatTable()->GetNullDate() );
                        AbstractScDPDateGroupDlg* pDlg = pFact->CreateScDPDateGroupDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPDATEGROUP,
                            aNumInfo, nParts, aNullDate );
                        DBG_ASSERT( pDlg, "Dialog create fail!" );
                        if( pDlg->Execute() == RET_OK )
                        {
                            aNumInfo = pDlg->GetGroupInfo();
                            pTabViewShell->DateGroupDataPilot( aNumInfo, pDlg->GetDatePart() );
                        }
                    }
                    else if ( pTabViewShell->HasSelectionForNumGroup( aNumInfo ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT( pFact, "ScAbstractFactory create fail!" );
                        AbstractScDPNumGroupDlg* pDlg = pFact->CreateScDPNumGroupDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPNUMGROUP, aNumInfo );
                        DBG_ASSERT( pDlg, "Dialog create fail!" );
                        if( pDlg->Execute() == RET_OK )
                            pTabViewShell->NumGroupDataPilot( pDlg->GetGroupInfo() );
                    }
                    else
                        pTabViewShell->GroupDataPilot();

                    bOk = FALSE;
                }
                else if( pReqArgs != NULL )
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
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScGroupDlg* pDlg = pFact->CreateAbstractScGroupDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_GRP_MAKE, RID_SCDLG_GRP_MAKE,FALSE);
                        DBG_ASSERT(pDlg, "Dialog create fail!");
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
                BOOL bColumns = FALSE;
                BOOL bOk = TRUE;

                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    pTabViewShell->UngroupDataPilot();
                    bOk = FALSE;
                }
                else if( pReqArgs != NULL )
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
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                        AbstractScGroupDlg* pDlg = pFact->CreateAbstractScGroupDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_GRP_KILL, RID_SCDLG_GRP_KILL,TRUE);
                        DBG_ASSERT(pDlg, "Dialog create fail!");
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
                pTabViewShell->CopyToClip( NULL, FALSE, FALSE, TRUE );
                rReq.Done();
                GetViewData()->SetPasteMode( (ScPasteFlags) (SC_PASTE_MODE | SC_PASTE_BORDER) );
                pTabViewShell->ShowCursor();
                pTabViewShell->UpdateCopySourceOverlay();
            }
            break;

        case SID_CUT:               // fuer Grafiken in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CutToClip( NULL, TRUE );
                rReq.Done();
                GetViewData()->SetPasteMode( SC_PASTE_MODE );
                pTabViewShell->UpdateCopySourceOverlay();
            }
            break;

        case SID_PASTE:
            {
                PasteFromClipboard ( GetViewData(), pTabViewShell, true );
                rReq.Done();
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );

                ULONG nFormat = 0;
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                }

                if ( nFormat )
                {
                    Window* pWin = GetViewData()->GetActiveWin();
                    BOOL bCells = ( ScTransferObj::GetOwnClipboard( pWin ) != NULL );
                    BOOL bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );
                    BOOL bOle = ( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE );

                    if ( bCells && bOle )
                        pTabViewShell->PasteFromSystem();
                    else if ( bDraw && bOle )
                        pTabViewShell->PasteDraw();
                    else
                        pTabViewShell->PasteFromSystem(nFormat);
                }
                //?else
                //? pTabViewShell->PasteFromSystem();

                rReq.Done();
            }
            pTabViewShell->CellContentChanged();
            break;

        case FID_INS_CELL_CONTENTS:
            {
                USHORT nFlags = IDF_NONE;
                USHORT nFunction = PASTE_NOFUNC;
                BOOL bSkipEmpty = FALSE;
                BOOL bTranspose = FALSE;
                BOOL bAsLink    = FALSE;
                InsCellCmd eMoveMode = INS_NONE;

                Window* pWin = GetViewData()->GetActiveWin();
                ScDocument* pDoc = GetViewData()->GetDocument();
                BOOL bOtherDoc = !pDoc->IsClipboardSource();
                ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
                if ( pOwnClip )
                {
                    // #129384# keep a reference in case the clipboard is changed during dialog or PasteFromClip
                    uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
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

                        SFX_REQUEST_ARG( rReq, pFuncItem, SfxUInt16Item, FN_PARAM_1, sal_False );
                        SFX_REQUEST_ARG( rReq, pSkipItem, SfxBoolItem, FN_PARAM_2, sal_False );
                        SFX_REQUEST_ARG( rReq, pTransposeItem, SfxBoolItem, FN_PARAM_3, sal_False );
                        SFX_REQUEST_ARG( rReq, pLinkItem, SfxBoolItem, FN_PARAM_4, sal_False );
                        SFX_REQUEST_ARG( rReq, pMoveItem, SfxInt16Item, FN_PARAM_5, sal_False );
                        if ( pFuncItem )
                            nFunction = pFuncItem->GetValue();
                        if ( pSkipItem )
                            bSkipEmpty = pSkipItem->GetValue();
                        if ( pTransposeItem )
                            bTranspose = pTransposeItem->GetValue();
                        if ( pLinkItem )
                            bAsLink = pLinkItem->GetValue();
                        if ( pMoveItem )
                            eMoveMode = (InsCellCmd) pMoveItem->GetValue();
                    }
                    else
                    {
                        ScEditableTester aTester( pTabViewShell );
                        if (aTester.IsEditable())
                        {
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                            AbstractScInsertContentsDlg* pDlg = pFact->CreateScInsertContentsDlg( pTabViewShell->GetDialogParent(),
                                                                                                    RID_SCDLG_INSCONT);
                            DBG_ASSERT(pDlg, "Dialog create fail!");
                            pDlg->SetOtherDoc( bOtherDoc );
                            // #53661# bei ChangeTrack MoveMode disablen
                            pDlg->SetChangeTrack( pDoc->GetChangeTrack() != NULL );
                            // #72930# cut/move references may disable shift
                            // directions if source and destination ranges intersect
                            if ( !bOtherDoc )
                            {
                                if ( pOwnClip && pOwnClip->GetDocument()->IsCutMode() )
                                {
                                    ScViewData* pData = GetViewData();
                                    if ( pData->GetMarkData().GetTableSelect(
                                            pData->GetTabNo() ) )
                                    {
                                        SCCOL nPosX = pData->GetCurX();
                                        SCROW nPosY = pData->GetCurY();
                                        SCCOL nClipStartX, nClipSizeX;
                                        SCROW  nClipStartY, nClipSizeY;
                                        pOwnClip->GetDocument()->GetClipStart( nClipStartX, nClipStartY );
                                        // for CutMode, filtered rows can always be included
                                        pOwnClip->GetDocument()->GetClipArea( nClipSizeX, nClipSizeY, TRUE );
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
                            pTabViewShell->ErrorMessage(aTester.GetMessageId());
                    }

                    if( nFlags != IDF_NONE )
                    {
                        {
                            WaitObject aWait( GetViewData()->GetDialogParent() );
                            if ( bAsLink && bOtherDoc )
                                pTabViewShell->PasteFromSystem(SOT_FORMATSTR_ID_LINK);  // DDE einfuegen
                            else
                                pTabViewShell->PasteFromClip( nFlags, pOwnClip->GetDocument(),
                                    nFunction, bSkipEmpty, bTranspose, bAsLink,
                                    eMoveMode, IDF_NONE, TRUE );    // allow warning dialog
                        }

                        if( !pReqArgs )
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
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bSkipEmpty ) );
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_3, bTranspose ) );
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_4, bAsLink ) );
                            rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, nFunction ) );
                            rReq.AppendItem( SfxInt16Item( FN_PARAM_5, (sal_Int16) eMoveMode ) );
                            rReq.Done();
                        }
                    }
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromXXX ???
            break;

        case SID_PASTE_SPECIAL:
            // Unterscheidung, ob eigene oder fremde Daten,
            // dadurch FID_INS_CELL_CONTENTS ueberfluessig
            {
                Window* pWin = GetViewData()->GetActiveWin();

                //  Clipboard-ID als Parameter angegeben? Basic "PasteSpecial(Format)"
                const SfxPoolItem* pItem=NULL;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, TRUE, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    ULONG nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                    BOOL bRet=TRUE;
                    {
                        WaitObject aWait( GetViewData()->GetDialogParent() );
                        BOOL bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );
                        if ( bDraw && nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                            pTabViewShell->PasteDraw();
                        else
                            bRet = pTabViewShell->PasteFromSystem(nFormat, TRUE);       // TRUE: keine Fehlermeldungen
                    }

                    if ( bRet )
                    {
                        rReq.SetReturnValue(SfxInt16Item(nSlot, bRet)); // 1 = Erfolg, 0 = Fehler
                        rReq.Done();
                    }
                    else
                        // if format is not available -> fallback to request without parameters
                        pItem = NULL;
                }

                if ( !pItem )
                {
                    if ( ScTransferObj::GetOwnClipboard( pWin ) )  // own cell data
                    {
                        rReq.SetSlot( FID_INS_CELL_CONTENTS );
                        ExecuteSlot( rReq, GetInterface() );
                        rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = Erfolg
                    }
                    else                                    // Zeichenobjekte oder fremde Daten
                    {
                        BOOL bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );

                        SvxClipboardFmtItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                        GetPossibleClipboardFormats( aFormats );

                        USHORT nFormatCount = aFormats.Count();
                        if ( nFormatCount )
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pTabViewShell->GetDialogParent() );
                            if ( pDlg )
                            {
                            for (USHORT i=0; i<nFormatCount; i++)
                            {
                                ULONG nFormatId = aFormats.GetClipbrdFormatId( i );
                                String aName = aFormats.GetClipbrdFormatName( i );
                                // special case for paste dialog: '*' is replaced by object type
                                if ( nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE )
                                    aName.Assign((sal_Unicode)'*');
                                pDlg->Insert( nFormatId, aName );
                            }

                            TransferableDataHelper aDataHelper(
                                TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
                            ULONG nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );
                            if (nFormat > 0)
                            {
                                {
                                    WaitObject aWait( GetViewData()->GetDialogParent() );
                                    if ( bDraw && nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                                        pTabViewShell->PasteDraw();
                                    else
                                        pTabViewShell->PasteFromSystem(nFormat);
                                }
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = Erfolg
                                rReq.AppendItem( SfxUInt32Item( nSlot, nFormat ) );
                                rReq.Done();
                            }
                            else
                            {
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 0));    // 0 = Fehler
                                rReq.Ignore();
                            }

                            delete pDlg;
                            }
                        }
                        else
                            rReq.SetReturnValue(SfxInt16Item(nSlot, 0));        // 0 = Fehler
                    }
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
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

        case SID_DETECTIVE_MARK_PRED:
            pTabViewShell->DetectiveMarkPred();
            break;
        case SID_DETECTIVE_MARK_SUCC:
            pTabViewShell->DetectiveMarkSucc();
            break;
        case SID_INSERT_CURRENT_DATE:
            pTabViewShell->InsertCurrentTime(
                NUMBERFORMAT_DATE, ScGlobal::GetRscString(STR_UNDO_INSERT_CURRENT_DATE));
            break;
        case SID_INSERT_CURRENT_TIME:
            pTabViewShell->InsertCurrentTime(
                NUMBERFORMAT_TIME, ScGlobal::GetRscString(STR_UNDO_INSERT_CURRENT_TIME));
            break;

        case SID_SPELL_DIALOG:
            {
                SfxViewFrame* pViewFrame = pTabViewShell->GetViewFrame();
                if( rReq.GetArgs() )
                    pViewFrame->SetChildWindow( SID_SPELL_DIALOG,
                        static_cast< const SfxBoolItem& >( rReq.GetArgs()->
                            Get( SID_SPELL_DIALOG ) ).GetValue() );
                else
                    pViewFrame->ToggleChildWindow( SID_SPELL_DIALOG );

                pViewFrame->GetBindings().Invalidate( SID_SPELL_DIALOG );
                rReq.Ignore();
            }
            break;

        case SID_HANGUL_HANJA_CONVERSION:
            pTabViewShell->DoHangulHanjaConversion();
            break;

        case SID_CHINESE_CONVERSION:
            {
                //open ChineseTranslationDialog
                Reference< XComponentContext > xContext(
                    ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
                if(xContext.is())
                {
                    Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                    if(xMCF.is())
                    {
                        Reference< ui::dialogs::XExecutableDialog > xDialog(
                                xMCF->createInstanceWithContext(
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.ChineseTranslationDialog"))
                                    , xContext), UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(0);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow"));
                            aParam.Value <<= makeAny(xDialogParentWindow);
                            pArray[0] <<= makeAny(aParam);
                            xInit->initialize( aSeq );

                            //execute dialog
                            sal_Int16 nDialogRet = xDialog->execute();
                            if( RET_OK == nDialogRet )
                            {
                                //get some parameters from the dialog
                                sal_Bool bToSimplified = sal_True;
                                sal_Bool bUseVariants = sal_True;
                                sal_Bool bCommonTerms = sal_True;
                                Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                                if( xProp.is() )
                                {
                                    try
                                    {
                                        xProp->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsDirectionToSimplified")) ) >>= bToSimplified;
                                        xProp->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsUseCharacterVariants")) ) >>= bUseVariants;
                                        xProp->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsTranslateCommonTerms")) ) >>= bCommonTerms;
                                    }
                                    catch( Exception& )
                                    {
                                    }
                                }

                                //execute translation
                                LanguageType eSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                                LanguageType eTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                                sal_Int32 nOptions = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                                if( !bCommonTerms )
                                    nOptions |= i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                                Font aTargetFont = GetViewData()->GetActiveWin()->GetDefaultFont(
                                                    DEFAULTFONT_CJK_SPREADSHEET,
                                                    eTargetLang, DEFAULTFONT_FLAGS_ONLYONE );
                                ScConversionParam aConvParam( SC_CONVERSION_CHINESE_TRANSL,
                                    eSourceLang, eTargetLang, aTargetFont, nOptions, false );
                                pTabViewShell->DoSheetConversion( aConvParam );
                            }
                        }
                        Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
                        if( xComponent.is() )
                            xComponent->dispose();
                    }
                }
            }
            break;

        case SID_THESAURUS:
            pTabViewShell->DoThesaurus();
            break;

        case SID_TOGGLE_REL:
            pTabViewShell->DoRefConversion();
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

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                AbstractScNameCreateDlg* pDlg = pFact->CreateScNameCreateDlg(pTabViewShell->GetDialogParent(), nFlags, RID_SCDLG_NAMES_CREATE );
                DBG_ASSERT(pDlg, "Dialog create fail!");

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
                rReq.Ignore();      // only SID_ENTER_STRING is recorded
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
            if (pReqArgs)
            {
                const ScTabOpItem& rItem =
                        (const ScTabOpItem&)
                            pReqArgs->Get( SID_TABOP );

                pTabViewShell->TabOp( rItem.GetData() );

                rReq.Done( *pReqArgs );
            }
            break;

        case SID_SOLVE:
            if (pReqArgs)
            {
                const ScSolveItem& rItem =
                        (const ScSolveItem&)
                            pReqArgs->Get( SCITEM_SOLVEDATA );

                pTabViewShell->Solve( rItem.GetData() );

                rReq.Done( *pReqArgs );
            }
            break;

        case FID_INSERT_NAME:
            {
                ScDocument*     pDoc = GetViewData()->GetDocument();
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                AbstractScNamePasteDlg* pDlg = pFact->CreateScNamePasteDlg( pTabViewShell->GetDialogParent(), pDoc->GetRangeName(), RID_SCDLG_NAMES_PASTE );
                DBG_ASSERT(pDlg, "Dialog create fail!");
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
                const SfxStringItem& rTextItem = (const SfxStringItem&)pReqArgs->Get( SID_RANGE_NOTETEXT );

                //  #43343# immer Cursorposition
                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                pTabViewShell->SetNoteText( aPos, rTextItem.GetValue() );
                rReq.Done();
            }
            break;

        case SID_INSERT_POSTIT:
            if ( pReqArgs )
            {
                const SvxPostItAuthorItem&  rAuthorItem = (const SvxPostItAuthorItem&)pReqArgs->Get( SID_ATTR_POSTIT_AUTHOR );
                const SvxPostItDateItem&    rDateItem   = (const SvxPostItDateItem&)  pReqArgs->Get( SID_ATTR_POSTIT_DATE );
                const SvxPostItTextItem&    rTextItem   = (const SvxPostItTextItem&)  pReqArgs->Get( SID_ATTR_POSTIT_TEXT );

                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                pTabViewShell->ReplaceNote( aPos, rTextItem.GetValue(), &rAuthorItem.GetValue(), &rDateItem.GetValue() );
                rReq.Done();
            }
            else
            {
                pTabViewShell->EditNote();                  // Zeichenobjekt zum Editieren
            }
            break;

        case FID_NOTE_VISIBLE:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                if( ScPostIt* pNote = pDoc->GetNote( aPos ) )
                {
                    bool bShow;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs && (pReqArgs->GetItemState( FID_NOTE_VISIBLE, TRUE, &pItem ) == SFX_ITEM_SET) )
                        bShow = ((const SfxBoolItem*) pItem)->GetValue();
                    else
                        bShow = !pNote->IsCaptionShown();

                    pTabViewShell->ShowNote( bShow );

                    if (!pReqArgs)
                        rReq.AppendItem( SfxBoolItem( FID_NOTE_VISIBLE, bShow ) );

                    rReq.Done();
                    rBindings.Invalidate( FID_NOTE_VISIBLE );
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_DELETE_NOTE:
            pTabViewShell->DeleteContents( IDF_NOTE );      // delete all notes in selection
            rReq.Done();
            break;

        case SID_CHARMAP:
            if( pReqArgs != NULL )
            {
                String aChars, aFontName;
                const SfxItemSet *pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem = 0;
                if ( pArgs )
                    pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), FALSE, &pItem);
                if ( pItem )
                {
                    const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, pItem );
                    if ( pStringItem )
                        aChars = pStringItem->GetValue();
                    const SfxPoolItem* pFtItem = NULL;
                    pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), FALSE, &pFtItem);
                    const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
                    if ( pFontItem )
                        aFontName = pFontItem->GetValue();
                }

                if ( aChars.Len() )
                {
                    Font aFont;
                    pTabViewShell->GetSelectionPattern()->GetFont( aFont, SC_AUTOCOL_BLACK, NULL, NULL, NULL,
                                                                pTabViewShell->GetSelectionScriptType() );
                    if ( aFontName.Len() )
                        aFont = Font( aFontName, Size(1,1) );
                    pTabViewShell->InsertSpecialChar( aChars, aFont );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

                // font color doesn't matter here
                Font             aCurFont;
                pTabViewShell->GetSelectionPattern()->GetFont( aCurFont, SC_AUTOCOL_BLACK, NULL, NULL, NULL,
                                                                pTabViewShell->GetSelectionScriptType() );

                SfxAllItemSet aSet( GetPool() );
                aSet.Put( SfxBoolItem( FN_PARAM_1, FALSE ) );
                aSet.Put( SvxFontItem( aCurFont.GetFamily(), aCurFont.GetName(), aCurFont.GetStyleName(), aCurFont.GetPitch(), aCurFont.GetCharSet(), GetPool().GetWhich(SID_ATTR_CHAR_FONT) ) );

                SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( pTabViewShell->GetDialogParent(), aSet,
                    pTabViewShell->GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );

                if ( pDlg->Execute() == RET_OK )
                {
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pItem, SfxStringItem, SID_CHARMAP, FALSE );
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, FALSE );

                    if ( pItem && pFontItem )
                    {
                        Font aNewFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), Size(1,1) );
                        aNewFont.SetCharSet( pFontItem->GetCharSet() );
                        aNewFont.SetPitch( pFontItem->GetPitch() );
                        pTabViewShell->InsertSpecialChar( pItem->GetValue(), aNewFont );
                        rReq.AppendItem( *pFontItem );
                        rReq.AppendItem( *pItem );
                        rReq.Done();
                    }
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
                    {
                        DBG_ERROR("NULL");
                    }
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
                    rReq.Done();
                }
                else
                    rReq.Ignore();
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

        case SID_EXTERNAL_SOURCE:
            {
                String aFile;
                String aFilter;
                String aOptions;
                String aSource;
                ULONG nRefresh=0;

                SFX_REQUEST_ARG( rReq, pFile, SfxStringItem, SID_FILE_NAME, sal_False );
                SFX_REQUEST_ARG( rReq, pSource, SfxStringItem, FN_PARAM_1, sal_False );
                if ( pFile && pSource )
                {
                    aFile = pFile->GetValue();
                    aSource = pSource->GetValue();
                    SFX_REQUEST_ARG( rReq, pFilter, SfxStringItem, SID_FILTER_NAME, sal_False );
                    if ( pFilter )
                        aFilter = pFilter->GetValue();
                    SFX_REQUEST_ARG( rReq, pOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
                    if ( pOptions )
                        aOptions = pOptions->GetValue();
                    SFX_REQUEST_ARG( rReq, pRefresh, SfxUInt32Item, FN_PARAM_2, sal_False );
                    if ( pRefresh )
                        nRefresh = pRefresh->GetValue();
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                    delete pImpl->m_pLinkedDlg;
                    pImpl->m_pLinkedDlg =
                        pFact->CreateScLinkedAreaDlg( pTabViewShell->GetDialogParent(),
                                                      RID_SCDLG_LINKAREA);
                    DBG_ASSERT(pImpl->m_pLinkedDlg, "Dialog create fail!");
                    delete pImpl->m_pRequest;
                    pImpl->m_pRequest = new SfxRequest( rReq );
                    pImpl->m_pLinkedDlg->StartExecuteModal( LINK( this, ScCellShell, DialogClosed ) );
                    return;
                }

                ExecuteExternalSource( aFile, aFilter, aOptions, aSource, nRefresh, rReq );
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

void ScCellShell::ExecuteTrans( SfxRequest& rReq )
{
    sal_Int32 nType = ScViewUtil::GetTransliterationType( rReq.GetSlot() );
    if ( nType )
    {
        GetViewData()->GetView()->TransliterateText( nType );
        rReq.Done();
    }
}

void ScCellShell::ExecuteExternalSource(
    const String& _rFile, const String& _rFilter, const String& _rOptions,
    const String& _rSource, ULONG _nRefresh, SfxRequest& _rRequest )
{
    if ( _rFile.Len() && _rSource.Len() )         // filter may be empty
    {
        ScRange aLinkRange;
        BOOL bMove = FALSE;

        ScViewData* pData = GetViewData();
        ScMarkData& rMark = pData->GetMarkData();
        rMark.MarkToSimple();
        if ( rMark.IsMarked() )
        {
            rMark.GetMarkArea( aLinkRange );
            bMove = TRUE;                       // insert/delete cells to fit range
        }
        else
            aLinkRange = ScRange( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );

        ScDocFunc aFunc(*pData->GetDocShell());
        aFunc.InsertAreaLink( _rFile, _rFilter, _rOptions, _rSource,
                                aLinkRange, _nRefresh, bMove, FALSE );
        _rRequest.Done();
    }
    else
        _rRequest.Ignore();
}

void ScCellShell::ExecuteDataPilotDialog()
{
    ScModule* pScMod = SC_MOD();
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScViewData* pData = GetViewData();
    ScDocument* pDoc = pData->GetDocument();

    ::boost::scoped_ptr<ScDPObject> pNewDPObject(NULL);

    // ScPivot is no longer used...
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(
                                pData->GetCurX(), pData->GetCurY(),
                                pData->GetTabNo() );
    if ( pDPObj )   // on an existing table?
    {
        pNewDPObject.reset(new ScDPObject(*pDPObj));
    }
    else            // create new table
    {
        //  select database range or data
        pTabViewShell->GetDBData( TRUE, SC_DB_OLD );
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
            pTabViewShell->MarkDataArea( FALSE );

        //  output to cursor position for non-sheet data
        ScAddress aDestPos( pData->GetCurX(), pData->GetCurY(),
                                pData->GetTabNo() );

        //  first select type of source data

        BOOL bEnableExt = ScDPObject::HasRegisteredSources();

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

        ::boost::scoped_ptr<AbstractScDataPilotSourceTypeDlg> pTypeDlg(
            pFact->CreateScDataPilotSourceTypeDlg(
                pTabViewShell->GetDialogParent(), bEnableExt, RID_SCDLG_DAPITYPE));

        DBG_ASSERT(pTypeDlg, "Dialog create fail!");
        if ( pTypeDlg->Execute() == RET_OK )
        {
            if ( pTypeDlg->IsExternal() )
            {
                uno::Sequence<rtl::OUString> aSources = ScDPObject::GetRegisteredSources();
                ::boost::scoped_ptr<AbstractScDataPilotServiceDlg> pServDlg(
                    pFact->CreateScDataPilotServiceDlg(
                        pTabViewShell->GetDialogParent(), aSources, RID_SCDLG_DAPISERVICE));

                DBG_ASSERT(pServDlg, "Dialog create fail!");
                if ( pServDlg->Execute() == RET_OK )
                {
                    ScDPServiceDesc aServDesc(
                            pServDlg->GetServiceName(),
                            pServDlg->GetParSource(),
                            pServDlg->GetParName(),
                            pServDlg->GetParUser(),
                            pServDlg->GetParPass() );
                    pNewDPObject.reset(new ScDPObject(pDoc));
                    pNewDPObject->SetServiceData( aServDesc );
                }
            }
            else if ( pTypeDlg->IsDatabase() )
            {
                DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

                ::boost::scoped_ptr<AbstractScDataPilotDatabaseDlg> pDataDlg(
                    pFact->CreateScDataPilotDatabaseDlg(
                        pTabViewShell->GetDialogParent(), RID_SCDLG_DAPIDATA));

                DBG_ASSERT(pDataDlg, "Dialog create fail!");
                if ( pDataDlg->Execute() == RET_OK )
                {
                    ScImportSourceDesc aImpDesc(pDoc);
                    pDataDlg->GetValues( aImpDesc );
                    pNewDPObject.reset(new ScDPObject(pDoc));
                    pNewDPObject->SetImportDesc( aImpDesc );
                }
            }
            else        // selection
            {
                //! use database ranges (select before type dialog?)
                ScRange aRange;
                ScMarkType eType = GetViewData()->GetSimpleArea(aRange);
                if ( (eType & SC_MARK_SIMPLE) == SC_MARK_SIMPLE )
                {
                    // Shrink the range to the data area.
                    SCCOL nStartCol = aRange.aStart.Col(), nEndCol = aRange.aEnd.Col();
                    SCROW nStartRow = aRange.aStart.Row(), nEndRow = aRange.aEnd.Row();
                    if (pDoc->ShrinkToDataArea(aRange.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow))
                    {
                        aRange.aStart.SetCol(nStartCol);
                        aRange.aStart.SetRow(nStartRow);
                        aRange.aEnd.SetCol(nEndCol);
                        aRange.aEnd.SetRow(nEndRow);
                        rMark.SetMarkArea(aRange);
                        pTabViewShell->MarkRange(aRange);
                    }

                    BOOL bOK = TRUE;
                    if ( pDoc->HasSubTotalCells( aRange ) )
                    {
                        //  confirm selection if it contains SubTotal cells

                        QueryBox aBox( pTabViewShell->GetDialogParent(),
                                        WinBits(WB_YES_NO | WB_DEF_YES),
                                        ScGlobal::GetRscString(STR_DATAPILOT_SUBTOTAL) );
                        if (aBox.Execute() == RET_NO)
                            bOK = FALSE;
                    }
                    if (bOK)
                    {
                        ScSheetSourceDesc aShtDesc(pDoc);
                        aShtDesc.SetSourceRange(aRange);
                        pNewDPObject.reset(new ScDPObject(pDoc));
                        pNewDPObject->SetSheetDesc( aShtDesc );

                        //  output below source data
                        if ( aRange.aEnd.Row()+2 <= MAXROW - 4 )
                            aDestPos = ScAddress( aRange.aStart.Col(),
                                                    aRange.aEnd.Row()+2,
                                                    aRange.aStart.Tab() );
                    }
                }
            }
        }

        if ( pNewDPObject )
            pNewDPObject->SetOutRange( aDestPos );
    }

    pTabViewShell->SetDialogDPObject( pNewDPObject.get() );   // is copied
    if ( pNewDPObject )
    {
        //  start layout dialog

        USHORT nId  = ScPivotLayoutWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );
        pScMod->SetRefDialog( nId, pWnd ? FALSE : TRUE );
    }
}

IMPL_LINK( ScCellShell, DialogClosed, AbstractScLinkedAreaDlg*, EMPTYARG )
{
    DBG_ASSERT( pImpl->m_pLinkedDlg, "ScCellShell::DialogClosed(): invalid request" );
    DBG_ASSERT( pImpl->m_pRequest, "ScCellShell::DialogClosed(): invalid request" );
    String sFile, sFilter, sOptions, sSource;
    ULONG nRefresh = 0;

    if ( pImpl->m_pLinkedDlg->GetResult() == RET_OK )
    {
        sFile = pImpl->m_pLinkedDlg->GetURL();
        sFilter = pImpl->m_pLinkedDlg->GetFilter();
        sOptions = pImpl->m_pLinkedDlg->GetOptions();
        sSource = pImpl->m_pLinkedDlg->GetSource();
        nRefresh = pImpl->m_pLinkedDlg->GetRefresh();
        if ( sFile.Len() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILE_NAME, sFile ) );
        if ( sFilter.Len() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILTER_NAME, sFilter ) );
        if ( sOptions.Len() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, sOptions ) );
        if ( sSource.Len() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( FN_PARAM_1, sSource ) );
        if ( nRefresh )
            pImpl->m_pRequest->AppendItem( SfxUInt32Item( FN_PARAM_2, nRefresh ) );
    }

    ExecuteExternalSource( sFile, sFilter, sOptions, sSource, nRefresh, *(pImpl->m_pRequest) );
    return 0;
}

void ScCellShell::PasteFromClipboard( ScViewData* pViewData, ScTabViewShell* pTabViewShell, bool bShowDialog )
{
    Window* pWin = pViewData->GetActiveWin();
    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
    ScDocument* pThisDoc = pViewData->GetDocument();
    ScDPObject* pDPObj = pThisDoc->GetDPAtCursor( pViewData->GetCurX(),
                         pViewData->GetCurY(), pViewData->GetTabNo() );
    if ( pOwnClip && pDPObj )
    {
        // paste from Calc into DataPilot table: sort (similar to drag & drop)

        ScDocument* pClipDoc = pOwnClip->GetDocument();
        SCTAB nSourceTab = pOwnClip->GetVisibleTab();

        SCCOL nClipStartX;
        SCROW nClipStartY;
        SCCOL nClipEndX;
        SCROW nClipEndY;
        pClipDoc->GetClipStart( nClipStartX, nClipStartY );
        pClipDoc->GetClipArea( nClipEndX, nClipEndY, TRUE );
        nClipEndX = nClipEndX + nClipStartX;
        nClipEndY = nClipEndY + nClipStartY;   // GetClipArea returns the difference

        ScRange aSource( nClipStartX, nClipStartY, nSourceTab, nClipEndX, nClipEndY, nSourceTab );
        BOOL bDone = pTabViewShell->DataPilotMove( aSource, pViewData->GetCurPos() );
        if ( !bDone )
            pTabViewShell->ErrorMessage( STR_ERR_DATAPILOT_INPUT );
    }
    else
    {
        // normal paste
        WaitObject aWait( pViewData->GetDialogParent() );
        if (!pOwnClip)
            pTabViewShell->PasteFromSystem();
        else
        {
            ScDocument* pClipDoc = pOwnClip->GetDocument();
            sal_uInt16 nFlags = IDF_ALL;
            if (pClipDoc->GetClipParam().isMultiRange())
                // For multi-range paste, we paste values by default.
                nFlags &= ~IDF_FORMULA;

            pTabViewShell->PasteFromClip( nFlags, pClipDoc,
                    PASTE_NOFUNC, FALSE, FALSE, FALSE, INS_NONE, IDF_NONE,
                    bShowDialog );      // allow warning dialog
        }
    }
    pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
