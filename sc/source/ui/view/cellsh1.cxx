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

#include <config_features.h>

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
#include <vcl/builderfactory.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/editview.hxx>

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
#include "dpgroup.hxx"
#include "spellparam.hxx"
#include "postit.hxx"
#include "clipparam.hxx"
#include "pivot.hxx"
#include "dpsdbtab.hxx"
#include "dpshttab.hxx"
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
#include <tokenstringcontext.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <formulacell.hxx>
#include <gridwin.hxx>
#include <searchresults.hxx>

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

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

        //  insert / delete cells / rows / columns

        case FID_INS_ROW:
        case FID_INS_ROWS_BEFORE:
            pTabViewShell->InsertCells(INS_INSROWS_BEFORE);
            rReq.Done();
            break;

        case FID_INS_COLUMN:
        case FID_INS_COLUMNS_BEFORE:
            pTabViewShell->InsertCells(INS_INSCOLS_BEFORE);
            rReq.Done();
            break;

        case FID_INS_ROWS_AFTER:
            pTabViewShell->InsertCells(INS_INSROWS_AFTER);
            rReq.Done();
            break;

        case FID_INS_COLUMNS_AFTER:
            pTabViewShell->InsertCells(INS_INSCOLS_AFTER);
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
                    OUString aFlags;

                    if( pReqArgs->HasItem( FID_INS_CELL, &pItem ) )
                        aFlags = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( !aFlags.isEmpty() )
                    {
                        switch( aFlags[0] )
                        {
                            case 'V': eCmd = INS_CELLSDOWN ;break;
                            case '>': eCmd = INS_CELLSRIGHT ;break;
                            case 'R': eCmd = INS_INSROWS_BEFORE ;break;
                            case 'C': eCmd = INS_INSCOLS_BEFORE ;break;
                        }
                    }
                }
                else
                {
                    if ( GetViewData()->SimpleColMarked() )
                        eCmd = INS_INSCOLS_BEFORE;
                    else if ( GetViewData()->SimpleRowMarked() )
                        eCmd = INS_INSROWS_BEFORE;
                    else
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        bool bTheFlag=(pDoc->GetChangeTrack()!=nullptr);

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScInsertCellDlg> pDlg(pFact->CreateScInsertCellDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_INSCELL, bTheFlag));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if (pDlg->Execute() == RET_OK)
                            eCmd = pDlg->GetInsCellCmd();
                    }
                }

                if (eCmd!=INS_NONE)
                {
                    pTabViewShell->InsertCells( eCmd );

                    if( ! rReq.IsAPI() )
                    {
                        OUString aParam;

                        switch( eCmd )
                        {
                            case INS_CELLSDOWN: aParam = "V"; break;
                            case INS_CELLSRIGHT: aParam = ">"; break;
                            case INS_INSROWS_BEFORE: aParam = "R"; break;
                            case INS_INSCOLS_BEFORE: aParam = "C"; break;
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
                    OUString aFlags;

                    if( pReqArgs->HasItem( FID_DELETE_CELL, &pItem ) )
                        aFlags = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( !aFlags.isEmpty() )
                    {
                        switch( aFlags[0] )
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
                            (pDoc->GetChangeTrack() != nullptr);

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScDeleteCellDlg> pDlg(pFact->CreateScDeleteCellDlg( pTabViewShell->GetDialogParent(), bTheFlag ));
                        OSL_ENSURE(pDlg, "Dialog create fail!");

                        if (pDlg->Execute() == RET_OK)
                            eCmd = pDlg->GetDelCellCmd();
                    }
                }

                if (eCmd != DEL_NONE )
                {
                    pTabViewShell->DeleteCells( eCmd );

                    if( ! rReq.IsAPI() )
                    {
                        OUString aParam;

                        switch( eCmd )
                        {
                            case DEL_CELLSUP: aParam = "U"; break;
                            case DEL_CELLSLEFT: aParam = "L"; break;
                            case DEL_DELROWS: aParam = "R"; break;
                            case DEL_DELCOLS: aParam = "C"; break;
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

        //  delete contents from cells

        case SID_DELETE_CONTENTS:
            pTabViewShell->DeleteContents( InsertDeleteFlags::CONTENTS );
            rReq.Done();
            break;

        case SID_DELETE:
            {
                InsertDeleteFlags nFlags = InsertDeleteFlags::NONE;

                if ( pReqArgs!=nullptr && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    OUString aFlags('A');

                    if( pReqArgs->HasItem( SID_DELETE, &pItem ) )
                        aFlags = static_cast<const SfxStringItem*>(pItem)->GetValue();

                    aFlags = aFlags.toAsciiUpperCase();
                    bool    bCont = true;

                    for (sal_Int32 i=0 ; bCont && i<aFlags.getLength(); ++i)
                    {
                        switch (aFlags[i])
                        {
                            case 'A': // all
                            nFlags |= InsertDeleteFlags::ALL;
                            bCont = false; // don't continue!
                            break;
                            case 'S': nFlags |= InsertDeleteFlags::STRING; break;
                            case 'V': nFlags |= InsertDeleteFlags::VALUE; break;
                            case 'D': nFlags |= InsertDeleteFlags::DATETIME; break;
                            case 'F': nFlags |= InsertDeleteFlags::FORMULA; break;
                            case 'N': nFlags |= InsertDeleteFlags::NOTE; break;
                            case 'T': nFlags |= InsertDeleteFlags::ATTRIB; break;
                            case 'O': nFlags |= InsertDeleteFlags::OBJECTS; break;
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

                        std::unique_ptr<AbstractScDeleteContentsDlg> pDlg(pFact->CreateScDeleteContentsDlg(pTabViewShell->GetDialogParent()));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        SCTAB nTab = GetViewData()->GetTabNo();
                        if ( pDoc->IsTabProtected(nTab) )
                            pDlg->DisableObjects();
                        if (pDlg->Execute() == RET_OK)
                        {
                            nFlags = pDlg->GetDelContentsCmdBits();
                        }
                    }
                    else
                        pTabViewShell->ErrorMessage(aTester.GetMessageId());
                }

                if( nFlags != InsertDeleteFlags::NONE )
                {
                    pTabViewShell->DeleteContents( nFlags );

                    if( ! rReq.IsAPI() )
                    {
                        OUString  aFlags;

                        if( nFlags == InsertDeleteFlags::ALL )
                        {
                            aFlags += "A";
                        }
                        else
                        {
                            if( nFlags & InsertDeleteFlags::STRING ) aFlags += "S";
                            if( nFlags & InsertDeleteFlags::VALUE ) aFlags += "V";
                            if( nFlags & InsertDeleteFlags::DATETIME ) aFlags += "D";
                            if( nFlags & InsertDeleteFlags::FORMULA ) aFlags += "F";
                            if( nFlags & InsertDeleteFlags::NOTE ) aFlags += "N";
                            if( nFlags & InsertDeleteFlags::ATTRIB ) aFlags += "T";
                            if( nFlags & InsertDeleteFlags::OBJECTS ) aFlags += "O";
                        }

                        rReq.AppendItem( SfxStringItem( SID_DELETE, aFlags ) );
                        rReq.Done();
                    }
                }
            }
            break;

        //  fill...

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
                InsertDeleteFlags nFlags = InsertDeleteFlags::NONE;
                ScPasteFunc nFunction = ScPasteFunc::NONE;
                bool bSkipEmpty = false;
                bool bAsLink    = false;

                if ( pReqArgs!=nullptr && pTabViewShell->SelectionEditable() )
                {
                    const   SfxPoolItem* pItem;
                    OUString aFlags('A');

                    if( pReqArgs->HasItem( FID_FILL_TAB, &pItem ) )
                        aFlags = static_cast<const SfxStringItem*>(pItem)->GetValue();

                    aFlags = aFlags.toAsciiUpperCase();
                    bool    bCont = true;

                    for (sal_Int32 i=0; bCont && i < aFlags.getLength(); ++i)
                    {
                        switch (aFlags[i])
                        {
                            case 'A': // all
                            nFlags |= InsertDeleteFlags::ALL;
                            bCont = false; // don't continue!
                            break;
                            case 'S': nFlags |= InsertDeleteFlags::STRING; break;
                            case 'V': nFlags |= InsertDeleteFlags::VALUE; break;
                            case 'D': nFlags |= InsertDeleteFlags::DATETIME; break;
                            case 'F': nFlags |= InsertDeleteFlags::FORMULA; break;
                            case 'N': nFlags |= InsertDeleteFlags::NOTE; break;
                            case 'T': nFlags |= InsertDeleteFlags::ATTRIB; break;
                        }
                    }
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    std::unique_ptr<AbstractScInsertContentsDlg> pDlg(pFact->CreateScInsertContentsDlg( pTabViewShell->GetDialogParent(),
                                                                                            InsertDeleteFlags::NONE, /* nCheckDefaults */
                                                                                            &ScGlobal::GetRscString(STR_FILL_TAB)));
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
                }

                if( nFlags != InsertDeleteFlags::NONE )
                {
                    pTabViewShell->FillTab( nFlags, nFunction, bSkipEmpty, bAsLink );

                    if( ! rReq.IsAPI() )
                    {
                        OUString  aFlags;

                        if( nFlags == InsertDeleteFlags::ALL )
                        {
                            aFlags += "A";
                        }
                        else
                        {
                            if( nFlags & InsertDeleteFlags::STRING ) aFlags += "S";
                            if( nFlags & InsertDeleteFlags::VALUE ) aFlags += "V";
                            if( nFlags & InsertDeleteFlags::DATETIME ) aFlags += "D";
                            if( nFlags & InsertDeleteFlags::FORMULA ) aFlags += "F";
                            if( nFlags & InsertDeleteFlags::NOTE ) aFlags += "N";
                            if( nFlags & InsertDeleteFlags::ATTRIB ) aFlags += "T";
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
                bool   bDoIt     = false;

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
                    OUString  aFillDir, aFillCmd, aFillDateCmd;
                    OUString  aFillStep, aFillStart, aFillMax;
                    sal_uInt32 nKey;
                    double  fTmpVal;

                    bDoIt=false;

                    if( pReqArgs->HasItem( FID_FILL_SERIES, &pItem ) )
                        aFillDir = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                        aFillCmd = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                        aFillDateCmd = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_3, &pItem ) )
                        aFillStep = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_4, &pItem ) )
                        aFillStart = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_5, &pItem ) )
                        aFillMax = static_cast<const SfxStringItem*>(pItem)->GetValue();

                    if( !aFillDir.isEmpty() )
                        switch( aFillDir[0] )
                        {
                            case 'B': case 'b': eFillDir=FILL_TO_BOTTOM; break;
                            case 'R': case 'r': eFillDir=FILL_TO_RIGHT; break;
                            case 'T': case 't': eFillDir=FILL_TO_TOP; break;
                            case 'L': case 'l': eFillDir=FILL_TO_LEFT; break;
                        }

                    if( !aFillCmd.isEmpty() )
                        switch( aFillCmd[0] )
                        {
                            case 'S': case 's': eFillCmd=FILL_SIMPLE; break;
                            case 'L': case 'l': eFillCmd=FILL_LINEAR; break;
                            case 'G': case 'g': eFillCmd=FILL_GROWTH; break;
                            case 'D': case 'd': eFillCmd=FILL_DATE; break;
                            case 'A': case 'a': eFillCmd=FILL_AUTO; break;
                        }

                    if( !aFillDateCmd.isEmpty() )
                        switch( aFillDateCmd[0] )
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

                    bDoIt   = true;

                }
                else // (pReqArgs == nullptr) => raise Dialog
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
                        if ( ( nPrivType & css::util::NumberFormat::DATE)>0)
                        {
                           eFillCmd=FILL_DATE;
                        }
                        else if(eCellType==CELLTYPE_STRING)
                        {
                           eFillCmd=FILL_AUTO;
                        }
                    }

                    OUString aStartStr;

                    //  suggest default Startvalue only, when just 1 row or column
                    if ( nStartCol == nEndCol || nStartRow == nEndRow )
                    {
                        double fInputEndVal = 0.0;
                        OUString aEndStr;

                        pDoc->GetInputString( nStartCol, nStartRow, nStartTab, aStartStr);
                        pDoc->GetValue( nStartCol, nStartRow, nStartTab, fStartVal );

                        if(eFillDir==FILL_TO_BOTTOM && nStartRow < nEndRow )
                        {
                            pDoc->GetInputString( nStartCol, nStartRow+1, nStartTab, aEndStr);
                            if(!aEndStr.isEmpty())
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
                                if(!aEndStr.isEmpty())
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

                    std::unique_ptr<AbstractScFillSeriesDlg> pDlg(pFact->CreateScFillSeriesDlg( pTabViewShell->GetDialogParent(),
                                                            *pDoc,
                                                            eFillDir, eFillCmd, eFillDateCmd,
                                                            aStartStr, fIncVal, fMaxVal,
                                                            nPossDir));
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    if ( nStartCol != nEndCol && nStartRow != nEndRow )
                    {
                        pDlg->SetEdStartValEnabled();
                    }

                    if ( pDlg->Execute() == RET_OK )
                    {
                        eFillDir        = pDlg->GetFillDir();
                        eFillCmd        = pDlg->GetFillCmd();
                        eFillDateCmd    = pDlg->GetFillDateCmd();

                        if(eFillCmd==FILL_AUTO)
                        {
                            OUString aStr = pDlg->GetStartStr();
                            if(!aStr.isEmpty())
                                pTabViewShell->EnterData( nStartCol, nStartRow, nStartTab, aStr );
                        }
                        fStartVal       = pDlg->GetStart();
                        fIncVal         = pDlg->GetStep();
                        fMaxVal         = pDlg->GetMax();
                        bDoIt           = true;
                    }
                }

                if( bDoIt )
                {
                    //nScFillModeMouseModifier = 0; // no Ctrl/Copy
                    pTabViewShell->FillSeries( eFillDir, eFillCmd, eFillDateCmd, fStartVal, fIncVal, fMaxVal );

                    if( ! rReq.IsAPI() )
                    {
                        OUString  aPara;
                        Color*  pColor=nullptr;

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

                        sal_uLong nFormatKey = pFormatter->GetStandardFormat(css::util::NumberFormat::NUMBER,
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

                if( pReqArgs != nullptr )
                {
                    const SfxPoolItem* pItem;

                    if( pReqArgs->HasItem( FID_FILL_AUTO, &pItem ) )
                    {
                        ScAddress aScAddress;
                        OUString aArg = static_cast<const SfxStringItem*>(pItem)->GetValue();

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
                                ScAddress aAdr( nFillCol, nFillRow, 0 );
                                OUString  aAdrStr(aAdr.Format(SCR_ABS, pDoc, pDoc->GetAddressConvention()));

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
        case FID_FILL_SINGLE_EDIT:
            ExecuteFillSingleEdit();
            break;
        case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
            {
                sal_uInt16 nId  = ScRandomNumberGeneratorDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;
        case SID_SAMPLING_DIALOG:
            {
                sal_uInt16 nId  = ScSamplingDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            {
                sal_uInt16 nId  = ScDescriptiveStatisticsDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_ANALYSIS_OF_VARIANCE_DIALOG:
            {
                sal_uInt16 nId  = ScAnalysisOfVarianceDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_CORRELATION_DIALOG:
            {
                sal_uInt16 nId  = ScCorrelationDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_COVARIANCE_DIALOG:
            {
                sal_uInt16 nId  = ScCovarianceDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_EXPONENTIAL_SMOOTHING_DIALOG:
            {
                sal_uInt16 nId  = ScExponentialSmoothingDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_MOVING_AVERAGE_DIALOG:
            {
                sal_uInt16 nId  = ScMovingAverageDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_REGRESSION_DIALOG:
            {
                sal_uInt16 nId  = ScRegressionDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case SID_TTEST_DIALOG:
            {
                sal_uInt16 nId  = ScTTestDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;
        case SID_FTEST_DIALOG:
            {
                sal_uInt16 nId  = ScFTestDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;
        case SID_ZTEST_DIALOG:
            {
                sal_uInt16 nId  = ScZTestDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;
        case SID_CHI_SQUARE_TEST_DIALOG:
            {
                sal_uInt16 nId  = ScChiSquareTestDialogWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;

        case SID_SEARCH_RESULTS_DIALOG:
        {
            const SfxPoolItem* pItem = nullptr;
            if (pReqArgs && pReqArgs->HasItem(SID_SEARCH_RESULTS_DIALOG, &pItem))
            {
                bool bVisible = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                // The window ID should equal the slot ID, but not a biggie if it wasn't.
                sal_uInt16 nId = sc::SearchResultsDlgWrapper::GetChildWindowId();
                pViewFrm->SetChildWindow(nId, bVisible, false);
            }
            rReq.Done();
        }
        break;

        //  disposal (Outlines)
        //  SID_AUTO_OUTLINE, SID_OUTLINE_DELETEALL in Execute (in docsh.idl)

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
                bool bColumns = false;
                bool bOk = true;

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
                else if( pReqArgs != nullptr )
                {
                    const SfxPoolItem* pItem;
                    bOk = false;

                    if( pReqArgs->HasItem( SID_OUTLINE_MAKE, &pItem ) )
                    {
                        OUString aCol = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        aCol = aCol.toAsciiUpperCase();

                        switch( aCol[0] )
                        {
                            case 'R': bColumns=false; bOk = true;break;
                            case 'C': bColumns=true; bOk = true;break;
                        }
                    }
                }
                else            // Dialog, when not whole rows/columns are marked
                {
                    if ( GetViewData()->SimpleColMarked() && !GetViewData()->SimpleRowMarked() )
                        bColumns = true;
                    else if ( !GetViewData()->SimpleColMarked() && GetViewData()->SimpleRowMarked() )
                        bColumns = false;
                    else
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScGroupDlg> pDlg(pFact->CreateAbstractScGroupDlg(pTabViewShell->GetDialogParent()));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = false;
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
                bool bColumns = false;
                bool bOk = true;

                if ( GetViewData()->GetDocument()->GetDPAtCursor( GetViewData()->GetCurX(),
                                        GetViewData()->GetCurY(), GetViewData()->GetTabNo() ) )
                {
                    pTabViewShell->UngroupDataPilot();
                    bOk = false;
                }
                else if( pReqArgs != nullptr )
                {
                    const SfxPoolItem* pItem;
                    bOk = false;

                    if( pReqArgs->HasItem( SID_OUTLINE_REMOVE, &pItem ) )
                    {
                        OUString aCol = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        aCol = aCol.toAsciiUpperCase();

                        switch (aCol[0])
                        {
                            case 'R': bColumns=false; bOk = true;break;
                            case 'C': bColumns=true; bOk = true;break;
                        }
                    }
                }
                else            // Dialog only when removal for rows and columns is possible
                {
                    bool bColPoss, bRowPoss;
                    pTabViewShell->TestRemoveOutline( bColPoss, bRowPoss );
                    if ( bColPoss && bRowPoss )
                    {
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScGroupDlg> pDlg(pFact->CreateAbstractScGroupDlg(pTabViewShell->GetDialogParent(), true));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                            bColumns = pDlg->GetColsChecked();
                        else
                            bOk = false;
                    }
                    else if ( bColPoss )
                        bColumns = true;
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

        //  Clipboard

        case SID_COPY:              // for graphs in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CopyToClip( nullptr, false, false, true );
                rReq.Done();
                GetViewData()->SetPasteMode( (ScPasteFlags) (SC_PASTE_MODE | SC_PASTE_BORDER) );
                pTabViewShell->ShowCursor();
                pTabViewShell->UpdateCopySourceOverlay();
            }
            break;

        case SID_CUT:               // for graphs in DrawShell
            {
                WaitObject aWait( GetViewData()->GetDialogParent() );
                pTabViewShell->CutToClip( nullptr, true );
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

                SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET &&
                     dynamic_cast<const SfxUInt32Item*>( pItem) !=  nullptr )
                {
                    nFormat = static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pItem)->GetValue());
                }

                if ( nFormat != SotClipboardFormatId::NONE )
                {
                    vcl::Window* pWin = GetViewData()->GetActiveWin();
                    bool bCells = ( ScTransferObj::GetOwnClipboard( pWin ) != nullptr );
                    bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != nullptr );
                    bool bOle = ( nFormat == SotClipboardFormatId::EMBED_SOURCE );

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
                InsertDeleteFlags nFlags = InsertDeleteFlags::NONE;
                ScPasteFunc nFunction = ScPasteFunc::NONE;
                InsCellCmd eMoveMode = INS_NONE;

                vcl::Window* pWin = GetViewData()->GetActiveWin();
                ScDocument* pDoc = GetViewData()->GetDocument();
                bool bOtherDoc = !pDoc->IsClipboardSource();
                ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
                if ( pOwnClip )
                {
                    bool bSkipEmpty = false;
                    bool bTranspose = false;
                    bool bAsLink    = false;

                    // keep a reference in case the clipboard is changed during dialog or PasteFromClip
                    uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
                    if ( pReqArgs!=nullptr && pTabViewShell->SelectionEditable() )
                    {
                        const   SfxPoolItem* pItem;
                        OUString aFlags('A');

                        if( pReqArgs->HasItem( FID_INS_CELL_CONTENTS, &pItem ) )
                            aFlags = static_cast<const SfxStringItem*>(pItem)->GetValue();

                        aFlags = aFlags.toAsciiUpperCase();
                        bool    bCont = true;

                        for (sal_Int32 i=0 ; bCont && i<aFlags.getLength(); ++i)
                        {
                            switch (aFlags[i])
                            {
                                case 'A': // all
                                nFlags |= InsertDeleteFlags::ALL;
                                bCont = false; // don't continue!
                                break;
                                case 'S': nFlags |= InsertDeleteFlags::STRING; break;
                                case 'V': nFlags |= InsertDeleteFlags::VALUE; break;
                                case 'D': nFlags |= InsertDeleteFlags::DATETIME; break;
                                case 'F': nFlags |= InsertDeleteFlags::FORMULA; break;
                                case 'N': nFlags |= InsertDeleteFlags::NOTE; break;
                                case 'T': nFlags |= InsertDeleteFlags::ATTRIB; break;
                            }
                        }

                        const SfxUInt16Item* pFuncItem = rReq.GetArg<SfxUInt16Item>(FN_PARAM_1);
                        const SfxBoolItem* pSkipItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_2);
                        const SfxBoolItem* pTransposeItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_3);
                        const SfxBoolItem* pLinkItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_4);
                        const SfxInt16Item* pMoveItem = rReq.GetArg<SfxInt16Item>(FN_PARAM_5);
                        if ( pFuncItem )
                            nFunction = static_cast<ScPasteFunc>(pFuncItem->GetValue());
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

                            std::unique_ptr<AbstractScInsertContentsDlg> pDlg(pFact->CreateScInsertContentsDlg(pTabViewShell->GetDialogParent()));
                            OSL_ENSURE(pDlg, "Dialog create fail!");
                            pDlg->SetOtherDoc( bOtherDoc );
                            // if ChangeTrack MoveMode disable
                            pDlg->SetChangeTrack( pDoc->GetChangeTrack() != nullptr );
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
                                    pOwnClip->GetDocument()->GetClipArea( nClipSizeX, nClipSizeY, true );

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
                                                    MAXCOL, nStartY + nRangeSizeY ) )
                                            nDisableShiftX = SC_CELL_SHIFT_DISABLE_RIGHT;

                                        //check if vertical shift will fit
                                        if ( !pData->GetDocument()->IsBlockEmpty( nStartTab,
                                                    nStartX, MAXROW - nRangeSizeY,
                                                    nStartX + nRangeSizeX, MAXROW ) )
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
                        }
                        else
                            pTabViewShell->ErrorMessage(aTester.GetMessageId());
                    }

                    if( nFlags != InsertDeleteFlags::NONE )
                    {
                        {
                            WaitObject aWait( GetViewData()->GetDialogParent() );
                            if ( bAsLink && bOtherDoc )
                                pTabViewShell->PasteFromSystem(SotClipboardFormatId::LINK);  // DDE insert
                            else
                            {
                                pTabViewShell->PasteFromClip( nFlags, pOwnClip->GetDocument(),
                                    nFunction, bSkipEmpty, bTranspose, bAsLink,
                                    eMoveMode, InsertDeleteFlags::NONE, true );    // allow warning dialog
                            }
                        }

                        if( !pReqArgs )
                        {
                            OUString  aFlags;

                            if( nFlags == InsertDeleteFlags::ALL )
                            {
                                aFlags += "A";
                            }
                            else
                            {
                                if( nFlags & InsertDeleteFlags::STRING ) aFlags += "S";
                                if( nFlags & InsertDeleteFlags::VALUE ) aFlags += "V";
                                if( nFlags & InsertDeleteFlags::DATETIME ) aFlags += "D";
                                if( nFlags & InsertDeleteFlags::FORMULA ) aFlags += "F";
                                if( nFlags & InsertDeleteFlags::NOTE ) aFlags += "N";
                                if( nFlags & InsertDeleteFlags::ATTRIB ) aFlags += "T";
                            }

                            rReq.AppendItem( SfxStringItem( FID_INS_CELL_CONTENTS, aFlags ) );
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bSkipEmpty ) );
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_3, bTranspose ) );
                            rReq.AppendItem( SfxBoolItem( FN_PARAM_4, bAsLink ) );
                            rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nFunction) ) );
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
            vcl::Window* pWin = GetViewData()->GetActiveWin();
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
                vcl::Window* pWin = GetViewData()->GetActiveWin();

                //  Clipboard-ID given as parameter? Basic "PasteSpecial(Format)"
                const SfxPoolItem* pItem=nullptr;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET &&
                     dynamic_cast<const SfxUInt32Item*>( pItem) !=  nullptr )
                {
                    SotClipboardFormatId nFormat = static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pItem)->GetValue());
                    bool bRet=true;
                    {
                        WaitObject aWait( GetViewData()->GetDialogParent() );
                        bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != nullptr );
                        if ( bDraw && nFormat == SotClipboardFormatId::EMBED_SOURCE )
                            pTabViewShell->PasteDraw();
                        else
                            bRet = pTabViewShell->PasteFromSystem(nFormat, true);       // TRUE: no error messages
                    }

                    if ( bRet )
                    {
                        rReq.SetReturnValue(SfxInt16Item(nSlot, bRet ? 1 : 0)); // 1 = success, 0 = fail
                        rReq.Done();
                    }
                    else
                        // if format is not available -> fallback to request without parameters
                        pItem = nullptr;
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
                        bool bDraw = ( ScDrawTransferObj::GetOwnClipboard( pWin ) != nullptr );

                        SvxClipboardFormatItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                        GetPossibleClipboardFormats( aFormats );

                        sal_uInt16 nFormatCount = aFormats.Count();
                        if ( nFormatCount )
                        {
                            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                            std::unique_ptr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( pTabViewShell->GetDialogParent() ));
                            if ( pDlg )
                            {
                            for (sal_uInt16 i=0; i<nFormatCount; i++)
                            {
                                SotClipboardFormatId nFormatId = aFormats.GetClipbrdFormatId( i );
                                OUString aName = aFormats.GetClipbrdFormatName( i );
                                // special case for paste dialog: '*' is replaced by object type
                                if ( nFormatId == SotClipboardFormatId::EMBED_SOURCE )
                                    aName = "*";
                                pDlg->Insert( nFormatId, aName );
                            }

                            TransferableDataHelper aDataHelper(
                                TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
                            SotClipboardFormatId nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );
                            if (nFormat != SotClipboardFormatId::NONE)
                            {
                                {
                                    WaitObject aWait( GetViewData()->GetDialogParent() );
                                    if ( bDraw && nFormat == SotClipboardFormatId::EMBED_SOURCE )
                                        pTabViewShell->PasteDraw();
                                    else
                                        pTabViewShell->PasteFromSystem(nFormat);
                                }
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 1));    // 1 = success
                                rReq.AppendItem( SfxUInt32Item( nSlot, static_cast<sal_uInt32>(nFormat) ) );
                                rReq.Done();
                            }
                            else
                            {
                                rReq.SetReturnValue(SfxInt16Item(nSlot, 0));    // 0 = fail
                                rReq.Ignore();
                            }
                            }
                        }
                        else
                            rReq.SetReturnValue(SfxInt16Item(nSlot, 0));        // 0 = fail
                    }
                }
            }
            pTabViewShell->CellContentChanged();        // => PasteFromSystem() ???
            break;

        //  other

        case FID_INS_ROWBRK:
            pTabViewShell->InsertPageBreak( false );
            rReq.Done();
            break;

        case FID_INS_COLBRK:
            pTabViewShell->InsertPageBreak( true );
            rReq.Done();
            break;

        case FID_DEL_ROWBRK:
            pTabViewShell->DeletePageBreak( false );
            rReq.Done();
            break;

        case FID_DEL_COLBRK:
            pTabViewShell->DeletePageBreak( true );
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
                css::util::NumberFormat::DATE, ScGlobal::GetRscString(STR_UNDO_INSERT_CURRENT_DATE));
            break;
        case SID_INSERT_CURRENT_TIME:
            pTabViewShell->InsertCurrentTime(
                css::util::NumberFormat::TIME, ScGlobal::GetRscString(STR_UNDO_INSERT_CURRENT_TIME));
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
                                    "com.sun.star.linguistic2.ChineseTranslationDialog"
                                    , xContext),
                                UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(nullptr);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = "ParentWindow";
                            aParam.Value <<= makeAny(xDialogParentWindow);
                            pArray[0] <<= makeAny(aParam);
                            xInit->initialize( aSeq );

                            //execute dialog
                            sal_Int16 nDialogRet = xDialog->execute();
                            if( RET_OK == nDialogRet )
                            {
                                //get some parameters from the dialog
                                bool bToSimplified = true;
                                bool bUseVariants = true;
                                bool bCommonTerms = true;
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

                                vcl::Font aTargetFont = OutputDevice::GetDefaultFont(
                                                    DefaultFontType::CJK_SPREADSHEET,
                                                    eTargetLang, GetDefaultFontFlags::OnlyOne );
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

        case SID_CONVERT_FORMULA_TO_VALUE:
        {
            pTabViewShell->ConvertFormulaToValue();
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
            pTabViewShell->ChangeIndent( true );
            break;

        case FID_USE_NAME:
            {
                sal_uInt16 nFlags = pTabViewShell->GetCreateNameFlags();

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                std::unique_ptr<AbstractScNameCreateDlg> pDlg(pFact->CreateScNameCreateDlg(pTabViewShell->GetDialogParent(), nFlags));
                OSL_ENSURE(pDlg, "Dialog create fail!");

                if( pDlg->Execute() )
                {
                    nFlags = pDlg->GetFlags();
                    pTabViewShell->CreateNames(nFlags);
                    rReq.Done();
                }
            }
            break;

        case SID_CONSOLIDATE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET ==
                        pReqArgs->GetItemState( SCITEM_CONSOLIDATEDATA, true, &pItem ) )
                {
                    const ScConsolidateParam& rParam =
                            static_cast<const ScConsolidateItem*>(pItem)->GetData();

                    pTabViewShell->Consolidate( rParam );
                    GetViewData()->GetDocument()->SetConsolidateDlgData( &rParam );

                    rReq.Done();
                }
#if HAVE_FEATURE_SCRIPTING
                else if (rReq.IsAPI())
                    SbxBase::SetError(ERRCODE_SBX_BAD_PARAMETER);
#endif
            }
            break;

        case SID_INS_FUNCTION:
            {
                const SfxBoolItem* pOkItem = static_cast<const SfxBoolItem*>(&pReqArgs->Get( SID_DLG_RETOK ));

                if ( pOkItem->GetValue() )      // OK
                {
                    OUString             aFormula;
                    const SfxStringItem* pSItem      = static_cast<const SfxStringItem*>(&pReqArgs->Get( SCITEM_STRING ));
                    const SfxBoolItem*   pMatrixItem = static_cast<const SfxBoolItem*>(&pReqArgs->Get( SID_DLG_MATRIX ));

                    aFormula += pSItem->GetValue();
                    pScMod->ActivateInputWindow( &aFormula, pMatrixItem->GetValue() );
                }
                else                            // CANCEL
                {
                    pScMod->ActivateInputWindow();
                }
                rReq.Ignore();      // only SID_ENTER_STRING is recorded
            }
            break;

        case FID_DEFINE_NAME:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                OUString  aName, aSymbol, aAttrib;

                if( pReqArgs->HasItem( FID_DEFINE_NAME, &pItem ) )
                    aName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                    aSymbol = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                    aAttrib = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if ( !aName.isEmpty() && !aSymbol.isEmpty() )
                {
                    if (pTabViewShell->InsertName( aName, aSymbol, aAttrib ))
                        rReq.Done();
#if HAVE_FEATURE_SCRIPTING
                    else
                        SbxBase::SetError( ERRCODE_SBX_BAD_PARAMETER );  // Basic-error
#endif
                }
            }
            else
            {
                sal_uInt16          nId  = ScNameDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;
        case FID_ADD_NAME:
            {
                sal_uInt16          nId  = ScNameDefDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_OPENDLG_CONDFRMT:
        case SID_OPENDLG_COLORSCALE:
        case SID_OPENDLG_DATABAR:
        case SID_OPENDLG_ICONSET:
        case SID_OPENDLG_CONDDATE:
            {
                sal_uInt32  nIndex      = sal_uInt32(-1);
                sal_uInt8   nType       = 0;
                bool        bManaged    = false;

                // Get the pool item stored it by Conditional Format Manager Dialog.
                sal_uInt32 nItems(pTabViewShell->GetPool().GetItemCount2( SCITEM_STRING ));
                for( sal_uInt32 nIter = 0; nIter < nItems; ++nIter )
                {
                    const SfxPoolItem* pItem = pTabViewShell->GetPool().GetItem2( SCITEM_STRING, nIter );
                    if( pItem != nullptr )
                    {
                        if ( ScCondFormatDlg::ParseXmlString(
                                static_cast<const SfxStringItem*>(pItem)->GetValue(),
                                nIndex, nType, bManaged))
                            break;
                    }
                }

                // Check if the Conditional Manager Dialog is editing or adding
                // conditional format item.
                if ( bManaged )
                {
                    sal_uInt16 nId = ScCondFormatDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd == nullptr );
                    break;
                }

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

                const ScConditionalFormat* pCondFormat = nullptr;
                const ScPatternAttr* pPattern = pDoc->GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
                const std::vector<sal_uInt32>& rCondFormats = static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData();
                bool bContainsCondFormat = !rCondFormats.empty();
                bool bCondFormatDlg = false;
                if(bContainsCondFormat)
                {
                    bool bContainsExistingCondFormat = false;
                    ScConditionalFormatList* pList = pDoc->GetCondFormList(aPos.Tab());
                    for (std::vector<sal_uInt32>::const_iterator itr = rCondFormats.begin(), itrEnd = rCondFormats.end();
                                            itr != itrEnd; ++itr)
                    {
                        // check if at least one existing conditional format has the same range
                        pCondFormat = pList->GetFormat(*itr);
                        if(!pCondFormat)
                            continue;

                        bContainsExistingCondFormat = true;
                        const ScRangeList& rCondFormatRange = pCondFormat->GetRange();
                        if(rCondFormatRange == aRangeList)
                        {
                            // found a matching range, edit this conditional format
                            bCondFormatDlg = true;
                            nIndex = pCondFormat->GetKey();
                            break;
                        }
                    }

                    // if not found a conditional format ask whether we should edit one of the existing
                    // or should create a new overlapping conditional format
                    if(!bCondFormatDlg && bContainsExistingCondFormat)
                    {
                        ScopedVclPtrInstance<QueryBox> aBox( pTabViewShell->GetDialogParent(), WinBits( WB_YES_NO | WB_DEF_YES ),
                               ScGlobal::GetRscString(STR_EDIT_EXISTING_COND_FORMATS) );
                        bool bEditExisting = aBox->Execute() == RET_YES;
                        if(bEditExisting)
                        {
                            // differentiate between ranges where one conditional format is defined
                            // and several formats are defined
                            // if we have only one => open the cond format dlg to edit it
                            // otherwise open the manage cond format dlg
                            if(rCondFormats.size() == 1)
                            {
                                pCondFormat = pList->GetFormat(rCondFormats[0]);
                                assert(pCondFormat);
                                bCondFormatDlg = true;
                            }
                            else
                            {
                                // Queue message to open Conditional Format Manager Dialog.
                                GetViewData()->GetDispatcher().Execute( SID_OPENDLG_CONDFRMT_MANAGER, SfxCallMode::ASYNCHRON );
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


                if(bCondFormatDlg || !bContainsCondFormat)
                {
                    // Put the xml string parameter to initialize the
                    // Conditional Format Dialog.
                    pTabViewShell->GetPool().Put( SfxStringItem( SCITEM_STRING,
                                                      ScCondFormatDlg::GenerateXmlString(
                                                        pCondFormat ? pCondFormat->GetKey() : sal_uInt32(-1),
                                                        sal_uInt8(eType),
                                                        false
                                                      ) ) );

                    sal_uInt16      nId      = ScCondFormatDlgWrapper::GetChildWindowId();
                    SfxViewFrame*   pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd     = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd == nullptr );
                }
            }
            break;

        case SID_DEFINE_COLROWNAMERANGES:
            {

                sal_uInt16          nId  = ScColRowNameRangesDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;

        case SID_UPDATECHART:
            {
                bool bAll = false;

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;

                    if( pReqArgs->HasItem( SID_UPDATECHART, &pItem ) )
                        bAll = static_cast<const SfxBoolItem*>(pItem)->GetValue();
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
                        static_cast<const ScTabOpItem&>(
                            pReqArgs->Get( SID_TABOP ));

                pTabViewShell->TabOp( rItem.GetData() );

                rReq.Done( *pReqArgs );
            }
            break;

        case SID_SOLVE:
            if (pReqArgs)
            {
                const ScSolveItem& rItem =
                        static_cast<const ScSolveItem&>(
                            pReqArgs->Get( SCITEM_SOLVEDATA ));

                pTabViewShell->Solve( rItem.GetData() );

                rReq.Done( *pReqArgs );
            }
            break;

        case FID_INSERT_NAME:
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                std::unique_ptr<AbstractScNamePasteDlg> pDlg(pFact->CreateScNamePasteDlg( pTabViewShell->GetDialogParent(), GetViewData()->GetDocShell() ));
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
                                (void)pScMod->InputKeyEvent( KeyEvent('=', vcl::KeyCode()) );

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
            }
            break;

        case SID_RANGE_NOTETEXT:
            if (pReqArgs)
            {
                const SfxStringItem& rTextItem = static_cast<const SfxStringItem&>(pReqArgs->Get( SID_RANGE_NOTETEXT ));

                //  always cursor position
                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                pTabViewShell->SetNoteText( aPos, rTextItem.GetValue() );
                rReq.Done();
            }
            break;

        case SID_INSERT_POSTIT:
        case SID_EDIT_POSTIT:
            {
                const SfxPoolItem* pAuthor;
                const SfxPoolItem* pDate;
                const SfxPoolItem* pText;

                if ( pReqArgs && pReqArgs->HasItem( SID_ATTR_POSTIT_AUTHOR, &pAuthor ) &&
                                 pReqArgs->HasItem( SID_ATTR_POSTIT_DATE, &pDate) &&
                                 pReqArgs->HasItem( SID_ATTR_POSTIT_TEXT, &pText) )
                {
                    const SvxPostItAuthorItem*  pAuthorItem = static_cast<const SvxPostItAuthorItem*>( pAuthor );
                    const SvxPostItDateItem*    pDateItem   = static_cast<const SvxPostItDateItem*>( pDate );
                    const SvxPostItTextItem*    pTextItem   = static_cast<const SvxPostItTextItem*>( pText );

                    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                    pTabViewShell->ReplaceNote( aPos, pTextItem->GetValue(), &pAuthorItem->GetValue(), &pDateItem->GetValue() );
                }
                else
                {
                    pTabViewShell->EditNote();                  // note object to edit
                }
                rReq.Done();
            }
            break;

        case FID_NOTE_VISIBLE:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
                if( ScPostIt* pNote = pDoc->GetNote(aPos) )
                {
                    bool bShow;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs && (pReqArgs->GetItemState( FID_NOTE_VISIBLE, true, &pItem ) == SfxItemState::SET) )
                        bShow = static_cast<const SfxBoolItem*>(pItem)->GetValue();
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
                    if( pDoc->GetNote(aPos) )
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

                    OUString aUndo = ScGlobal::GetRscString( bShowNote ? STR_UNDO_SHOWNOTE : STR_UNDO_HIDENOTE );
                    pData->GetDocShell()->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, pData->GetViewShell()->GetViewShellId() );

                    for ( size_t i = 0; i < nRangeSize; ++i )
                    {
                        const ScRange * pRange = aRanges[i];
                        const SCROW nRow0 = pRange->aStart.Row();
                        const SCROW nRow1 = pRange->aEnd.Row();
                        const SCCOL nCol0 = pRange->aStart.Col();
                        const SCCOL nCol1 = pRange->aEnd.Col();
                        const SCTAB nRangeTab = pRange->aStart.Tab();
                        // Check by each cell
                        for ( SCROW nRow = nRow0; nRow <= nRow1; ++nRow )
                        {
                            for ( SCCOL nCol = nCol0; nCol <= nCol1; ++nCol )
                            {
                                if ( pDoc->HasNote(nCol, nRow, nRangeTab) && pDoc->IsBlockEditable( nRangeTab, nCol,nRow, nCol,nRow ) )
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
            pTabViewShell->DeleteContents( InsertDeleteFlags::NOTE );      // delete all notes in selection
            rReq.Done();
            break;

        case SID_CHARMAP:
            if( pReqArgs != nullptr )
            {
                OUString aChars, aFontName;
                const SfxItemSet *pArgs = rReq.GetArgs();
                const SfxPoolItem* pItem = nullptr;
                if ( pArgs )
                    pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), false, &pItem);
                if ( pItem )
                {
                    const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( pItem  );
                    if ( pStringItem )
                        aChars = pStringItem->GetValue();
                    const SfxPoolItem* pFtItem = nullptr;
                    pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
                    const SfxStringItem* pFontItem = dynamic_cast<const SfxStringItem*>( pFtItem  );
                    if ( pFontItem )
                        aFontName = pFontItem->GetValue();
                }

                if ( !aChars.isEmpty() )
                {
                    vcl::Font aFont;
                    pTabViewShell->GetSelectionPattern()->GetFont( aFont, SC_AUTOCOL_BLACK, nullptr, nullptr, nullptr,
                                                                pTabViewShell->GetSelectionScriptType() );
                    if ( !aFontName.isEmpty() )
                        aFont = vcl::Font( aFontName, Size(1,1) );
                    pTabViewShell->InsertSpecialChar( aChars, aFont );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

                // font color doesn't matter here
                vcl::Font         aCurFont;
                pTabViewShell->GetSelectionPattern()->GetFont( aCurFont, SC_AUTOCOL_BLACK, nullptr, nullptr, nullptr,
                                                                pTabViewShell->GetSelectionScriptType() );

                SfxAllItemSet aSet( GetPool() );
                aSet.Put( SfxBoolItem( FN_PARAM_1, false ) );
                aSet.Put( SvxFontItem( aCurFont.GetFamily(), aCurFont.GetFamilyName(), aCurFont.GetStyleName(), aCurFont.GetPitch(), aCurFont.GetCharSet(), GetPool().GetWhich(SID_ATTR_CHAR_FONT) ) );

                std::unique_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( pTabViewShell->GetDialogParent(), aSet,
                    pTabViewShell->GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));

                if ( pDlg->Execute() == RET_OK )
                {
                    const SfxStringItem* pItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
                    const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);

                    if ( pItem && pFontItem )
                    {
                        vcl::Font aNewFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), Size(1,1) );
                        aNewFont.SetCharSet( pFontItem->GetCharSet() );
                        aNewFont.SetPitch( pFontItem->GetPitch() );
                        pTabViewShell->InsertSpecialChar( pItem->GetValue(), aNewFont );
                        rReq.AppendItem( *pFontItem );
                        rReq.AppendItem( *pItem );
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_SELECT_SCENARIO:
            {
                // Testing

                if ( pReqArgs )
                {
                    const SfxStringItem* pItem =
                        static_cast<const SfxStringItem*>(&pReqArgs->Get( SID_SELECT_SCENARIO ));

                    if( pItem )
                    {
                        pTabViewShell->UseScenario( pItem->GetValue() );
                        //! why should the return value be valid?!?!
                        rReq.SetReturnValue( SfxStringItem( SID_SELECT_SCENARIO, pItem->GetValue() ) );
                        rReq.Done();
                    }
                    else
                    {
                        OSL_FAIL("nullptr");
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
                    const SvxHyperlinkItem* pHyper = static_cast<const SvxHyperlinkItem*>(pItem);
                    const OUString& rName   = pHyper->GetName();
                    const OUString& rURL    = pHyper->GetURL();
                    const OUString& rTarget = pHyper->GetTargetFrame();
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
                std::unique_ptr<AbstractScCondFormatManagerDlg> pDlg(pFact->CreateScCondFormatMgrDlg(
                    pTabViewShell->GetDialogParent(), pDoc, pList, RID_SCDLG_COND_FORMAT_MANAGER));
                short nRet = pDlg->Execute();
                if(nRet == RET_OK && pDlg->CondFormatsChanged())
                {
                    ScConditionalFormatList* pCondFormatList = pDlg->GetConditionalFormatList();
                    pData->GetDocShell()->GetDocFunc().SetConditionalFormatList(pCondFormatList, aPos.Tab());
                }
                else if(nRet == DLG_RET_ADD)
                {
                    // Put the xml string parameter to initialize the
                    // Conditional Format Dialog. ( add new )
                    pTabViewShell->GetPool().Put( SfxStringItem( SCITEM_STRING,
                                                      ScCondFormatDlg::GenerateXmlString(
                                                        sal_uInt32(-1),
                                                        sal_uInt8(condformat::dialog::ScCondFormatDialogType::CONDITION),
                                                        true
                                                      ) ) );

                    // Queue message to open Conditional Format Dialog
                    GetViewData()->GetDispatcher().Execute( SID_OPENDLG_CONDFRMT, SfxCallMode::ASYNCHRON );
                }
                else if (nRet == DLG_RET_EDIT)
                {
                    ScConditionalFormat* pFormat = pDlg->GetCondFormatSelected();
                    // Put the xml string parameter to initialize the
                    // Conditional Format Dialog. ( edit selected conditional format )
                    pTabViewShell->GetPool().Put( SfxStringItem( SCITEM_STRING,
                                                      ScCondFormatDlg::GenerateXmlString(
                                                        pFormat ? pFormat->GetKey() : sal_uInt32(-1),
                                                        sal_uInt8(condformat::dialog::ScCondFormatDialogType::CONDITION),
                                                        true
                                                      ) ) );

                    // Queue message to open Conditional Format Dialog
                    GetViewData()->GetDispatcher().Execute( SID_OPENDLG_CONDFRMT, SfxCallMode::ASYNCHRON );
                }
            }
            break;

        case SID_EXTERNAL_SOURCE:
            {
                OUString aFile;
                OUString aFilter;
                OUString aOptions;
                OUString aSource;
                sal_uLong nRefresh=0;

                const SfxStringItem* pFile = rReq.GetArg<SfxStringItem>(SID_FILE_NAME);
                const SfxStringItem* pSource = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
                if ( pFile && pSource )
                {
                    aFile = pFile->GetValue();
                    aSource = pSource->GetValue();
                    const SfxStringItem* pFilter = rReq.GetArg<SfxStringItem>(SID_FILTER_NAME);
                    if ( pFilter )
                        aFilter = pFilter->GetValue();
                    const SfxStringItem* pOptions = rReq.GetArg<SfxStringItem>(SID_FILE_FILTEROPTIONS);
                    if ( pOptions )
                        aOptions = pOptions->GetValue();
                    const SfxUInt32Item* pRefresh = rReq.GetArg<SfxUInt32Item>(FN_PARAM_2);
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

        case SID_AUTO_SUM:
            {
                bool bSubTotal = false;
                bool bRangeFinder = false;
                const OUString aFormula = pTabViewShell->DoAutoSum( bRangeFinder, bSubTotal );
                if ( !aFormula.isEmpty() )
                {
                    const sal_Int32 nPar = aFormula.indexOf( '(' );
                    const sal_Int32 nLen = aFormula.getLength();
                    ScInputHandler* pHdl = pScMod->GetInputHdl( pTabViewShell );

                    if ( pHdl && nPar != -1 )
                    {
                        if ( !pScMod->IsEditMode() )
                        {
                            pScMod->SetInputMode( SC_INPUT_TABLE );
                        }

                        EditView *pEditView=pHdl->GetActiveView();
                        if ( pEditView )
                        {
                            ESelection aTextSel = pEditView->GetSelection();
                            aTextSel.nStartPos = 0;
                            aTextSel.nEndPos = EE_TEXTPOS_ALL;
                            pHdl->DataChanging();
                            pEditView->SetSelection(aTextSel);
                            pEditView->InsertText(aFormula);
                            pEditView->SetSelection( bRangeFinder ? ESelection( 0, nPar + ( bSubTotal ? 3 : 1 ), 0, nLen - 1 ) : ESelection( 0, nLen - 1, 0, nLen - 1 ) );
                            pHdl->DataChanged();

                            if ( bRangeFinder )
                            {
                                pHdl->InitRangeFinder( aFormula );
                            }
                        }
                    }
                }
            }
            break;

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
    const OUString& _rFile, const OUString& _rFilter, const OUString& _rOptions,
    const OUString& _rSource, sal_uLong _nRefresh, SfxRequest& _rRequest )
{
    if ( !_rFile.isEmpty() && !_rSource.isEmpty() )         // filter may be empty
    {
        ScRange aLinkRange;
        bool bMove = false;

        ScViewData* pData = GetViewData();
        ScMarkData& rMark = pData->GetMarkData();
        rMark.MarkToSimple();
        if ( rMark.IsMarked() )
        {
            rMark.GetMarkArea( aLinkRange );
            bMove = true;                       // insert/delete cells to fit range
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
        const ScDPDimensionSaveData* pDimData = nullptr;
        if (pSaveData)
            pDimData = pSaveData->GetExistingDimensionData();

        const ScDPCache* pCache = pDesc->CreateCache(pDimData);
        if (!pCache)
            // cache creation failed, probably due to invalid connection.
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

    std::unique_ptr<ScDPObject> pNewDPObject(nullptr);

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

        std::unique_ptr<AbstractScDataPilotSourceTypeDlg> pTypeDlg(
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
                std::unique_ptr<AbstractScDataPilotServiceDlg> pServDlg(
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

                std::unique_ptr<AbstractScDataPilotDatabaseDlg> pDataDlg(
                    pFact->CreateScDataPilotDatabaseDlg(
                        pTabViewShell->GetDialogParent()));

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

                        ScopedVclPtrInstance<QueryBox> aBox( pTabViewShell->GetDialogParent(),
                                        WinBits(WB_YES_NO | WB_DEF_YES),
                                        ScGlobal::GetRscString(STR_DATAPILOT_SUBTOTAL) );
                        if (aBox->Execute() == RET_NO)
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
            ScopedVclPtrInstance< InfoBox > aBox(pTabViewShell->GetDialogParent(), ScGlobal::GetRscString(nSrcErrorId));
            aBox->Execute();
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
        pScMod->SetRefDialog( nId, pWnd == nullptr );
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
    pScMod->SetRefDialog(nId, pWnd == nullptr);
}

void ScCellShell::ExecuteSubtotals(SfxRequest& rReq)
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    if ( pArgs )
    {
        pTabViewShell->DoSubTotals( static_cast<const ScSubTotalItem&>( pArgs->Get( SCITEM_SUBTDATA )).
                        GetSubTotalData() );
        rReq.Done();
        return;
    }

    std::unique_ptr<SfxAbstractTabDialog> pDlg;
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

    pDBData->GetSubTotalParam( aSubTotalParam );
    aSubTotalParam.bRemoveOnly = false;

    aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, GetViewData(), &aSubTotalParam ) );
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    assert(pFact); //"ScAbstractFactory create fail

    pDlg.reset(pFact->CreateScSubTotalDlg(pTabViewShell->GetDialogParent(), &aArgSet));
    assert(pDlg); // "Dialog create fail
    pDlg->SetCurPageId(1);

    short bResult = pDlg->Execute();

    if ( (bResult == RET_OK) || (bResult == SCRET_REMOVE) )
    {
        const SfxItemSet* pOutSet = nullptr;

        if ( bResult == RET_OK )
        {
            pOutSet = pDlg->GetOutputItemSet();
            aSubTotalParam =
                static_cast<const ScSubTotalItem&>(
                    pOutSet->Get( SCITEM_SUBTDATA )).
                        GetSubTotalData();
        }
        else // if (bResult == SCRET_REMOVE)
        {
            pOutSet = &aArgSet;
            aSubTotalParam.bRemoveOnly = true;
            aSubTotalParam.bReplace    = true;
            aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA,
                                         GetViewData(),
                                         &aSubTotalParam ) );
        }

        pTabViewShell->DoSubTotals( aSubTotalParam );
        rReq.Done( *pOutSet );
    }
    else
        GetViewData()->GetDocShell()->CancelAutoDBRange();
}

void ScCellShell::ExecuteFillSingleEdit()
{
    ScAddress aCurPos = GetViewData()->GetCurPos();

    OUString aInit;

    if (aCurPos.Row() > 0)
    {
        // Get the initial text value from the above cell.

        ScDocument* pDoc = GetViewData()->GetDocument();
        ScAddress aPrevPos = aCurPos;
        aPrevPos.IncRow(-1);
        ScRefCellValue aCell(*pDoc, aPrevPos);

        if (aCell.meType == CELLTYPE_FORMULA)
        {
            aInit = "=";
            const ScTokenArray* pCode = aCell.mpFormula->GetCode();
            sc::TokenStringContext aCxt(pDoc, pDoc->GetGrammar());
            aInit += pCode->CreateString(aCxt, aCurPos);
        }
        else
            aInit = aCell.getString(pDoc);
    }

    SC_MOD()->SetInputMode(SC_INPUT_TABLE, &aInit);
}

IMPL_LINK_NOARG_TYPED(ScCellShell, DialogClosed, Dialog&, void)
{
    assert(pImpl->m_pLinkedDlg && "ScCellShell::DialogClosed(): invalid request");
    assert(pImpl->m_pRequest && "ScCellShell::DialogClosed(): invalid request");
    OUString sFile, sFilter, sOptions, sSource;
    sal_uLong nRefresh = 0;

    if ( pImpl->m_pLinkedDlg->GetResult() == RET_OK )
    {
        sFile = pImpl->m_pLinkedDlg->GetURL();
        sFilter = pImpl->m_pLinkedDlg->GetFilter();
        sOptions = pImpl->m_pLinkedDlg->GetOptions();
        sSource = pImpl->m_pLinkedDlg->GetSource();
        nRefresh = pImpl->m_pLinkedDlg->GetRefresh();
        if ( !sFile.isEmpty() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILE_NAME, sFile ) );
        if ( !sFilter.isEmpty() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILTER_NAME, sFilter ) );
        if ( !sOptions.isEmpty() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, sOptions ) );
        if ( !sSource.isEmpty() )
            pImpl->m_pRequest->AppendItem( SfxStringItem( FN_PARAM_1, sSource ) );
        if ( nRefresh )
            pImpl->m_pRequest->AppendItem( SfxUInt32Item( FN_PARAM_2, nRefresh ) );
    }

    ExecuteExternalSource( sFile, sFilter, sOptions, sSource, nRefresh, *(pImpl->m_pRequest) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
