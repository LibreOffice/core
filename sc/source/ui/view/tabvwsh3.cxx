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
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/objface.hxx>
#include <vcl/vclenum.hxx>

#include <globstr.hrc>
#include <strings.hrc>
#include <scmod.hxx>
#include <appoptio.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <sc.hrc>
#include <helpids.h>
#include <inputwin.hxx>
#include <scresid.hxx>
#include <printfun.hxx>
#include <docsh.hxx>
#include <rangelst.hxx>
#include <prevwsh.hxx>
#include <rangeutl.hxx>
#include <reffact.hxx>
#include <uiitems.hxx>
#include <formulacell.hxx>
#include <inputhdl.hxx>
#include <autoform.hxx>
#include <autofmt.hxx>
#include <dwfunctr.hxx>
#include <shtabdlg.hxx>
#include <tabprotection.hxx>
#include <protectiondlg.hxx>
#include <markdata.hxx>

#include <svl/ilstitem.hxx>
#include <vector>

#include <svx/zoomslideritem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <comphelper/string.hxx>
#include <scabstdlg.hxx>

#include <memory>

namespace
{
    enum class DetectFlags
    {
        NONE,
        RANGE,
        ADDRESS
    };

    struct ScRefFlagsAndType
    {
        ScRefFlags nResult;
        DetectFlags eDetected;
    };

    ScRefFlagsAndType lcl_ParseRangeOrAddress(ScRange& rScRange, ScAddress& rScAddress,
                                              const OUString& aAddress, const ScDocument* pDoc)
    {
        ScRefFlagsAndType aRet;

        formula::FormulaGrammar::AddressConvention eConv;

        // start with the address convention set in the document
        eConv = pDoc->GetAddressConvention();
        aRet.nResult = rScRange.Parse(aAddress, pDoc, eConv);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, pDoc, eConv);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try the default Calc (A1) address convention
        aRet.nResult = rScRange.Parse(aAddress, pDoc);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, pDoc);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try the Excel A1 address convention
        aRet.nResult = rScRange.Parse(aAddress, pDoc, formula::FormulaGrammar::CONV_XL_A1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        // try the Excel A1 address convention
        aRet.nResult = rScAddress.Parse(aAddress, pDoc, formula::FormulaGrammar::CONV_XL_A1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try Excel R1C1 address convention
        aRet.nResult = rScRange.Parse(aAddress, pDoc, formula::FormulaGrammar::CONV_XL_R1C1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, pDoc, formula::FormulaGrammar::CONV_XL_R1C1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        aRet.nResult = ScRefFlags::ZERO;
        aRet.eDetected = DetectFlags::NONE;

        return aRet;
    }
}

void ScTabViewShell::Execute( SfxRequest& rReq )
{
    SfxViewFrame*       pThisFrame  = GetViewFrame();
    SfxBindings&        rBindings   = pThisFrame->GetBindings();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // comes with MouseButtonUp
        HideListBox();                  // Autofilter-DropDown-Listbox

    switch ( nSlot )
    {
        case FID_INSERT_FILE:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs &&
                     pReqArgs->GetItemState(FID_INSERT_FILE,true,&pItem) == SfxItemState::SET )
                {
                    OUString aFileName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                        // insert position

                    Point aInsertPos;
                    if ( pReqArgs->GetItemState(FN_PARAM_1,true,&pItem) == SfxItemState::SET )
                        aInsertPos = static_cast<const SfxPointItem*>(pItem)->GetValue();
                    else
                        aInsertPos = GetInsertPos();

                        // as Link?

                    bool bAsLink = false;
                    if ( pReqArgs->GetItemState(FN_PARAM_2,true,&pItem) == SfxItemState::SET )
                        bAsLink = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                        // execute

                    PasteFile( aInsertPos, aFileName, bAsLink );
                }
            }
            break;

        case SID_OPENDLG_EDIT_PRINTAREA:
            {
                sal_uInt16          nId  = ScPrintAreasDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_CHANGE_PRINTAREA:
            {
                if ( pReqArgs )         // OK from dialog
                {
                    OUString aPrintStr;
                    OUString aRowStr;
                    OUString aColStr;
                    bool bEntire = false;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_CHANGE_PRINTAREA, true, &pItem ) == SfxItemState::SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SfxItemState::SET )
                        aRowStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_3, true, &pItem ) == SfxItemState::SET )
                        aColStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if ( pReqArgs->GetItemState( FN_PARAM_4, true, &pItem ) == SfxItemState::SET )
                        bEntire = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                    SetPrintRanges( bEntire, &aPrintStr, &aColStr, &aRowStr, false );

                    rReq.Done();
                }
            }
            break;

        case SID_ADD_PRINTAREA:
        case SID_DEFINE_PRINTAREA:      // menu or basic
            {
                bool bAdd = ( nSlot == SID_ADD_PRINTAREA );
                if ( pReqArgs )
                {
                    OUString aPrintStr;
                    const SfxPoolItem* pItem;
                    if ( pReqArgs->GetItemState( SID_DEFINE_PRINTAREA, true, &pItem ) == SfxItemState::SET )
                        aPrintStr = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    SetPrintRanges( false, &aPrintStr, nullptr, nullptr, bAdd );
                }
                else
                {
                    SetPrintRanges( false, nullptr, nullptr, nullptr, bAdd );      // from selection
                    rReq.Done();
                }
            }
            break;

        case SID_DELETE_PRINTAREA:
            {
                // Clear currently defined print range if any, and reset it to
                // print entire sheet which is the default.
                OUString aEmpty;
                SetPrintRanges(true, &aEmpty, nullptr, nullptr, false);
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
            SetPrintZoom( 100 );     // 100%, not on pages
            rReq.Done();
            break;

        case SID_FORMATPAGE:
        case SID_STATUS_PAGESTYLE:
        case SID_HFEDIT:
            GetViewData().GetDocShell()->
                ExecutePageStyle( *this, rReq, GetViewData().GetTabNo() );
            break;

        case SID_JUMPTOMARK:
        case SID_CURRENTCELL:
            if ( pReqArgs )
            {
                OUString aAddress;
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                    aAddress = static_cast<const SfxStringItem*>(pItem)->GetValue();
                else if ( nSlot == SID_JUMPTOMARK && pReqArgs->GetItemState(
                                            SID_JUMPTOMARK, true, &pItem ) == SfxItemState::SET )
                    aAddress = static_cast<const SfxStringItem*>(pItem)->GetValue();

                //  #i14927# SID_CURRENTCELL with a single cell must unmark if FN_PARAM_1
                //  isn't set (for recorded macros, because IsAPI is no longer available).
                //  ScGridWindow::MouseButtonUp no longer executes the slot for a single
                //  cell if there is a multi selection.
                bool bUnmark = ( nSlot == SID_CURRENTCELL );
                if ( pReqArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
                    bUnmark = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                bool bAlignToCursor = true;
                if (pReqArgs->GetItemState(FN_PARAM_2, true, &pItem) == SfxItemState::SET)
                    bAlignToCursor = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                if ( nSlot == SID_JUMPTOMARK )
                {
                    //  URL has to be decoded for escaped characters (%20)
                    aAddress = INetURLObject::decode( aAddress,
                                               INetURLObject::DecodeMechanism::WithCharset );
                }

                bool bFound = false;
                ScViewData& rViewData = GetViewData();
                ScDocument* pDoc      = rViewData.GetDocument();
                ScMarkData& rMark     = rViewData.GetMarkData();
                ScRange     aScRange;
                ScAddress   aScAddress;
                ScRefFlagsAndType aResult = lcl_ParseRangeOrAddress(aScRange, aScAddress, aAddress, pDoc);
                ScRefFlags  nResult = aResult.nResult;
                SCTAB       nTab = rViewData.GetTabNo();
                bool        bMark = true;

                // Is this a range ?
                if (aResult.eDetected == DetectFlags::RANGE)
                {
                    if ( nResult & ScRefFlags::TAB_3D )
                    {
                        if( aScRange.aStart.Tab() != nTab )
                        {
                            nTab = aScRange.aStart.Tab();
                            SetTabNo( nTab );
                        }
                    }
                    else
                    {
                        aScRange.aStart.SetTab( nTab );
                        aScRange.aEnd.SetTab( nTab );
                    }
                }
                // Is this a cell ?
                else if (aResult.eDetected == DetectFlags::ADDRESS)
                {
                    if ( nResult & ScRefFlags::TAB_3D )
                    {
                        if( aScAddress.Tab() != nTab )
                        {
                            nTab = aScAddress.Tab();
                            SetTabNo( nTab );
                        }
                    }
                    else
                        aScAddress.SetTab( nTab );

                    aScRange = ScRange( aScAddress, aScAddress );
                    // cells should not be marked
                    bMark = false;
                }
                // Is it a named area (first named ranges then database ranges)?
                else
                {
                    ScRangeUtil     aRangeUtil;
                    formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
                    if( ScRangeUtil::MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_NAMES, eConv ) ||
                        ScRangeUtil::MakeRangeFromName( aAddress, pDoc, nTab, aScRange, RUTL_DBASE, eConv ) )
                    {
                        nResult |= ScRefFlags::VALID;
                        if( aScRange.aStart.Tab() != nTab )
                        {
                            nTab = aScRange.aStart.Tab();
                            SetTabNo( nTab );
                        }
                    }
                }

                if ( !(nResult & ScRefFlags::VALID) && comphelper::string::isdigitAsciiString(aAddress) )
                {
                    sal_Int32 nNumeric = aAddress.toInt32();
                    if ( nNumeric > 0 && nNumeric <= MAXROW+1 )
                    {
                        // one-based row numbers

                        aScAddress.SetRow( static_cast<SCROW>(nNumeric - 1) );
                        aScAddress.SetCol( rViewData.GetCurX() );
                        aScAddress.SetTab( nTab );
                        aScRange = ScRange( aScAddress, aScAddress );
                        bMark    = false;
                        nResult  = ScRefFlags::VALID;
                    }
                }

                if ( !ValidRow(aScRange.aStart.Row()) || !ValidRow(aScRange.aEnd.Row()) )
                    nResult = ScRefFlags::ZERO;

                // we have found something
                if( nResult & ScRefFlags::VALID )
                {
                    bFound = true;
                    SCCOL nCol = aScRange.aStart.Col();
                    SCROW nRow = aScRange.aStart.Row();
                    bool bNothing = ( rViewData.GetCurX()==nCol && rViewData.GetCurY()==nRow );

                    // mark
                    if( bMark )
                    {
                        if (rMark.IsMarked())           // is the same range already marked?
                        {
                            ScRange aOldMark;
                            rMark.GetMarkArea( aOldMark );
                            aOldMark.PutInOrder();
                            ScRange aCurrent = aScRange;
                            aCurrent.PutInOrder();
                            bNothing = ( aCurrent == aOldMark );
                        }
                        else
                            bNothing = false;

                        if (!bNothing)
                            MarkRange( aScRange, false );   // cursor comes after...
                    }
                    else
                    {
                        //  remove old selection, unless bUnmark argument is sal_False (from navigator)
                        if( bUnmark )
                        {
                            MoveCursorAbs( nCol, nRow,
                                SC_FOLLOW_NONE, false, false );
                        }
                    }

                    // and set cursor

                    // consider merged cells:
                    pDoc->SkipOverlapped(nCol, nRow, nTab);

                    // navigator calls are not part of the API!!!

                    if( bNothing )
                    {
                        if (rReq.IsAPI())
                            rReq.Ignore();      // if macro, then nothing
                        else
                            rReq.Done();        // then at least paint it
                    }
                    else
                    {
                        rViewData.ResetOldCursor();
                        SetCursor( nCol, nRow );
                        rBindings.Invalidate( SID_CURRENTCELL );
                        rBindings.Update( nSlot );

                        if (!rReq.IsAPI())
                            rReq.Done();
                    }

                    if (bAlignToCursor)
                    {
                        // align to cursor even if the cursor position hasn't changed,
                        // because the cursor may be set outside the visible area.
                        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
                    }

                    rReq.SetReturnValue( SfxStringItem( SID_CURRENTCELL, aAddress ) );
                }

                if (!bFound)    // no valid range
                {
                    // if it is a sheet name, then switch (for Navigator/URL)

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
                    // test graphics objects (only for URL)

                    bFound = SelectObject( aAddress );
                }

                if (!bFound && !rReq.IsAPI())
                    ErrorMessage( STR_ERR_INVALID_AREA );
            }
            break;

        case SID_CURRENTOBJECT:
            if ( pReqArgs )
            {
                OUString aName = static_cast<const SfxStringItem&>(pReqArgs->Get(nSlot)).GetValue();
                SelectObject( aName );
            }
            break;

        case SID_CURRENTTAB:
            if ( pReqArgs )
            {
                // sheet for basic is one-based
                SCTAB nTab = static_cast<const SfxUInt16Item&>(pReqArgs->Get(nSlot)).GetValue() - 1;
                ScDocument* pDoc = GetViewData().GetDocument();
                if ( nTab < pDoc->GetTableCount() )
                {
                    SetTabNo( nTab );
                    rBindings.Update( nSlot );

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                //! otherwise an error ?
            }
            break;

        case SID_CURRENTDOC:
            if ( pReqArgs )
            {
                OUString aStrDocName( static_cast<const SfxStringItem&>(pReqArgs->
                                        Get(nSlot)).GetValue() );

                SfxViewFrame*   pViewFrame = nullptr;
                ScDocShell*     pDocSh = static_cast<ScDocShell*>(SfxObjectShell::GetFirst());
                bool            bFound = false;

                // search for ViewFrame to be activated

                while ( pDocSh && !bFound )
                {
                    if ( pDocSh->GetTitle() == aStrDocName )
                    {
                        pViewFrame = SfxViewFrame::GetFirst( pDocSh );
                        bFound = ( nullptr != pViewFrame );
                    }

                    pDocSh = static_cast<ScDocShell*>(SfxObjectShell::GetNext( *pDocSh ));
                }

                if ( bFound )
                    pViewFrame->GetFrame().Appear();

                rReq.Ignore();//XXX is handled by SFX
            }
            break;

        case SID_PRINTPREVIEW:
            {
                if ( !pThisFrame->GetFrame().IsInPlace() )          // not for OLE
                {
                    //  print preview is now always in the same frame as the tab view
                    //  -> always switch this frame back to normal view
                    //  (ScPreviewShell ctor reads view data)

                    // #102785#; finish input
                    pScMod->InputEnterHandler();

                    pThisFrame->GetDispatcher()->Execute( SID_VIEWSHELL1, SfxCallMode::ASYNCHRON );
                }
                //  else error (e.g. Ole)
            }
            break;

        case SID_DETECTIVE_DEL_ALL:
            DetectiveDelAll();
            rReq.Done();
            break;

        // SID_TABLE_ACTIVATE and SID_MARKAREA are called by basic for the
        // hidden View, to mark/switch on the visible View:

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

                // check whether there is an explicit argument, use it
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                {
                    bool bItemValue = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    bWantPageBreak = (nSlot == FID_PAGEBREAKMODE) == bItemValue;
                }

                if( GetViewData().IsPagebreakMode() != bWantPageBreak )
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
                // First make sure that the sidebar is visible
                pThisFrame->ShowChildWindow(SID_SIDEBAR);

                ::sfx2::sidebar::Sidebar::ShowPanel("ScFunctionsPanel",
                                                    pThisFrame->GetFrame().GetFrameInterface());
                rReq.Done ();
            }
            break;

        case FID_TOGGLESYNTAX:
            {
                bool bSet = !GetViewData().IsSyntaxMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                GetViewData().SetSyntaxMode( bSet );
                PaintGrid();
                rBindings.Invalidate( FID_TOGGLESYNTAX );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;
        case FID_TOGGLEHEADERS:
            {
                bool bSet = !GetViewData().IsHeaderMode();
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                GetViewData().SetHeaderMode( bSet );
                RepeatResize();
                rBindings.Invalidate( FID_TOGGLEHEADERS );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;

        case FID_TOGGLEFORMULA:
            {
                ScViewData& rViewData = GetViewData();
                const ScViewOptions& rOpts = rViewData.GetOptions();
                bool bFormulaMode = !rOpts.GetOption( VOPT_FORMULAS );
                const SfxPoolItem *pItem;
                if( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bFormulaMode = static_cast<const SfxBoolItem *>(pItem)->GetValue();

                ScViewOptions aSetOpts = rOpts;
                aSetOpts.SetOption( VOPT_FORMULAS, bFormulaMode );
                rViewData.SetOptions( aSetOpts );

                rViewData.GetDocShell()->PostPaintGridAll();

                rBindings.Invalidate( FID_TOGGLEFORMULA );
                rReq.AppendItem( SfxBoolItem( nSlot, bFormulaMode ) );
                rReq.Done();
            }
            break;

        case FID_TOGGLEINPUTLINE:
            {
                sal_uInt16          nId  = ScInputWindowWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                bool bSet = ( pWnd == nullptr );
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                pThisFrame->SetChildWindow( nId, bSet );
                rBindings.Invalidate( FID_TOGGLEINPUTLINE );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;

        case SID_ATTR_ZOOM: // status row
        case FID_SCALE:
            {
                bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                SvxZoomType eOldZoomType = GetZoomType();
                SvxZoomType eNewZoomType = eOldZoomType;
                const Fraction& rOldY = GetViewData().GetZoomY();  // Y is shown
                sal_uInt16 nOldZoom = static_cast<sal_uInt16>(long( rOldY * 100 ));
                sal_uInt16 nZoom = nOldZoom;
                bool bCancel = false;

                if ( pReqArgs )
                {
                    const SvxZoomItem& rZoomItem = pReqArgs->Get(SID_ATTR_ZOOM);

                    eNewZoomType = rZoomItem.GetType();
                    nZoom     = rZoomItem.GetValue();
                }
                else
                {
                    SfxItemSet      aSet     ( GetPool(), svl::Items<SID_ATTR_ZOOM, SID_ATTR_ZOOM>{} );
                    SvxZoomItem     aZoomItem( eOldZoomType, nOldZoom, SID_ATTR_ZOOM );
                    ScopedVclPtr<AbstractSvxZoomDialog> pDlg;
                    ScMarkData&     rMark = GetViewData().GetMarkData();
                    SvxZoomEnableFlags nBtnFlags = SvxZoomEnableFlags::N50
                                                | SvxZoomEnableFlags::N75
                                                | SvxZoomEnableFlags::N100
                                                | SvxZoomEnableFlags::N150
                                                | SvxZoomEnableFlags::N200
                                                | SvxZoomEnableFlags::WHOLEPAGE
                                                | SvxZoomEnableFlags::PAGEWIDTH;

                    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
                        nBtnFlags = nBtnFlags | SvxZoomEnableFlags::OPTIMAL;

                    aZoomItem.SetValueSet( nBtnFlags );
                    aSet.Put( aZoomItem );
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    vcl::Window* pWin = GetDialogParent();
                    pDlg.disposeAndReset(pFact->CreateSvxZoomDialog(pWin ? pWin->GetFrameWeld() : nullptr, aSet));
                    pDlg->SetLimits( MINZOOM, MAXZOOM );

                    bCancel = ( RET_CANCEL == pDlg->Execute() );

                    // bCancel is True only if we were in the previous if block,
                    // so no need to check again pDlg
                    if ( !bCancel )
                    {
                        const SvxZoomItem&  rZoomItem = pDlg->GetOutputItemSet()->
                                                    Get( SID_ATTR_ZOOM );

                        eNewZoomType = rZoomItem.GetType();
                        nZoom     = rZoomItem.GetValue();
                    }
                }

                if ( !bCancel )
                {
                    if ( eNewZoomType == SvxZoomType::PERCENT )
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
                        case SvxZoomType::WHOLEPAGE:
                        case SvxZoomType::PAGEWIDTH:
                            SetZoomType( eNewZoomType, bSyncZoom );
                            break;

                        default:
                            SetZoomType( SvxZoomType::PERCENT, bSyncZoom );
                    }
                }

                if ( nZoom != nOldZoom && !bCancel )
                {
                    if (!GetViewData().IsPagebreakMode())
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
                const SfxPoolItem* pItem = nullptr;
                bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
                if ( pReqArgs && pReqArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem) == SfxItemState::SET )
                {
                    const sal_uInt16 nCurrentZoom = static_cast<const SvxZoomSliderItem *>(pItem)->GetValue();
                    if( nCurrentZoom )
                    {
                        SetZoomType( SvxZoomType::PERCENT, bSyncZoom );
                        if (!GetViewData().IsPagebreakMode())
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
            ScViewData& rViewData = GetViewData();
            ScDocument& rDoc = *rViewData.GetDocument();
            ScMarkData& rMark = rViewData.GetMarkData();
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nTab;

            ::std::vector < sal_Int32 > aIndexList;
            const SfxIntegerListItem* pItem = rReq.GetArg<SfxIntegerListItem>(SID_SELECT_TABLES);
            if ( pItem )
                aIndexList = pItem->GetList();
            else
            {
                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                ScopedVclPtr<AbstractScShowTabDlg> pDlg(pFact->CreateScShowTabDlg(GetFrameWeld()));
                pDlg->SetDescription(
                    ScResId( STR_DLG_SELECTTABLES_TITLE ),
                    ScResId( STR_DLG_SELECTTABLES_LBNAME ),
                    GetStaticInterface()->GetSlot(SID_SELECT_TABLES)->GetCommand(), HID_SELECTTABLES );

                // fill all table names with selection state
                OUString aTabName;
                for( nTab = 0; nTab < nTabCount; ++nTab )
                {
                    rDoc.GetName( nTab, aTabName );
                    pDlg->Insert( aTabName, rMark.GetTableSelect( nTab ) );
                }

                if( pDlg->Execute() == RET_OK )
                {
                    aIndexList = pDlg->GetSelectedRows();
                    pDlg.disposeAndClear();
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

                // special case: only hidden tables selected -> do nothing
                bool bVisSelected = false;
                for( nSelIx = 0; !bVisSelected && (nSelIx < nSelCount); ++nSelIx )
                {
                    nFirstVisTab = static_cast<SCTAB>(aIndexList[nSelIx]);
                    bVisSelected = rDoc.IsVisible( nFirstVisTab );
                }
                if( !bVisSelected )
                    nSelCount = 0;

                // select the tables
                if( nSelCount )
                {
                    for( nTab = 0; nTab < nTabCount; ++nTab )
                        rMark.SelectTable( nTab, false );

                    for( nSelIx = 0; nSelIx < nSelCount; ++nSelIx )
                        rMark.SelectTable( static_cast<SCTAB>(aIndexList[nSelIx]), true );

                    // activate another table, if current is deselected
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
                ScSplitMode eHSplit = GetViewData().GetHSplitMode();
                ScSplitMode eVSplit = GetViewData().GetVSplitMode();
                if ( eHSplit == SC_SPLIT_NORMAL || eVSplit == SC_SPLIT_NORMAL )     // remove
                    RemoveSplit();
                else if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )      // normal
                    FreezeSplitters( false );
                else                                                                // create
                    SplitAtCursor();
                rReq.Done();

                InvalidateSplit();
            }
            break;

        case SID_WINDOW_FIX:
            {
                ScSplitMode eHSplit = GetViewData().GetHSplitMode();
                ScSplitMode eVSplit = GetViewData().GetVSplitMode();
                if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )           // remove
                    RemoveSplit();
                else
                    FreezeSplitters( true, SC_SPLIT_METHOD_CURSOR);        // create or fixate

                rReq.Done();
                InvalidateSplit();
            }
            break;

        case SID_WINDOW_FIX_COL:
            {
                FreezeSplitters( true, SC_SPLIT_METHOD_FIRST_COL);
                rReq.Done();
                InvalidateSplit();
            }
            break;

        case SID_WINDOW_FIX_ROW:
            {
                FreezeSplitters( true, SC_SPLIT_METHOD_FIRST_ROW);
                rReq.Done();
                InvalidateSplit();
            }
            break;

        case FID_CHG_SHOW:
            {
                sal_uInt16          nId  = ScHighlightChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
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
                ScViewData& rData = GetViewData();
                ScAddress aCursorPos( rData.GetCurX(), rData.GetCurY(), rData.GetTabNo() );
                ScDocShell* pDocSh = rData.GetDocShell();

                ScChangeAction* pAction = pDocSh->GetChangeAction( aCursorPos );
                if ( pAction )
                {
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                         pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET &&
                         dynamic_cast<const SfxStringItem*>( pItem) !=  nullptr )
                    {
                        OUString aComment = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        pDocSh->SetChangeComment( pAction, aComment );
                        rReq.Done();
                    }
                    else
                    {
                        pDocSh->ExecuteChangeCommentDialog(pAction, GetFrameWeld());
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_CREATE_SW_DRAWVIEW:
            //  is called by Forms, when the DrawView has to be created with all
            //  the extras
            if (!GetScDrawView())
            {
                GetViewData().GetDocShell()->MakeDrawLayer();
                rBindings.InvalidateAll(false);
            }
            break;

        case FID_PROTECT_DOC:
            {
                ScDocument*         pDoc = GetViewData().GetDocument();

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_PROTECT_DOC, &pItem ) &&
                        static_cast<const SfxBoolItem*>(pItem)->GetValue() == pDoc->IsDocProtected() )
                    {
                        rReq.Ignore();
                        break;
                    }
                }

                vcl::Window* pWin = GetDialogParent();
                ScDocProtection* pProtect = pDoc->GetDocProtection();
                if (pProtect && pProtect->isProtected())
                {
                    bool bCancel = false;
                    OUString aPassword;

                    if (pProtect->isProtectedWithPass())
                    {
                        OUString aText(ScResId(SCSTR_PASSWORD));

                        SfxPasswordDialog aDlg(pWin ? pWin->GetFrameWeld() : nullptr, &aText);
                        aDlg.set_title(ScResId(SCSTR_UNPROTECTDOC));
                        aDlg.SetMinLen(0);
                        aDlg.set_help_id(GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand());
                        aDlg.SetEditHelpId(HID_PASSWD_DOC);

                        if (aDlg.run() == RET_OK)
                            aPassword = aDlg.GetPassword();
                        else
                            bCancel = true;
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

                    SfxPasswordDialog aDlg(pWin ? pWin->GetFrameWeld() : nullptr, &aText);
                    aDlg.set_title(ScResId(SCSTR_PROTECTDOC));
                    aDlg.SetMinLen( 0 );
                    aDlg.set_help_id(GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand());
                    aDlg.SetEditHelpId(HID_PASSWD_DOC);
                    aDlg.ShowExtras(SfxShowExtras::CONFIRM);
                    aDlg.SetConfirmHelpId(HID_PASSWD_DOC_CONFIRM);

                    if (aDlg.run() == RET_OK)
                    {
                        OUString aPassword = aDlg.GetPassword();
                        Protect( TABLEID_DOC, aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, true ) );
                        rReq.Done();
                    }
                }
                rBindings.Invalidate( FID_PROTECT_DOC );
            }
            break;

        case FID_PROTECT_TABLE:
        {
            ScDocument* pDoc = GetViewData().GetDocument();
            SCTAB       nTab = GetViewData().GetTabNo();
            bool        bOldProtection = pDoc->IsTabProtected(nTab);

            if( pReqArgs )
            {
                const SfxPoolItem* pItem;
                bool bNewProtection = !bOldProtection;
                if( pReqArgs->HasItem( FID_PROTECT_TABLE, &pItem ) )
                    bNewProtection = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                if( bNewProtection == bOldProtection )
                {
                    rReq.Ignore();
                    break;
                }
            }

            if (bOldProtection)
            {
                // Unprotect a protected sheet.

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect && pProtect->isProtectedWithPass())
                {
                    OUString aText( ScResId(SCSTR_PASSWORDOPT) );
                    vcl::Window* pWin = GetDialogParent();
                    SfxPasswordDialog aDlg(pWin ? pWin->GetFrameWeld() : nullptr, &aText);
                    aDlg.set_title(ScResId(SCSTR_UNPROTECTTAB));
                    aDlg.SetMinLen(0);
                    aDlg.set_help_id(GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand());
                    aDlg.SetEditHelpId(HID_PASSWD_TABLE);

                    if (aDlg.run() == RET_OK)
                    {
                        OUString aPassword = aDlg.GetPassword();
                        Unprotect(nTab, aPassword);
                    }
                }
                else
                    // this sheet is not password-protected.
                    Unprotect(nTab, OUString());

                if (!pReqArgs)
                {
                    rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
                    rReq.Done();
                }
            }
            else
            {
                // Protect a current sheet.

                vcl::Window* pWin = GetDialogParent();
                ScTableProtectionDlg aDlg(pWin ? pWin->GetFrameWeld() : nullptr);

                ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
                if (pProtect)
                    aDlg.SetDialogData(*pProtect);

                if (aDlg.run() == RET_OK)
                {
                    pScMod->InputEnterHandler();

                    ScTableProtection aNewProtect;
                    aDlg.WriteData(aNewProtect);
                    ProtectSheet(nTab, aNewProtect);
                    if (!pReqArgs)
                    {
                        rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, true) );
                        rReq.Done();
                    }
                }
            }
            TabChanged();
            UpdateInputHandler(true);   // to immediately enable input again
            SelectionChanged();
        }
        break;

        case SID_OPT_LOCALE_CHANGED :
            {   // locale changed, SYSTEM number formats changed => repaint cell contents
                PaintGrid();
                rReq.Done();
            }
            break;

        default:
            OSL_FAIL("Unknown Slot at ScTabViewShell::Execute");
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
