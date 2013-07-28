/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>

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
#include "dpsdbtab.hxx"     // ScImportSourceDesc
#include "dpshttab.hxx"     // ScSheetSourceDesc
#include "dbdata.hxx"
#include "docsh.hxx"
#include "cliputil.hxx"
#include "markdata.hxx"
#include "docpool.hxx"
#include "condformatdlg.hxx"
#include "attrib.hxx"

#include "globstr.hrc"
#include "scui_def.hxx"
#include <svx/dialogs.hrc>
#include "scabstdlg.hxx"

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

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
    sal_uInt16              nSlot       = rReq.GetSlot();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    // finish input
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        switch ( nSlot )
        {
            case FID_DEFINE_NAME:
            case FID_ADD_NAME:
            case FID_USE_NAME:
            case FID_INSERT_NAME:
            case SID_SPELL_DIALOG:
            case SID_HANGUL_HANJA_CONVERSION:
            case SID_OPENDLG_CONDFRMT:
            case SID_OPENDLG_COLORSCALE:
            case SID_OPENDLG_DATABAR:

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
        //  insert / delete cells / rows / columns
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

                    if( pReqArgs->HasItem( FID_INS_CELL, &pItem ) )
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
                        sal_Bool bTheFlag=(pDoc->GetChangeTrack()!=NULL);

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScInsertCellDlg* pDlg = pFact->CreateScInsertCellDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_INSCELL, bTheFlag);
                        OSL_ENSURE(pDlg, "Dialog create fail!");
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

                    if( pReqArgs->HasItem( FID_DELETE_CELL, &pItem ) )
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
                        bool bTheFlag=GetViewData()->IsMultiMarked() ||
                            (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE_FILTERED) ||
                            (pDoc->GetChangeTrack() != NULL);

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScDeleteCellDlg* pDlg = pFact->CreateScDeleteCellDlg( pTabViewShell->GetDialogParent(), bTheFlag );
                        OSL_ENSURE(pDlg, "Dialog create fail!");

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
        //  delete contents from cells
        //

        case SID_DELETE_CONTENTS:
            pTabViewShell->DeleteContents( IDF_CONTENTS );
            rReq.Done();
            break;

        case SID_DELETE:
            {
                sal_uInt16  nFlags = IDF_NONE;

                if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    String  aFlags = OUString('A');

                    if( pReqArgs->HasItem( SID_DELETE, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();

                    aFlags.ToUpperAscii();
                    sal_Bool    bCont = sal_True;

                    for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                    {
                        switch( aFlags.GetChar(i) )
                        {
                            case 'A': // all
                            nFlags |= IDF_ALL;
                            bCont = false; // don't continue!
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
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScDeleteContentsDlg* pDlg = pFact->CreateScDeleteContentsDlg(pTabViewShell->GetDialogParent());
                        OSL_ENSURE(pDlg, "Dialog create fail!");
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
        //  fill...
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
                sal_uInt16 nFlags = IDF_NONE;
                sal_uInt16 nFunction = PASTE_NOFUNC;
                sal_Bool bSkipEmpty = false;
                sal_Bool bAsLink    = false;

                if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    String  aFlags = OUString('A');

                    if( pReqArgs->HasItem( FID_FILL_TAB, &pItem ) )
                        aFlags = ((const SfxStringItem*)pItem)->GetValue();

                    aFlags.ToUpperAscii();
                    sal_Bool    bCont = sal_True;

                    for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                    {
                        switch( aFlags.GetChar(i) )
                        {
                            case 'A': // all
                            nFlags |= IDF_ALL;
                            bCont = false; // don't continue!
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
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScInsertContentsDlg* pDlg = pFact->CreateScInsertContentsDlg( pTabViewShell->GetDialogParent(),
                                                                                            RID_SCDLG_INSCONT, 0, /* nCheckDefaults */
                                                                                            &ScGlobal::GetRscString(STR_FILL_TAB));
                    OSL_ENSURE(pDlg, "Dialog create fail!");
                    pDlg->SetFillMode(true);

                    if (pDlg->Execute() == RET_OK)
                    {
                        nFlags     = pDlg->GetInsContentsCmdBits();
                        nFunction  = pDlg->GetFormulaCmdBits();
                        bSkipEmpty = pDlg->IsSkipEmptyCells();
                        bAsLink    = pDlg->IsLink();
                        //  there is no MoveMode with fill tabs
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
                sal_uInt16 nPossDir = FDS_OPT_NONE;
                FillDir     eFillDir     = FILL_TO_BOTTOM;
                FillCmd     eFillCmd     = FILL_LINEAR;
                FillDateCmd eFillDateCmd = FILL_DAY;
                double fStartVal = MAXDOUBLE;
                double fIncVal   = 1;
                double fMaxVal   = MAXDOUBLE;
                sal_Bool   bDoIt     = false;

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

                    bDoIt=false;

                    if( pReqArgs->HasItem( FID_FILL_SERIES, &pItem ) )
                        aFillDir = ((const SfxStringItem*)pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                        aFillCmd = ((const SfxStringItem*)pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                        aFillDateCmd = ((const SfxStringItem*)pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_3, &pItem ) )
                        aFillStep = ((const SfxStringItem*)pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_4, &pItem ) )
                        aFillStart = ((const SfxStringItem*)pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_5, &pItem ) )
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

                    bDoIt   = sal_True;

                }
                else // (pReqArgs == NULL) => raise Dialog
                {
                    sal_uInt32 nPrivFormat;
                    CellType eCellType;
                    pDoc->GetNumberFormat( nStartCol, nStartRow, nStartTab, nPrivFormat );
                    pDoc->GetCellType( nStartCol, nStartRow, nStartTab,eCellType );
                    const SvNumberformat* pPrivEntry = pFormatter->GetEntry( nPrivFormat );
                    if (!pPrivEntry)
                    {
                        OSL_FAIL("Numberformat not found !!!");
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

                    //  suggest default Startvalue only, when just 1 row or column
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
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScFillSeriesDlg* pDlg = pFact->CreateScFillSeriesDlg( pTabViewShell->GetDialogParent(),
                                                            *pDoc,
                                                            eFillDir, eFillCmd, eFillDateCmd,
                                                            aStartStr, fIncVal, fMaxVal,
                                                            nPossDir);
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    if ( nStartCol != nEndCol && nStartRow != nEndRow )
                    {
                        pDlg->SetEdStartValEnabled(false);
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
                        bDoIt           = sal_True;
                    }
                    delete pDlg;
                }

                if( bDoIt )
                {
                    //nScFillModeMouseModifier = 0; // no Ctrl/Copy
                    pTabViewShell->FillSeries( eFillDir, eFillCmd, eFillDateCmd, fStartVal, fIncVal, fMaxVal );

                    if( ! rReq.IsAPI() )
                    {
                        OUString  aPara;
                        Color*  pColor=0;

                        switch( eFillDir )
                        {
                        case FILL_TO_BOTTOM:    aPara = "B"; break;
                        case FILL_TO_RIGHT:     aPara = "R"; break;
                        case FILL_TO_TOP:       aPara = "T"; break;
                        case FILL_TO_LEFT:      aPara = "L"; break;
                        default: break;
                        }
                        rReq.AppendItem( SfxStringItem( FID_FILL_SERIES, aPara ) );

                        switch( eFillCmd )
                        {
                        case FILL_SIMPLE:       aPara = "S"; break;
                        case FILL_LINEAR:       aPara = "L"; break;
                        case FILL_GROWTH:       aPara = "G"; break;
                        case FILL_DATE:         aPara = "D"; break;
                        case FILL_AUTO:         aPara = "A"; break;
                        default: break;
                        }
                        rReq.AppendItem( SfxStringItem( FN_PARAM_1, aPara ) );

                        switch( eFillDateCmd )
                        {
                        case FILL_DAY:          aPara = "D"; break;
                        case FILL_WEEKDAY:      aPara = "W"; break;
                        case FILL_MONTH:        aPara = "M"; break;
                        case FILL_YEAR:         aPara = "Y"; break;
                        default: break;
                        }
                        rReq.AppendItem( SfxStringItem( FN_PARAM_2, aPara ) );

                        sal_uLong nFormatKey = pFormatter->GetStandardFormat(NUMBERFORMAT_NUMBER,
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

                    if( pReqArgs->HasItem( FID_FILL_AUTO, &pItem ) )
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
                else    // call via mouse
                {
                    //  not in a merged cell

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
                        OSL_FAIL( "Direction not unique for autofill" );
                    }
                }
            }
            break;
        case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
            {
                sal_uInt16 nId  = ScRandomNumberGeneratorDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case SID_SAMPLING_DIALOG:
            {
                sal_uInt16 nId  = ScSamplingDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            {
                sal_uInt16 nId  = ScDescriptiveStatisticsDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case SID_ANALYSIS_OF_VARIANCE_DIALOG:
            {
                sal_uInt16 nId  = ScAnalysisOfVarianceDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case SID_CORRELATION_DIALOG:
            {
                sal_uInt16 nId  = ScCorrelationDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case SID_COVARIANCE_DIALOG:
            {
                sal_uInt16 nId  = ScCovarianceDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        //
        //  disposal (Outlines)
        //  SID_AUTO_OUTLINE, SID_OUTLINE_DELETEALL in Execute (in docsh.idl)
        //

        case SID_OUTLINE_HIDE:
            if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                    GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                pTabViewShell->SetDataPilotDetails( false );
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
                    sal_uInt16 nOrientation;
                    if ( pTabViewShell->HasSelectionForDrillDown( nOrientation ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScDPShowDetailDlg* pDlg = pFact->CreateScDPShowDetailDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPSHOWDETAIL, *pDPObj, nOrientation );
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                        {
                            OUString aNewDimName( pDlg->GetDimensionName() );
                            pTabViewShell->SetDataPilotDetails( true, &aNewDimName );
                        }
                    }
                    else if ( !pDPObj->IsServiceData() &&
                               pDPObj->GetDataFieldPositionData(
                                   ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() ),
                                   aFilters ) )
                        pTabViewShell->ShowDataPilotSourceData( *pDPObj, aFilters );
                    else
                        pTabViewShell->SetDataPilotDetails(true);
                }
                else
                    pTabViewShell->ShowMarkedOutlines();
                rReq.Done();
            }
            break;

        case SID_OUTLINE_MAKE:
            {
                sal_Bool bColumns = false;
                sal_Bool bOk = sal_True;

                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    ScDPNumGroupInfo aNumInfo;
                    aNumInfo.mbEnable    = true;
                    aNumInfo.mbAutoStart = true;
                    aNumInfo.mbAutoEnd   = true;
                    sal_Int32 nParts = 0;
                    if ( pTabViewShell->HasSelectionForDateGroup( aNumInfo, nParts ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE( pFact, "ScAbstractFactory create fail!" );
                        Date aNullDate( *GetViewData()->GetDocument()->GetFormatTable()->GetNullDate() );
                        AbstractScDPDateGroupDlg* pDlg = pFact->CreateScDPDateGroupDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPDATEGROUP,
                            aNumInfo, nParts, aNullDate );
                        OSL_ENSURE( pDlg, "Dialog create fail!" );
                        if( pDlg->Execute() == RET_OK )
                        {
                            aNumInfo = pDlg->GetGroupInfo();
                            pTabViewShell->DateGroupDataPilot( aNumInfo, pDlg->GetDatePart() );
                        }
                    }
                    else if ( pTabViewShell->HasSelectionForNumGroup( aNumInfo ) )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE( pFact, "ScAbstractFactory create fail!" );
                        AbstractScDPNumGroupDlg* pDlg = pFact->CreateScDPNumGroupDlg(
                            pTabViewShell->GetDialogParent(), RID_SCDLG_DPNUMGROUP, aNumInfo );
                        OSL_ENSURE( pDlg, "Dialog create fail!" );
                        if( pDlg->Execute() == RET_OK )
                            pTabViewShell->NumGroupDataPilot( pDlg->GetGroupInfo() );
                    }
                    else
                        pTabViewShell->GroupDataPilot();

                    bOk = false;
                }
                else if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    bOk = false;

                    if( pReqArgs->HasItem( SID_OUTLINE_MAKE, &pItem ) )
                    {
                        String aCol = ((const SfxStringItem*)pItem)->GetValue();
                        aCol.ToUpperAscii();

                        switch( aCol.GetChar(0) )
                        {
                            case 'R': bColumns=false; bOk = sal_True;break;
                            case 'C': bColumns=sal_True; bOk = sal_True;break;
                        }
                    }
                }
                else            // Dialog, when not whole rows/columns are marked
                {
                    if ( GetViewData()->SimpleColMarked() && !GetViewData()->SimpleRowMarked() )
                        bColumns = sal_True;
                    else if ( !GetViewData()->SimpleColMarked() && GetViewData()->SimpleRowMarked() )
                        bColumns = false;
                    else
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScGroupDlg* pDlg = pFact->CreateAbstractScGroupDlg(pTabViewShell->GetDialogParent(), false);
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = false;
                        delete pDlg;
                    }
                }
                if (bOk)
                {
                    pTabViewShell->MakeOutline( bColumns );

                    if( ! rReq.IsAPI() )
                    {
                        OUString aCol = bColumns ? OUString('C') : OUString('R');
                        rReq.AppendItem( SfxStringItem( SID_OUTLINE_MAKE, aCol ) );
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_OUTLINE_REMOVE:
            {
                sal_Bool bColumns = false;
                sal_Bool bOk = sal_True;

                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    pTabViewShell->UngroupDataPilot();
                    bOk = false;
                }
                else if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    bOk = false;

                    if( pReqArgs->HasItem( SID_OUTLINE_REMOVE, &pItem ) )
                    {
                        String aCol = ((const SfxStringItem*)pItem)->GetValue();
                        aCol.ToUpperAscii();

                        switch( aCol.GetChar(0) )
                        {
                            case 'R': bColumns=false; bOk = sal_True;break;
                            case 'C': bColumns=sal_True; bOk = sal_True;break;
                        }
                    }
                }
                else            // Dialog only when removal for rows and columns is possible
                {
                    sal_Bool bColPoss, bRowPoss;
                    pTabViewShell->TestRemoveOutline( bColPoss, bRowPoss );
                    if ( bColPoss && bRowPoss )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScGroupDlg* pDlg = pFact->CreateAbstractScGroupDlg(pTabViewShell->GetDialogParent(), true);
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = false;
                        delete pDlg;
                    }
                    else if ( bColPoss )
                        bColumns = sal_True;
                    else if ( bRowPoss )
                        bColumns = false;
                    else
                        bOk = false;
                }
                if (bOk)
                {
                    pTabViewShell->RemoveOutline( bColumns );

                    if( ! rReq.IsAPI() )
                    {
                        OUString aCol = bColumns ? OUString('C') : OUString('R');
                        rReq.AppendItem( SfxStringItem( SID_OUTLINE_REMOVE, aCol ) );
                        rReq.Done();
                    }
                }
            }
            break;

        //
        //  Clipboard
        //


        case SID_COPY:              // for graphs in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CopyToClip( NULL, false, false, true );
                rReq.Done();
                GetViewData()->SetPasteMode( (ScPasteFlags) (SC_PASTE_MODE | SC_PASTE_BORDER) );
                pTabViewShell->ShowCursor();
                pTabViewShell->UpdateCopySourceOverlay();
            }
            break;

        case SID_CUT:               // for graphs in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CutToClip( NULL, true );
                rReq.Done();
                GetViewData()->SetPasteMode( (ScPasteFlags)(SC_PASTE_MODE | SC_PASTE_BORDER));
                pTabViewShell->ShowCursor();
                pTabViewShell->UpdateCopySourceOverlay();
            }
            break;

        case SID_PASTE:
            {
                ScClipUtil::PasteFromClipboard ( GetViewData(), pTabViewShell, true );
                rReq.Done();
            }
            break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );

                sal_uLong nFormat = 0;
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                }

                if ( nFormat )
                {
                    Window* pWin = GetViewData()->GetActiveWin();
                    sal_Bool bCells = ( ScTransferObj::GetOwnClipboard( pWin ) != NULL );
                    sal_Bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );
                    sal_Bool bOle = ( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE );

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
                sal_uInt16 nFlags = IDF_NONE;
                sal_uInt16 nFunction = PASTE_NOFUNC;
                sal_Bool bSkipEmpty = false;
                sal_Bool bTranspose = false;
                sal_Bool bAsLink    = false;
                InsCellCmd eMoveMode = INS_NONE;

                Window* pWin = GetViewData()->GetActiveWin();
                ScDocument* pDoc = GetViewData()->GetDocument();
                sal_Bool bOtherDoc = !pDoc->IsClipboardSource();
                ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
                if ( pOwnClip )
                {
                    // keep a reference in case the clipboard is changed during dialog or PasteFromClip
                    uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
                    if ( pReqArgs!=NULL && pTabViewShell->SelectionEditable() )
                    {
                        const   SfxPoolItem* pItem;
                        String  aFlags = OUString('A');

                        if( pReqArgs->HasItem( FID_INS_CELL_CONTENTS, &pItem ) )
                            aFlags = ((const SfxStringItem*)pItem)->GetValue();

                        aFlags.ToUpperAscii();
                        sal_Bool    bCont = sal_True;

                        for( xub_StrLen i=0 ; bCont && i<aFlags.Len() ; i++ )
                        {
                            switch( aFlags.GetChar(i) )
                            {
                                case 'A': // all
                                nFlags |= IDF_ALL;
                                bCont = false; // don't continue!
                                break;
                                case 'S': nFlags |= IDF_STRING; break;
                                case 'V': nFlags |= IDF_VALUE; break;
                                case 'D': nFlags |= IDF_DATETIME; break;
                                case 'F': nFlags |= IDF_FORMULA; break;
                                case 'N': nFlags |= IDF_NOTE; break;
                                case 'T': nFlags |= IDF_ATTRIB; break;
                            }
                        }

                        SFX_REQUEST_ARG( rReq, pFuncItem, SfxUInt16Item, FN_PARAM_1, false );
                        SFX_REQUEST_ARG( rReq, pSkipItem, SfxBoolItem, FN_PARAM_2, false );
                        SFX_REQUEST_ARG( rReq, pTransposeItem, SfxBoolItem, FN_PARAM_3, false );
                        SFX_REQUEST_ARG( rReq, pLinkItem, SfxBoolItem, FN_PARAM_4, false );
                        SFX_REQUEST_ARG( rReq, pMoveItem, SfxInt16Item, FN_PARAM_5, false );
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
                            OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                            AbstractScInsertContentsDlg* pDlg = pFact->CreateScInsertContentsDlg( pTabViewShell->GetDialogParent(),
                                                                                                    RID_SCDLG_INSCONT);
                            OSL_ENSURE(pDlg, "Dialog create fail!");
                            pDlg->SetOtherDoc( bOtherDoc );
                            // if ChangeTrack MoveMode disable
                            pDlg->SetChangeTrack( pDoc->GetChangeTrack() != NULL );
                            // fdo#56098  disable shift if necessary
                            if ( !bOtherDoc  && pOwnClip )
                            {
                                ScViewData* pData = GetViewData();
                                if ( pData->GetMarkData().GetTableSelect( pData->GetTabNo() ) )
                                {
                                    SCCOL nStartX, nEndX, nClipStartX, nClipSizeX, nRangeSizeX;
                                    SCROW nStartY, nEndY, nClipStartY, nClipSizeY, nRangeSizeY;
                                    SCTAB nStartTab, nEndTab;
                                    pOwnClip->GetDocument()->GetClipStart( nClipStartX, nClipStartY );
                                    pOwnClip->GetDocument()->GetClipArea( nClipSizeX, nClipSizeY, sal_True );

                                    if ( !( pData->GetSimpleArea( nStartX, nStartY, nStartTab,
                                                   nEndX, nEndY, nEndTab ) == SC_MARK_SIMPLE &&
                                                   nStartTab == nEndTab ) )
                                    {
                                        // the destination is not a simple range,
                                        // assume the destination as the current cell
                                        nStartX = nEndX = pData->GetCurX();
                                        nStartY = nEndY = pData->GetCurY();
                                        nStartTab = pData->GetTabNo();
                                    }
                                    // we now have clip- and range dimensions
                                    // the size of the destination area is the larger of the two
                                    nRangeSizeX = nClipSizeX >= nEndX - nStartX ? nClipSizeX : nEndX - nStartX;
                                    nRangeSizeY = nClipSizeY >= nEndY - nStartY ? nClipSizeY : nEndY - nStartY;
                                    // When the source and destination areas intersect things may go wrong,
                                    // especially if the area contains references. This may produce data loss
                                    // (e.g. formulas that get wrong references), this scenario _must_ be avoided.
                                    ScRange aSource( nClipStartX, nClipStartY, nStartTab,
                                                     nClipStartX + nClipSizeX, nClipStartY + nClipSizeY, nStartTab );
                                    ScRange aDest( nStartX, nStartY, nStartTab,
                                                   nStartX + nRangeSizeX, nStartY + nRangeSizeY, nStartTab );
                                    if ( pOwnClip->GetDocument()->IsCutMode() && aSource.Intersects( aDest ) )
                                        pDlg->SetCellShiftDisabled( SC_CELL_SHIFT_DISABLE_DOWN | SC_CELL_SHIFT_DISABLE_RIGHT );
                                    else
                                    {
                                        //no conflict with intersecting ranges,
                                        //check if paste plus shift will fit on sheet
                                        //and disable shift-option if no fit
                                        int nDisableShiftX = 0;
                                        int nDisableShiftY = 0;

                                        //check if horizontal shift will fit
                                        if ( !pData->GetDocument()->IsBlockEmpty( nStartTab,
                                                    MAXCOL - nRangeSizeX, nStartY,
                                                    MAXCOL, nStartY + nRangeSizeY, false ) )
                                            nDisableShiftX = SC_CELL_SHIFT_DISABLE_RIGHT;

                                        //check if vertical shift will fit
                                        if ( !pData->GetDocument()->IsBlockEmpty( nStartTab,
                                                    nStartX, MAXROW - nRangeSizeY,
                                                    nStartX + nRangeSizeX, MAXROW, false ) )
                                            nDisableShiftY = SC_CELL_SHIFT_DISABLE_DOWN;

                                        if ( nDisableShiftX || nDisableShiftY )
                                            pDlg->SetCellShiftDisabled( nDisableShiftX | nDisableShiftY );
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
                                pTabViewShell->PasteFromSystem(SOT_FORMATSTR_ID_LINK);  // DDE insert
                            else
                            {
                                pTabViewShell->PasteFromClip( nFlags, pOwnClip->GetDocument(),
                                    nFunction, bSkipEmpty, bTranspose, bAsLink,
                                    eMoveMode, IDF_NONE, sal_True );    // allow warning dialog
                            }
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
        case SID_PASTE_ONLY_VALUE:
        case SID_PASTE_ONLY_TEXT:
        case SID_PASTE_ONLY_FORMULA:
        {
            Window* pWin = GetViewData()->GetActiveWin();
            if ( ScTransferObj::GetOwnClipboard( pWin ) )  // own cell data
            {
                rReq.SetSlot( FID_INS_CELL_CONTENTS );
                OUString aFlags;
                if ( nSlot == SID_PASTE_ONLY_VALUE )
                    aFlags = "V";
                else if ( nSlot == SID_PASTE_ONLY_TEXT )
                    aFlags = "S";
                else
                    aFlags = "F";
                rReq.AppendItem( SfxStringItem( FID_INS_CELL_CONTENTS, aFlags ) );
                ExecuteSlot( rReq, GetInterface() );
                rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = success
                pTabViewShell->CellContentChanged();
            }
            else
                rReq.SetReturnValue(SfxInt16Item(nSlot, 0));        // 0 = fail
            break;
        }
        case SID_PASTE_SPECIAL:
            // differentiate between own cell data and draw objects/external data
            // this makes FID_INS_CELL_CONTENTS superfluous
            {
                Window* pWin = GetViewData()->GetActiveWin();

                //  Clipboard-ID given as parameter? Basic "PasteSpecial(Format)"
                const SfxPoolItem* pItem=NULL;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET &&
                     pItem->ISA(SfxUInt32Item) )
                {
                    sal_uLong nFormat = ((const SfxUInt32Item*)pItem)->GetValue();
                    sal_Bool bRet=sal_True;
                    {
                        WaitObject aWait( GetViewData()->GetDialogParent() );
                        sal_Bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );
                        if ( bDraw && nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                            pTabViewShell->PasteDraw();
                        else
                            bRet = pTabViewShell->PasteFromSystem(nFormat, sal_True);       // TRUE: no error messages
                    }

                    if ( bRet )
                    {
                        rReq.SetReturnValue(SfxInt16Item(nSlot, bRet)); // 1 = success, 0 = fail
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
                        rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = success
                    }
                    else                                    // draw objects or external data
                    {
                        sal_Bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != NULL );

                        SvxClipboardFmtItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                        GetPossibleClipboardFormats( aFormats );

                        sal_uInt16 nFormatCount = aFormats.Count();
                        if ( nFormatCount )
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( pTabViewShell->GetDialogParent() );
                            if ( pDlg )
                            {
                            for (sal_uInt16 i=0; i<nFormatCount; i++)
                            {
                                sal_uLong nFormatId = aFormats.GetClipbrdFormatId( i );
                                String aName = aFormats.GetClipbrdFormatName( i );
                                // special case for paste dialog: '*' is replaced by object type
                                if ( nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE )
                                    aName.Assign((sal_Unicode)'*');
                                pDlg->Insert( nFormatId, aName );
                            }

                            TransferableDataHelper aDataHelper(
                                TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
                            sal_uLong nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );
                            if (nFormat > 0)
                            {
                                {
                                    WaitObject aWait( GetViewData()->GetDialogParent() );
                                    if ( bDraw && nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                                        pTabViewShell->PasteDraw();
                                    else
                                        pTabViewShell->PasteFromSystem(nFormat);
                                }
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = success
                                rReq.AppendItem( SfxUInt32Item( nSlot, nFormat ) );
                                rReq.Done();
                            }
                            else
                            {
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 0));    // 0 = fail
                                rReq.Ignore();
                            }

                            delete pDlg;
                            }
                        }
                        else
                            rReq.SetReturnValue(SfxInt16Item(nSlot, 0));        // 0 = fail
                    }
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
            break;

        //
        //  other
        //

        case FID_INS_ROWBRK:
            pTabViewShell->InsertPageBreak( false );
            rReq.Done();
            break;

        case FID_INS_COLBRK:
            pTabViewShell->InsertPageBreak( sal_True );
            rReq.Done();
            break;

        case FID_DEL_ROWBRK:
            pTabViewShell->DeletePageBreak( false );
            rReq.Done();
            break;

        case FID_DEL_COLBRK:
            pTabViewShell->DeletePageBreak( sal_True );
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
                                    OUString("com.sun.star.linguistic2.ChineseTranslationDialog")
                                    , xContext), UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(0);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = OUString("ParentWindow");
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
                                        xProp->getPropertyValue("IsDirectionToSimplified") >>= bToSimplified;
                                        xProp->getPropertyValue("IsUseCharacterVariants") >>= bUseVariants;
                                        xProp->getPropertyValue("IsTranslateCommonTerms") >>= bCommonTerms;
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
            pTabViewShell->ChangeIndent( false );
            break;
        case SID_INC_INDENT:
            pTabViewShell->ChangeIndent( sal_True );
            break;

        case FID_USE_NAME:
            {
                sal_uInt16 nFlags = pTabViewShell->GetCreateNameFlags();

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                AbstractScNameCreateDlg* pDlg = pFact->CreateScNameCreateDlg(pTabViewShell->GetDialogParent(), nFlags);
                OSL_ENSURE(pDlg, "Dialog create fail!");

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
                        pReqArgs->GetItemState( SCITEM_CONSOLIDATEDATA, sal_True, &pItem ) )
                {
                    const ScConsolidateParam& rParam =
                            ((const ScConsolidateItem*)pItem)->GetData();

                    pTabViewShell->Consolidate( rParam );
                    GetViewData()->GetDocument()->SetConsolidateDlgData( &rParam );

                    rReq.Done();
                }
#ifndef DISABLE_SCRIPTING
                else if (rReq.IsAPI())
                    SbxBase::SetError(SbxERR_BAD_PARAMETER);
#endif
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

                if( pReqArgs->HasItem( FID_DEFINE_NAME, &pItem ) )
                    aName = ((const SfxStringItem*)pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                    aSymbol = ((const SfxStringItem*)pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                    aAttrib = ((const SfxStringItem*)pItem)->GetValue();

                if ( aName.Len() && aSymbol.Len() )
                {
                    if (pTabViewShell->InsertName( aName, aSymbol, aAttrib ))
                        rReq.Done();
#ifndef DISABLE_SCRIPTING
                    else
                        SbxBase::SetError( SbxERR_BAD_PARAMETER );  // Basic-error
#endif
                }
            }
            else
            {
                sal_uInt16          nId  = ScNameDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;
        case FID_ADD_NAME:
            {
                sal_uInt16          nId  = ScNameDefDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case SID_OPENDLG_CONDFRMT:
        case SID_OPENDLG_COLORSCALE:
        case SID_OPENDLG_DATABAR:
        case SID_OPENDLG_ICONSET:
        case SID_OPENDLG_CONDDATE:
            {

                ScRangeList aRangeList;
                ScViewData* pData = GetViewData();
                pData->GetMarkData().FillRangeListWithMarks(&aRangeList, false);

                ScDocument* pDoc = GetViewData()->GetDocument();
                if(pDoc->IsTabProtected(pData->GetTabNo()))
                {
                    //ErrorMessage( STR_ERR_CONDFORMAT_PROTECTED );
                    break;
                }

                ScAddress aPos(pData->GetCurX(), pData->GetCurY(), pData->GetTabNo());
                if(aRangeList.empty())
                {
                    ScRange* pRange = new ScRange(aPos);
                    aRangeList.push_back(pRange);
                }

                sal_Int32 nKey = 0;
                const ScPatternAttr* pPattern = pDoc->GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
                const std::vector<sal_uInt32>& rCondFormats = static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData();
                bool bContainsCondFormat = !rCondFormats.empty();
                boost::scoped_ptr<ScCondFormatDlg> pCondFormatDlg;
                if(bContainsCondFormat)
                {
                    bool bContainsExistingCondFormat = false;
                    ScConditionalFormatList* pList = pDoc->GetCondFormList(aPos.Tab());
                    for (std::vector<sal_uInt32>::const_iterator itr = rCondFormats.begin(), itrEnd = rCondFormats.end();
                                            itr != itrEnd; ++itr)
                    {
                        // check if at least one existing conditional format has the same range
                        const ScConditionalFormat* pCondFormat = pList->GetFormat(*itr);
                        if(!pCondFormat)
                            continue;

                        bContainsExistingCondFormat = true;
                        const ScRangeList& rCondFormatRange = pCondFormat->GetRange();
                        if(rCondFormatRange == aRangeList)
                        {
                            // found a matching range, edit this conditional format
                            nKey = pCondFormat->GetKey();
                            pCondFormatDlg.reset( new ScCondFormatDlg( pTabViewShell->GetDialogParent(), pDoc, pCondFormat, rCondFormatRange, aPos, condformat::dialog::NONE ) );
                            break;
                        }
                    }

                    // if not found a conditional format ask whether we should edit one of the existing
                    // or should create a new overlapping conditional format

                    if(!pCondFormatDlg && bContainsExistingCondFormat)
                    {
                        QueryBox aBox( pTabViewShell->GetDialogParent(), WinBits( WB_YES_NO | WB_DEF_YES ),
                               ScGlobal::GetRscString(STR_EDIT_EXISTING_COND_FORMATS) );
                        bool bEditExisting = aBox.Execute() == RET_YES;
                        if(bEditExisting)
                        {
                            // differentiate between ranges where one conditional format is defined
                            // and several formats are defined
                            // if we have only one => open the cond format dlg to edit it
                            // otherwise open the manage cond format dlg
                            if(rCondFormats.size() == 1)
                            {
                                const ScConditionalFormat* pCondFormat = pList->GetFormat(rCondFormats[0]);
                                assert(pCondFormat);
                                const ScRangeList& rCondFormatRange = pCondFormat->GetRange();
                                nKey = pCondFormat->GetKey();
                                pCondFormatDlg.reset( new ScCondFormatDlg( pTabViewShell->GetDialogParent(), pDoc, pCondFormat, rCondFormatRange, aPos, condformat::dialog::NONE ) );
                            }
                            else
                            {
                                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                                boost::scoped_ptr<AbstractScCondFormatManagerDlg> pDlg(pFact->CreateScCondFormatMgrDlg( pTabViewShell->GetDialogParent(), pDoc, pList, aPos, RID_SCDLG_COND_FORMAT_MANAGER));
                                if(pDlg->Execute() == RET_OK && pDlg->CondFormatsChanged())
                                {
                                    ScConditionalFormatList* pCondFormatList = pDlg->GetConditionalFormatList();
                                    pData->GetDocShell()->GetDocFunc().SetConditionalFormatList(pCondFormatList, aPos.Tab());
                                }
                                // we need step out here because we don't want to open the normal dialog
                                break;
                            }
                        }
                        else
                        {
                            // define an overlapping conditional format
                            // does not need to be handled here
                        }

                    }
                }

                if(!pCondFormatDlg)
                {
                    condformat::dialog::ScCondFormatDialogType eType = condformat::dialog::NONE;
                    switch(nSlot)
                    {
                        case SID_OPENDLG_CONDFRMT:
                            eType = condformat::dialog::CONDITION;
                            break;
                        case SID_OPENDLG_COLORSCALE:
                            eType = condformat::dialog::COLORSCALE;
                            break;
                        case SID_OPENDLG_DATABAR:
                            eType = condformat::dialog::DATABAR;
                            break;
                        case SID_OPENDLG_ICONSET:
                            eType = condformat::dialog::ICONSET;
                            break;
                        case SID_OPENDLG_CONDDATE:
                            eType = condformat::dialog::DATE;
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    pCondFormatDlg.reset( new ScCondFormatDlg( pTabViewShell->GetDialogParent(), pDoc, NULL, aRangeList, aRangeList.GetTopLeftCorner(), eType ) );
                }

                sal_uInt16 nId = 1;
                pScMod->SetRefDialog( nId, true );

                if( pCondFormatDlg->Execute() == RET_OK )
                {
                    ScConditionalFormat* pFormat = pCondFormatDlg->GetConditionalFormat();
                    if(pFormat)
                        pData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(nKey, pFormat, aPos.Tab(), pFormat->GetRange());
                    else
                        pData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(nKey, NULL, aPos.Tab(), ScRangeList());
                }

                pScMod->SetRefDialog( nId, false );


            }
            break;

        case SID_DEFINE_COLROWNAMERANGES:
            {

                sal_uInt16          nId  = ScColRowNameRangesDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );

            }
            break;

        case SID_UPDATECHART:
            {
                sal_Bool bAll = false;

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;

                    if( pReqArgs->HasItem( SID_UPDATECHART, &pItem ) )
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
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                AbstractScNamePasteDlg* pDlg = pFact->CreateScNamePasteDlg( pTabViewShell->GetDialogParent(), GetViewData()->GetDocShell() );
                OSL_ENSURE(pDlg, "Dialog create fail!");
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
                                //  "=" in KeyEvent, switches to input-mode
                                pScMod->InputKeyEvent( KeyEvent('=',KeyCode()) );

                                std::vector<OUString> aNames = pDlg->GetSelectedNames();
                                if (!aNames.empty())
                                {
                                    OUStringBuffer aBuffer;
                                    for (std::vector<OUString>::const_iterator itr = aNames.begin();
                                            itr != aNames.end(); ++itr)
                                    {
                                        aBuffer.append(*itr).append(' ');
                                    }
                                    pHdl->InsertFunction( aBuffer.makeStringAndClear(), false );       // without "()"
                                }
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

                //  always cursor position
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
                pTabViewShell->EditNote();                  // note object to edit
            }
            break;

        case FID_NOTE_VISIBLE:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                if( ScPostIt* pNote = pDoc->GetNotes( aPos.Tab() )->findByAddress(aPos) )
                {
                    bool bShow;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs && (pReqArgs->GetItemState( FID_NOTE_VISIBLE, sal_True, &pItem ) == SFX_ITEM_SET) )
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

        case FID_HIDE_NOTE:
        case FID_SHOW_NOTE:
            {
                bool bShowNote     = nSlot == FID_SHOW_NOTE;
                ScViewData* pData  = GetViewData();
                ScDocument* pDoc   = pData->GetDocument();
                ScMarkData& rMark  = pData->GetMarkData();
                bool bDone = false;

                if (!rMark.IsMarked() && !rMark.IsMultiMarked())
                {
                    // Check current cell
                    ScAddress aPos( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
                    if( pDoc->GetNotes( aPos.Tab() )->findByAddress(aPos) )
                    {
                        pData->GetDocShell()->GetDocFunc().ShowNote( aPos, bShowNote );
                        bDone = true;
                    }
                }
                else
                {
                    // Check selection range
                    ScRangeListRef aRangesRef;
                    pData->GetMultiArea(aRangesRef);
                    ScRangeList aRanges = *aRangesRef;
                    size_t nRangeSize = aRanges.size();

                    String aUndo = ScGlobal::GetRscString( bShowNote ? STR_UNDO_SHOWNOTE : STR_UNDO_HIDENOTE );
                    pData->GetDocShell()->GetUndoManager()->EnterListAction( aUndo, aUndo );

                    for ( size_t i = 0; i < nRangeSize; ++i )
                    {
                        const ScRange * pRange = aRanges[i];
                        const SCROW nRow0 = pRange->aStart.Row();
                        const SCROW nRow1 = pRange->aEnd.Row();
                        const SCCOL nCol0 = pRange->aStart.Col();
                        const SCCOL nCol1 = pRange->aEnd.Col();
                        const SCTAB nRangeTab = pRange->aStart.Tab();
                        const size_t nCellNumber = ( nRow1 - nRow0 ) * ( nCol1 - nCol0 );
                        ScNotes *pNotes = pDoc->GetNotes(nRangeTab);

                        if ( nCellNumber < pNotes->size() )
                        {
                            // Check by each cell
                            for ( SCROW nRow = nRow0; nRow <= nRow1; ++nRow )
                            {
                                for ( SCCOL nCol = nCol0; nCol <= nCol1; ++nCol )
                                {
                                    ScPostIt* pNote = pNotes->findByAddress(nCol, nRow);
                                    if ( pNote && pDoc->IsBlockEditable( nRangeTab, nCol,nRow, nCol,nRow ) )
                                    {
                                        ScAddress aPos( nCol, nRow, nRangeTab );
                                        pData->GetDocShell()->GetDocFunc().ShowNote( aPos, bShowNote );
                                        bDone = true;
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Check by each document note
                            for (ScNotes::const_iterator itr = pNotes->begin(); itr != pNotes->end(); ++itr)
                            {
                                SCCOL nCol = itr->first.first;
                                SCROW nRow = itr->first.second;

                                if ( nCol <= nCol1 && nRow <= nRow1 && nCol >= nCol0 && nRow >= nRow0 )
                                {
                                    ScAddress aPos( nCol, nRow, nRangeTab );
                                    pData->GetDocShell()->GetDocFunc().ShowNote( aPos, bShowNote );
                                    bDone = true;
                                }
                            }
                        }
                    }

                    pData->GetDocShell()->GetUndoManager()->LeaveListAction();

                    if ( bDone )
                    {
                        rReq.Done();
                        rBindings.Invalidate( nSlot );
                    }
                    else
                         rReq.Ignore();
                }
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
                    pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), false, &pItem);
                if ( pItem )
                {
                    const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, pItem );
                    if ( pStringItem )
                        aChars = pStringItem->GetValue();
                    const SfxPoolItem* pFtItem = NULL;
                    pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
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
                aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
                aSet.Put( SvxFontItem( aCurFont.GetFamily(), aCurFont.GetName(), aCurFont.GetStyleName(), aCurFont.GetPitch(), aCurFont.GetCharSet(), GetPool().GetWhich(SID_ATTR_CHAR_FONT) ) );

                SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( pTabViewShell->GetDialogParent(), aSet,
                    pTabViewShell->GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );

                if ( pDlg->Execute() == RET_OK )
                {
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pItem, SfxStringItem, SID_CHARMAP, false );
                    SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, false );

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
                        //! why should the return value be valid?!?!
                        rReq.SetReturnValue( SfxStringItem( SID_SELECT_SCENARIO, pItem->GetValue() ) );
                        rReq.Done();
                    }
                    else
                    {
                        OSL_FAIL("NULL");
                    }
                }
            }
            break;

        case SID_HYPERLINK_SETLINK:
            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if( pReqArgs->HasItem( SID_HYPERLINK_SETLINK, &pItem ) )
                {
                    const SvxHyperlinkItem* pHyper = (const SvxHyperlinkItem*) pItem;
                    const String& rName   = pHyper->GetName();
                    const String& rURL    = pHyper->GetURL();
                    const String& rTarget = pHyper->GetTargetFrame();
                    sal_uInt16 nType = (sal_uInt16) pHyper->GetInsertMode();

                    pTabViewShell->InsertURL( rName, rURL, rTarget, nType );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONDFRMT_MANAGER:
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                ScViewData* pData = GetViewData();
                ScDocument* pDoc = pData->GetDocument();

                if(pDoc->IsTabProtected(pData->GetTabNo()))
                {
                    pTabViewShell->ErrorMessage( STR_ERR_CONDFORMAT_PROTECTED );
                    break;
                }

                ScAddress aPos(pData->GetCurX(), pData->GetCurY(), pData->GetTabNo());

                ScConditionalFormatList* pList = pDoc->GetCondFormList( aPos.Tab() );
                boost::scoped_ptr<AbstractScCondFormatManagerDlg> pDlg(pFact->CreateScCondFormatMgrDlg( pTabViewShell->GetDialogParent(), pDoc, pList, aPos, RID_SCDLG_COND_FORMAT_MANAGER));
                if(pDlg->Execute() == RET_OK && pDlg->CondFormatsChanged())
                {
                    ScConditionalFormatList* pCondFormatList = pDlg->GetConditionalFormatList();
                    pData->GetDocShell()->GetDocFunc().SetConditionalFormatList(pCondFormatList, aPos.Tab());
                }
            }
            break;

        case SID_EXTERNAL_SOURCE:
            {
                String aFile;
                String aFilter;
                String aOptions;
                String aSource;
                sal_uLong nRefresh=0;

                SFX_REQUEST_ARG( rReq, pFile, SfxStringItem, SID_FILE_NAME, false );
                SFX_REQUEST_ARG( rReq, pSource, SfxStringItem, FN_PARAM_1, false );
                if ( pFile && pSource )
                {
                    aFile = pFile->GetValue();
                    aSource = pSource->GetValue();
                    SFX_REQUEST_ARG( rReq, pFilter, SfxStringItem, SID_FILTER_NAME, false );
                    if ( pFilter )
                        aFilter = pFilter->GetValue();
                    SFX_REQUEST_ARG( rReq, pOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, false );
                    if ( pOptions )
                        aOptions = pOptions->GetValue();
                    SFX_REQUEST_ARG( rReq, pRefresh, SfxUInt32Item, FN_PARAM_2, false );
                    if ( pRefresh )
                        nRefresh = pRefresh->GetValue();
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    delete pImpl->m_pLinkedDlg;
                    pImpl->m_pLinkedDlg =
                        pFact->CreateScLinkedAreaDlg(pTabViewShell->GetDialogParent());
                    OSL_ENSURE(pImpl->m_pLinkedDlg, "Dialog create fail!");
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
            OSL_FAIL("incorrect slot in ExecuteEdit");
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

void ScCellShell::ExecuteRotateTrans( SfxRequest& rReq )
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
        GetViewData()->GetView()->TransliterateText( m_aRotateCase.getNextMode() );
}

void ScCellShell::ExecuteExternalSource(
    const String& _rFile, const String& _rFilter, const String& _rOptions,
    const String& _rSource, sal_uLong _nRefresh, SfxRequest& _rRequest )
{
    if ( _rFile.Len() && _rSource.Len() )         // filter may be empty
    {
        ScRange aLinkRange;
        sal_Bool bMove = false;

        ScViewData* pData = GetViewData();
        ScMarkData& rMark = pData->GetMarkData();
        rMark.MarkToSimple();
        if ( rMark.IsMarked() )
        {
            rMark.GetMarkArea( aLinkRange );
            bMove = sal_True;                       // insert/delete cells to fit range
        }
        else
            aLinkRange = ScRange( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );

        pData->GetDocFunc().InsertAreaLink( _rFile, _rFilter, _rOptions, _rSource,
                                            aLinkRange, _nRefresh, bMove, false );
        _rRequest.Done();
    }
    else
        _rRequest.Ignore();
}

namespace {

bool isDPSourceValid(const ScDPObject& rDPObj)
{
    if (rDPObj.IsImportData())
    {
        // If the data type is database, check if the database is still valid.
        const ScImportSourceDesc* pDesc = rDPObj.GetImportSourceDesc();
        if (!pDesc)
            return false;

        const ScDPSaveData* pSaveData = rDPObj.GetSaveData();
        const ScDPDimensionSaveData* pDimData = NULL;
        if (pSaveData)
            pDimData = pSaveData->GetExistingDimensionData();

        const ScDPCache* pCache = pDesc->CreateCache(pDimData);
        if (!pCache)
            // cashe creation failed, probably due to invalid connection.
            return false;
    }
    return true;
}

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
        if (isDPSourceValid(*pDPObj))
            pNewDPObject.reset(new ScDPObject(*pDPObj));
    }
    else            // create new table
    {
        sal_uLong nSrcErrorId = 0;

        //  select database range or data
        pTabViewShell->GetDBData( true, SC_DB_OLD );
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
            pTabViewShell->MarkDataArea( false );

        //  output to cursor position for non-sheet data
        ScAddress aDestPos( pData->GetCurX(), pData->GetCurY(),
                                pData->GetTabNo() );

        //  first select type of source data

        bool bEnableExt = ScDPObject::HasRegisteredSources();

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

        ::boost::scoped_ptr<AbstractScDataPilotSourceTypeDlg> pTypeDlg(
            pFact->CreateScDataPilotSourceTypeDlg(
                pTabViewShell->GetDialogParent(), bEnableExt));

        // Populate named ranges (if any).
        ScRangeName* pRangeName = pDoc->GetRangeName();
        if (pRangeName)
        {
            ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
            for (; itr != itrEnd; ++itr)
                pTypeDlg->AppendNamedRange(itr->second->GetName());
        }

        OSL_ENSURE(pTypeDlg, "Dialog create fail!");
        if ( pTypeDlg->Execute() == RET_OK )
        {
            if ( pTypeDlg->IsExternal() )
            {
                uno::Sequence<OUString> aSources = ScDPObject::GetRegisteredSources();
                ::boost::scoped_ptr<AbstractScDataPilotServiceDlg> pServDlg(
                    pFact->CreateScDataPilotServiceDlg(
                        pTabViewShell->GetDialogParent(), aSources, RID_SCDLG_DAPISERVICE));

                OSL_ENSURE(pServDlg, "Dialog create fail!");
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
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                ::boost::scoped_ptr<AbstractScDataPilotDatabaseDlg> pDataDlg(
                    pFact->CreateScDataPilotDatabaseDlg(
                        pTabViewShell->GetDialogParent(), RID_SCDLG_DAPIDATA));

                OSL_ENSURE(pDataDlg, "Dialog create fail!");
                if ( pDataDlg->Execute() == RET_OK )
                {
                    ScImportSourceDesc aImpDesc(pDoc);
                    pDataDlg->GetValues( aImpDesc );
                    pNewDPObject.reset(new ScDPObject(pDoc));
                    pNewDPObject->SetImportDesc( aImpDesc );
                }
            }
            else if (pTypeDlg->IsNamedRange())
            {
                OUString aName = pTypeDlg->GetSelectedNamedRange();
                ScSheetSourceDesc aShtDesc(pDoc);
                aShtDesc.SetRangeName(aName);
                nSrcErrorId = aShtDesc.CheckSourceRange();
                if (!nSrcErrorId)
                {
                    pNewDPObject.reset(new ScDPObject(pDoc));
                    pNewDPObject->SetSheetDesc(aShtDesc);
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

                    bool bOK = true;
                    if ( pDoc->HasSubTotalCells( aRange ) )
                    {
                        //  confirm selection if it contains SubTotal cells

                        QueryBox aBox( pTabViewShell->GetDialogParent(),
                                        WinBits(WB_YES_NO | WB_DEF_YES),
                                        ScGlobal::GetRscString(STR_DATAPILOT_SUBTOTAL) );
                        if (aBox.Execute() == RET_NO)
                            bOK = false;
                    }
                    if (bOK)
                    {
                        ScSheetSourceDesc aShtDesc(pDoc);
                        aShtDesc.SetSourceRange(aRange);
                        nSrcErrorId = aShtDesc.CheckSourceRange();
                        if (!nSrcErrorId)
                        {
                            pNewDPObject.reset(new ScDPObject(pDoc));
                            pNewDPObject->SetSheetDesc( aShtDesc );
                        }

                        //  output below source data
                        if ( aRange.aEnd.Row()+2 <= MAXROW - 4 )
                            aDestPos = ScAddress( aRange.aStart.Col(),
                                                    aRange.aEnd.Row()+2,
                                                    aRange.aStart.Tab() );
                    }
                }
            }
        }

        if (nSrcErrorId)
        {
            // Error occurred during data creation.  Launch an error and bail out.
            InfoBox aBox(pTabViewShell->GetDialogParent(), ScGlobal::GetRscString(nSrcErrorId));
            aBox.Execute();
            return;
        }

        if ( pNewDPObject )
            pNewDPObject->SetOutRange( aDestPos );
    }

    pTabViewShell->SetDialogDPObject( pNewDPObject.get() );   // is copied
    if ( pNewDPObject )
    {
        //  start layout dialog

        sal_uInt16 nId  = ScPivotLayoutWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );
        pScMod->SetRefDialog( nId, pWnd ? false : true );
    }
}

void ScCellShell::ExecuteXMLSourceDialog()
{
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    if (!pFact)
        return;

    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    if (!pTabViewShell)
        return;

    ScModule* pScMod = SC_MOD();

    sal_uInt16 nId = ScXMLSourceDlgWrapper::GetChildWindowId();
    SfxViewFrame* pViewFrame = pTabViewShell->GetViewFrame();
    SfxChildWindow* pWnd = pViewFrame->GetChildWindow(nId);
    pScMod->SetRefDialog(nId, pWnd ? false : true);
}

void ScCellShell::ExecuteSubtotals(SfxRequest& rReq)
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        pTabViewShell->DoSubTotals( ((const ScSubTotalItem&) pArgs->Get( SCITEM_SUBTDATA )).
                        GetSubTotalData() );
        rReq.Done();
        return;
    }

    SfxAbstractTabDialog * pDlg = NULL;
    ScSubTotalParam aSubTotalParam;
    SfxItemSet aArgSet( GetPool(), SCITEM_SUBTDATA, SCITEM_SUBTDATA );

    // Only get existing named database range.
    ScDBData* pDBData = pTabViewShell->GetDBData(true, SC_DB_OLD);
    if (!pDBData)
    {
        // No existing DB data at this position.  Create an
        // anonymous DB.
        pDBData = pTabViewShell->GetAnonymousDBData();
        ScRange aDataRange;
        pDBData->GetArea(aDataRange);
        pTabViewShell->MarkRange(aDataRange, false);
    }
    if (!pDBData)
        return;

    pDBData->GetSubTotalParam( aSubTotalParam );
    aSubTotalParam.bRemoveOnly = false;

    aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, GetViewData(), &aSubTotalParam ) );
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    pDlg = pFact->CreateScSubTotalDlg( pTabViewShell->GetDialogParent(), &aArgSet, RID_SCDLG_SUBTOTALS );
    OSL_ENSURE(pDlg, "Dialog create fail!");
    pDlg->SetCurPageId(1);

    short bResult = pDlg->Execute();

    if ( (bResult == RET_OK) || (bResult == SCRET_REMOVE) )
    {
        const SfxItemSet* pOutSet = NULL;

        if ( bResult == RET_OK )
        {
            pOutSet = pDlg->GetOutputItemSet();
            aSubTotalParam =
                ((const ScSubTotalItem&)
                    pOutSet->Get( SCITEM_SUBTDATA )).
                        GetSubTotalData();
        }
        else // if (bResult == SCRET_REMOVE)
        {
            pOutSet = &aArgSet;
            aSubTotalParam.bRemoveOnly = sal_True;
            aSubTotalParam.bReplace    = sal_True;
            aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA,
                                         GetViewData(),
                                         &aSubTotalParam ) );
        }

        pTabViewShell->DoSubTotals( aSubTotalParam );
        rReq.Done( *pOutSet );
    }
    else
        GetViewData()->GetDocShell()->CancelAutoDBRange();

    delete pDlg;
}

IMPL_LINK_NOARG(ScCellShell, DialogClosed)
{
    OSL_ENSURE( pImpl->m_pLinkedDlg, "ScCellShell::DialogClosed(): invalid request" );
    OSL_ENSURE( pImpl->m_pRequest, "ScCellShell::DialogClosed(): invalid request" );
    String sFile, sFilter, sOptions, sSource;
    sal_uLong nRefresh = 0;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
