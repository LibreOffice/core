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

#include <scitems.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/zformat.hxx>
#include <editeng/boxitem.hxx>
#include <svx/numinf.hxx>
#include <svl/srchitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/int64item.hxx>
#include <svx/zoomslideritem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <global.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <cellform.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <appoptio.hxx>
#include <sc.hrc>
#include <stlpool.hxx>
#include <tabvwsh.hxx>
#include <dwfunctr.hxx>
#include <scabstdlg.hxx>
#include <compiler.hxx>
#include <markdata.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <memory>

#include <svx/unobrushitemhelper.hxx>

using namespace com::sun::star;

bool ScTabViewShell::GetFunction( OUString& rFuncStr, FormulaError nErrCode )
{
    OUString aStr;

    sal_uInt32 nFuncs = SC_MOD()->GetAppOptions().GetStatusFunc();
    ScViewData& rViewData   = GetViewData();
    ScMarkData& rMark       = rViewData.GetMarkData();
    bool bIgnoreError = (rMark.IsMarked() || rMark.IsMultiMarked());
    bool bFirst = true;
    for ( sal_uInt16 nFunc = 0; nFunc < 32; nFunc++ )
    {
        if ( !(nFuncs & (1 << nFunc)) )
            continue;
        ScSubTotalFunc eFunc = static_cast<ScSubTotalFunc>(nFunc);

        if (bIgnoreError && (eFunc == SUBTOTAL_FUNC_CNT || eFunc == SUBTOTAL_FUNC_CNT2))
            nErrCode = FormulaError::NONE;

        if (nErrCode != FormulaError::NONE)
        {
            rFuncStr = ScGlobal::GetLongErrorString(nErrCode);
            return true;
        }

        const char* pGlobStrId = nullptr;
        switch (eFunc)
        {
            case SUBTOTAL_FUNC_AVE:  pGlobStrId = STR_FUN_TEXT_AVG; break;
            case SUBTOTAL_FUNC_CNT:  pGlobStrId = STR_FUN_TEXT_COUNT; break;
            case SUBTOTAL_FUNC_CNT2: pGlobStrId = STR_FUN_TEXT_COUNT2; break;
            case SUBTOTAL_FUNC_MAX:  pGlobStrId = STR_FUN_TEXT_MAX; break;
            case SUBTOTAL_FUNC_MIN:  pGlobStrId = STR_FUN_TEXT_MIN; break;
            case SUBTOTAL_FUNC_SUM:  pGlobStrId = STR_FUN_TEXT_SUM; break;
            case SUBTOTAL_FUNC_SELECTION_COUNT: pGlobStrId = STR_FUN_TEXT_SELECTION_COUNT; break;

            default:
            {
                // added to avoid warnings
            }
        }
        if (pGlobStrId)
        {
            ScDocument* pDoc        = rViewData.GetDocument();
            SCCOL       nPosX       = rViewData.GetCurX();
            SCROW       nPosY       = rViewData.GetCurY();
            SCTAB       nTab        = rViewData.GetTabNo();

            aStr = ScResId(pGlobStrId);
            aStr += ": ";

            ScAddress aCursor( nPosX, nPosY, nTab );
            double nVal;
            if ( pDoc->GetSelectionFunction( eFunc, aCursor, rMark, nVal ) )
            {
                if ( nVal == 0.0 )
                    aStr += "0";
                else
                {
                    // Number in the standard format, the other on the cursor position
                    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                    sal_uInt32 nNumFmt = 0;
                    if ( eFunc != SUBTOTAL_FUNC_CNT && eFunc != SUBTOTAL_FUNC_CNT2 && eFunc != SUBTOTAL_FUNC_SELECTION_COUNT)
                    {
                        // number format from attributes or formula
                        pDoc->GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
                    }

                    OUString aValStr;
                    Color* pDummy;
                    pFormatter->GetOutputString( nVal, nNumFmt, aValStr, &pDummy );
                    aStr += aValStr;
                }
            }
            if ( bFirst )
            {
                rFuncStr += aStr;
                bFirst = false;
            }
            else
                rFuncStr += ("; " + aStr);
        }
    }

    return !rFuncStr.isEmpty();
}

//  Functions that are disabled, depending on the selection
//  Default:
//      SID_DELETE,
//      SID_DELETE_CONTENTS,
//      FID_DELETE_CELL
//      FID_VALIDATION

void ScTabViewShell::GetState( SfxItemSet& rSet )
{
    ScViewData& rViewData   = GetViewData();
    ScDocument* pDoc        = rViewData.GetDocument();
    ScDocShell* pDocShell   = rViewData.GetDocShell();
    ScMarkData& rMark       = rViewData.GetMarkData();
    SCCOL       nPosX       = rViewData.GetCurX();
    SCROW       nPosY       = rViewData.GetCurY();
    SCTAB       nTab        = rViewData.GetTabNo();

    SfxViewFrame* pThisFrame = GetViewFrame();
    bool bOle = GetViewFrame()->GetFrame().IsInPlace();

    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case FID_CHG_COMMENT:
                {
                    ScDocShell* pDocSh = GetViewData().GetDocShell();
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
                if ( pDocShell && pDocShell->IsDocShared() )
                {
                    rSet.DisableItem( nWhich );
                }
                else if (pDoc->IsPrintEntireSheet(nTab))
                    rSet.DisableItem(nWhich);
                break;

            case SID_STATUS_PAGESTYLE:
            case SID_HFEDIT:
                GetViewData().GetDocShell()->GetStatePageStyle( rSet, nTab );
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
                    SearchOptionFlags nOptions = SearchOptionFlags::ALL;

                    // No replacement if ReadOnly
                    if (GetViewData().GetDocShell()->IsReadOnly())
                        nOptions &= ~SearchOptionFlags( SearchOptionFlags::REPLACE | SearchOptionFlags::REPLACE_ALL );
                    rSet.Put( SfxUInt16Item( nWhich, static_cast<sal_uInt16>(nOptions) ) );
                }
                break;

            case SID_CURRENTCELL:
                {
                    ScAddress aScAddress( GetViewData().GetCurX(), GetViewData().GetCurY(), 0 );
                    OUString  aAddr(aScAddress.Format(ScRefFlags::ADDR_ABS, nullptr, pDoc->GetAddressConvention()));
                    SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );

                    rSet.Put( aPosItem );
                }
                break;

            case SID_CURRENTTAB:
                // Table for Basic is 1-based
                rSet.Put( SfxUInt16Item( nWhich, static_cast<sal_uInt16>(GetViewData().GetTabNo()) + 1 ) );
                break;

            case SID_CURRENTDOC:
                rSet.Put( SfxStringItem( nWhich, GetViewData().GetDocShell()->GetTitle() ) );
                break;

            case FID_TOGGLEINPUTLINE:
                {
                    sal_uInt16 nId = ScInputWindowWrapper::GetChildWindowId();

                    if ( pThisFrame->KnowsChildWindow( nId ) )
                    {
                        SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                        rSet.Put( SfxBoolItem( nWhich, pWnd != nullptr ) );
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

                    OUString aStyleName = pDoc->GetPageStyle( nTab );
                    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName,
                                                    SfxStyleFamily::Page );
                    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                        sal_uInt16 nScale = rStyleSet.Get(ATTR_PAGE_SCALE).GetValue();
                        sal_uInt16 nPages = rStyleSet.Get(ATTR_PAGE_SCALETOPAGES).GetValue();
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
                    const Fraction& rOldY = GetViewData().GetZoomY();
                    sal_uInt16 nZoom = static_cast<sal_uInt16>(long( rOldY * 100 ));
                    rSet.Put( SvxZoomItem( SvxZoomType::PERCENT, nZoom, nWhich ) );
                }
                break;

            case SID_ATTR_ZOOMSLIDER:
                {
                    if ( bOle )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        const Fraction& rOldY = GetViewData().GetZoomY();
                        sal_uInt16 nCurrentZoom = static_cast<sal_uInt16>(long( rOldY * 100 ));

                        if( nCurrentZoom )
                        {
                            SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM, SID_ATTR_ZOOMSLIDER );
                            aZoomSliderItem.AddSnappingPoint( 100 );
                            rSet.Put( aZoomSliderItem );
                        }
                    }
                }
                break;

            case FID_FUNCTION_BOX:
            {
                const bool bBoxOpen = ::sfx2::sidebar::Sidebar::IsPanelVisible("ScFunctionsPanel",
                                                    pThisFrame->GetFrame().GetFrameInterface());
                rSet.Put(SfxBoolItem(nWhich, bBoxOpen));
                break;
            }

            case FID_TOGGLESYNTAX:
                rSet.Put(SfxBoolItem(nWhich, GetViewData().IsSyntaxMode()));
                break;

            case FID_TOGGLEHEADERS:
                rSet.Put(SfxBoolItem(nWhich, GetViewData().IsHeaderMode()));
                break;

            case FID_TOGGLEFORMULA:
                {
                    const ScViewOptions& rOpts = rViewData.GetOptions();
                    bool bFormulaMode = rOpts.GetOption( VOPT_FORMULAS );
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
                    rSet.Put(SfxBoolItem(FID_NORMALVIEWMODE, !GetViewData().IsPagebreakMode()));
                    rSet.Put(SfxBoolItem(FID_PAGEBREAKMODE, GetViewData().IsPagebreakMode()));
                }
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
                    if (pDoc->GetChangeTrack()!=nullptr || GetViewData().IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_OUTLINE_DELETEALL:
                {
                    SCTAB nOlTab = GetViewData().GetTabNo();
                    ScOutlineTable* pOlTable = pDoc->GetOutlineTable( nOlTab );
                    if (pOlTable == nullptr)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_WINDOW_SPLIT:
                rSet.Put(SfxBoolItem(nWhich,
                            rViewData.GetHSplitMode() == SC_SPLIT_NORMAL ||
                            rViewData.GetVSplitMode() == SC_SPLIT_NORMAL ));
                break;

            case SID_WINDOW_FIX:
                rSet.Put(SfxBoolItem(nWhich,
                            rViewData.GetHSplitMode() == SC_SPLIT_FIX ||
                            rViewData.GetVSplitMode() == SC_SPLIT_FIX ));
                break;

            case FID_CHG_SHOW:
                {
                    if ( pDoc->GetChangeTrack() == nullptr || ( pDocShell && pDocShell->IsDocShared() ) )
                        rSet.DisableItem( nWhich );
                }
                break;
            case FID_CHG_ACCEPT:
                {
                    if(
                       ( !pDoc->GetChangeTrack() &&  !pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
                       ||
                       ( pDocShell && pDocShell->IsDocShared() )
                      )
                    {
                        rSet.DisableItem( nWhich);
                    }
                    else
                    {
                        rSet.Put(SfxBoolItem(FID_CHG_ACCEPT,
                            pThisFrame->HasChildWindow(FID_CHG_ACCEPT)));
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
                rSet.Put( SfxBoolItem( nWhich, GetViewData().GetDocShell()->IsReadOnly() ) );
                break;

            case FID_TAB_DESELECTALL:
                if ( nTabSelCount == 1 )
                    rSet.DisableItem( nWhich );     // enabled only if several sheets are selected
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

void ScTabViewShell::ExecuteCellFormatDlg(SfxRequest& rReq, const OString &rName)
{
    ScDocument*             pDoc    = GetViewData().GetDocument();

    SvxBoxItem              aLineOuter( ATTR_BORDER );
    SvxBoxInfoItem          aLineInner( ATTR_BORDER_INNER );

    const ScPatternAttr*    pOldAttrs       = GetSelectionPattern();

    std::shared_ptr<SfxItemSet> pOldSet(new SfxItemSet(pOldAttrs->GetItemSet()));
    std::shared_ptr<SvxNumberInfoItem> pNumberInfoItem;

    pOldSet->MergeRange(XATTR_FILLSTYLE, XATTR_FILLCOLOR);

    pOldSet->MergeRange(SID_ATTR_BORDER_STYLES, SID_ATTR_BORDER_DEFAULT_WIDTH);

    // We only allow these border line types.
    const std::vector<sal_Int32> aBorderStyles{
        table::BorderLineStyle::SOLID,
        table::BorderLineStyle::DOTTED,
        table::BorderLineStyle::DASHED,
        table::BorderLineStyle::FINE_DASHED,
        table::BorderLineStyle::DASH_DOT,
        table::BorderLineStyle::DASH_DOT_DOT,
        table::BorderLineStyle::DOUBLE_THIN };

    SfxIntegerListItem aBorderStylesItem(SID_ATTR_BORDER_STYLES, aBorderStyles);
    pOldSet->Put(aBorderStylesItem);

    // Set the default border width to 0.75 points.
    SfxInt64Item aBorderWidthItem(SID_ATTR_BORDER_DEFAULT_WIDTH, 75);
    pOldSet->Put(aBorderWidthItem);

    // Get border items and put them in the set:
    GetSelectionFrame( aLineOuter, aLineInner );
    //Fix border incorrect for RTL fdo#62399
    if( pDoc->IsLayoutRTL( GetViewData().GetTabNo() ) )
    {
        SvxBoxItem     aNewFrame( aLineOuter );
        SvxBoxInfoItem aTempInfo( aLineInner );

        if ( aLineInner.IsValid(SvxBoxInfoItemValidFlags::LEFT) )
            aNewFrame.SetLine( aLineOuter.GetLeft(), SvxBoxItemLine::RIGHT );
        if ( aLineInner.IsValid(SvxBoxInfoItemValidFlags::RIGHT) )
            aNewFrame.SetLine( aLineOuter.GetRight(), SvxBoxItemLine::LEFT );

        aLineInner.SetValid( SvxBoxInfoItemValidFlags::LEFT, aTempInfo.IsValid(SvxBoxInfoItemValidFlags::RIGHT));
        aLineInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT, aTempInfo.IsValid(SvxBoxInfoItemValidFlags::LEFT));

        pOldSet->Put( aNewFrame );
    }
    else
        pOldSet->Put( aLineOuter );

    pOldSet->Put( aLineInner );

    // Generate NumberFormat Value from Value and Language and box it.
    pOldSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
        pOldAttrs->GetNumberFormat( pDoc->GetFormatTable() ) ) );

    pNumberInfoItem.reset(MakeNumberInfoItem(pDoc, &GetViewData()));

    pOldSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
    pOldSet->Put(*pNumberInfoItem );

    bInFormatDialog = true;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScAttrDlg(GetDialogParent(), pOldSet.get()));

    if (!rName.isEmpty())
        pDlg->SetCurPageId(rName);

    std::shared_ptr<SfxRequest> pRequest(new SfxRequest(rReq));
    rReq.Ignore(); // the 'old' request is not relevant any more

    pDlg->StartExecuteAsync([=](sal_Int32 nResult){
            bInFormatDialog = false;

            if ( nResult == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                const SfxPoolItem* pItem=nullptr;
                if(pOutSet->GetItemState(SID_ATTR_NUMBERFORMAT_INFO,true,&pItem)==SfxItemState::SET)
                {
                    UpdateNumberFormatter(static_cast<const SvxNumberInfoItem&>(*pItem));
                }

                ApplyAttributes(pOutSet, pOldSet.get());

                pRequest->Done(*pOutSet);
            }
        });
}

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
                OUString aString = pHdl->GetEditString();
                if ( !pHdl->GetSelIsRef() && aString.getLength() > 1 &&
                     ( aString[0] == '+' || aString[0] == '-' ) )
                {
                    const ScViewData& rViewData = GetViewData();
                    ScDocument* pDoc = rViewData.GetDocument();
                    if ( pDoc )
                    {
                        const ScAddress aPos( rViewData.GetCurPos() );
                        ScCompiler aComp( pDoc, aPos, pDoc->GetGrammar() );
                        aComp.SetCloseBrackets( false );
                        std::unique_ptr<ScTokenArray> pArr(aComp.CompileString(aString));
                        if ( pArr && pArr->MayReferenceFollow() )
                        {
                            return true;
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

void ScTabViewShell::UpdateInputHandler( bool bForce /* = sal_False */, bool bStopEditing /* = sal_True */ )
{
    ScInputHandler* pHdl = mpInputHandler ? mpInputHandler.get() : SC_MOD()->GetInputHdl();

    if ( pHdl )
    {
        OUString                aString;
        const EditTextObject*   pObject     = nullptr;
        ScViewData&             rViewData   = GetViewData();
        ScDocument*             pDoc        = rViewData.GetDocument();
        SCCOL                   nPosX       = rViewData.GetCurX();
        SCROW                   nPosY       = rViewData.GetCurY();
        SCTAB                   nTab        = rViewData.GetTabNo();
        SCTAB                   nStartTab   = 0;
        SCTAB                   nEndTab     = 0;
        SCCOL                   nStartCol   = 0;
        SCROW                   nStartRow   = 0;
        SCCOL                   nEndCol     = 0;
        SCROW                   nEndRow     = 0;
        ScAddress aPos = rViewData.GetCurPos();

        rViewData.GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,   nEndTab );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        bool bHideFormula = false;
        bool bHideAll     = false;

        if (pDoc->IsTabProtected(nTab))
        {
            const ScProtectionAttr* pProt = pDoc->GetAttr( nPosX,nPosY,nTab,
                                                           ATTR_PROTECTION);
            bHideFormula = pProt->GetHideFormula();
            bHideAll     = pProt->GetHideCell();
        }

        if (!bHideAll)
        {
            ScRefCellValue rCell(*pDoc, aPos);
            if (rCell.meType == CELLTYPE_FORMULA)
            {
                if (!bHideFormula)
                    rCell.mpFormula->GetFormula(aString);
            }
            else if (rCell.meType == CELLTYPE_EDIT)
            {
                pObject = rCell.mpEditText;
            }
            else
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                sal_uInt32 nNumFmt = pDoc->GetNumberFormat( aPos );

                ScCellFormat::GetInputString( rCell, nNumFmt, aString, *pFormatter, pDoc );
                if (rCell.meType == CELLTYPE_STRING)
                {
                    // Put a ' in front if necessary, so that the string is not
                    // unintentionally interpreted as a number, and to show the
                    // user that it is a string (#35060#).
                    //! also for numberformat "Text"? -> then remove when editing
                    double fDummy;
                    if ( pFormatter->IsNumberFormat(aString, nNumFmt, fDummy) )
                        aString = "'" + aString;
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
        ScTabViewShell* pSourceSh = mpInputHandler ? this : nullptr;

        pHdl->NotifyChange( &aState, bForce, pSourceSh, bStopEditing );
    }

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_STATUS_SUM );         // always together with the input row
    rBindings.Invalidate( SID_ATTR_SIZE );
    rBindings.Invalidate( SID_TABLE_CELL );
}

void ScTabViewShell::UpdateInputHandlerCellAdjust( SvxCellHorJustify eJust )
{
    if( ScInputHandler* pHdl = mpInputHandler ? mpInputHandler.get() : SC_MOD()->GetInputHdl() )
        pHdl->UpdateCellAdjust( eJust );
}

void ScTabViewShell::ExecuteSave( SfxRequest& rReq )
{
    // only SID_SAVEDOC / SID_SAVEASDOC
    bool bCommitChanges = true;
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;

    if (pReqArgs && pReqArgs->HasItem(FN_PARAM_1, &pItem))
        bCommitChanges = !static_cast<const SfxBoolItem*>(pItem)->GetValue();

    // Finish entering unless 'DontTerminateEdit' is specified, even if a formula is being processed
    if (bCommitChanges)
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            // Normally this isn't needed, but in Calc when editing a cell formula
            // and manually saving (without changing cells or hitting enter), while
            // InputEnterHandler will mark the doc as modified (when it is), because
            // we will save the doc immediately afterwards, the modified state event
            // is clobbered. To avoid that, we notify all views immediately of the
            // modified state, apply the modification, then save the document.
            ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
            if (pHdl != nullptr && pHdl->GetModified())
                SfxLokHelper::notifyAllViews(LOK_CALLBACK_STATE_CHANGED, ".uno:ModifiedStatus=true");
        }

        SC_MOD()->InputEnterHandler();
    }

    if ( GetViewData().GetDocShell()->IsDocShared() )
    {
        GetViewData().GetDocShell()->SetDocumentModified();
    }

    // otherwise as normal
    GetViewData().GetDocShell()->ExecuteSlot( rReq );
}

void ScTabViewShell::GetSaveState( SfxItemSet& rSet )
{
    SfxShell* pDocSh = GetViewData().GetDocShell();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if ( nWhich != SID_SAVEDOC || !GetViewData().GetDocShell()->IsDocShared() )
        {
            // get state from DocShell
            pDocSh->GetSlotState( nWhich, nullptr, &rSet );
        }
        nWhich = aIter.NextWhich();
    }
}

void ScTabViewShell::ExecDrawOpt( const SfxRequest& rReq )
{
    ScViewOptions aViewOptions = GetViewData().GetOptions();
    ScGridOptions aGridOptions = aViewOptions.GetGridOptions();

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    sal_uInt16 nSlotId = rReq.GetSlot();
    switch (nSlotId)
    {
        case SID_GRID_VISIBLE:
            if ( pArgs && pArgs->GetItemState(nSlotId,true,&pItem) == SfxItemState::SET )
            {
                aGridOptions.SetGridVisible( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_VISIBLE);
            }
            break;

        case SID_GRID_USE:
            if ( pArgs && pArgs->GetItemState(nSlotId,true,&pItem) == SfxItemState::SET )
            {
                aGridOptions.SetUseGridSnap( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_USE);
            }
            break;

        case SID_HELPLINES_MOVE:
            if ( pArgs && pArgs->GetItemState(nSlotId,true,&pItem) == SfxItemState::SET )
            {
                aViewOptions.SetOption( VOPT_HELPLINES, static_cast<const SfxBoolItem*>(pItem)->GetValue() );
                rBindings.Invalidate(SID_HELPLINES_MOVE);
            }
            break;
    }

    GetViewData().SetOptions(aViewOptions);
}

void ScTabViewShell::GetDrawOptState( SfxItemSet& rSet )
{
    SfxBoolItem aBool;

    const ScViewOptions& rViewOptions = GetViewData().GetOptions();
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
