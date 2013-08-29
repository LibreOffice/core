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

#include "scitems.hxx"
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/zformat.hxx>
#include <editeng/boxitem.hxx>
#include <svx/numinf.hxx>
#include <svl/srchitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "document.hxx"
#include "formulacell.hxx"             // Input Status Edit-Zellen
#include "globstr.hrc"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "appoptio.hxx"
#include "sc.hrc"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "dwfunctr.hxx"
#include "scabstdlg.hxx"
#include "compiler.hxx"
#include "markdata.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"

sal_Bool ScTabViewShell::GetFunction( String& rFuncStr, sal_uInt16 nErrCode )
{
    String aStr;

    ScSubTotalFunc eFunc = (ScSubTotalFunc) SC_MOD()->GetAppOptions().GetStatusFunc();
    ScViewData* pViewData   = GetViewData();
    ScMarkData& rMark       = pViewData->GetMarkData();
    bool bIgnoreError = (rMark.IsMarked() || rMark.IsMultiMarked());

    if (bIgnoreError && (eFunc == SUBTOTAL_FUNC_CNT || eFunc == SUBTOTAL_FUNC_CNT2))
        nErrCode = 0;

    if (nErrCode)
    {
        rFuncStr = ScGlobal::GetLongErrorString(nErrCode);
        return true;
    }

    sal_uInt16 nGlobStrId = 0;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_AVE:  nGlobStrId = STR_FUN_TEXT_AVG; break;
        case SUBTOTAL_FUNC_CNT:  nGlobStrId = STR_FUN_TEXT_COUNT; break;
        case SUBTOTAL_FUNC_CNT2: nGlobStrId = STR_FUN_TEXT_COUNT2; break;
        case SUBTOTAL_FUNC_MAX:  nGlobStrId = STR_FUN_TEXT_MAX; break;
        case SUBTOTAL_FUNC_MIN:  nGlobStrId = STR_FUN_TEXT_MIN; break;
        case SUBTOTAL_FUNC_SUM:  nGlobStrId = STR_FUN_TEXT_SUM; break;
        case SUBTOTAL_FUNC_SELECTION_COUNT: nGlobStrId = STR_FUN_TEXT_SELECTION_COUNT; break;

        default:
        {
            // added to avoid warnings
        }
    }
    if (nGlobStrId)
    {
        ScDocument* pDoc        = pViewData->GetDocument();
        SCCOL       nPosX       = pViewData->GetCurX();
        SCROW       nPosY       = pViewData->GetCurY();
        SCTAB       nTab        = pViewData->GetTabNo();

        aStr = ScGlobal::GetRscString(nGlobStrId);
        aStr += '=';

        ScAddress aCursor( nPosX, nPosY, nTab );
        double nVal;
        if ( pDoc->GetSelectionFunction( eFunc, aCursor, rMark, nVal ) )
        {
            if ( nVal == 0.0 )
                aStr += '0';
            else
            {
                // Number in the standard format, the other on the cursor position
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                sal_uInt32 nNumFmt = 0;
                if ( eFunc != SUBTOTAL_FUNC_CNT && eFunc != SUBTOTAL_FUNC_CNT2 && eFunc != SUBTOTAL_FUNC_SELECTION_COUNT)
                {
                    //  Zahlformat aus Attributen oder Formel
                    pDoc->GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
                }

                OUString aValStr;
                Color* pDummy;
                pFormatter->GetOutputString( nVal, nNumFmt, aValStr, &pDummy );
                aStr += aValStr;
            }
        }

        rFuncStr = aStr;
        return sal_True;
    }

    return false;
}



//  Functions that are disabled, depending on the selection
//  Default:
//      SID_DELETE,
//      SID_DELETE_CONTENTS,
//      FID_DELETE_CELL
//      FID_VALIDATION


void ScTabViewShell::GetState( SfxItemSet& rSet )
{
    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScDocShell* pDocShell   = pViewData->GetDocShell();
    ScMarkData& rMark       = pViewData->GetMarkData();
    SCCOL       nPosX       = pViewData->GetCurX();
    SCROW       nPosY       = pViewData->GetCurY();
    SCTAB       nTab        = pViewData->GetTabNo();
    sal_uInt16      nMyId       = 0;

    SfxViewFrame* pThisFrame = GetViewFrame();
    sal_Bool bOle = GetViewFrame()->GetFrame().IsInPlace();

    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case FID_CHG_COMMENT:
                {
                    ScDocShell* pDocSh = GetViewData()->GetDocShell();
                    ScAddress aPos( nPosX, nPosY, nTab );
                    if ( pDocSh->IsReadOnly() || !pDocSh->GetChangeAction(aPos) || pDocSh->IsDocShared() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OPENDLG_EDIT_PRINTAREA:
            case SID_ADD_PRINTAREA:
            case SID_DEFINE_PRINTAREA:
                {
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_DELETE_PRINTAREA:
                if ( nTabSelCount > 1 )
                {
                    // #i22589# also take "Print Entire Sheet" into account here
                    sal_Bool bHas = false;
                    for (SCTAB i=0; !bHas && i<nTabCount; i++)
                        bHas = rMark.GetTableSelect(i) && (pDoc->GetPrintRangeCount(i) || pDoc->IsPrintEntireSheet(i));
                    if (!bHas)
                        rSet.DisableItem( nWhich );
                }
                else if ( !pDoc->GetPrintRangeCount( nTab ) && !pDoc->IsPrintEntireSheet( nTab ) )
                    rSet.DisableItem( nWhich );
                if ( pDocShell && pDocShell->IsDocShared() )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_STATUS_PAGESTYLE:
            case SID_HFEDIT:
                GetViewData()->GetDocShell()->GetStatePageStyle( *this, rSet, nTab );
                break;

            case SID_SEARCH_ITEM:
            {
                SvxSearchItem aItem(ScGlobal::GetSearchItem()); // make a copy.
                // Search on current selection if a range is marked.
                aItem.SetSelection(rMark.IsMarked());
                rSet.Put(aItem);
                break;
            }

            case SID_SEARCH_OPTIONS:
                {
                    // Anything goes
                    sal_uInt16 nOptions = 0xffff;

                    // No replacement if ReadOnly
                    if (GetViewData()->GetDocShell()->IsReadOnly())
                        nOptions &= ~( SEARCH_OPTIONS_REPLACE | SEARCH_OPTIONS_REPLACE_ALL );
                    rSet.Put( SfxUInt16Item( nWhich, nOptions ) );
                }
                break;

            case SID_CURRENTCELL:
                {
                    ScAddress aScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), 0 );
                    OUString  aAddr(aScAddress.Format(SCA_ABS, NULL, pDoc->GetAddressConvention()));
                    SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );

                    rSet.Put( aPosItem );
                }
                break;

            case SID_CURRENTTAB:
                // Table for Basic is 1-based
                rSet.Put( SfxUInt16Item( nWhich, static_cast<sal_uInt16>(GetViewData()->GetTabNo()) + 1 ) );
                break;

            case SID_CURRENTDOC:
                rSet.Put( SfxStringItem( nWhich, GetViewData()->GetDocShell()->GetTitle() ) );
                break;

            case FID_TOGGLEINPUTLINE:
                {
                    sal_uInt16 nId = ScInputWindowWrapper::GetChildWindowId();

                    if ( pThisFrame->KnowsChildWindow( nId ) )
                    {
                        SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                        rSet.Put( SfxBoolItem( nWhich, pWnd ? sal_True : false ) );
                    }
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_DEL_MANUALBREAKS:
                if (!pDoc->HasManualBreaks(nTab))
                    rSet.DisableItem( nWhich );
                break;

            case FID_RESET_PRINTZOOM:
                {
                    // disable if already set to default

                    String aStyleName = pDoc->GetPageStyle( nTab );
                    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName,
                                                    SFX_STYLE_FAMILY_PAGE );
                    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                        sal_uInt16 nScale = ((const SfxUInt16Item&)
                                            rStyleSet.Get(ATTR_PAGE_SCALE)).GetValue();
                        sal_uInt16 nPages = ((const SfxUInt16Item&)
                                            rStyleSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
                        if ( nScale == 100 && nPages == 0 )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_SCALE:
            case SID_ATTR_ZOOM:
                if ( bOle )
                    rSet.DisableItem( nWhich );
                else
                {
                    const Fraction& rOldY = GetViewData()->GetZoomY();
                    sal_uInt16 nZoom = (sal_uInt16)(( rOldY.GetNumerator() * 100 )
                                                / rOldY.GetDenominator());
                    rSet.Put( SvxZoomItem( SVX_ZOOM_PERCENT, nZoom, nWhich ) );
                }
                break;

            case SID_ATTR_ZOOMSLIDER:
                {
                    if ( bOle )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        const Fraction& rOldY = GetViewData()->GetZoomY();
                        sal_uInt16 nCurrentZoom = (sal_uInt16)(( rOldY.GetNumerator() * 100 ) / rOldY.GetDenominator());

                        if( nCurrentZoom )
                        {
                            SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM, SID_ATTR_ZOOMSLIDER );
                            aZoomSliderItem.AddSnappingPoint( 100 );
                            rSet.Put( aZoomSliderItem );
                        }
                    }
                }
                break;

            case FID_TOGGLESYNTAX:
                rSet.Put(SfxBoolItem(nWhich, GetViewData()->IsSyntaxMode()));
                break;

            case FID_TOGGLEHEADERS:
                rSet.Put(SfxBoolItem(nWhich, GetViewData()->IsHeaderMode()));
                break;

            case FID_TOGGLEFORMULA:
                {
                    const ScViewOptions& rOpts = pViewData->GetOptions();
                    sal_Bool bFormulaMode = rOpts.GetOption( VOPT_FORMULAS );
                    rSet.Put(SfxBoolItem(nWhich, bFormulaMode ));
                }
                break;

            case FID_NORMALVIEWMODE:
            case FID_PAGEBREAKMODE:
                // always handle both slots - they exclude each other
                if ( bOle )
                {
                    rSet.DisableItem( FID_NORMALVIEWMODE );
                    rSet.DisableItem( FID_PAGEBREAKMODE );
                }
                else
                {
                    rSet.Put(SfxBoolItem(FID_NORMALVIEWMODE, !GetViewData()->IsPagebreakMode()));
                    rSet.Put(SfxBoolItem(FID_PAGEBREAKMODE, GetViewData()->IsPagebreakMode()));
                }
                break;

            case FID_FUNCTION_BOX:
                nMyId = ScFunctionChildWindow::GetChildWindowId();
                rSet.Put(SfxBoolItem(FID_FUNCTION_BOX, pThisFrame->HasChildWindow(nMyId)));
                break;

            case FID_PROTECT_DOC:
                {
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                    else
                    {
                        rSet.Put( SfxBoolItem( nWhich, pDoc->IsDocProtected() ) );
                    }
                }
                break;

            case FID_PROTECT_TABLE:
                {
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                    else
                    {
                        rSet.Put( SfxBoolItem( nWhich, pDoc->IsTabProtected( nTab ) ) );
                    }
                }
                break;

            case SID_AUTO_OUTLINE:
                {
                    if (pDoc->GetChangeTrack()!=NULL || GetViewData()->IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_OUTLINE_DELETEALL:
                {
                    SCTAB nOlTab = GetViewData()->GetTabNo();
                    ScOutlineTable* pOlTable = pDoc->GetOutlineTable( nOlTab );
                    if (pOlTable == NULL)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_WINDOW_SPLIT:
                rSet.Put(SfxBoolItem(nWhich,
                            pViewData->GetHSplitMode() == SC_SPLIT_NORMAL ||
                            pViewData->GetVSplitMode() == SC_SPLIT_NORMAL ));
                break;

            case SID_WINDOW_FIX:
                rSet.Put(SfxBoolItem(nWhich,
                            pViewData->GetHSplitMode() == SC_SPLIT_FIX ||
                            pViewData->GetVSplitMode() == SC_SPLIT_FIX ));
                break;

            case FID_CHG_SHOW:
                {
                    if ( pDoc->GetChangeTrack() == NULL || ( pDocShell && pDocShell->IsDocShared() ) )
                        rSet.DisableItem( nWhich );
                }
                break;
            case FID_CHG_ACCEPT:
                {
                    rSet.Put(SfxBoolItem(FID_CHG_ACCEPT,
                            pThisFrame->HasChildWindow(FID_CHG_ACCEPT)));
                    if(pDoc->GetChangeTrack()==NULL)
                    {
                        if ( !pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
                        {
                            rSet.DisableItem( nWhich);
                        }
                    }
                    if ( pDocShell && pDocShell->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_FORMATPAGE:
                // in protected tables
                if ( pDocShell && ( pDocShell->IsReadOnly() || pDocShell->IsDocShared() ) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_PRINTPREVIEW:
                // Toggle slot needs a State
                rSet.Put( SfxBoolItem( nWhich, false ) );
                break;

            case SID_READONLY_MODE:
                rSet.Put( SfxBoolItem( nWhich, GetViewData()->GetDocShell()->IsReadOnly() ) );
                break;

            case FID_TAB_DESELECTALL:
                if ( nTabSelCount == 1 )
                    rSet.DisableItem( nWhich );     // enabled only if several sheets are selected
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

//------------------------------------------------------------------
void ScTabViewShell::ExecuteCellFormatDlg(SfxRequest& rReq, const OString &rName)
{
    SfxAbstractTabDialog * pDlg = NULL;
    ScDocument*             pDoc    = GetViewData()->GetDocument();

    SvxBoxItem              aLineOuter( ATTR_BORDER );
    SvxBoxInfoItem          aLineInner( ATTR_BORDER_INNER );

    SvxNumberInfoItem*      pNumberInfoItem = NULL;
    const ScPatternAttr*    pOldAttrs       = GetSelectionPattern();
    SfxItemSet*             pOldSet         = new SfxItemSet(
                                                    pOldAttrs->GetItemSet() );


    // Get border items and put them in the set:
    GetSelectionFrame( aLineOuter, aLineInner );
    //Fix border incorrect for RTL fdo#62399
    if( pDoc->IsLayoutRTL( GetViewData()->GetTabNo() ) )
    {
        SvxBoxItem     aNewFrame( aLineOuter );
        SvxBoxInfoItem aTempInfo( aLineInner );

        if ( aLineInner.IsValid(VALID_LEFT) )
            aNewFrame.SetLine( aLineOuter.GetLeft(), BOX_LINE_RIGHT );
        if ( aLineInner.IsValid(VALID_RIGHT) )
            aNewFrame.SetLine( aLineOuter.GetRight(), BOX_LINE_LEFT );

        aLineInner.SetValid( VALID_LEFT, aTempInfo.IsValid(VALID_RIGHT));
        aLineInner.SetValid( VALID_RIGHT, aTempInfo.IsValid(VALID_LEFT));

        pOldSet->Put( aNewFrame );
    }
    else
        pOldSet->Put( aLineOuter );

    pOldSet->Put( aLineInner );

    // Generate NumberFormat Value from Value and Language and box it.
    pOldSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
        pOldAttrs->GetNumberFormat( pDoc->GetFormatTable() ) ) );

    MakeNumberInfoItem( pDoc, GetViewData(), &pNumberInfoItem );

    pOldSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
    pOldSet->Put(*pNumberInfoItem );

    bInFormatDialog = true;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    pDlg = pFact->CreateScAttrDlg(GetViewFrame(), GetDialogParent(), pOldSet);
    OSL_ENSURE(pDlg, "Dialog create fail!");
    if (!rName.isEmpty())
        pDlg->SetCurPageId(rName);
    short nResult = pDlg->Execute();
    bInFormatDialog = false;

    if ( nResult == RET_OK )
    {
        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

        const SfxPoolItem* pItem=NULL;
        if(pOutSet->GetItemState(SID_ATTR_NUMBERFORMAT_INFO,sal_True,&pItem)==SFX_ITEM_SET)
        {

            UpdateNumberFormatter((const SvxNumberInfoItem&)*pItem);
        }

        ApplyAttributes( pOutSet, pOldSet );

        rReq.Done( *pOutSet );
    }
    delete pOldSet;
    delete pNumberInfoItem;
    delete pDlg;
}

//------------------------------------------------------------------

bool ScTabViewShell::IsRefInputMode() const
{
    ScModule* pScMod = SC_MOD();
    if ( pScMod )
    {
        if( pScMod->IsRefDialogOpen() )
            return pScMod->IsFormulaMode();
        if( pScMod->IsFormulaMode() )
        {
            ScInputHandler* pHdl = pScMod->GetInputHdl();
            if ( pHdl )
            {
                String aString = pHdl->GetEditString();
                if ( !pHdl->GetSelIsRef() && aString.Len() > 1 &&
                     ( aString.GetChar(0) == '+' || aString.GetChar(0) == '-' ) )
                {
                    const ScViewData* pViewData = GetViewData();
                    if ( pViewData )
                    {
                        ScDocument* pDoc = pViewData->GetDocument();
                        if ( pDoc )
                        {
                            const ScAddress aPos( pViewData->GetCurPos() );
                            ScCompiler aComp( pDoc, aPos );
                            aComp.SetGrammar(pDoc->GetGrammar());
                            aComp.SetCloseBrackets( false );
                            ScTokenArray* pArr = aComp.CompileString( aString );
                            if ( pArr && pArr->MayReferenceFollow() )
                            {
                                return true;
                            }
                        }
                    }
                }
                else
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//------------------------------------------------------------------

void ScTabViewShell::ExecuteInputDirect()
{
    if ( !IsRefInputMode() )
    {
        ScModule* pScMod = SC_MOD();
        if ( pScMod )
        {
            pScMod->InputEnterHandler();
        }
    }
}

//------------------------------------------------------------------

void ScTabViewShell::UpdateInputHandler( sal_Bool bForce /* = sal_False */, sal_Bool bStopEditing /* = sal_True */ )
{
    ScInputHandler* pHdl = pInputHandler ? pInputHandler : SC_MOD()->GetInputHdl();

    if ( pHdl )
    {
        String                  aString;
        const EditTextObject*   pObject     = NULL;
        ScViewData*             pViewData   = GetViewData();
        ScDocument*             pDoc        = pViewData->GetDocument();
        CellType                eType;
        SCCOL                   nPosX       = pViewData->GetCurX();
        SCROW                   nPosY       = pViewData->GetCurY();
        SCTAB                   nTab        = pViewData->GetTabNo();
        SCTAB                   nStartTab   = 0;
        SCTAB                   nEndTab     = 0;
        SCCOL                   nStartCol   = 0;
        SCROW                   nStartRow   = 0;
        SCCOL                   nEndCol     = 0;
        SCROW                   nEndRow     = 0;
        ScAddress aPos = pViewData->GetCurPos();

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,   nEndTab );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        bool bHideFormula = false;
        bool bHideAll     = false;

        if (pDoc->IsTabProtected(nTab))
        {
            const ScProtectionAttr* pProt = (const ScProtectionAttr*)
                                            pDoc->GetAttr( nPosX,nPosY,nTab,
                                                           ATTR_PROTECTION);
            bHideFormula = pProt->GetHideFormula();
            bHideAll     = pProt->GetHideCell();
        }

        if (!bHideAll)
        {
            eType = pDoc->GetCellType(aPos);
            if (eType == CELLTYPE_FORMULA)
            {
                if (!bHideFormula)
                    pDoc->GetFormula( nPosX, nPosY, nTab, aString );
            }
            else if (eType == CELLTYPE_EDIT)
            {
                pObject = pDoc->GetEditText(aPos);
            }
            else
            {
                pDoc->GetInputString( nPosX, nPosY, nTab, aString );
                if (eType == CELLTYPE_STRING)
                {
                    //  Bei Bedarf ein ' vorneweg, damit der String nicht ungewollt
                    //  als Zahl interpretiert wird, und um dem Benutzer zu zeigen,
                    //  dass es ein String ist (#35060#).
                    //! Auch bei Zahlformat "Text"? -> dann beim Editieren wegnehmen

                    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                    sal_uInt32 nNumFmt;
                    pDoc->GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
                    double fDummy;
                    if ( pFormatter->IsNumberFormat(aString, nNumFmt, fDummy) )
                        aString.Insert('\'',0);
                }
            }
        }

        ScInputHdlState aState( ScAddress( nPosX,     nPosY,     nTab ),
                                ScAddress( nStartCol, nStartRow, nTab ),
                                ScAddress( nEndCol,   nEndRow,   nTab ),
                                aString,
                                pObject );

        //  if using the view's local input handler, this view can always be set
        //  as current view inside NotifyChange.
        ScTabViewShell* pSourceSh = pInputHandler ? this : NULL;

        pHdl->NotifyChange( &aState, bForce, pSourceSh, bStopEditing );
    }

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_STATUS_SUM );         // immer zusammen mit Eingabezeile
    rBindings.Invalidate( SID_ATTR_SIZE );
    rBindings.Invalidate( SID_TABLE_CELL );
}

void ScTabViewShell::UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust )
{
    if( ScInputHandler* pHdl = pInputHandler ? pInputHandler : SC_MOD()->GetInputHdl() )
        pHdl->UpdateCellAdjust( eJust );
}

//------------------------------------------------------------------

void ScTabViewShell::ExecuteSave( SfxRequest& rReq )
{
    // only SID_SAVEDOC / SID_SAVEASDOC

    // Finish entering in any case, even if a formula is being processed
    SC_MOD()->InputEnterHandler();

    if ( GetViewData()->GetDocShell()->IsDocShared() )
    {
        GetViewData()->GetDocShell()->SetDocumentModified();
    }

    // otherwise as normal
    GetViewData()->GetDocShell()->ExecuteSlot( rReq );
}

void ScTabViewShell::GetSaveState( SfxItemSet& rSet )
{
    SfxShell* pDocSh = GetViewData()->GetDocShell();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if ( nWhich != SID_SAVEDOC || !GetViewData()->GetDocShell()->IsDocShared() )
        {
            // get state from DocShell
            pDocSh->GetSlotState( nWhich, NULL, &rSet );
        }
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::ExecDrawOpt( SfxRequest& rReq )
{
    ScViewOptions aViewOptions = GetViewData()->GetOptions();
    ScGridOptions aGridOptions = aViewOptions.GetGridOptions();

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    sal_uInt16 nSlotId = rReq.GetSlot();
    switch (nSlotId)
    {
        case SID_GRID_VISIBLE:
            if ( pArgs && pArgs->GetItemState(nSlotId,sal_True,&pItem) == SFX_ITEM_SET )
            {
                aGridOptions.SetGridVisible( ((const SfxBoolItem*)pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_VISIBLE);
            }
            break;

        case SID_GRID_USE:
            if ( pArgs && pArgs->GetItemState(nSlotId,sal_True,&pItem) == SFX_ITEM_SET )
            {
                aGridOptions.SetUseGridSnap( ((const SfxBoolItem*)pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_USE);
            }
            break;

        case SID_HELPLINES_MOVE:
            if ( pArgs && pArgs->GetItemState(nSlotId,sal_True,&pItem) == SFX_ITEM_SET )
            {
                aViewOptions.SetOption( VOPT_HELPLINES, ((const SfxBoolItem*)pItem)->GetValue() );
                rBindings.Invalidate(SID_HELPLINES_MOVE);
            }
            break;
    }

    GetViewData()->SetOptions(aViewOptions);
}

void ScTabViewShell::GetDrawOptState( SfxItemSet& rSet )
{
    SfxBoolItem aBool;

    const ScViewOptions& rViewOptions = GetViewData()->GetOptions();
    const ScGridOptions& rGridOptions = rViewOptions.GetGridOptions();

    aBool.SetValue(rGridOptions.GetGridVisible());
    aBool.SetWhich( SID_GRID_VISIBLE );
    rSet.Put( aBool );

    aBool.SetValue(rGridOptions.GetUseGridSnap());
    aBool.SetWhich( SID_GRID_USE );
    rSet.Put( aBool );

    aBool.SetValue(rViewOptions.GetOption( VOPT_HELPLINES ));
    aBool.SetWhich( SID_HELPLINES_MOVE );
    rSet.Put( aBool );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
