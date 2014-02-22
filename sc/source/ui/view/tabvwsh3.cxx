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
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/request.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/objface.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/vclenum.hxx>

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
#include "formulacell.hxx"
#include "inputhdl.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "dwfunctr.hxx"
#include "shtabdlg.hxx"
#include "tabprotection.hxx"
#include "protectiondlg.hxx"
#include "markdata.hxx"

#include <svl/ilstitem.hxx>
#include <vector>

#include <svx/zoomslideritem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <comphelper/string.hxx>
#include "scabstdlg.hxx"

#include <memory>

using ::std::auto_ptr;



/** Try to parse the given range using Calc-style syntax first, then
    Excel-style if that fails. */
static sal_uInt16 lcl_ParseRange(ScRange& rScRange, const OUString& aAddress, ScDocument* pDoc, sal_uInt16 /* nSlot */)
{
    sal_uInt16 nResult = rScRange.Parse(aAddress, pDoc);
    if ( (nResult & SCA_VALID) )
        return nResult;

    return rScRange.Parse(aAddress, pDoc, ScAddress::Details(formula::FormulaGrammar::CONV_XL_A1, 0, 0));
}

/** Try to parse the given address using Calc-style syntax first, then
    Excel-style if that fails. */
static sal_uInt16 lcl_ParseAddress(ScAddress& rScAddress, const OUString& aAddress, ScDocument* pDoc, sal_uInt16 /* nSlot */)
{
    sal_uInt16 nResult = rScAddress.Parse(aAddress, pDoc);
    if ( (nResult & SCA_VALID) )
        return nResult;

    return rScAddress.Parse(aAddress, pDoc, ScAddress::Details(formula::FormulaGrammar::CONV_XL_A1, 0, 0));
}

void ScTabViewShell::Execute( SfxRequest& rReq )
{
    SfxViewFrame*       pThisFrame  = GetViewFrame();
    SfxBindings&        rBindings   = pThisFrame->GetBindings();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       
        HideListBox();                  

    switch ( nSlot )
    {
        case FID_INSERT_FILE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(FID_INSERT_FILE,true,&pItem) == SFX_ITEM_SET )
                {
                    OUString aFileName = ((const SfxStringItem*)pItem)->GetValue();

                        

                    Point aInsertPos;
                    if ( pReqArgs->GetItemState(FN_PARAM_1,true,&pItem) == SFX_ITEM_SET )
                        aInsertPos = ((const SfxPointItem*)pItem)->GetValue();
                    else
                        aInsertPos = GetInsertPos();

                        

                    bool bAsLink = false;
                    if ( pReqArgs->GetItemState(FN_PARAM_2,true,&pItem) == SFX_ITEM_SET )
                        bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

                        

                    PasteFile( aInsertPos, aFileName, bAsLink );
                }
            }
            break;

        case SID_OPENDLG_EDIT_PRINTAREA:
            {
                sal_uInt16          nId  = ScPrintAreasDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case SID_CHANGE_PRINTAREA:
            {
                if ( pReqArgs )         
                {
                    OUString aPrintStr;
                    OUString aRowStr;
                    OUString aColStr;
                    bool bEntire = false;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_CHANGE_PRINTAREA, true, &pItem ) == SFX_ITEM_SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SFX_ITEM_SET )
                        aRowStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_3, true, &pItem ) == SFX_ITEM_SET )
                        aColStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_4, true, &pItem ) == SFX_ITEM_SET )
                        bEntire = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                    SetPrintRanges( bEntire, &aPrintStr, &aColStr, &aRowStr, false );

                    rReq.Done();
                }
            }
            break;

        case SID_ADD_PRINTAREA:
        case SID_DEFINE_PRINTAREA:      
            {
                bool bAdd = ( nSlot == SID_ADD_PRINTAREA );
                if ( pReqArgs )
                {
                    OUString aPrintStr;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_DEFINE_PRINTAREA, true, &pItem ) == SFX_ITEM_SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    SetPrintRanges( false, &aPrintStr, NULL, NULL, bAdd );
                }
                else
                {
                    SetPrintRanges( false, NULL, NULL, NULL, bAdd );      
                    rReq.Done();
                }
            }
            break;

        case SID_DELETE_PRINTAREA:
            {
                OUString aEmpty;
                SetPrintRanges( false, &aEmpty, NULL, NULL, false );        
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
            SetPrintZoom( 100, 0 );     
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
                OUString aAddress;
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SFX_ITEM_SET )
                    aAddress = ((const SfxStringItem*)pItem)->GetValue();
                else if ( nSlot == SID_JUMPTOMARK && pReqArgs->GetItemState(
                                            SID_JUMPTOMARK, true, &pItem ) == SFX_ITEM_SET )
                    aAddress = ((const SfxStringItem*)pItem)->GetValue();

                
                
                
                
                bool bUnmark = ( nSlot == SID_CURRENTCELL );
                if ( pReqArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SFX_ITEM_SET )
                    bUnmark = ((const SfxBoolItem*)pItem)->GetValue();

                bool bAlignToCursor = true;
                if (pReqArgs->GetItemState(FN_PARAM_2, true, &pItem) == SFX_ITEM_SET)
                    bAlignToCursor = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                if ( nSlot == SID_JUMPTOMARK )
                {
                    
                    aAddress = INetURLObject::decode( aAddress, INET_HEX_ESCAPE,
                                               INetURLObject::DECODE_WITH_CHARSET,
                                            RTL_TEXTENCODING_UTF8 );
                }

                bool bFound = false;
                ScViewData* pViewData = GetViewData();
                ScDocument* pDoc      = pViewData->GetDocument();
                ScMarkData& rMark     = pViewData->GetMarkData();
                ScRange     aScRange;
                ScAddress   aScAddress;
                sal_uInt16      nResult = lcl_ParseRange(aScRange, aAddress, pDoc, nSlot);
                SCTAB       nTab = pViewData->GetTabNo();
                bool        bMark = true;

                
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
                
                else if ( (nResult = lcl_ParseAddress(aScAddress, aAddress, pDoc, nSlot)) & SCA_VALID )
                {
                    if ( nResult & SCA_TAB_3D )
                    {
                        if( aScAddress.Tab() != nTab )
                            SetTabNo( nTab = aScAddress.Tab() );
                    }
                    else
                        aScAddress.SetTab( nTab );

                    aScRange = ScRange( aScAddress, aScAddress );
                    
                    bMark = false;
                }
                
                else
                {
                    ScRangeUtil     aRangeUtil;
                    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
                    if( aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_NAMES, eConv ) ||
                        aRangeUtil.MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_DBASE, eConv ) )
                    {
                        nResult |= SCA_VALID;
                        if( aScRange.aStart.Tab() != nTab )
                            SetTabNo( nTab = aScRange.aStart.Tab() );
                    }
                }

                if ( !(nResult & SCA_VALID) && comphelper::string::isdigitAsciiString(aAddress) )
                {
                    sal_Int32 nNumeric = aAddress.toInt32();
                    if ( nNumeric > 0 && nNumeric <= MAXROW+1 )
                    {
                        

                        aScAddress.SetRow( (SCROW)(nNumeric - 1) );
                        aScAddress.SetCol( pViewData->GetCurX() );
                        aScAddress.SetTab( nTab );
                        aScRange = ScRange( aScAddress, aScAddress );
                        bMark    = false;
                        nResult  = SCA_VALID;
                    }
                }

                if ( !ValidRow(aScRange.aStart.Row()) || !ValidRow(aScRange.aEnd.Row()) )
                    nResult = 0;

                
                if( nResult & SCA_VALID )
                {
                    bFound = true;
                    SCCOL nCol = aScRange.aStart.Col();
                    SCROW nRow = aScRange.aStart.Row();
                    bool bNothing = ( pViewData->GetCurX()==nCol && pViewData->GetCurY()==nRow );

                    
                    if( bMark )
                    {
                        if (rMark.IsMarked())           
                        {
                            ScRange aOldMark;
                            rMark.GetMarkArea( aOldMark );
                            aOldMark.Justify();
                            ScRange aCurrent = aScRange;
                            aCurrent.Justify();
                            bNothing = ( aCurrent == aOldMark );
                        }
                        else
                            bNothing = false;

                        if (!bNothing)
                            MarkRange( aScRange, false );   
                    }
                    else
                    {
                        
                        if( bUnmark )
                        {
                            MoveCursorAbs( nCol, nRow,
                                SC_FOLLOW_NONE, false, false );
                        }
                    }

                    

                    
                    pDoc->SkipOverlapped(nCol, nRow, nTab);

                    

                    if( bNothing )
                    {
                        if (rReq.IsAPI())
                            rReq.Ignore();      
                        else
                            rReq.Done();        
                    }
                    else
                    {
                        pViewData->ResetOldCursor();
                        SetCursor( nCol, nRow );
                        rBindings.Invalidate( SID_CURRENTCELL );
                        rBindings.Update( nSlot );

                        if (!rReq.IsAPI())
                            rReq.Done();
                    }

                    if (bAlignToCursor)
                    {
                        
                        
                        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
                    }

                    rReq.SetReturnValue( SfxStringItem( SID_CURRENTCELL, aAddress ) );
                }

                if (!bFound)    
                {
                    

                    SCTAB nNameTab;
                    if ( pDoc->GetTable( aAddress, nNameTab ) )
                    {
                        bFound = true;
                        if ( nNameTab != nTab )
                            SetTabNo( nNameTab );
                    }
                }

                if ( !bFound && nSlot == SID_JUMPTOMARK )
                {
                    

                    bFound = SelectObject( aAddress );
                }

                if (!bFound && !rReq.IsAPI())
                    ErrorMessage( STR_ERR_INVALID_AREA );
            }
            break;

        case SID_CURRENTOBJECT:
            if ( pReqArgs )
            {
                OUString aName = ((const SfxStringItem&)pReqArgs->Get(nSlot)).GetValue();
                SelectObject( aName );
            }
            break;

        case SID_CURRENTTAB:
            if ( pReqArgs )
            {
                
                SCTAB nTab = ((const SfxUInt16Item&)pReqArgs->Get(nSlot)).GetValue() - 1;
                ScDocument* pDoc = GetViewData()->GetDocument();
                if ( nTab < pDoc->GetTableCount() )
                {
                    SetTabNo( nTab );
                    rBindings.Update( nSlot );

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                
            }
            break;

        case SID_CURRENTDOC:
            if ( pReqArgs )
            {
                OUString aStrDocName( ((const SfxStringItem&)pReqArgs->
                                        Get(nSlot)).GetValue() );

                SfxViewFrame*   pViewFrame = NULL;
                ScDocShell*     pDocSh = (ScDocShell*)SfxObjectShell::GetFirst();
                bool            bFound = false;

                

                while ( pDocSh && !bFound )
                {
                    if ( pDocSh->GetTitle() == aStrDocName )
                    {
                        pViewFrame = SfxViewFrame::GetFirst( pDocSh );
                        bFound = ( NULL != pViewFrame );
                    }

                    pDocSh = (ScDocShell*)SfxObjectShell::GetNext( *pDocSh );
                }

                if ( bFound )
                    pViewFrame->GetFrame().Appear();

                rReq.Ignore();
            }

        case SID_ATTR_SIZE:
            break;


        case SID_PRINTPREVIEW:
            {
                if ( !pThisFrame->GetFrame().IsInPlace() )          
                {
                    
                    
                    

                    
                    pScMod->InputEnterHandler();

                    pThisFrame->GetDispatcher()->Execute( SID_VIEWSHELL1, SFX_CALLMODE_ASYNCHRON );
                }
                
            }
            break;

        case SID_DETECTIVE_DEL_ALL:
            DetectiveDelAll();
            rReq.Done();
            break;

        
        

        case SID_TABLE_ACTIVATE:
            OSL_FAIL("old slot SID_TABLE_ACTIVATE");
            break;

        case SID_REPAINT:
            PaintGrid();
            PaintTop();
            PaintLeft();
            PaintExtras();
            rReq.Done();
            break;

        case FID_NORMALVIEWMODE:
        case FID_PAGEBREAKMODE:
            {
                bool bWantPageBreak = nSlot == FID_PAGEBREAKMODE;

                
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SFX_ITEM_SET )
                {
                    bool bItemValue = ((const SfxBoolItem*)pItem)->GetValue();
                    bWantPageBreak = (nSlot == FID_PAGEBREAKMODE) == bItemValue;
                }

                if( GetViewData()->IsPagebreakMode() != bWantPageBreak )
                {
                    SetPagebreakMode( bWantPageBreak );
                    UpdatePageBreakData();
                    SetCurSubShell( GetCurObjectSelectionType(), true );
                    PaintGrid();
                    PaintTop();
                    PaintLeft();
                    rBindings.Invalidate( nSlot );
                    rReq.AppendItem( SfxBoolItem( nSlot, true ) );
                    rReq.Done();
                }
            }
            break;

        case FID_FUNCTION_BOX:
            {
                sal_uInt16 nChildId = ScFunctionChildWindow::GetChildWindowId();
                if ( rReq.GetArgs() )
                    pThisFrame->SetChildWindow( nChildId, ((const SfxBoolItem&) (rReq.GetArgs()->Get(FID_FUNCTION_BOX))).GetValue());
                else
                {
                    pThisFrame->ToggleChildWindow( nChildId );
                    rReq.AppendItem( SfxBoolItem( FID_FUNCTION_BOX , pThisFrame->HasChildWindow( nChildId ) ) );
                }

                GetViewFrame()->GetBindings().Invalidate(FID_FUNCTION_BOX);
                rReq.Done ();
            }
            break;


        case FID_TOGGLESYNTAX:
            {
                bool bSet = !GetViewData()->IsSyntaxMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                GetViewData()->SetSyntaxMode( bSet );
                PaintGrid();
                rBindings.Invalidate( FID_TOGGLESYNTAX );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;
        case FID_TOGGLEHEADERS:
            {
                bool bSet = !GetViewData()->IsHeaderMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();
                GetViewData()->SetHeaderMode( bSet );
                RepeatResize();
                rBindings.Invalidate( FID_TOGGLEHEADERS );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;

        case FID_TOGGLEFORMULA:
            {
                ScViewData* pViewData = GetViewData();
                const ScViewOptions& rOpts = pViewData->GetOptions();
                bool bFormulaMode = !rOpts.GetOption( VOPT_FORMULAS );
                const SfxPoolItem *pItem;
                if( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SFX_ITEM_SET )
                    bFormulaMode = ((const SfxBoolItem *)pItem)->GetValue();

                ScViewOptions rSetOpts = ScViewOptions( rOpts );
                rSetOpts.SetOption( VOPT_FORMULAS, bFormulaMode );
                pViewData->SetOptions( rSetOpts );

                pViewData->GetDocShell()->PostPaintGridAll();

                rBindings.Invalidate( FID_TOGGLEFORMULA );
                rReq.AppendItem( SfxBoolItem( nSlot, bFormulaMode ) );
                rReq.Done();
            }
            break;

        case FID_TOGGLEINPUTLINE:
            {
                sal_uInt16          nId  = ScInputWindowWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                bool bSet = ( pWnd == NULL );
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SFX_ITEM_SET )
                    bSet = ((const SfxBoolItem*)pItem)->GetValue();

                pThisFrame->SetChildWindow( nId, bSet );
                rBindings.Invalidate( FID_TOGGLEINPUTLINE );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;

        case SID_ATTR_ZOOM: 
        case FID_SCALE:
            {
                bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                SvxZoomType eOldZoomType = GetZoomType();
                SvxZoomType eNewZoomType = eOldZoomType;
                const Fraction& rOldY = GetViewData()->GetZoomY();  
                sal_uInt16 nOldZoom = (sal_uInt16)(( rOldY.GetNumerator() * 100 )
                                            / rOldY.GetDenominator());
                sal_uInt16 nZoom = nOldZoom;
                bool bCancel = false;

                if ( pReqArgs )
                {
                    const SvxZoomItem& rZoomItem = (const SvxZoomItem&)
                                                   pReqArgs->Get(SID_ATTR_ZOOM);

                    eNewZoomType = rZoomItem.GetType();
                    nZoom     = rZoomItem.GetValue();
                }
                else
                {
                    SfxItemSet      aSet     ( GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
                    SvxZoomItem     aZoomItem( eOldZoomType, nOldZoom, SID_ATTR_ZOOM );
                    AbstractSvxZoomDialog* pDlg = NULL;
                    ScMarkData&     rMark = GetViewData()->GetMarkData();
                    sal_uInt16          nBtnFlags =   SVX_ZOOM_ENABLE_50
                                                | SVX_ZOOM_ENABLE_75
                                                | SVX_ZOOM_ENABLE_100
                                                | SVX_ZOOM_ENABLE_150
                                                | SVX_ZOOM_ENABLE_200
                                                | SVX_ZOOM_ENABLE_WHOLEPAGE
                                                | SVX_ZOOM_ENABLE_PAGEWIDTH;

                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                        nBtnFlags = nBtnFlags | SVX_ZOOM_ENABLE_OPTIMAL;

                    aZoomItem.SetValueSet( nBtnFlags );
                    aSet.Put( aZoomItem );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        pDlg = pFact->CreateSvxZoomDialog(GetDialogParent(), aSet );
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");
                    }
                    if (pDlg)
                    {
                       pDlg->SetLimits( MINZOOM, MAXZOOM );

                       bCancel = ( RET_CANCEL == pDlg->Execute() );
                    }
                    
                    
                    if ( !bCancel )
                    {
                        const SvxZoomItem&  rZoomItem = (const SvxZoomItem&)
                                                pDlg->GetOutputItemSet()->
                                                    Get( SID_ATTR_ZOOM );

                        eNewZoomType = rZoomItem.GetType();
                        nZoom     = rZoomItem.GetValue();
                    }

                    delete pDlg;
                }

                if ( !bCancel )
                {
                    if ( eNewZoomType == SVX_ZOOM_PERCENT )
                    {
                        if ( nZoom < MINZOOM )  nZoom = MINZOOM;
                        if ( nZoom > MAXZOOM )  nZoom = MAXZOOM;
                    }
                    else
                    {
                        nZoom = CalcZoom( eNewZoomType, nOldZoom );
                        bCancel = nZoom == 0;
                    }

                    switch ( eNewZoomType )
                    {
                        case SVX_ZOOM_WHOLEPAGE:
                        case SVX_ZOOM_PAGEWIDTH:
                            SetZoomType( eNewZoomType, bSyncZoom );
                            break;

                        default:
                            SetZoomType( SVX_ZOOM_PERCENT, bSyncZoom );
                    }
                }

                if ( nZoom != nOldZoom && !bCancel )
                {
                    if (!GetViewData()->IsPagebreakMode())
                    {
                        ScAppOptions aNewOpt = pScMod->GetAppOptions();
                        aNewOpt.SetZoom( nZoom );
                        aNewOpt.SetZoomType( GetZoomType() );
                        pScMod->SetAppOptions( aNewOpt );
                    }
                    Fraction aFract( nZoom, 100 );
                    SetZoom( aFract, aFract, bSyncZoom );
                    PaintGrid();
                    PaintTop();
                    PaintLeft();
                    rBindings.Invalidate( SID_ATTR_ZOOM );
                    rReq.AppendItem( SvxZoomItem( GetZoomType(), nZoom, nSlot ) );
                    rReq.Done();
                }
            }
            break;

        case SID_ATTR_ZOOMSLIDER:
            {
                const SfxPoolItem* pItem = NULL;
                bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                if ( pReqArgs && pReqArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem) == SFX_ITEM_SET )
                {
                    const sal_uInt16 nCurrentZoom = ((const SvxZoomSliderItem *)pItem)->GetValue();
                    if( nCurrentZoom )
                    {
                        SetZoomType( SVX_ZOOM_PERCENT, bSyncZoom );
                        if (!GetViewData()->IsPagebreakMode())
                        {
                            ScAppOptions aNewOpt = pScMod->GetAppOptions();
                            aNewOpt.SetZoom( nCurrentZoom );
                            aNewOpt.SetZoomType( GetZoomType() );
                            pScMod->SetAppOptions( aNewOpt );
                        }
                        Fraction aFract( nCurrentZoom,100 );
                        SetZoom( aFract, aFract, bSyncZoom );
                        PaintGrid();
                        PaintTop();
                        PaintLeft();
                        rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
                        rReq.Done();
                    }
                }
            }
            break;

        

        case FID_TAB_SELECTALL:
            SelectAllTables();
            rReq.Done();
            break;

        case FID_TAB_DESELECTALL:
            DeselectAllTables();
            rReq.Done();
            break;

        case SID_SELECT_TABLES:
        {
            ScViewData& rViewData = *GetViewData();
            ScDocument& rDoc = *rViewData.GetDocument();
            ScMarkData& rMark = rViewData.GetMarkData();
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nTab;

            ::std::vector < sal_Int32 > aIndexList;
            SFX_REQUEST_ARG( rReq, pItem, SfxIntegerListItem, SID_SELECT_TABLES, false );
            if ( pItem )
                pItem->GetList( aIndexList );
            else
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                AbstractScShowTabDlg* pDlg = pFact->CreateScShowTabDlg(GetDialogParent());
                OSL_ENSURE(pDlg, "Dialog create fail!");
                pDlg->SetDescription(
                    OUString( ScResId( STR_DLG_SELECTTABLES_TITLE ) ),
                    OUString( ScResId( STR_DLG_SELECTTABLES_LBNAME ) ),
                    GetStaticInterface()->GetSlot(SID_SELECT_TABLES)->GetCommand(), HID_SELECTTABLES );

                
                OUString aTabName;
                for( nTab = 0; nTab < nTabCount; ++nTab )
                {
                    rDoc.GetName( nTab, aTabName );
                    pDlg->Insert( aTabName, rMark.GetTableSelect( nTab ) );
                }

                if( pDlg->Execute() == RET_OK )
                {
                    sal_uInt16 nSelCount = pDlg->GetSelectEntryCount();
                    sal_uInt16 nSelIx;
                    for( nSelIx = 0; nSelIx < nSelCount; ++nSelIx )
                        aIndexList.insert( aIndexList.begin()+nSelIx, pDlg->GetSelectEntryPos( nSelIx ) );
                    delete pDlg;
                    rReq.AppendItem( SfxIntegerListItem( SID_SELECT_TABLES, aIndexList ) );
                }
                else
                    rReq.Ignore();
            }

            if ( !aIndexList.empty() )
            {
                sal_uInt16 nSelCount = aIndexList.size();
                sal_uInt16 nSelIx;
                SCTAB nFirstVisTab = 0;

                
                bool bVisSelected = false;
                for( nSelIx = 0; !bVisSelected && (nSelIx < nSelCount); ++nSelIx )
                    bVisSelected = rDoc.IsVisible( nFirstVisTab = static_cast<SCTAB>(aIndexList[nSelIx]) );
                if( !bVisSelected )
                    nSelCount = 0;

                
                if( nSelCount )
                {
                    for( nTab = 0; nTab < nTabCount; ++nTab )
                        rMark.SelectTable( nTab, false );

                    for( nSelIx = 0; nSelIx < nSelCount; ++nSelIx )
                        rMark.SelectTable( static_cast<SCTAB>(aIndexList[nSelIx]), true );

                    
                    if( !rMark.GetTableSelect( rViewData.GetTabNo() ) )
                    {
                        rMark.SelectTable( nFirstVisTab, true );
                        SetTabNo( nFirstVisTab );
                    }

                    rViewData.GetDocShell()->PostPaintExtras();
                    SfxBindings& rBind = rViewData.GetBindings();
                    rBind.Invalidate( FID_FILL_TAB );
                    rBind.Invalidate( FID_TAB_DESELECTALL );
                }

                rReq.Done();
            }
        }
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
                if ( eHSplit == SC_SPLIT_NORMAL || eVSplit == SC_SPLIT_NORMAL )     
                    RemoveSplit();
                else if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )      
                    FreezeSplitters( false );
                else                                                                
                    SplitAtCursor();
                rReq.Done();

                InvalidateSplit();
            }
            break;

        case SID_WINDOW_FIX:
            {
                ScSplitMode eHSplit = GetViewData()->GetHSplitMode();
                ScSplitMode eVSplit = GetViewData()->GetVSplitMode();
                if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )           
                    RemoveSplit();
                else
                    FreezeSplitters( true );        
                rReq.Done();

                InvalidateSplit();
            }
            break;

        

        case FID_CHG_SHOW:
            {
                sal_uInt16          nId  = ScHighlightChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case FID_CHG_ACCEPT:
            {
                pThisFrame->ToggleChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId());
                GetViewFrame()->GetBindings().Invalidate(FID_CHG_ACCEPT);
                rReq.Done ();

                /*
                sal_uInt16          nId  = ScAcceptChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
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
                         pReqArgs->GetItemState( nSlot, true, &pItem ) == SFX_ITEM_SET &&
                         pItem->ISA( SfxStringItem ) )
                    {
                        OUString aComment = ((const SfxStringItem*)pItem)->GetValue();
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
            
            
            if (!GetScDrawView())
            {
                GetViewData()->GetDocShell()->MakeDrawLayer();
                rBindings.InvalidateAll(false);
            }
            break;

        case FID_PROTECT_DOC:
            {
                ScDocument*         pDoc = GetViewData()->GetDocument();
                SfxPasswordDialog*  pDlg;

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_PROTECT_DOC, &pItem ) &&
                        ((const SfxBoolItem*)pItem)->GetValue() == pDoc->IsDocProtected() )
                    {
                        rReq.Ignore();
                        break;
                    }
                }

                ScDocProtection* pProtect = pDoc->GetDocProtection();
                if (pProtect && pProtect->isProtected())
                {
                    bool bCancel = false;
                    OUString aPassword;

                    if (pProtect->isProtectedWithPass())
                    {
                        OUString aText(ScResId(SCSTR_PASSWORD));

                        pDlg = new SfxPasswordDialog(GetDialogParent(), &aText);
                        pDlg->SetText( ScResId(SCSTR_UNPROTECTDOC) );
                        pDlg->SetMinLen( 0 );
                        pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand() );
                        pDlg->SetEditHelpId( HID_PASSWD_DOC );

                        if (pDlg->Execute() == RET_OK)
                            aPassword = pDlg->GetPassword();
                        else
                            bCancel = true;
                        delete pDlg;
                    }
                    if (!bCancel)
                    {
                        Unprotect( TABLEID_DOC, aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, false ) );
                        rReq.Done();
                    }
                }
                else
                {
                    OUString aText(ScResId(SCSTR_PASSWORDOPT));

                    pDlg = new SfxPasswordDialog(   GetDialogParent(), &aText );
                    pDlg->SetText( ScResId(SCSTR_PROTECTDOC) );
                    pDlg->SetMinLen( 0 );
                    pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand() );
                    pDlg->SetEditHelpId( HID_PASSWD_DOC );
                    pDlg->ShowExtras( SHOWEXTRAS_CONFIRM );

                    if (pDlg->Execute() == RET_OK)
                    {
                        OUString aPassword = pDlg->GetPassword();
                        Protect( TABLEID_DOC, aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, true ) );
                        rReq.Done();
                    }

                    delete pDlg;
                }
                rBindings.Invalidate( FID_PROTECT_DOC );
            }
            break;


        case FID_PROTECT_TABLE:
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            SCTAB       nTab = GetViewData()->GetTabNo();
            bool        bOldProtection = pDoc->IsTabProtected(nTab);

            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                bool bNewProtection = !bOldProtection;
                if( pReqArgs->HasItem( FID_PROTECT_TABLE, &pItem ) )
                    bNewProtection = ((const SfxBoolItem*)pItem)->GetValue();
                if( bNewProtection == bOldProtection )
                {
                    rReq.Ignore();
                    break;
                }
            }

            if (bOldProtection)
            {
                

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect && pProtect->isProtectedWithPass())
                {
                    OUString aText( ScResId(SCSTR_PASSWORDOPT) );
                    auto_ptr<SfxPasswordDialog> pDlg(new SfxPasswordDialog(GetDialogParent(), &aText));
                    pDlg->SetText( ScResId(SCSTR_UNPROTECTTAB) );
                    pDlg->SetMinLen( 0 );
                    pDlg->SetHelpId( GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand() );
                    pDlg->SetEditHelpId( HID_PASSWD_TABLE );

                    if (pDlg->Execute() == RET_OK)
                    {
                        OUString aPassword = pDlg->GetPassword();
                        Unprotect(nTab, aPassword);
                    }
                }
                else
                    
                    Unprotect(nTab, OUString());

                if (!pReqArgs)
                {
                    rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
                    rReq.Done();
                }
            }
            else
            {
                

                auto_ptr<ScTableProtectionDlg> pDlg(new ScTableProtectionDlg(GetDialogParent()));

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect)
                    pDlg->SetDialogData(*pProtect);

                if (pDlg->Execute() == RET_OK)
                {
                    pScMod->InputEnterHandler();

                    ScTableProtection aNewProtect;
                    pDlg->WriteData(aNewProtect);
                    ProtectSheet(nTab, aNewProtect);
                    if (!pReqArgs)
                    {
                        rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, true) );
                        rReq.Done();
                    }
                }
            }
            TabChanged();
            UpdateInputHandler(true);   
            SelectionChanged();
        }
        break;

        case SID_OPT_LOCALE_CHANGED :
            {   
                PaintGrid();
                rReq.Done();
            }
            break;

        default:
            OSL_FAIL("Unbekannter Slot bei ScTabViewShell::Execute");
            break;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
