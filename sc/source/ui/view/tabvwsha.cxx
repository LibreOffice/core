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
#include <officecfg/Office/Calc.hxx>

#include <comphelper/lok.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/langitem.hxx>
#include <o3tl/temporary.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/tplpitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/int64item.hxx>
#include <svl/ptitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/srchdefs.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svx/numinf.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
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
#include <printfun.hxx>
#include <undostyl.hxx>
#include <futext.hxx>

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
        if ( !(nFuncs & (1U << nFunc)) )
            continue;
        ScSubTotalFunc eFunc = static_cast<ScSubTotalFunc>(nFunc);

        if (bIgnoreError && (eFunc == SUBTOTAL_FUNC_CNT || eFunc == SUBTOTAL_FUNC_CNT2))
            nErrCode = FormulaError::NONE;

        if (nErrCode != FormulaError::NONE)
        {
            rFuncStr = ScGlobal::GetLongErrorString(nErrCode);
            return true;
        }

        TranslateId pGlobStrId;
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
                    ScInterpreterContext& rContext = rDoc.GetNonThreadedContext();
                    sal_uInt32 nNumFmt = 0;
                    if ( eFunc != SUBTOTAL_FUNC_CNT && eFunc != SUBTOTAL_FUNC_CNT2 && eFunc != SUBTOTAL_FUNC_SELECTION_COUNT)
                    {
                        // number format from attributes or formula
                        nNumFmt = rDoc.GetNumberFormat( nPosX, nPosY, nTab );
                        // If the number format is time (without date) and the
                        // result is not within 24 hours, use a duration
                        // format. Summing date+time doesn't make much sense
                        // otherwise but we also don't want to display duration
                        // for a single date+time value.
                        if (nVal < 0.0 || nVal >= 1.0)
                        {
                            const SvNumberformat* pFormat = rContext.NFGetFormatEntry(nNumFmt);
                            if (pFormat && (pFormat->GetType() == SvNumFormatType::TIME))
                                nNumFmt = rContext.NFGetTimeFormat( nVal, pFormat->GetLanguage(), true);
                        }
                    }

                    OUString aValStr;
                    const Color* pDummy;
                    rContext.NFGetOutputString( nVal, nNumFmt, aValStr, &pDummy );
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

    SfxViewFrame& rThisFrame = GetViewFrame();
    bool bOle = GetViewFrame().GetFrame().IsInPlace();

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

                    if ( rThisFrame.KnowsChildWindow( nId ) )
                    {
                        SfxChildWindow* pWnd = rThisFrame.GetChildWindow( nId );
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

            case SID_ZOOM_IN:
                {
                    const Fraction& rZoomY = GetViewData().GetZoomY();
                    tools::Long nZoom = tools::Long(rZoomY * 100);
                    if (nZoom >= MAXZOOM)
                        rSet.DisableItem(nWhich);
                }
                break;
            case SID_ZOOM_OUT:
                {
                    const Fraction& rZoomY = GetViewData().GetZoomY();
                    tools::Long nZoom = tools::Long(rZoomY * 100);
                    if (nZoom <= MINZOOM)
                        rSet.DisableItem(nWhich);
                }
                break;

            case FID_SCALE:
            case SID_ATTR_ZOOM:
                if ( bOle )
                    rSet.DisableItem( nWhich );
                else
                {
                    const Fraction& rOldY = GetViewData().GetZoomY();
                    sal_uInt16 nZoom = static_cast<sal_uInt16>(tools::Long( rOldY * 100 ));
                    rSet.Put( SvxZoomItem( SvxZoomType::PERCENT, nZoom, TypedWhichId<SvxZoomItem>(nWhich) ) );
                }
                break;

            case SID_ATTR_ZOOMSLIDER:
                {
                    if ( bOle )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        const Fraction& rOldY = GetViewData().GetZoomY();
                        sal_uInt16 nCurrentZoom = static_cast<sal_uInt16>(tools::Long( rOldY * 100 ));

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
                const bool bBoxOpen = ::sfx2::sidebar::Sidebar::IsPanelVisible(u"ScFunctionsPanel",
                                                    rThisFrame.GetFrame().GetFrameInterface());
                rSet.Put(SfxBoolItem(nWhich, bBoxOpen));
                break;
            }

            case FID_TOGGLESYNTAX:
                rSet.Put(SfxBoolItem(nWhich, GetViewData().IsSyntaxMode()));
                break;

            case FID_TOGGLECOLROWHIGHLIGHTING:
                rSet.Put(SfxBoolItem(
                    nWhich,
                    officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::get()));
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
                    Point aPos;
                    bool bIsCol = (nWhich == SID_WINDOW_FIX_COL);
                    aPos.setX(rViewData.GetLOKSheetFreezeIndex(bIsCol));
                    aPos.setY(rViewData.GetTabNo());
                    rSet.Put(SfxPointItem(nWhich, aPos));
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
                       ( !rDoc.GetChangeTrack() && !rThisFrame.HasChildWindow(FID_CHG_ACCEPT) )
                       ||
                       ( pDocShell && pDocShell->IsDocShared() )
                      )
                    {
                        rSet.DisableItem( nWhich);
                    }
                    else
                    {
                        rSet.Put(SfxBoolItem(FID_CHG_ACCEPT,
                            rThisFrame.HasChildWindow(FID_CHG_ACCEPT)));
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

            case FID_TOGGLEHIDDENCOLROW:
                const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
                rSet.Put( SfxBoolItem( nWhich, rColorCfg.GetColorValue(svtools::CALCHIDDENROWCOL).bIsVisible) );
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

void ScTabViewShell::ExecuteCellFormatDlg(SfxRequest& rReq, const OUString &rName)
{
    ScDocument& rDoc = GetViewData().GetDocument();

    std::shared_ptr<SvxBoxItem> aLineOuter(std::make_shared<SvxBoxItem>(ATTR_BORDER));
    std::shared_ptr<SvxBoxInfoItem> aLineInner(std::make_shared<SvxBoxInfoItem>(ATTR_BORDER_INNER));

    const ScPatternAttr*    pOldAttrs       = GetSelectionPattern();

    auto pOldSet = std::make_shared<SfxItemSet>(pOldAttrs->GetItemSet());

    pOldSet->MergeRange(XATTR_FILLSTYLE, XATTR_FILLCOLOR);

    pOldSet->MergeRange(SID_ATTR_BORDER_STYLES, SID_ATTR_BORDER_DEFAULT_WIDTH);

    // We only allow these border line types.
    std::vector<sal_Int32> aBorderStyles{
        table::BorderLineStyle::SOLID,
        table::BorderLineStyle::DOTTED,
        table::BorderLineStyle::DASHED,
        table::BorderLineStyle::FINE_DASHED,
        table::BorderLineStyle::DASH_DOT,
        table::BorderLineStyle::DASH_DOT_DOT,
        table::BorderLineStyle::DOUBLE_THIN };

    pOldSet->Put(SfxIntegerListItem(SID_ATTR_BORDER_STYLES, std::move(aBorderStyles)));

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

        pOldSet->Put( std::move(aNewFrame) );
    }
    else
    {
        pOldSet->Put( *aLineOuter );
    }

    pOldSet->Put( *aLineInner );

    // Generate NumberFormat Value from Value and Language and box it.
    pOldSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
        pOldAttrs->GetNumberFormat( rDoc.GetFormatTable() ) ) );

    std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem = MakeNumberInfoItem(rDoc, GetViewData());
    pOldSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
    pOldSet->Put( std::move(pNumberInfoItem) );

    bInFormatDialog = true;
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScAttrDlg(GetFrameWeld(), pOldSet.get()));

    if (!rName.isEmpty())
        pDlg->SetCurPageId(rName);

    auto xRequest = std::make_shared<SfxRequest>(rReq);
    rReq.Ignore(); // the 'old' request is not relevant any more

    pDlg->StartExecuteAsync([pDlg, pOldSet, xRequest=std::move(xRequest), this](sal_Int32 nResult){
        bInFormatDialog = false;

        if ( nResult == RET_OK )
        {
            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
            if(const SvxNumberInfoItem* pItem = pOutSet->GetItemIfSet(SID_ATTR_NUMBERFORMAT_INFO))
            {
                UpdateNumberFormatter(*pItem);
            }

            ApplyAttributes(*pOutSet, *pOldSet);

            xRequest->Done(*pOutSet);
        }

        pDlg->disposeOnce();
    });
}

const OUString* ScTabViewShell::GetEditString() const
{
    if (mpInputHandler)
        return &mpInputHandler->GetEditString();

    return nullptr;
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
                const ScViewData& rViewData = GetViewData();
                ScDocument& rDoc = rViewData.GetDocument();
                const ScAddress aPos( rViewData.GetCurPos() );
                const sal_uInt32 nIndex = rDoc.GetAttr(aPos, ATTR_VALUE_FORMAT )->GetValue();
                const SvNumFormatType nType = rDoc.GetFormatTable()->GetType(nIndex);
                if (nType == SvNumFormatType::TEXT)
                {
                    return false;
                }
                OUString aString = pHdl->GetEditString();
                if ( !pHdl->GetSelIsRef() && aString.getLength() > 1 &&
                     ( aString[0] == '+' || aString[0] == '-' ) )
                {
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
            if (rCell.getType() == CELLTYPE_FORMULA)
            {
                if (!bHideFormula)
                    aString = rCell.getFormula()->GetFormula();
            }
            else if (rCell.getType() == CELLTYPE_EDIT)
            {
                pObject = rCell.getEditText();
            }
            else
            {
                ScInterpreterContext& rContext = rDoc.GetNonThreadedContext();
                sal_uInt32 nNumFmt = rDoc.GetNumberFormat( aPos );

                aString = ScCellFormat::GetInputString( rCell, nNumFmt, &rContext, rDoc );
                if (rCell.getType() == CELLTYPE_STRING)
                {
                    // Put a ' in front if necessary, so that the string is not
                    // unintentionally interpreted as a number, and to show the
                    // user that it is a string (#35060#).
                    // If cell is not formatted as Text, a leading apostrophe
                    // needs another prepended, also '=' or '+' or '-'
                    // otherwise starting a formula.
                    // NOTE: this corresponds with
                    // sc/source/core/data/column3.cxx ScColumn::ParseString()
                    // removing one apostrophe.
                    // For number format Text IsNumberFormat() would never
                    // result in numeric anyway.
                    if (!rContext.NFIsTextFormat(nNumFmt) && (aString.startsWith("'")
                                || aString.startsWith("=") || aString.startsWith("+") || aString.startsWith("-")
                                || rContext.NFIsNumberFormat(aString, nNumFmt, o3tl::temporary(double()))))
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

    SfxBindings& rBindings = GetViewFrame().GetBindings();
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
        bool bLOKActive = comphelper::LibreOfficeKit::isActive();

        // Disable error dialog box when about to save in lok mode as
        // this ultimately invokes SvpSalInstance::DoYield() when we want
        // to save immediately without committing any erroneous input in possibly
        // a cell with validation rules. After save is complete the user
        // can continue editing.
        SC_MOD()->InputEnterHandler(ScEnterMode::NORMAL, bLOKActive /* bBeforeSavingInLOK */);

        if (bLOKActive)
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

    SfxBindings& rBindings = GetViewFrame().GetBindings();
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

void ScTabViewShell::ExecStyle( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const sal_uInt16  nSlotId = rReq.GetSlot();
    if ( !pArgs && nSlotId != SID_STYLE_NEW_BY_EXAMPLE && nSlotId != SID_STYLE_UPDATE_BY_EXAMPLE )
    {
        // in case of vertical toolbar
        GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
        return;
    }

    SfxBindings&        rBindings   = GetViewData().GetBindings();
    const SCTAB         nCurTab     = GetViewData().GetTabNo();
    ScDocShell*         pDocSh      = GetViewData().GetDocShell();
    ScDocument&         rDoc        = pDocSh->GetDocument();
    ScMarkData&         rMark       = GetViewData().GetMarkData();
    ScModule*           pScMod      = SC_MOD();
    SdrObject*          pEditObject = GetDrawView()->GetTextEditObject();
    OutlinerView*       pOLV        = GetDrawView()->GetTextEditOutlinerView();
    ESelection          aSelection  = pOLV ? pOLV->GetSelection() : ESelection();
    OUString            aRefName;
    bool                bUndo       = rDoc.IsUndoEnabled();

    SfxStyleSheetBasePool*  pStylePool  = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase*      pStyleSheet = nullptr;

    bool bStyleToMarked = false;
    bool bListAction = false;
    bool bAddUndo = false;          // add ScUndoModifyStyle (style modified)
    ScStyleSaveData aOldData;       // for undo/redo
    ScStyleSaveData aNewData;

    SfxStyleFamily eFamily = SfxStyleFamily::Para;
    const SfxUInt16Item* pFamItem;
    const SfxStringItem* pFamilyNameItem;
    if ( pArgs && (pFamItem = pArgs->GetItemIfSet( SID_STYLE_FAMILY )) )
        eFamily = static_cast<SfxStyleFamily>(pFamItem->GetValue());
    else if ( pArgs && (pFamilyNameItem = pArgs->GetItemIfSet( SID_STYLE_FAMILYNAME )) )
    {
        OUString sFamily = pFamilyNameItem->GetValue();
        if (sFamily == "CellStyles")
            eFamily = SfxStyleFamily::Para;
        else if (sFamily == "PageStyles")
            eFamily = SfxStyleFamily::Page;
        else if (sFamily == "GraphicStyles")
            eFamily = SfxStyleFamily::Frame;
    }

    OUString                aStyleName;
    sal_uInt16              nRetMask = 0xffff;

    switch ( nSlotId )
    {
        case SID_STYLE_NEW:
            {
                const SfxPoolItem* pNameItem;
                if (pArgs && SfxItemState::SET == pArgs->GetItemState( nSlotId, true, &pNameItem ))
                    aStyleName  = static_cast<const SfxStringItem*>(pNameItem)->GetValue();

                const SfxStringItem* pRefItem=nullptr;
                if (pArgs && (pRefItem = pArgs->GetItemIfSet( SID_STYLE_REFERENCE )))
                {
                    aRefName  = pRefItem->GetValue();
                }

                pStyleSheet = &(pStylePool->Make( aStyleName, eFamily,
                                                  SfxStyleSearchBits::UserDefined ) );

                if (pStyleSheet->HasParentSupport())
                    pStyleSheet->SetParent(aRefName);
            }
            break;

        case SID_STYLE_APPLY:
        {
            const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_APPLY_STYLE);
            const SfxStringItem* pFamilyItem = rReq.GetArg<SfxStringItem>(SID_STYLE_FAMILYNAME);
            if ( pFamilyItem && pNameItem )
            {
                try
                {
                    css::uno::Reference< css::container::XNameAccess > xStyles;
                    css::uno::Reference< css::container::XNameAccess > xCont = pDocSh->GetModel()->getStyleFamilies();
                    xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                    css::uno::Reference< css::beans::XPropertySet > xInfo;
                    xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                    OUString aUIName;
                    xInfo->getPropertyValue(u"DisplayName"_ustr) >>= aUIName;
                    if ( !aUIName.isEmpty() )
                        rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                }
                catch( css::uno::Exception& )
                {
                }
            }
            [[fallthrough]];
        }
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_NEW_BY_EXAMPLE:
            {
                const SfxPoolItem* pNameItem;
                if (pArgs && SfxItemState::SET == pArgs->GetItemState(nSlotId, true, &pNameItem))
                    aStyleName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
                else if ( nSlotId == SID_STYLE_NEW_BY_EXAMPLE )
                {
                    weld::Window* pDialogParent = rReq.GetFrameWeld();
                    if (!pDialogParent)
                        pDialogParent = GetFrameWeld();
                    SfxNewStyleDlg aDlg(pDialogParent, *pStylePool, eFamily);
                    if (aDlg.run() != RET_OK)
                        return;
                    aStyleName = aDlg.GetName();
                }

                pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                aOldData.InitFromStyle( pStyleSheet );
            }
            break;

        case SID_STYLE_WATERCAN:
        {
            bool bWaterCan = pScMod->GetIsWaterCan();

            if( !bWaterCan )
            {
                const SfxPoolItem* pItem;

                if (pArgs && SfxItemState::SET == pArgs->GetItemState(nSlotId, true, &pItem))
                {
                    const SfxStringItem* pStrItem = dynamic_cast< const SfxStringItem *>( pItem );
                    if ( pStrItem )
                    {
                        aStyleName  = pStrItem->GetValue();
                        pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                        if ( pStyleSheet )
                        {
                            static_cast<ScStyleSheetPool*>(pStylePool)->
                                    SetActualStyleSheet( pStyleSheet );
                            rReq.Done();
                        }
                    }
                }
            }

            if ( !bWaterCan && pStyleSheet )
            {
                pScMod->SetWaterCan( true );
                SetActivePointer( PointerStyle::Fill );
                rReq.Done();
            }
            else
            {
                pScMod->SetWaterCan( false );
                SetActivePointer( PointerStyle::Arrow );
                rReq.Done();
            }
        }
        break;

        default:
            break;
    }

    // set new style for paintbrush format mode
    if ( nSlotId == SID_STYLE_APPLY && pScMod->GetIsWaterCan() && pStyleSheet )
        static_cast<ScStyleSheetPool*>(pStylePool)->SetActualStyleSheet( pStyleSheet );

    switch ( eFamily )
    {
        case SfxStyleFamily::Para:
        {
            switch ( nSlotId )
            {
                case SID_STYLE_DELETE:
                {
                    if ( pStyleSheet )
                    {
                        RemoveStyleSheetInUse( pStyleSheet );
                        pStylePool->Remove( pStyleSheet );
                        InvalidateAttribs();
                        nRetMask = sal_uInt16(true);
                        bAddUndo = true;
                        rReq.Done();
                    }
                    else
                        nRetMask = sal_uInt16(false);
                }
                break;

                case SID_STYLE_HIDE:
                case SID_STYLE_SHOW:
                {
                    if ( pStyleSheet )
                    {
                        pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                        InvalidateAttribs();
                        rReq.Done();
                    }
                    else
                        nRetMask = sal_uInt16(false);
                }
                break;

                case SID_STYLE_APPLY:
                {
                    if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                    {
                        // apply style sheet to document
                        SetStyleSheetToMarked( static_cast<SfxStyleSheet*>(pStyleSheet) );
                        InvalidateAttribs();
                        rReq.Done();
                    }
                }
                break;

                case SID_STYLE_NEW_BY_EXAMPLE:
                case SID_STYLE_UPDATE_BY_EXAMPLE:
                {
                    // create/replace style sheet by attributes
                    // at cursor position:

                    const ScPatternAttr* pAttrItem = nullptr;

                    // The query if marked, was always wrong here,
                    // so now no more, and just from the cursor.
                    // If attributes are to be removed from the selection, still need to be
                    // cautious not to adopt items from templates
                    // (GetSelectionPattern also collects items from originals) (# 44748 #)
                    SCCOL       nCol = GetViewData().GetCurX();
                    SCROW       nRow = GetViewData().GetCurY();
                    pAttrItem = rDoc.GetPattern( nCol, nRow, nCurTab );

                    SfxItemSet aAttrSet = pAttrItem->GetItemSet();
                    aAttrSet.ClearItem( ATTR_MERGE );
                    aAttrSet.ClearItem( ATTR_MERGE_FLAG );

                    // Do not adopt conditional formatting and validity,
                    // because they can not be edited in the template
                    aAttrSet.ClearItem( ATTR_VALIDDATA );
                    aAttrSet.ClearItem( ATTR_CONDITIONAL );

                    if ( SID_STYLE_NEW_BY_EXAMPLE == nSlotId )
                    {
                        if ( bUndo )
                        {
                            OUString aUndo = ScResId( STR_UNDO_EDITCELLSTYLE );
                            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewShellId() );
                            bListAction = true;
                        }

                        bool bConvertBack = false;
                        SfxStyleSheet*  pSheetInUse = const_cast<SfxStyleSheet*>(GetStyleSheetFromMarked());

                        // when a new style is present and is used in the selection,
                        // then the parent can not be adopted:
                        if ( pStyleSheet && pSheetInUse && pStyleSheet == pSheetInUse )
                            pSheetInUse = nullptr;

                        // if already present, first remove ...
                        if ( pStyleSheet )
                        {
                            // style pointer to names before erase,
                            // otherwise cells will get invalid pointer
                            //!!! As it happens, a method that does it for a particular style
                            rDoc.getCellAttributeHelper().AllStylesToNames();
                            bConvertBack = true;
                            pStylePool->Remove(pStyleSheet);
                        }

                        // ...and create new
                        pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                         SfxStyleSearchBits::UserDefined );

                        // when a style is present, then this will become
                        // the parent of the new style:
                        if ( pSheetInUse && pStyleSheet->HasParentSupport() )
                            pStyleSheet->SetParent( pSheetInUse->GetName() );

                        if ( bConvertBack )
                            // Name to style pointer
                            rDoc.getCellAttributeHelper().UpdateAllStyleSheets(rDoc);
                        else
                            rDoc.getCellAttributeHelper().CellStyleCreated(rDoc, aStyleName);

                        // Adopt attribute and use style
                        pStyleSheet->GetItemSet().Put( aAttrSet );
                        UpdateStyleSheetInUse( pStyleSheet );

                        //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                        //  (pStyleSheet pointer is used!)
                        bStyleToMarked = true;
                    }
                    else // ( nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE )
                    {
                        pStyleSheet = const_cast<SfxStyleSheet*>(GetStyleSheetFromMarked());

                        if ( pStyleSheet )
                        {
                            aOldData.InitFromStyle( pStyleSheet );

                            if ( bUndo )
                            {
                                OUString aUndo = ScResId( STR_UNDO_EDITCELLSTYLE );
                                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewShellId() );
                                bListAction = true;
                            }

                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            UpdateStyleSheetInUse( pStyleSheet );

                            //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                            //  (pStyleSheet pointer is used!)
                            bStyleToMarked = true;
                        }
                    }

                    aNewData.InitFromStyle( pStyleSheet );
                    bAddUndo = true;
                    rReq.Done();
                }
                break;

                default:
                    break;
            }
        } // case SfxStyleFamily::Para:
        break;

        case SfxStyleFamily::Page:
        {
            switch ( nSlotId )
            {
                case SID_STYLE_DELETE:
                {
                    nRetMask = sal_uInt16( nullptr != pStyleSheet );
                    if ( pStyleSheet )
                    {
                        if ( rDoc.RemovePageStyleInUse( pStyleSheet->GetName() ) )
                        {
                            ScPrintFunc( pDocSh, GetPrinter(true), nCurTab ).UpdatePages();
                            rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                            rBindings.Invalidate( FID_RESET_PRINTZOOM );
                        }
                        pStylePool->Remove( pStyleSheet );
                        rBindings.Invalidate( SID_STYLE_FAMILY4 );
                        pDocSh->SetDocumentModified();
                        bAddUndo = true;
                        rReq.Done();
                    }
                }
                break;

                case SID_STYLE_HIDE:
                case SID_STYLE_SHOW:
                {
                    nRetMask = sal_uInt16( nullptr != pStyleSheet );
                    if ( pStyleSheet )
                    {
                        pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                        rBindings.Invalidate( SID_STYLE_FAMILY4 );
                        pDocSh->SetDocumentModified();
                        rReq.Done();
                    }
                }
                break;

                case SID_STYLE_APPLY:
                {
                    nRetMask = sal_uInt16( nullptr != pStyleSheet );
                    if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                    {
                        std::unique_ptr<ScUndoApplyPageStyle> pUndoAction;
                        SCTAB nTabCount = rDoc.GetTableCount();
                        for (const auto& rTab : rMark)
                        {
                            if (rTab >= nTabCount)
                                break;
                            OUString aOldName = rDoc.GetPageStyle( rTab );
                            if ( aOldName != aStyleName )
                            {
                                rDoc.SetPageStyle( rTab, aStyleName );
                                ScPrintFunc( pDocSh, GetPrinter(true), rTab ).UpdatePages();
                                if( !pUndoAction )
                                    pUndoAction.reset(new ScUndoApplyPageStyle( pDocSh, aStyleName ));
                                pUndoAction->AddSheetAction( rTab, aOldName );
                            }
                        }
                        if( pUndoAction )
                        {
                            pDocSh->GetUndoManager()->AddUndoAction( std::move(pUndoAction) );
                            pDocSh->SetDocumentModified();
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                            rBindings.Invalidate( FID_RESET_PRINTZOOM );
                        }
                        rReq.Done();
                    }
                }
                break;

                case SID_STYLE_NEW_BY_EXAMPLE:
                {
                    const OUString& rStrCurStyle = rDoc.GetPageStyle( nCurTab );

                    if ( rStrCurStyle != aStyleName )
                    {
                        SfxStyleSheetBase*  pCurStyle = pStylePool->Find( rStrCurStyle, eFamily );
                        SfxItemSet          aAttrSet  = pCurStyle->GetItemSet();
                        SCTAB               nInTab;
                        bool                bUsed = rDoc.IsPageStyleInUse( aStyleName, &nInTab );

                        // if already present, first remove...
                        if ( pStyleSheet )
                            pStylePool->Remove( pStyleSheet );

                        // ...and create new
                        pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                         SfxStyleSearchBits::UserDefined );

                        // Adopt attribute
                        pStyleSheet->GetItemSet().Put( aAttrSet );
                        pDocSh->SetDocumentModified();

                        // If being used -> Update
                        if ( bUsed )
                            ScPrintFunc( pDocSh, GetPrinter(true), nInTab ).UpdatePages();

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = true;
                        rReq.Done();
                        nRetMask = sal_uInt16(true);
                    }
                }
                break;

                default:
                    break;
            } // switch ( nSlotId )
        } // case SfxStyleFamily::Page:
        break;

        case SfxStyleFamily::Frame:
        {
            switch ( nSlotId )
            {
                case SID_STYLE_DELETE:
                {
                    if ( pStyleSheet )
                    {
                        pStylePool->Remove( pStyleSheet );
                        InvalidateAttribs();
                        pDocSh->SetDocumentModified();
                        nRetMask = sal_uInt16(true);
                        bAddUndo = true;
                        rReq.Done();
                    }
                    else
                        nRetMask = sal_uInt16(false);
                }
                break;

                case SID_STYLE_HIDE:
                case SID_STYLE_SHOW:
                {
                    if ( pStyleSheet )
                    {
                        pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                        InvalidateAttribs();
                        rReq.Done();
                    }
                    else
                        nRetMask = sal_uInt16(false);
                }
                break;

                case SID_STYLE_APPLY:
                {
                    if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                    {
                        GetScDrawView()->ScEndTextEdit();
                        GetScDrawView()->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), false);

                        GetScDrawView()->InvalidateAttribs();
                        InvalidateAttribs();
                        rReq.Done();
                    }
                }
                break;

                case SID_STYLE_NEW_BY_EXAMPLE:
                case SID_STYLE_UPDATE_BY_EXAMPLE:
                {
                    if (nSlotId == SID_STYLE_NEW_BY_EXAMPLE)
                    {
                        pStyleSheet = &pStylePool->Make( aStyleName, eFamily, SfxStyleSearchBits::UserDefined );

                        // when a style is present, then this will become
                        // the parent of the new style:
                        if (SfxStyleSheet* pOldStyle = GetDrawView()->GetStyleSheet())
                            pStyleSheet->SetParent(pOldStyle->GetName());
                    }
                    else
                    {
                        pStyleSheet = GetDrawView()->GetStyleSheet();
                        aOldData.InitFromStyle( pStyleSheet );
                    }

                    if ( bUndo )
                    {
                        OUString aUndo = ScResId( STR_UNDO_EDITGRAPHICSTYLE );
                        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewShellId() );
                        bListAction = true;
                    }

                    SfxItemSet aCoreSet(GetDrawView()->GetModel().GetItemPool());
                    GetDrawView()->GetAttributes(aCoreSet, true);

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put(aCoreSet);
                    static_cast<SfxStyleSheet*>(pStyleSheet)->Broadcast(SfxHint(SfxHintId::DataChanged));

                    aNewData.InitFromStyle( pStyleSheet );
                    bAddUndo = true;

                    //  call SetStyleSheet after adding the ScUndoModifyStyle
                    //  (pStyleSheet pointer is used!)
                    bStyleToMarked = true;
                    rReq.Done();
                }
                break;
                default:
                    break;
            }
        }
        break;
        default:
            break;
    } // switch ( eFamily )

    // create new or process through Dialog:
    if ( nSlotId == SID_STYLE_NEW || nSlotId == SID_STYLE_EDIT )
    {
        if (pStyleSheet)
        {
            ExecuteStyleEdit(rReq, pStyleSheet, nRetMask, nSlotId, bAddUndo, bUndo,
                    aOldData, aNewData, eFamily, bStyleToMarked, bListAction, pEditObject, aSelection);
            return; // skip calling ExecuteStyleEditPost because we invoked an async dialog
        }
    }

    ExecuteStyleEditPost(rReq, pStyleSheet, nSlotId, nRetMask, bAddUndo, bUndo,
            eFamily, aOldData, aNewData, bStyleToMarked, bListAction, pEditObject, aSelection);
}

void ScTabViewShell::ExecuteStyleEdit(SfxRequest& rReq, SfxStyleSheetBase* pStyleSheet, sal_uInt16 nRetMask,
                        sal_uInt16 nSlotId, bool bAddUndo, bool bUndo,
                        ScStyleSaveData& rOldData, ScStyleSaveData& rNewData, SfxStyleFamily eFamily,
                        bool bStyleToMarked, bool bListAction,
                        SdrObject* pEditObject, ESelection aSelection)
{
    ScDocShell*     pDocSh      = GetViewData().GetDocShell();
    ScDocument&     rDoc        = pDocSh->GetDocument();
    SfxStyleFamily  eFam = pStyleSheet->GetFamily();
    VclPtr<SfxAbstractTabDialog> pDlg;
    bool bPage = false;

    // Store old Items from the style
    std::shared_ptr<SfxItemSet> xOldSet = std::make_shared<SfxItemSet>(pStyleSheet->GetItemSet());
    OUString aOldName = pStyleSheet->GetName();

    switch ( eFam )
    {
        case SfxStyleFamily::Page:
            bPage = true;
            break;

        case SfxStyleFamily::Para:
            {
                SfxItemSet& rSet = pStyleSheet->GetItemSet();

                if ( const SfxUInt32Item* pItem = rSet.GetItemIfSet( ATTR_VALUE_FORMAT,
                        false ) )
                {
                    // Produce and format NumberFormat Value from Value and Language
                    sal_uLong nFormat = pItem->GetValue();
                    LanguageType eLang =
                        rSet.Get(ATTR_LANGUAGE_FORMAT ).GetLanguage();
                    sal_uLong nLangFormat = rDoc.GetFormatTable()->
                        GetFormatForLanguageIfBuiltIn( nFormat, eLang );
                    if ( nLangFormat != nFormat )
                    {
                        SfxUInt32Item aNewItem( ATTR_VALUE_FORMAT, nLangFormat );
                        rSet.Put( aNewItem );
                        xOldSet->Put( aNewItem );
                        // Also in aOldSet for comparison after the  dialog,
                        // Otherwise might miss a language change
                    }
                }

                std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem(
                    ScTabViewShell::MakeNumberInfoItem(rDoc, GetViewData()));

                pDocSh->PutItem( *pNumberInfoItem );
                bPage = false;

                // Definitely a SvxBoxInfoItem with Table = sal_False in set:
                // (If there is no item, the dialogue will also delete the
                // BORDER_OUTER SvxBoxItem from the Template Set)
                if ( rSet.GetItemState( ATTR_BORDER_INNER, false ) != SfxItemState::SET )
                {
                    SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                    aBoxInfoItem.SetTable(false);       // no inner lines
                    aBoxInfoItem.SetDist(true);
                    aBoxInfoItem.SetMinDist(false);
                    rSet.Put( aBoxInfoItem );
                }
            }
            break;

        case SfxStyleFamily::Frame:
        default:
            break;
    }

    SetInFormatDialog(true);

    SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
    rStyleSet.MergeRange( XATTR_FILL_FIRST, XATTR_FILL_LAST );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    weld::Window* pDialogParent = rReq.GetFrameWeld();
    if (!pDialogParent)
        pDialogParent = GetFrameWeld();

    if (eFam == SfxStyleFamily::Frame)
        pDlg = pFact->CreateScDrawStyleDlg(pDialogParent, *pStyleSheet, GetDrawView());
    else
        pDlg = pFact->CreateScStyleDlg(pDialogParent, *pStyleSheet, bPage);

    auto xRequest = std::make_shared<SfxRequest>(rReq);
    rReq.Ignore(); // the 'old' request is not relevant any more
    pDlg->StartExecuteAsync(
        [this, pDlg, xRequest=std::move(xRequest), pStyleSheet, nRetMask, xOldSet, nSlotId, bAddUndo, bUndo,
            aOldData=rOldData, aNewData=rNewData, aOldName, eFamily, bStyleToMarked, bListAction,
            pEditObject, aSelection]
        (sal_Int32 nResult) mutable -> void
        {
            SetInFormatDialog(false);
            ExecuteStyleEditDialog(pDlg, pStyleSheet, nResult, nRetMask, xOldSet, nSlotId,
                    bAddUndo, aNewData, aOldName);
            pDlg->disposeOnce();
            ExecuteStyleEditPost(*xRequest, pStyleSheet, nSlotId, nRetMask, bAddUndo, bUndo, eFamily,
                    aOldData, aNewData, bStyleToMarked, bListAction, pEditObject, aSelection);
        }
    );
}

void ScTabViewShell::ExecuteStyleEditDialog(VclPtr<SfxAbstractTabDialog> pDlg,
                        SfxStyleSheetBase* pStyleSheet, sal_uInt16 nResult,
                        sal_uInt16& rnRetMask, std::shared_ptr<SfxItemSet> xOldSet, const sal_uInt16 nSlotId,
                        bool& rbAddUndo, ScStyleSaveData& rNewData, std::u16string_view aOldName)
{
    ScDocShell*     pDocSh = GetViewData().GetDocShell();
    ScDocument&     rDoc = pDocSh->GetDocument();
    SfxBindings&    rBindings = GetViewData().GetBindings();
    SfxStyleSheetBasePool* pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleFamily  eFam = pStyleSheet->GetFamily();
    if ( nResult == RET_OK )
    {
        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

        if ( pOutSet )
        {
            rnRetMask = sal_uInt16(pStyleSheet->GetMask());

            // Attribute comparisons (earlier in ModifyStyleSheet) now here
            // with the old values (the style is already changed)
            if ( SfxStyleFamily::Para == eFam )
            {
                SfxItemSet& rNewSet = pStyleSheet->GetItemSet();
                bool bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate(
                                    bNumFormatChanged, rNewSet, *xOldSet ) )
                    rDoc.InvalidateTextWidth( nullptr, nullptr, bNumFormatChanged );

                SCTAB nTabCount = rDoc.GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    rDoc.SetStreamValid(nTab, false);

                sal_uLong nOldFormat = xOldSet->Get( ATTR_VALUE_FORMAT ).GetValue();
                sal_uLong nNewFormat = rNewSet.Get( ATTR_VALUE_FORMAT ).GetValue();
                if ( nNewFormat != nOldFormat )
                {
                    ScInterpreterContext& rContext = rDoc.GetNonThreadedContext();
                    const SvNumberformat* pOld = rContext.NFGetFormatEntry( nOldFormat );
                    const SvNumberformat* pNew = rContext.NFGetFormatEntry( nNewFormat );
                    if ( pOld && pNew && pOld->GetLanguage() != pNew->GetLanguage() )
                        rNewSet.Put( SvxLanguageItem(
                                        pNew->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
                }

                rDoc.getCellAttributeHelper().CellStyleCreated(rDoc, pStyleSheet->GetName());
            }
            else if ( SfxStyleFamily::Page == eFam )
            {
                //! Here also queries for Page Styles

                OUString aNewName = pStyleSheet->GetName();
                if ( aNewName != aOldName &&
                        rDoc.RenamePageStyleInUse( aOldName, aNewName ) )
                {
                    rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                    rBindings.Invalidate( FID_RESET_PRINTZOOM );
                }

                rDoc.ModifyStyleSheet( *pStyleSheet, *pOutSet );
                rBindings.Invalidate( FID_RESET_PRINTZOOM );
            }
            else
            {
                SfxItemSet& rAttr = pStyleSheet->GetItemSet();
                sdr::properties::CleanupFillProperties(rAttr);

                // check for unique names of named items for xml
                auto checkForUniqueItem = [&] (auto nWhichId)
                {
                    if (auto pOldItem = rAttr.GetItemIfSet(nWhichId, false))
                    {
                        if (auto pNewItem = pOldItem->checkForUniqueItem(&GetDrawView()->GetModel()))
                            rAttr.Put(std::move(pNewItem));
                    }
                };

                checkForUniqueItem(XATTR_FILLBITMAP);
                checkForUniqueItem(XATTR_LINEDASH);
                checkForUniqueItem(XATTR_LINESTART);
                checkForUniqueItem(XATTR_LINEEND);
                checkForUniqueItem(XATTR_FILLGRADIENT);
                checkForUniqueItem(XATTR_FILLFLOATTRANSPARENCE);
                checkForUniqueItem(XATTR_FILLHATCH);

                static_cast<SfxStyleSheet*>(pStyleSheet)->Broadcast(SfxHint(SfxHintId::DataChanged));
                GetScDrawView()->InvalidateAttribs();
            }

            pDocSh->SetDocumentModified();

            if ( SfxStyleFamily::Para == eFam )
            {
                ScTabViewShell::UpdateNumberFormatter(
                        *( pDocSh->GetItem(SID_ATTR_NUMBERFORMAT_INFO) ));

                UpdateStyleSheetInUse( pStyleSheet );
                InvalidateAttribs();
            }

            rNewData.InitFromStyle( pStyleSheet );
            rbAddUndo = true;
        }
    }
    else
    {
        if ( nSlotId == SID_STYLE_NEW )
            pStylePool->Remove( pStyleSheet );
        else
        {
            // If in the meantime something was painted with the
            // temporary changed item set
            pDocSh->PostPaintGridAll();
        }
    }
}

void ScTabViewShell::ExecuteStyleEditPost(SfxRequest& rReq, SfxStyleSheetBase* pStyleSheet,
                        sal_uInt16 nSlotId, sal_uInt16 nRetMask, bool bAddUndo, bool bUndo,
                        const SfxStyleFamily eFamily,
                        ScStyleSaveData& rOldData, ScStyleSaveData& rNewData,
                        bool bStyleToMarked, bool bListAction,
                        SdrObject* pEditObject, ESelection aSelection)
{
    ScDocShell*     pDocSh = GetViewData().GetDocShell();

    rReq.SetReturnValue( SfxUInt16Item( nSlotId, nRetMask ) );

    if ( bAddUndo && bUndo)
        pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoModifyStyle>( pDocSh, eFamily, rOldData, rNewData ) );

    if ( bStyleToMarked )
    {
        //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle,
        //  so redo will find the modified style
        if (eFamily == SfxStyleFamily::Para)
        {
            SetStyleSheetToMarked( static_cast<SfxStyleSheet*>(pStyleSheet) );
        }
        else if (eFamily == SfxStyleFamily::Frame)
        {
            GetScDrawView()->ScEndTextEdit();
            GetScDrawView()->SetStyleSheet( static_cast<SfxStyleSheet*>(pStyleSheet), false );
        }
        InvalidateAttribs();
    }

    if ( bListAction )
        pDocSh->GetUndoManager()->LeaveListAction();

    // The above call to ScEndTextEdit left us in an inconsistent state:
    // Text editing isn't active, but the text edit shell still is. And we
    // couldn't just deactivate it fully, because in case of editing a
    // comment, that will make the comment disappear. So let's try to
    // reactivate text editing instead:
    auto pFuText = dynamic_cast<FuText*>(GetDrawFuncPtr());
    if (pFuText && pEditObject != GetDrawView()->GetTextEditObject())
    {
        pFuText->SetInEditMode(pEditObject);
        if (GetDrawView()->GetTextEditOutlinerView())
            GetDrawView()->GetTextEditOutlinerView()->SetSelection(aSelection);
    }
}

void ScTabViewShell::GetStyleState( SfxItemSet& rSet )
{
    ScDocument&             rDoc          = GetViewData().GetDocument();
    SfxStyleSheetBasePool*  pStylePool    = rDoc.GetStyleSheetPool();

    bool bProtected = false;
    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB i=0; i<nTabCount && !bProtected; i++)
        if (rDoc.IsTabProtected(i))                // look after protected table
            bProtected = true;

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();
    sal_uInt16          nSlotId = 0;

    while ( nWhich )
    {
        nSlotId = SfxItemPool::IsWhich( nWhich )
                    ? GetPool().GetSlotId( nWhich )
                    : nWhich;

        switch ( nSlotId )
        {
            case SID_STYLE_APPLY:
                if ( !pStylePool )
                    rSet.DisableItem( nSlotId );
                break;

            case SID_STYLE_FAMILY2:     // cell style sheets
            {
                SfxStyleSheet* pStyleSheet = const_cast<SfxStyleSheet*>(GetStyleSheetFromMarked());

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, pStyleSheet->GetName() ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, OUString() ) );
            }
            break;

            case SID_STYLE_FAMILY3:     // drawing style sheets
            {
                SfxStyleSheet* pStyleSheet = GetDrawView()->GetStyleSheet();

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, pStyleSheet->GetName() ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, OUString() ) );
            }
            break;

            case SID_STYLE_FAMILY4:     // page style sheets
            {
                SCTAB           nCurTab     = GetViewData().GetTabNo();
                OUString        aPageStyle  = rDoc.GetPageStyle( nCurTab );
                SfxStyleSheet*  pStyleSheet = pStylePool ? static_cast<SfxStyleSheet*>(pStylePool->
                                    Find( aPageStyle, SfxStyleFamily::Page )) : nullptr;

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, aPageStyle ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, OUString() ) );
            }
            break;

            case SID_STYLE_WATERCAN:
            {
                rSet.Put( SfxBoolItem( nSlotId, SC_MOD()->GetIsWaterCan() ) );
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                std::unique_ptr<SfxUInt16Item> pFamilyItem;
                GetViewFrame().GetBindings().QueryState(SID_STYLE_FAMILY, pFamilyItem);

                bool bPage = pFamilyItem && SfxStyleFamily::Page == static_cast<SfxStyleFamily>(pFamilyItem->GetValue());

                if ( bProtected || bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            {
                std::unique_ptr<SfxUInt16Item> pFamilyItem;
                GetViewFrame().GetBindings().QueryState(SID_STYLE_FAMILY, pFamilyItem);
                bool bPage = pFamilyItem && SfxStyleFamily::Page == static_cast<SfxStyleFamily>(pFamilyItem->GetValue());

                if ( bProtected && !bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            default:
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
