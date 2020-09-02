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

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <comphelper/lok.hxx>
#include <editeng/boxitem.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <o3tl/temporary.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/ilstitem.hxx>
#include <svl/int64item.hxx>
#include <svl/srchitem.hxx>
#include <svl/srchdefs.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svx/numinf.hxx>
#include <svx/zoomslideritem.hxx>

#include <global.hxx>
#include <appoptio.hxx>
#include <attrib.hxx>
#include <cellform.hxx>
#include <cellvalue.hxx>
#include <compiler.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <globstr.hrc>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <sc.hrc>
#include <scabstdlg.hxx>
#include <scitems.hxx>
#include <scmod.hxx>
#include <scresid.hxx>
#include <stlpool.hxx>
#include <tabvwsh.hxx>
#include <tokenarray.hxx>
#include <viewdata.hxx>

#include <memory>

using namespace com::sun::star;

bool ScTabViewShell::GetFunction( OUString& rFuncStr, FormulaError nErrCode )
{
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
            ScDocument& rDoc        = rViewData.GetDocument();
            SCCOL       nPosX       = rViewData.GetCurX();
            SCROW       nPosY       = rViewData.GetCurY();
            SCTAB       nTab        = rViewData.GetTabNo();

            OUString aStr = ScResId(pGlobStrId) + ": ";

            ScAddress aCursor( nPosX, nPosY, nTab );
            double nVal;
            if ( rDoc.GetSelectionFunction( eFunc, aCursor, rMark, nVal ) )
            {
                if ( nVal == 0.0 )
                    aStr += "0";
                else
                {
                    // Number in the standard format, the other on the cursor position
                    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                    sal_uInt32 nNumFmt = 0;
                    if ( eFunc != SUBTOTAL_FUNC_CNT && eFunc != SUBTOTAL_FUNC_CNT2 && eFunc != SUBTOTAL_FUNC_SELECTION_COUNT)
                    {
                        // number format from attributes or formula
                        rDoc.GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
                    }

                    OUString aValStr;
                    const Color* pDummy;
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
                rFuncStr += "; " + aStr;
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
    ScDocument& rDoc        = rViewData.GetDocument();
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
                else if (rDoc.IsPrintEntireSheet(nTab))
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
                    OUString  aAddr(aScAddress.Format(ScRefFlags::ADDR_ABS, nullptr, rDoc.GetAddressConvention()));
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
                if (!rDoc.HasManualBreaks(nTab))
                    rSet.DisableItem( nWhich );
                break;

            case FID_RESET_PRINTZOOM:
                {
                    // disable if already set to default

                    OUString aStyleName = rDoc.GetPageStyle( nTab );
                    ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
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
                        rSet.Put( SfxBoolItem( nWhich, rDoc.IsDocProtected() ) );
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
                        rSet.Put( SfxBoolItem( nWhich, rDoc.IsTabProtected( nTab ) ) );
                    }
                }
                break;

            case SID_AUTO_OUTLINE:
                {
                    if (rDoc.GetChangeTrack()!=nullptr || GetViewData().IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_OUTLINE_DELETEALL:
                {
                    SCTAB nOlTab = GetViewData().GetTabNo();
                    ScOutlineTable* pOlTable = rDoc.GetOutlineTable( nOlTab );
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
                if(!comphelper::LibreOfficeKit::isActive())
                {
                    rSet.Put(SfxBoolItem(nWhich,
                                rViewData.GetHSplitMode() == SC_SPLIT_FIX ||
                                rViewData.GetVSplitMode() == SC_SPLIT_FIX ));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nWhich,
                            rViewData.GetLOKSheetFreezeIndex(true) > 0 ||
                            rViewData.GetLOKSheetFreezeIndex(false) > 0 ));
                }
                break;

            case SID_WINDOW_FIX_COL:
            case SID_WINDOW_FIX_ROW:
                {
                    bool bIsCol = (nWhich == SID_WINDOW_FIX_COL);
                    sal_Int32 nFreezeIndex = rViewData.GetLOKSheetFreezeIndex(bIsCol);
                    rSet.Put(SfxInt32Item(nWhich, nFreezeIndex));
                }
                break;

            case FID_CHG_SHOW:
                {
                    if ( rDoc.GetChangeTrack() == nullptr || ( pDocShell && pDocShell->IsDocShared() ) )
                        rSet.DisableItem( nWhich );
                }
                break;
            case FID_CHG_ACCEPT:
                {
                    if(
                       ( !rDoc.GetChangeTrack() &&  !pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
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
    ScDocument& rDoc = GetViewData().GetDocument();

    std::shared_ptr<SvxBoxItem> aLineOuter(std::make_shared<SvxBoxItem>(ATTR_BORDER));
    std::shared_ptr<SvxBoxInfoItem> aLineInner(std::make_shared<SvxBoxInfoItem>(ATTR_BORDER_INNER));

    const ScPatternAttr*    pOldAttrs       = GetSelectionPattern();

    auto pOldSet = std::make_shared<SfxItemSet>(pOldAttrs->GetItemSet());
    std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem;

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
    if( rDoc.IsLayoutRTL( GetViewData().GetTabNo() ) )
    {
        std::unique_ptr<SvxBoxItem> aNewFrame(aLineOuter->Clone());
        std::unique_ptr<SvxBoxInfoItem> aTempInfo(aLineInner->Clone());

        if ( aLineInner->IsValid(SvxBoxInfoItemValidFlags::LEFT) )
            aNewFrame->SetLine( aLineOuter->GetLeft(), SvxBoxItemLine::RIGHT );
        if ( aLineInner->IsValid(SvxBoxInfoItemValidFlags::RIGHT) )
            aNewFrame->SetLine( aLineOuter->GetRight(), SvxBoxItemLine::LEFT );

        aLineInner->SetValid( SvxBoxInfoItemValidFlags::LEFT, aTempInfo->IsValid(SvxBoxInfoItemValidFlags::RIGHT));
        aLineInner->SetValid( SvxBoxInfoItemValidFlags::RIGHT, aTempInfo->IsValid(SvxBoxInfoItemValidFlags::LEFT));

        pOldSet->Put( *aNewFrame );
    }
    else
    {
        pOldSet->Put( *aLineOuter );
    }

    pOldSet->Put( *aLineInner );

    // Generate NumberFormat Value from Value and Language and box it.
    pOldSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
        pOldAttrs->GetNumberFormat( rDoc.GetFormatTable() ) ) );

    pNumberInfoItem = MakeNumberInfoItem(&rDoc, &GetViewData());

    pOldSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
    pOldSet->Put(*pNumberInfoItem );

    bInFormatDialog = true;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScAttrDlg(GetFrameWeld(), pOldSet.get()));

    if (!rName.isEmpty())
        pDlg->SetCurPageId(rName);

    auto pRequest = std::make_shared<SfxRequest>(rReq);
    rReq.Ignore(); // the 'old' request is not relevant any more

    pDlg->StartExecuteAsync([pDlg, pOldSet, pRequest, this](sal_Int32 nResult){
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

        pDlg->disposeOnce();
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
                    ScDocument& rDoc = rViewData.GetDocument();
                    const ScAddress aPos( rViewData.GetCurPos() );
                    ScCompiler aComp( rDoc, aPos, rDoc.GetGrammar() );
                    aComp.SetCloseBrackets( false );
                    std::unique_ptr<ScTokenArray> pArr(aComp.CompileString(aString));
                    if ( pArr && pArr->MayReferenceFollow() )
                    {
                        return true;
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
        ScDocument&             rDoc        = rViewData.GetDocument();
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

        if (rDoc.IsTabProtected(nTab))
        {
            const ScProtectionAttr* pProt = rDoc.GetAttr( nPosX,nPosY,nTab,
                                                           ATTR_PROTECTION);
            bHideFormula = pProt->GetHideFormula();
            bHideAll     = pProt->GetHideCell();
        }

        if (!bHideAll)
        {
            ScRefCellValue rCell(rDoc, aPos);
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
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                sal_uInt32 nNumFmt = rDoc.GetNumberFormat( aPos );

                ScCellFormat::GetInputString( rCell, nNumFmt, aString, *pFormatter, rDoc );
                if (rCell.meType == CELLTYPE_STRING)
                {
                    // Put a ' in front if necessary, so that the string is not
                    // unintentionally interpreted as a number, and to show the
                    // user that it is a string (#35060#).
                    //! also for numberformat "Text"? -> then remove when editing
                    if ( pFormatter->IsNumberFormat(aString, nNumFmt, o3tl::temporary(double())) )
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
        SC_MOD()->InputEnterHandler();

        if (comphelper::LibreOfficeKit::isActive())
        {
            // Normally this isn't needed, but in Calc when editing a cell formula
            // and manually saving (without changing cells or hitting enter), while
            // InputEnterHandler will mark the doc as modified (when it is), because
            // we will save the doc immediately afterwards, the modified state event
            // is clobbered. To avoid that, we need to update SID_DOC_MODIFIED so that
            // a possible state of "true" after "InputEnterHandler" will be sent
            // as a notification. It is important that the notification goes through
            // normal process (cache) rather than directly notifying the views.
            // Otherwise, because there is a previous state of "false" in cache, the
            // "false" state after saving will be ignored.
            // This will work only if .uno:ModifiedStatus message will be removed from
            // the mechanism that keeps in the message queue only last message of
            // a particular status even if the values are different.
            GetViewData().GetDocShell()->GetViewBindings()->Update(SID_DOC_MODIFIED);
        }
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
