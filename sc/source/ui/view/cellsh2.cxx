/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/moduleoptions.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

#include "cellsh.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "globalnames.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "dbfunc.hxx"
#include "dbdocfun.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#include "reffact.hxx"
#include "validat.hxx"
#include "scresid.hxx"

#include "validate.hrc"
#include "scui_def.hxx"
#include "scabstdlg.hxx"
#include "impex.hxx"
#include "asciiopt.hxx"
#include "datastream.hxx"
#include "datastreamdlg.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include <documentlinkmgr.hxx>

#include <config_orcus.h>

using namespace com::sun::star;

static bool lcl_GetTextToColumnsRange( const ScViewData* pData, ScRange& rRange )
{
    OSL_ENSURE( pData, "lcl_GetTextToColumnsRange: pData is null!" );

    bool bRet = false;
    const ScMarkData& rMark = pData->GetMarkData();

    if ( rMark.IsMarked() )
    {
        if ( !rMark.IsMultiMarked() )
        {
            rMark.GetMarkArea( rRange );
            if ( rRange.aStart.Col() == rRange.aEnd.Col() )
            {
                bRet = true;
            }
        }
    }
    else
    {
        const SCCOL nCol = pData->GetCurX();
        const SCROW nRow = pData->GetCurY();
        const SCTAB nTab = pData->GetTabNo();
        rRange = ScRange( nCol, nRow, nTab, nCol, nRow, nTab );
        bRet = true;
    }

    const ScDocument* pDoc = pData->GetDocument();
    OSL_ENSURE( pDoc, "lcl_GetTextToColumnsRange: pDoc is null!" );

    if ( bRet && pDoc->IsBlockEmpty( rRange.aStart.Tab(), rRange.aStart.Col(),
                                     rRange.aStart.Row(), rRange.aEnd.Col(),
                                     rRange.aEnd.Row() ) )
    {
        bRet = false;
    }

    return bRet;
}

static sal_Bool lcl_GetSortParam( const ScViewData* pData, ScSortParam& rSortParam )
{
    ScTabViewShell* pTabViewShell   = pData->GetViewShell();
    ScDBData*   pDBData             = pTabViewShell->GetDBData();
    ScDocument* pDoc                = pData->GetDocument();
    SCTAB nTab                      = pData->GetTabNo();
    ScDirection eFillDir            = DIR_TOP;
    sal_Bool  bSort                     = sal_True;
    ScRange aExternalRange;

    if( rSortParam.nCol1 != rSortParam.nCol2 )
        eFillDir = DIR_LEFT;
    if( rSortParam.nRow1 != rSortParam.nRow2 )
        eFillDir = DIR_TOP;

    if( rSortParam.nRow2 == MAXROW )
    {
        
        
        SCSIZE nCount = pDoc->GetEmptyLinesInBlock( rSortParam.nCol1, rSortParam.nRow1, nTab,
                                                    rSortParam.nCol2, rSortParam.nRow2, nTab, eFillDir );
        aExternalRange = ScRange( rSortParam.nCol1,
                ::std::min( rSortParam.nRow1 + sal::static_int_cast<SCROW>( nCount ), MAXROW), nTab );
    }
    else
        aExternalRange = ScRange( pData->GetCurX(), pData->GetCurY(), nTab );

    SCROW nStartRow = aExternalRange.aStart.Row();
    SCCOL nStartCol = aExternalRange.aStart.Col();
    SCROW nEndRow   = aExternalRange.aEnd.Row();
    SCCOL nEndCol   = aExternalRange.aEnd.Col();
    pDoc->GetDataArea( aExternalRange.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow, false, false );
    aExternalRange.aStart.SetRow( nStartRow );
    aExternalRange.aStart.SetCol( nStartCol );
    aExternalRange.aEnd.SetRow( nEndRow );
    aExternalRange.aEnd.SetCol( nEndCol );

    if(( rSortParam.nCol1 == rSortParam.nCol2 && aExternalRange.aStart.Col() != aExternalRange.aEnd.Col() ) ||
        ( rSortParam.nRow1 == rSortParam.nRow2 && aExternalRange.aStart.Row() != aExternalRange.aEnd.Row() ) )
    {
        sal_uInt16 nFmt = SCA_VALID;

        pTabViewShell->AddHighlightRange( aExternalRange,Color( COL_LIGHTBLUE ) );
        ScRange rExtendRange( aExternalRange.aStart.Col(), aExternalRange.aStart.Row(), nTab, aExternalRange.aEnd.Col(), aExternalRange.aEnd.Row(), nTab );
        OUString aExtendStr(rExtendRange.Format(nFmt, pDoc));

        ScRange rCurrentRange( rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab );
        OUString aCurrentStr(rCurrentRange.Format(nFmt, pDoc));

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

        AbstractScSortWarningDlg* pWarningDlg = pFact->CreateScSortWarningDlg( pTabViewShell->GetDialogParent(), aExtendStr, aCurrentStr );
        OSL_ENSURE(pWarningDlg, "Dialog create fail!");
        short bResult = pWarningDlg->Execute();
        if( bResult == BTN_EXTEND_RANGE || bResult == BTN_CURRENT_SELECTION )
        {
            if( bResult == BTN_EXTEND_RANGE )
            {
                pTabViewShell->MarkRange( aExternalRange, false );
                pDBData->SetArea( nTab, aExternalRange.aStart.Col(), aExternalRange.aStart.Row(), aExternalRange.aEnd.Col(), aExternalRange.aEnd.Row() );
            }
        }
        else
        {
            bSort = false;
            pData->GetDocShell()->CancelAutoDBRange();
        }

        delete pWarningDlg;
        pTabViewShell->ClearHighlightRanges();
    }
    return bSort;
}


namespace
{
    long DelayDeleteAbstractDialog( void *pAbstractDialog, void * /*pArg*/ )
    {
        delete reinterpret_cast<SfxAbstractTabDialog*>( pAbstractDialog );
        return 0;
    }
}

void ScCellShell::ExecuteDB( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    ScModule*           pScMod      = SC_MOD();

    pTabViewShell->HideListBox();                   

    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        pScMod->InputEnterHandler();
        pTabViewShell->UpdateInputHandler();
    }

    switch ( nSlotId )
    {
        case SID_VIEW_DATA_SOURCE_BROWSER:
            {
                

                SfxViewFrame* pViewFrame = pTabViewShell->GetViewFrame();
                sal_Bool bWasOpen = false;
                {
                    uno::Reference<frame::XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();
                    uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
                                                        OUString("_beamer"),
                                                        frame::FrameSearchFlag::CHILDREN);
                    if ( xBeamerFrame.is() )
                        bWasOpen = sal_True;
                }

                if ( bWasOpen )
                {
                    

                    pViewFrame->ExecuteSlot( rReq );
                }
                else
                {
                    

                    pViewFrame->ExecuteSlot( rReq, (sal_Bool) false );      

                    

                    ScImportParam aImportParam;
                    ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);       
                    if (pDBData)
                        pDBData->GetImportParam( aImportParam );

                    ScDBDocFunc::ShowInBeamer( aImportParam, pTabViewShell->GetViewFrame() );
                }
                rReq.Done();        
            }
            break;

        case SID_REIMPORT_DATA:
            {
                sal_Bool bOk = false;
                ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);
                if (pDBData)
                {
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    
                        bOk = sal_True;
                    }
                }

                if (!bOk && ! rReq.IsAPI() )
                    pTabViewShell->ErrorMessage(STR_REIMPORT_EMPTY);

                if( bOk )
                    rReq.Done();
            }
            break;

        case SID_REFRESH_DBAREA:
            {
                ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);
                if (pDBData)
                {
                    

                    sal_Bool bContinue = sal_True;
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        bContinue = pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    

                        
                        ScRange aNewRange;
                        pDBData->GetArea(aNewRange);
                        pTabViewShell->MarkRange(aNewRange);
                    }

                    if ( bContinue )        
                    {
                        

                        if ( pDBData->HasQueryParam() || pDBData->HasSortParam() ||
                                                          pDBData->HasSubTotalParam() )
                            pTabViewShell->RepeatDB();

                        

                        ScRange aRange;
                        pDBData->GetArea(aRange);
                        GetViewData()->GetDocShell()->RefreshPivotTables(aRange);
                    }
                }
                rReq.Done();
            }
            break;

        case SID_SBA_BRW_INSERT:
            {
                OSL_FAIL( "Deprecated Slot" );
            }
            break;

        case SID_DATA_FORM:
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                AbstractScDataFormDlg* pDlg = pFact->CreateScDataFormDlg(
                    pTabViewShell->GetDialogParent(), pTabViewShell);
                OSL_ENSURE(pDlg, "Dialog create fail!");

                pDlg->Execute();

                rReq.Done();
            }
            break;

        case SID_SUBTOTALS:
            ExecuteSubtotals(rReq);
            break;

        case SID_SORT_DESCENDING:
        case SID_SORT_ASCENDING:
            {
                
                
                ScSortParam aSortParam;
                ScDBData*   pDBData = pTabViewShell->GetDBData();
                ScViewData* pData   = GetViewData();

                pDBData->GetSortParam( aSortParam );

                if( lcl_GetSortParam( pData, aSortParam ) )
                {
                    SfxItemSet  aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );
                    SCCOL nCol  = GetViewData()->GetCurX();
                    SCCOL nTab  = GetViewData()->GetTabNo();
                    ScDocument* pDoc    = GetViewData()->GetDocument();

                    pDBData->GetSortParam( aSortParam );
                    sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, nTab );

                    if( nCol < aSortParam.nCol1 )
                        nCol = aSortParam.nCol1;
                    else if( nCol > aSortParam.nCol2 )
                        nCol = aSortParam.nCol2;

                    aSortParam.bHasHeader       = bHasHeader;
                    aSortParam.bByRow           = true;
                    aSortParam.bCaseSens        = false;
                    aSortParam.bNaturalSort     = false;
                    aSortParam.bIncludePattern  = true;
                    aSortParam.bInplace         = true;
                    aSortParam.maKeyState[0].bDoSort = true;
                    aSortParam.maKeyState[0].nField = nCol;
                    aSortParam.maKeyState[0].bAscending = ( nSlotId == SID_SORT_ASCENDING );

                    for ( sal_uInt16 i=1; i<aSortParam.GetSortKeyCount(); i++ )
                        aSortParam.maKeyState[i].bDoSort = false;

                    aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );

                    pTabViewShell->UISort( aSortParam );        

                    rReq.Done();
                }
            }
            break;

        case SID_SORT:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                
                

                if ( pArgs )        
                {
                    ScSortParam aSortParam;
                    ScDBData*   pDBData = pTabViewShell->GetDBData();
                    ScViewData* pData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( pData, aSortParam ) )
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();

                        pDBData->GetSortParam( aSortParam );
                        sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, pData->GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aSortParam.bInplace = true;             

                        const SfxPoolItem* pItem;
                        if ( pArgs->GetItemState( SID_SORT_BYROW, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bByRow = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_HASHEADER, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bHasHeader = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_CASESENS, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bCaseSens = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_NATURALSORT, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bNaturalSort = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_ATTRIBS, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bIncludePattern = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_USERDEF, true, &pItem ) == SFX_ITEM_SET )
                        {
                            sal_uInt16 nUserIndex = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                            aSortParam.bUserDef = ( nUserIndex != 0 );
                            if ( nUserIndex )
                                aSortParam.nUserIndex = nUserIndex - 1;     
                        }

                        SCCOLROW nField0 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SFX_ITEM_SET )
                            nField0 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[0].bDoSort = ( nField0 != 0 );
                        aSortParam.maKeyState[0].nField = nField0 > 0 ? (nField0-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.maKeyState[0].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        SCCOLROW nField1 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_3, true, &pItem ) == SFX_ITEM_SET )
                            nField1 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[1].bDoSort = ( nField1 != 0 );
                        aSortParam.maKeyState[1].nField = nField1 > 0 ? (nField1-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_4, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.maKeyState[1].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        SCCOLROW nField2 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_5, true, &pItem ) == SFX_ITEM_SET )
                            nField2 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[2].bDoSort = ( nField2 != 0 );
                        aSortParam.maKeyState[2].nField = nField2 > 0 ? (nField2-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_6, true, &pItem ) == SFX_ITEM_SET )
                            aSortParam.maKeyState[2].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                        
                        pTabViewShell->UISort( aSortParam );
                        rReq.Done();
                    }
                }
                else
                {
                    ScSortParam aSortParam;
                    ScDBData*   pDBData = pTabViewShell->GetDBData();
                    ScViewData* pData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( pData, aSortParam ) )
                    {
                        SfxAbstractTabDialog* pDlg = NULL;
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        SfxItemSet  aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );

                        pDBData->GetSortParam( aSortParam );
                        sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, pData->GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        assert(pFact); 

                        pDlg = pFact->CreateScSortDlg(pTabViewShell->GetDialogParent(),  &aArgSet);
                        assert(pDlg); 
                        pDlg->SetCurPageId("criteria");  

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            const ScSortParam& rOutParam = ((const ScSortItem&)
                                pOutSet->Get( SCITEM_SORTDATA )).GetSortData();

                            

                            pTabViewShell->UISort( rOutParam );

                            if ( rOutParam.bInplace )
                            {
                                rReq.AppendItem( SfxBoolItem( SID_SORT_BYROW,
                                    rOutParam.bByRow ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_HASHEADER,
                                    rOutParam.bHasHeader ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_CASESENS,
                                    rOutParam.bCaseSens ) );
                            rReq.AppendItem( SfxBoolItem( SID_SORT_NATURALSORT,
                                                rOutParam.bNaturalSort ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_ATTRIBS,
                                    rOutParam.bIncludePattern ) );
                                sal_uInt16 nUser = rOutParam.bUserDef ? ( rOutParam.nUserIndex + 1 ) : 0;
                                rReq.AppendItem( SfxUInt16Item( SID_SORT_USERDEF, nUser ) );
                                if ( rOutParam.maKeyState[0].bDoSort )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_1,
                                        rOutParam.maKeyState[0].nField + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_2,
                                        rOutParam.maKeyState[0].bAscending ) );
                                }
                                if ( rOutParam.maKeyState[1].bDoSort )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_3,
                                        rOutParam.maKeyState[1].nField + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_4,
                                        rOutParam.maKeyState[1].bAscending ) );
                                }
                                if ( rOutParam.maKeyState[2].bDoSort )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_5,
                                        rOutParam.maKeyState[2].nField + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_6,
                                        rOutParam.maKeyState[2].bAscending ) );
                                }
                            }

                            rReq.Done();
                        }
                        else
                            GetViewData()->GetDocShell()->CancelAutoDBRange();

                        delete pDlg;
                    }
                }
            }
            break;

        case SID_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("SID_FILTER with arguments?");
                    pTabViewShell->Query( ((const ScQueryItem&)
                            pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, true );
                    rReq.Done();
                }
                else
                {
                    sal_uInt16          nId  = ScFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
                }
            }
            break;

        case SID_SPECIAL_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("SID_SPECIAL_FILTER with arguments?");
                    pTabViewShell->Query( ((const ScQueryItem&)
                            pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, true );
                    rReq.Done();
                }
                else
                {
                    sal_uInt16          nId  = ScSpecialFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
                }
            }
            break;

        case FID_FILTER_OK:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET ==
                        pReqArgs->GetItemState( SCITEM_QUERYDATA, true, &pItem ) )
                {
                    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(*pItem);

                    SCTAB nCurTab = GetViewData()->GetTabNo();
                    SCTAB nRefTab = GetViewData()->GetRefTabNo();

                    
                    

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    ScRange aAdvSource;
                    if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
                        pTabViewShell->Query( rQueryItem.GetQueryData(), &aAdvSource, true );
                    else
                        pTabViewShell->Query( rQueryItem.GetQueryData(), NULL, true );
                    rReq.Done( *pReqArgs );
                }
            }
            break;

        case SID_UNFILTER:
            {
                ScQueryParam aParam;
                ScDBData*    pDBData = pTabViewShell->GetDBData();

                pDBData->GetQueryParam( aParam );
                SCSIZE nEC = aParam.GetEntryCount();
                for (SCSIZE i=0; i<nEC; i++)
                    aParam.GetEntry(i).bDoQuery = false;
                aParam.bDuplicate = true;
                pTabViewShell->Query( aParam, NULL, true );
                rReq.Done();
            }
            break;

        case SID_AUTO_FILTER:
            pTabViewShell->ToggleAutoFilter();
            rReq.Done();
            break;

        case SID_AUTOFILTER_HIDE:
            pTabViewShell->HideAutoFilter();
            rReq.Done();
            break;

        case SID_PIVOT_TABLE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET ==
                        pReqArgs->GetItemState( SCITEM_PIVOTDATA, true, &pItem ) )
                {
                    SCTAB nCurTab = GetViewData()->GetTabNo();
                    SCTAB nRefTab = GetViewData()->GetRefTabNo();

                    
                    

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    const ScDPObject* pDPObject = pTabViewShell->GetDialogDPObject();
                    if ( pDPObject )
                    {
                        const ScPivotItem* pPItem = (const ScPivotItem*)pItem;
                        bool bSuccess = pTabViewShell->MakePivotTable(
                            pPItem->GetData(), pPItem->GetDestRange(), pPItem->IsNewSheet(), *pDPObject );
                        SfxBoolItem aRet(0, bSuccess);
                        rReq.SetReturnValue(aRet);
                    }
                    rReq.Done();
                }
#ifndef DISABLE_SCRIPTING
                else if (rReq.IsAPI())
                    SbxBase::SetError(SbxERR_BAD_PARAMETER);
#endif
            }
            break;

        case SID_OPENDLG_PIVOTTABLE:
            ExecuteDataPilotDialog();
            break;
        case SID_DEFINE_DBNAME:
            {

                sal_uInt16          nId  = ScDbNameDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );

            }
            break;

        case SID_SELECT_DB:
            {
                if ( pReqArgs )
                {
                    const SfxStringItem* pItem =
                        (const SfxStringItem*)&pReqArgs->Get( SID_SELECT_DB );

                    if( pItem )
                    {
                        pTabViewShell->GotoDBArea( pItem->GetValue() );
                        rReq.Done();
                    }
                    else
                    {
                        OSL_FAIL("NULL");
                    }
                }
                else
                {
                    ScDocument*     pDoc   = GetViewData()->GetDocument();
                    ScDBCollection* pDBCol = pDoc->GetDBCollection();

                    if ( pDBCol )
                    {
                        std::vector<OUString> aList;
                        const ScDBCollection::NamedDBs& rDBs = pDBCol->getNamedDBs();
                        ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
                        for (; itr != itrEnd; ++itr)
                            aList.push_back(itr->GetName());

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScSelEntryDlg* pDlg = pFact->CreateScSelEntryDlg( pTabViewShell->GetDialogParent(),
                                                                                aList );
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                        {
                            OUString aName = pDlg->GetSelectEntry();
                            pTabViewShell->GotoDBArea( aName );
                            rReq.AppendItem( SfxStringItem( SID_SELECT_DB, aName ) );
                            rReq.Done();
                        }

                        delete pDlg;
                    }
                }
            }
            break;
        case SID_DATA_STREAMS:
        {
            sc::DataStreamDlg aDialog( GetViewData()->GetDocShell(), pTabViewShell->GetDialogParent() );
            ScDocument *pDoc = GetViewData()->GetDocument();
            sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                aDialog.Init(*pStrm);

            if (aDialog.Execute() == RET_OK)
                aDialog.StartStream();
        }
        break;
        case SID_DATA_STREAMS_PLAY:
        {
            ScDocument *pDoc = GetViewData()->GetDocument();
            sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                pStrm->StartImport();
        }
        break;
        case SID_DATA_STREAMS_STOP:
        {
            ScDocument *pDoc = GetViewData()->GetDocument();
            sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                pStrm->StopImport();
        }
        break;
        case SID_MANAGE_XML_SOURCE:
            ExecuteXMLSourceDialog();
        break;
        case FID_VALIDATION:
            {
                const SfxPoolItem* pItem;
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("later...");
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
                    ::GetTabPageRanges ScTPValidationValueGetRanges = pFact->GetTabPageRangesFunc(TP_VALIDATION_VALUES);
                    OSL_ENSURE(ScTPValidationValueGetRanges, "TabPage create fail!");
                    SfxItemSet aArgSet( GetPool(), (*ScTPValidationValueGetRanges)() );
                    ScValidationMode eMode = SC_VALID_ANY;
                    ScConditionMode eOper = SC_COND_EQUAL;
                    OUString aExpr1, aExpr2;
                    sal_Bool bBlank = sal_True;
                    sal_Int16 nListType = ValidListType::UNSORTED;
                    sal_Bool bShowHelp = false;
                    OUString aHelpTitle, aHelpText;
                    sal_Bool bShowError = false;
                    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
                    OUString aErrTitle, aErrText;

                    ScDocument* pDoc = GetViewData()->GetDocument();
                    SCCOL nCurX = GetViewData()->GetCurX();
                    SCROW nCurY = GetViewData()->GetCurY();
                    SCTAB nTab = GetViewData()->GetTabNo();
                    ScAddress aCursorPos( nCurX, nCurY, nTab );
                    sal_uLong nIndex = ((SfxUInt32Item*)pDoc->GetAttr(
                                nCurX, nCurY, nTab, ATTR_VALIDDATA ))->GetValue();
                    if ( nIndex )
                    {
                        const ScValidationData* pOldData = pDoc->GetValidationEntry( nIndex );
                        if ( pOldData )
                        {
                            eMode = pOldData->GetDataMode();
                            eOper = pOldData->GetOperation();
                            sal_uLong nNumFmt = 0;
                            if ( eMode == SC_VALID_DATE || eMode == SC_VALID_TIME )
                            {
                                short nType = ( eMode == SC_VALID_DATE ) ? NUMBERFORMAT_DATE
                                                                         : NUMBERFORMAT_TIME;
                                nNumFmt = pDoc->GetFormatTable()->GetStandardFormat(
                                                                    nType, ScGlobal::eLnge );
                            }
                            aExpr1 = pOldData->GetExpression( aCursorPos, 0, nNumFmt );
                            aExpr2 = pOldData->GetExpression( aCursorPos, 1, nNumFmt );
                            bBlank = pOldData->IsIgnoreBlank();
                            nListType = pOldData->GetListType();

                            bShowHelp = pOldData->GetInput( aHelpTitle, aHelpText );
                            bShowError = pOldData->GetErrMsg( aErrTitle, aErrText, eErrStyle );

                            aArgSet.Put( SfxAllEnumItem( FID_VALID_MODE,        sal::static_int_cast<sal_uInt16>(eMode) ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE,    sal::static_int_cast<sal_uInt16>(eOper) ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE1,      aExpr1 ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE2,      aExpr2 ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_BLANK,       bBlank ) );
                            aArgSet.Put( SfxInt16Item(   FID_VALID_LISTTYPE,    nListType ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWHELP,    bShowHelp ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTITLE,   aHelpTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTEXT,    aHelpText ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWERR,     bShowError ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE,    sal::static_int_cast<sal_uInt16>(eErrStyle) ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTITLE,    aErrTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTEXT,     aErrText ) );
                        }
                    }

                    
                    SfxAbstractTabDialog* pDlg = pFact->CreateScValidationDlg(NULL, &aArgSet, pTabViewShell);
                    assert(pDlg); 

                    short nResult = pDlg->Execute();
                    
                    pTabViewShell->SetTabNo( nTab );
                    if ( nResult == RET_OK )
                    {
                        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                        if ( pOutSet->GetItemState( FID_VALID_MODE, true, &pItem ) == SFX_ITEM_SET )
                            eMode = (ScValidationMode) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_CONDMODE, true, &pItem ) == SFX_ITEM_SET )
                            eOper = (ScConditionMode) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_VALUE1, true, &pItem ) == SFX_ITEM_SET )
                        {
                            OUString aTemp1 = ((const SfxStringItem*)pItem)->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (pDoc->GetFormatTable()->IsNumberFormat(aTemp1, nNumIndex, nVal))
                                    aExpr1 = OUString( ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::pLocaleData->getNumDecimalSep()[0], true));
                                else
                                    aExpr1 = aTemp1;
                            }
                            else
                                aExpr1 = aTemp1;
                        }
                        if ( pOutSet->GetItemState( FID_VALID_VALUE2, true, &pItem ) == SFX_ITEM_SET )
                        {
                            OUString aTemp2 = ((const SfxStringItem*)pItem)->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (pDoc->GetFormatTable()->IsNumberFormat(aTemp2, nNumIndex, nVal))
                                    aExpr2 = OUString( ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::pLocaleData->getNumDecimalSep()[0], true));
                                else
                                    aExpr2 = aTemp2;
                            }
                            else
                                aExpr2 = aTemp2;
                        }

                        if ( pOutSet->GetItemState( FID_VALID_BLANK, true, &pItem ) == SFX_ITEM_SET )
                            bBlank = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_LISTTYPE, true, &pItem ) == SFX_ITEM_SET )
                            nListType = ((const SfxInt16Item*)pItem)->GetValue();

                        if ( pOutSet->GetItemState( FID_VALID_SHOWHELP, true, &pItem ) == SFX_ITEM_SET )
                            bShowHelp = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_HELPTITLE, true, &pItem ) == SFX_ITEM_SET )
                            aHelpTitle = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_HELPTEXT, true, &pItem ) == SFX_ITEM_SET )
                            aHelpText = ((const SfxStringItem*)pItem)->GetValue();

                        if ( pOutSet->GetItemState( FID_VALID_SHOWERR, true, &pItem ) == SFX_ITEM_SET )
                            bShowError = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRSTYLE, true, &pItem ) == SFX_ITEM_SET )
                            eErrStyle = (ScValidErrorStyle) ((const SfxAllEnumItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRTITLE, true, &pItem ) == SFX_ITEM_SET )
                            aErrTitle = ((const SfxStringItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_ERRTEXT, true, &pItem ) == SFX_ITEM_SET )
                            aErrText = ((const SfxStringItem*)pItem)->GetValue();

                        ScValidationData aData( eMode, eOper, aExpr1, aExpr2, pDoc, aCursorPos );
                        aData.SetIgnoreBlank( bBlank );
                        aData.SetListType( nListType );

                        aData.SetInput(aHelpTitle, aHelpText);          
                        if (!bShowHelp)
                            aData.ResetInput();                         

                        aData.SetError(aErrTitle, aErrText, eErrStyle); 
                        if (!bShowError)
                            aData.ResetError();                         

                        pTabViewShell->SetValidation( aData );
                        pTabViewShell->TestHintWindow();
                        rReq.Done( *pOutSet );
                    }
                    
                    
                    Application::PostUserEvent( Link( pDlg, &DelayDeleteAbstractDialog ) );
                }
            }
            break;

        case SID_TEXT_TO_COLUMNS:
            {
                ScViewData* pData = GetViewData();
                OSL_ENSURE( pData, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pData is null!" );
                ScRange aRange;

                if ( lcl_GetTextToColumnsRange( pData, aRange ) )
                {
                    ScDocument* pDoc = pData->GetDocument();
                    OSL_ENSURE( pDoc, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDoc is null!" );

                    ScImportExport aExport( pDoc, aRange );
                    aExport.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::None, 0, false ) );

                    
                    aExport.SetDelimiter( static_cast< sal_Unicode >( 0 ) );

                    SvMemoryStream aStream;
                    aStream.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
                    ScImportExport::SetNoEndianSwap( aStream );
                    aExport.ExportStream( aStream, OUString(), FORMAT_STRING );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE( pFact, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pFact is null!" );
                    AbstractScImportAsciiDlg *pDlg = pFact->CreateScImportAsciiDlg(
                        NULL, OUString(), &aStream, SC_TEXTTOCOLUMNS);
                    OSL_ENSURE( pDlg, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDlg is null!" );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        ScDocShell* pDocSh = pData->GetDocShell();
                        OSL_ENSURE( pDocSh, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDocSh is null!" );

                        OUString aUndo = ScGlobal::GetRscString( STR_UNDO_TEXTTOCOLUMNS );
                        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

                        ScImportExport aImport( pDoc, aRange.aStart );
                        ScAsciiOptions aOptions;
                        pDlg->GetOptions( aOptions );
                        pDlg->SaveParameters();
                        aImport.SetExtOptions( aOptions );
                        aImport.SetApi( false );
                        aStream.Seek( 0 );
                        aImport.ImportStream( aStream, OUString(), FORMAT_STRING );

                        pDocSh->GetUndoManager()->LeaveListAction();
                    }
                    delete pDlg;
                }
            }
            break;
        }
}

void ScCellShell::GetDBState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScViewData* pData       = GetViewData();
    ScDocShell* pDocSh      = pData->GetDocShell();
    ScDocument* pDoc        = pDocSh->GetDocument();
    SCCOL       nPosX       = pData->GetCurX();
    SCROW       nPosY       = pData->GetCurY();
    SCTAB       nTab        = pData->GetTabNo();

    sal_Bool bAutoFilter = false;
    sal_Bool bAutoFilterTested = false;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_REFRESH_DBAREA:
                {
                    
                    
                    sal_Bool bOk = false;
                    ScDBData* pDBData = pTabViewShell->GetDBData(false,SC_DB_OLD);
                    if (pDBData && pDoc->GetChangeTrack() == NULL)
                    {
                        if ( pDBData->HasImportParam() )
                            bOk = !pDBData->HasImportSelection();
                        else
                        {
                            bOk = pDBData->HasQueryParam() ||
                                  pDBData->HasSortParam() ||
                                  pDBData->HasSubTotalParam();
                        }
                    }
                    if (!bOk)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_FILTER:
            case SID_SPECIAL_FILTER:
                {
                    ScRange aDummy;
                    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aDummy);
                    if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;


                
            case SID_SORT_ASCENDING:
            case SID_SORT_DESCENDING:
            case SCITEM_SORTDATA:
            case SCITEM_SUBTDATA:
            case SID_OPENDLG_PIVOTTABLE:
                {
                    

                    if ( pDocSh->IsReadOnly() || pDoc->GetChangeTrack()!=NULL ||
                            GetViewData()->IsMultiMarked() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_REIMPORT_DATA:
                {
                    
                    ScDBData* pDBData = pTabViewShell->GetDBData(false,SC_DB_OLD);
                    if (!pDBData || !pDBData->HasImportParam() || pDBData->HasImportSelection() ||
                        pDoc->GetChangeTrack()!=NULL)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_VIEW_DATA_SOURCE_BROWSER:
                {
                    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE))
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                    else
                        
                        pTabViewShell->GetViewFrame()->GetSlotState( nWhich, NULL, &rSet );
                }
                break;
            case SID_SBA_BRW_INSERT:
                {
                    

                    sal_Bool bEnable = sal_True;
                    rSet.Put(SfxBoolItem(nWhich, bEnable));
                }
                break;

            case SID_AUTO_FILTER:
            case SID_AUTOFILTER_HIDE:
                {
                    if (!bAutoFilterTested)
                    {
                        bAutoFilter = pDoc->HasAutoFilter( nPosX, nPosY, nTab );
                        bAutoFilterTested = sal_True;
                    }
                    if ( nWhich == SID_AUTO_FILTER )
                    {
                        ScRange aDummy;
                        ScMarkType eMarkType = GetViewData()->GetSimpleArea( aDummy);
                        if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else if (pDoc->GetDPAtBlock(aDummy))
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
                            rSet.Put( SfxBoolItem( nWhich, bAutoFilter ) );
                    }
                    else
                        if (!bAutoFilter)
                            rSet.DisableItem( nWhich );
                }
                break;

            case SID_UNFILTER:
                {
                    SCCOL nStartCol, nEndCol;
                    SCROW  nStartRow, nEndRow;
                    SCTAB  nStartTab, nEndTab;
                    sal_Bool bAnyQuery = false;

                    sal_Bool bSelected = (GetViewData()->GetSimpleArea(
                                nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab )
                            == SC_MARK_SIMPLE);

                    if ( bSelected )
                    {
                        if (nStartCol==nEndCol && nStartRow==nEndRow)
                            bSelected = false;
                    }
                    else
                    {
                        nStartCol = GetViewData()->GetCurX();
                        nStartRow = GetViewData()->GetCurY();
                        nStartTab = GetViewData()->GetTabNo();
                    }

                    ScDBData* pDBData = bSelected
                                ? pDoc->GetDBAtArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow )
                                : pDoc->GetDBAtCursor( nStartCol, nStartRow, nStartTab );

                    if ( pDBData )
                    {
                        ScQueryParam aParam;
                        pDBData->GetQueryParam( aParam );
                        if ( aParam.GetEntry(0).bDoQuery )
                            bAnyQuery = sal_True;
                    }

                    if ( !bAnyQuery )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DEFINE_DBNAME:
                {
                    if ( pDocSh && pDocSh->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_DATA_STREAMS:
            case SID_DATA_STREAMS_PLAY:
            case SID_DATA_STREAMS_STOP:
                {
                    SvtMiscOptions aMiscOptions;
                    if ( !aMiscOptions.IsExperimentalMode() )
                        rSet.DisableItem( nWhich );
                }
                break;
            case SID_TEXT_TO_COLUMNS:
                {
                    ScRange aRange;
                    if ( !lcl_GetTextToColumnsRange( pData, aRange ) )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
#if !ENABLE_ORCUS
            case SID_MANAGE_XML_SOURCE:
                rSet.DisableItem(nWhich);
            break;
#endif
        }
        nWhich = aIter.NextWhich();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
