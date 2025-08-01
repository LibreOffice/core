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
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

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
#include <docsh.hxx>
#include <rangeutl.hxx>
#include <reffact.hxx>
#include <tabprotection.hxx>
#include <protectiondlg.hxx>
#include <duplicaterecordsdlg.hxx>
#include <markdata.hxx>

#include <svl/ilstitem.hxx>
#include <vector>

#include <svx/zoomslideritem.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <sfx2/lokhelper.hxx>
#include <scabstdlg.hxx>
#include <officecfg/Office/Calc.hxx>

#include <basegfx/utils/zoomtools.hxx>

#include <svx/dialog/ThemeDialog.hxx>
#include <ThemeColorChanger.hxx>

namespace
{
    void collectUIInformation(const OUString& aZoom)
    {
        EventDescription aDescription;
        aDescription.aID = "grid_window";
        aDescription.aParameters = {{"ZOOM", aZoom}};
        aDescription.aAction = "SET";
        aDescription.aKeyWord = "ScGridWinUIObject";
        aDescription.aParent = "MainWindow";
        UITestLogger::getInstance().logEvent(aDescription);
    }

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
                                              const OUString& aAddress, const ScDocument& rDoc,
                                              SCCOL nCurCol, SCROW nCurRow)
    {
        ScRefFlagsAndType aRet;

        // Relative address parsing needs current position.
        // row,col parameters, not col,row!
        ScAddress::Details aDetails( rDoc.GetAddressConvention(), nCurRow, nCurCol);

        // start with the address convention set in the document
        aRet.nResult = rScRange.Parse(aAddress, rDoc, aDetails);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, rDoc, aDetails);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try the default Calc (A1) address convention
        aRet.nResult = rScRange.Parse(aAddress, rDoc);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, rDoc);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try the Excel A1 address convention
        aRet.nResult = rScRange.Parse(aAddress, rDoc, formula::FormulaGrammar::CONV_XL_A1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        // try the Excel A1 address convention
        aRet.nResult = rScAddress.Parse(aAddress, rDoc, formula::FormulaGrammar::CONV_XL_A1);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::ADDRESS;
            return aRet;
        }

        // try Excel R1C1 address convention
        aDetails.eConv = formula::FormulaGrammar::CONV_XL_R1C1;
        aRet.nResult = rScRange.Parse(aAddress, rDoc, aDetails);
        if (aRet.nResult & ScRefFlags::VALID)
        {
            aRet.eDetected = DetectFlags::RANGE;
            return aRet;
        }

        aRet.nResult = rScAddress.Parse(aAddress, rDoc, aDetails);
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

void ScTabViewShell::ExecGoToTab( SfxRequest& rReq, SfxBindings& rBindings )
{
    SCTAB nTab;
    ScViewData& rViewData = GetViewData();
    ScDocument& rDoc = rViewData.GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();

    if ( pReqArgs ) // command from Navigator with nTab
    {
        // sheet for basic is one-based
        nTab = static_cast<const SfxUInt16Item&>(pReqArgs->Get(nSlot)).GetValue() - 1;
        if ( nTab < nTabCount )
        {
            SetTabNo( nTab );
            rBindings.Update( nSlot );

            if( ! rReq.IsAPI() )
                rReq.Done();
        }
    }
    else            // command from Menu: ask for nTab
    {
        auto xRequest = std::make_shared<SfxRequest>(rReq);
        rReq.Ignore();

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

        VclPtr<AbstractScGoToTabDlg> pDlg(pFact->CreateScGoToTabDlg(GetFrameWeld()));
        pDlg->SetDescription(
            ScResId( STR_DLG_SELECTTABLE_TITLE ),
            ScResId( STR_DLG_SELECTTABLE_MASK ),
            ScResId( STR_DLG_SELECTTABLE_LBNAME ),
            GetStaticInterface()->GetSlot(SID_CURRENTTAB)->GetCommand(), HID_GOTOTABLEMASK, HID_GOTOTABLE );

        // fill all table names and select current tab
        OUString aTabName;
        for( nTab = 0; nTab < nTabCount; ++nTab )
        {
            if( rDoc.IsVisible( nTab ) )
            {
                rDoc.GetName( nTab, aTabName );
                pDlg->Insert( aTabName, rViewData.GetTabNo() == nTab );
            }
        }

        pDlg->StartExecuteAsync([this, nTab, nTabCount, pDlg,
                                 xRequest=std::move(xRequest)](sal_Int32 response) {
            if( response == RET_OK )
            {
                auto nTab2 = nTab;
                if( !GetViewData().GetDocument().GetTable( pDlg->GetSelectedEntry(), nTab2 ) )
                    nTab2 = nTabCount;
                if ( nTab2 < nTabCount )
                {
                    SetTabNo( nTab2 );

                    if ( !xRequest->IsAPI() )
                        xRequest->Done();
                }
            }
            else
            {
                xRequest->Ignore();
            }
            pDlg->disposeOnce();
        });
    }
}

void ScTabViewShell::FinishProtectTable()
{
    TabChanged();
    UpdateInputHandler(true);   // to immediately enable input again
    SelectionChanged();
}

void ScTabViewShell::ExecProtectTable( SfxRequest& rReq )
{
    ScModule* pScMod = ScModule::get();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    ScDocument&         rDoc = GetViewData().GetDocument();
    SCTAB               nTab = GetViewData().GetTabNo();
    bool                bOldProtection = rDoc.IsTabProtected(nTab);

    if( pReqArgs )
    {
        const SfxPoolItem* pItem;
        bool bNewProtection = !bOldProtection;
        if( pReqArgs->HasItem( FID_PROTECT_TABLE, &pItem ) )
            bNewProtection = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        if( bNewProtection == bOldProtection )
        {
            rReq.Ignore();
            return;
        }
    }

    if (bOldProtection)
    {
        // Unprotect a protected sheet.

        const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        if (pProtect && pProtect->isProtectedWithPass())
        {
            std::shared_ptr<SfxRequest> xRequest;
            if (!pReqArgs)
            {
                xRequest = std::make_shared<SfxRequest>(rReq);
                rReq.Ignore(); // the 'old' request is not relevant any more
            }

            OUString aText( ScResId(SCSTR_PASSWORDOPT) );
            auto pDlg = std::make_shared<SfxPasswordDialog>(GetFrameWeld(), &aText);
            pDlg->set_title(ScResId(SCSTR_UNPROTECTTAB));
            pDlg->SetMinLen(0);
            pDlg->set_help_id(GetStaticInterface()->GetSlot(FID_PROTECT_TABLE)->GetCommand());
            pDlg->SetEditHelpId(HID_PASSWD_TABLE);

            pDlg->PreRun();

            weld::DialogController::runAsync(pDlg, [this, nTab, pDlg,
                                                    xRequest=std::move(xRequest)](sal_Int32 response) {
                if (response == RET_OK)
                {
                    OUString aPassword = pDlg->GetPassword();
                    Unprotect(nTab, aPassword);
                }
                if (xRequest)
                {
                    xRequest->AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
                    xRequest->Done();
                }
                FinishProtectTable();
            });
            return;
        }
        else
            // this sheet is not password-protected.
            Unprotect(nTab, std::u16string_view());

        if (!pReqArgs)
        {
            rReq.AppendItem( SfxBoolItem(FID_PROTECT_TABLE, false) );
            rReq.Done();
        }
    }
    else
    {
        // Protect a current sheet.
        std::shared_ptr<SfxRequest> xRequest;
        if (!pReqArgs)
        {
            xRequest = std::make_shared<SfxRequest>(rReq);
            rReq.Ignore(); // the 'old' request is not relevant any more
        }

        auto pDlg = std::make_shared<ScTableProtectionDlg>(GetFrameWeld());

        const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        if (pProtect)
            pDlg->SetDialogData(*pProtect);
        weld::DialogController::runAsync(pDlg, [this, pDlg, pScMod, nTab,
                                               xRequest=std::move(xRequest)](sal_uInt32 nResult) {
            if (nResult == RET_OK)
            {
                pScMod->InputEnterHandler();

                ScTableProtection aNewProtect;
                pDlg->WriteData(aNewProtect);
                ProtectSheet(nTab, aNewProtect);
                if (xRequest)
                {
                    xRequest->AppendItem( SfxBoolItem(FID_PROTECT_TABLE, true) );
                    xRequest->Done();
                }
            }
            FinishProtectTable();
        });
        return;
    }
    FinishProtectTable();
}

void ScTabViewShell::Execute( SfxRequest& rReq )
{
    SfxViewFrame&       rThisFrame  = GetViewFrame();
    SfxBindings&        rBindings   = rThisFrame.GetBindings();
    ScModule* pScMod = ScModule::get();
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
                SfxChildWindow* pWnd = rThisFrame.GetChildWindow( nId );

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
            GetViewData().GetDocShell().
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

                bool bForceGlobalName = false;
                if (pReqArgs->GetItemState(FN_PARAM_3, true, &pItem) == SfxItemState::SET)
                    bForceGlobalName = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                if ( nSlot == SID_JUMPTOMARK )
                {
                    //  URL has to be decoded for escaped characters (%20)
                    aAddress = INetURLObject::decode( aAddress,
                                               INetURLObject::DecodeMechanism::WithCharset );
                }

                bool bFound = false;
                ScViewData& rViewData = GetViewData();
                ScDocument& rDoc      = rViewData.GetDocument();
                ScMarkData& rMark     = rViewData.GetMarkData();
                ScRange     aScRange;
                ScAddress   aScAddress;
                ScRefFlagsAndType aResult = lcl_ParseRangeOrAddress(aScRange, aScAddress, aAddress, rDoc,
                        rViewData.GetCurX(), rViewData.GetCurY());
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
                    const RutlNameScope eScope = (bForceGlobalName ? RUTL_NAMES_GLOBAL : RUTL_NAMES);
                    ScAddress::Details aDetails( rDoc.GetAddressConvention(), rViewData.GetCurY(), rViewData.GetCurX());
                    if (ScRangeUtil::MakeRangeFromName( aAddress, rDoc, nTab, aScRange, eScope, aDetails, true) ||
                        ScRangeUtil::MakeRangeFromName( aAddress, rDoc, nTab, aScRange, RUTL_DBASE, aDetails, true))
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
                    if ( nNumeric > 0 && nNumeric <= rDoc.MaxRow()+1 )
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

                if ( !rDoc.ValidRow(aScRange.aStart.Row()) || !rDoc.ValidRow(aScRange.aEnd.Row()) )
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
                            ScRange aOldMark = rMark.GetMarkArea();
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
                    rDoc.SkipOverlapped(nCol, nRow, nTab);

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
                        if ( nSlot == SID_JUMPTOMARK && comphelper::LibreOfficeKit::isActive() )
                            rViewData.GetActiveWin()->notifyKitCellFollowJump();
                    }

                    rReq.SetReturnValue( SfxStringItem( SID_CURRENTCELL, aAddress ) );
                }

                if (!bFound)    // no valid range
                {
                    // if it is a sheet name, then switch (for Navigator/URL)

                    SCTAB nNameTab;
                    if ( rDoc.GetTable( aAddress, nNameTab ) )
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
            {
                ExecGoToTab( rReq, rBindings );
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
                if ( !rThisFrame.GetFrame().IsInPlace() )          // not for OLE
                {
                    //  print preview is now always in the same frame as the tab view
                    //  -> always switch this frame back to normal view
                    //  (ScPreviewShell ctor reads view data)

                    // #102785#; finish input
                    pScMod->InputEnterHandler();

                    rThisFrame.GetDispatcher()->Execute( SID_VIEWSHELL1, SfxCallMode::ASYNCHRON );
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
                rThisFrame.ShowChildWindow(SID_SIDEBAR);

                ::sfx2::sidebar::Sidebar::ShowPanel(u"ScFunctionsPanel",
                                                    rThisFrame.GetFrame().GetFrameInterface(),
                                                    true);
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
        case FID_HANDLEDUPLICATERECORDS:
            {
                using namespace com::sun::star;
                table::CellRangeAddress aCellRange;
                uno::Reference<sheet::XSpreadsheet> xActiveSheet;
                DuplicatesResponse aResponse;
                bool bHasData = true;

                if (pReqArgs)
                {
                    const SfxPoolItem* pItem;

                    if (pReqArgs->HasItem(FID_HANDLEDUPLICATERECORDS, &pItem))
                        aResponse.bRemove = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_1, &pItem))
                        aResponse.bIncludesHeaders = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_2, &pItem))
                        aResponse.bDuplicateRows = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_3, &pItem))
                        aCellRange.StartColumn = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_4, &pItem))
                        aCellRange.StartRow = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_5, &pItem))
                        aCellRange.EndColumn = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_6, &pItem))
                        aCellRange.EndRow = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                    if (pReqArgs->HasItem(FN_PARAM_7, &pItem))
                        aCellRange.Sheet = static_cast<const SfxInt32Item*>(pItem)->GetValue();

                    // check for the tab range here
                    if (aCellRange.StartColumn < 0 || aCellRange.StartRow < 0
                        || aCellRange.EndColumn < 0 || aCellRange.EndRow < 0
                        || aCellRange.StartRow > aCellRange.EndRow
                        || aCellRange.StartColumn > aCellRange.EndColumn || aCellRange.Sheet < 0
                        || aCellRange.Sheet >= GetViewData().GetDocument().GetTableCount())
                    {
                        rReq.Done();
                        break;
                    }
                    xActiveSheet = GetViewData().GetViewShell()->GetRangeWithSheet(aCellRange,
                                                                                   bHasData, true);
                    if (!bHasData)
                    {
                        rReq.Done();
                        break;
                    }
                    int nLenEntries
                        = (aResponse.bDuplicateRows ? aCellRange.EndColumn - aCellRange.StartColumn
                                                   : aCellRange.EndRow - aCellRange.StartRow);
                    for (int i = 0; i <= nLenEntries; ++i)
                        aResponse.vEntries.push_back(i);
                }
                else
                {
                    xActiveSheet = GetViewData().GetViewShell()->GetRangeWithSheet(aCellRange,
                                                                                   bHasData, false);
                    if (bHasData)
                    {
                        if (!GetViewData().GetMarkData().IsMarked())
                            GetViewData().GetViewShell()->ExtendSingleSelection(aCellRange);

                        uno::Reference<frame::XModel> xModel(GetViewData().GetDocShell().GetModel());
                        uno::Reference<sheet::XSheetCellRange> xSheetRange(
                                xActiveSheet->getCellRangeByPosition(
                                    aCellRange.StartColumn, aCellRange.StartRow, aCellRange.EndColumn,
                                    aCellRange.EndRow),
                                uno::UNO_QUERY);

                        ScRange aRange(ScAddress(aCellRange.StartColumn, aCellRange.StartRow,
                                    GetViewData().GetTabNo()),
                                ScAddress(aCellRange.EndColumn, aCellRange.EndRow,
                                    GetViewData().GetTabNo()));

                        uno::Reference<sheet::XCellRangeData> xCellRangeData(xSheetRange,
                                uno::UNO_QUERY);
                        uno::Sequence<uno::Sequence<uno::Any>> aDataArray
                            = xCellRangeData->getDataArray();

                        ScDuplicateRecordsDlg aDlg(GetFrameWeld(), aDataArray, GetViewData(), aRange);

                        bHasData = aDlg.run();
                        if (bHasData)
                            aResponse = aDlg.GetDialogData();
                        else
                        {
                            rReq.Done();
                            break;
                        }
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> aDialog(
                            Application::CreateMessageDialog(GetFrameWeld(),
                                                             VclMessageType::Warning,
                                                             VclButtonsType::Ok,
                                                             ScResId(STR_DUPLICATERECORDSDLG_NODATAFOUND)));
                        aDialog->set_title(ScResId(STR_DUPLICATERECORDSDLG_WARNING));
                        aDialog->run();
                    }
                }

                if (bHasData)
                    GetViewData().GetViewShell()->HandleDuplicateRecords(
                            xActiveSheet, aCellRange, aResponse.bRemove, aResponse.bIncludesHeaders,
                            aResponse.bDuplicateRows, aResponse.vEntries);

                rReq.Done();
            }
            break;
        case FID_TOGGLECOLROWHIGHLIGHTING:
            {
                bool bNewVal = !officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::get();

                auto pChange(comphelper::ConfigurationChanges::create());
                officecfg::Office::Calc::Content::Display::ColumnRowHighlighting::set(bNewVal, pChange);
                pChange->commit();

                rReq.AppendItem(SfxBoolItem(nSlot, bNewVal));
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
                bool bFormulaMode = !rOpts.GetOption(sc::ViewOption::FORMULAS);
                const SfxPoolItem *pItem;
                if( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bFormulaMode = static_cast<const SfxBoolItem *>(pItem)->GetValue();

                ScViewOptions aSetOpts = rOpts;
                aSetOpts.SetOption(sc::ViewOption::FORMULAS, bFormulaMode);
                rViewData.SetOptions( aSetOpts );
                ScDocument& rDoc = rViewData.GetDocument();
                rDoc.SetViewOptions(aSetOpts);

                rViewData.GetDocShell().PostPaintGridAll();

                rBindings.Invalidate( FID_TOGGLEFORMULA );
                rReq.AppendItem( SfxBoolItem( nSlot, bFormulaMode ) );
                rReq.Done();
            }
            break;

        case FID_TOGGLEINPUTLINE:
            {
                sal_uInt16          nId  = ScInputWindowWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = rThisFrame.GetChildWindow( nId );
                bool bSet = ( pWnd == nullptr );
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                rThisFrame.SetChildWindow( nId, bSet );
                rBindings.Invalidate( FID_TOGGLEINPUTLINE );
                rReq.AppendItem( SfxBoolItem( nSlot, bSet ) );
                rReq.Done();
            }
            break;

        // handling for SID_ZOOM_IN and SID_ZOOM_OUT is ScTabView::ScrollCommand
        // CommandWheelMode::ZOOM inspired
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
            {
                HideNoteOverlay();

                if (!GetViewData().GetViewShell()->GetViewFrame().GetFrame().IsInPlace())
                {
                    //  for ole inplace editing, the scale is defined by the visarea and client size
                    //  and can't be changed directly

                    const Fraction& rOldY = GetViewData().GetZoomY();
                    sal_uInt16 nOld = tools::Long(rOldY * 100);
                    sal_uInt16 nNew;
                    if (SID_ZOOM_OUT == nSlot)
                        nNew = std::max(MINZOOM, basegfx::zoomtools::zoomOut(nOld));
                    else
                        nNew = std::min(MAXZOOM, basegfx::zoomtools::zoomIn(nOld));
                    if ( nNew != nOld)
                    {
                        bool bSyncZoom = pScMod->GetAppOptions().GetSynchronizeZoom();
                        SetZoomType(SvxZoomType::PERCENT, bSyncZoom);
                        Fraction aFract(nNew, 100);
                        SetZoom(aFract, aFract, bSyncZoom);
                        PaintGrid();
                        PaintTop();
                        PaintLeft();
                        rBindings.Invalidate(SID_ATTR_ZOOM);
                        rBindings.Invalidate(SID_ATTR_ZOOMSLIDER);
                        rBindings.Invalidate(SID_ZOOM_IN);
                        rBindings.Invalidate(SID_ZOOM_OUT);
                        rReq.Done();
                    }
                }
            }
            break;

        case SID_ATTR_ZOOM: // status row
        case FID_SCALE:
            {
                bool bSyncZoom = pScMod->GetAppOptions().GetSynchronizeZoom();
                SvxZoomType eOldZoomType = GetZoomType();
                SvxZoomType eNewZoomType = eOldZoomType;
                const Fraction& rOldY = GetViewData().GetZoomY();  // Y is shown
                sal_uInt16 nOldZoom = static_cast<sal_uInt16>(tools::Long( rOldY * 100 ));
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
                    SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM> aSet( GetPool() );
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
                    pDlg.disposeAndReset(pFact->CreateSvxZoomDialog(GetFrameWeld(), aSet));
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
                    rReq.AppendItem( SvxZoomItem( GetZoomType(), nZoom, TypedWhichId<SvxZoomItem>(nSlot) ) );
                    rReq.Done();
                }
            }
            break;

        case SID_ATTR_ZOOMSLIDER:
            {
                const SfxPoolItem* pItem = nullptr;
                bool bSyncZoom = pScMod->GetAppOptions().GetSynchronizeZoom();
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
                            collectUIInformation(OUString::number(nCurrentZoom));
                            aNewOpt.SetZoomType( GetZoomType() );
                            pScMod->SetAppOptions( aNewOpt );
                        }
                        Fraction aFract( nCurrentZoom,100 );
                        SetZoom( aFract, aFract, bSyncZoom );
                        PaintGrid();
                        PaintTop();
                        PaintLeft();
                        rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
                        rBindings.Invalidate( SID_ZOOM_IN );
                        rBindings.Invalidate( SID_ZOOM_OUT );
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
            ScDocument& rDoc = rViewData.GetDocument();
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
                    rReq.AppendItem( SfxIntegerListItem( SID_SELECT_TABLES, std::vector(aIndexList) ) );
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

                    rViewData.GetDocShell().PostPaintExtras();
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
                if (!comphelper::LibreOfficeKit::isActive())
                {
                    ScSplitMode eHSplit = GetViewData().GetHSplitMode();
                    ScSplitMode eVSplit = GetViewData().GetVSplitMode();
                    if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )           // remove
                        RemoveSplit();
                    else
                        FreezeSplitters( true, SC_SPLIT_METHOD_CURSOR);                 // create or fixate
                    rReq.Done();
                    InvalidateSplit();
                }
                else
                {
                    ScViewData& rViewData = GetViewData();
                    SCTAB nThisTab = rViewData.GetTabNo();
                    bool bChangedX = false, bChangedY = false;
                    if (rViewData.GetLOKSheetFreezeIndex(true) > 0 ||
                        rViewData.GetLOKSheetFreezeIndex(false) > 0 )                             // remove freeze
                    {
                        bChangedX = rViewData.RemoveLOKFreeze();
                    }                                                                            // create or fixate
                    else
                    {
                        bChangedX = rViewData.SetLOKSheetFreezeIndex(rViewData.GetCurX(), true);    // Freeze column
                        bChangedY = rViewData.SetLOKSheetFreezeIndex(rViewData.GetCurY(), false);   // Freeze row
                    }

                    rReq.Done();
                    if (bChangedX || bChangedY)
                    {
                        rBindings.Invalidate( SID_WINDOW_FIX );
                        rBindings.Invalidate( SID_WINDOW_FIX_COL );
                        rBindings.Invalidate( SID_WINDOW_FIX_ROW );
                        // Invalidate the slot for all views on the same tab of the document.
                        SfxLokHelper::forEachOtherView(this, [nThisTab](ScTabViewShell* pOther) {
                            ScViewData& rOtherViewData = pOther->GetViewData();
                            if (rOtherViewData.GetTabNo() != nThisTab)
                                return;

                            SfxBindings& rOtherBind = rOtherViewData.GetBindings();
                            rOtherBind.Invalidate( SID_WINDOW_FIX );
                            rOtherBind.Invalidate( SID_WINDOW_FIX_COL );
                            rOtherBind.Invalidate( SID_WINDOW_FIX_ROW );
                        });
                        if (!GetViewData().GetDocShell().IsReadOnly())
                            GetViewData().GetDocShell().SetDocumentModified();
                    }
                }
            }
            break;

        case SID_WINDOW_FIX_COL:
        case SID_WINDOW_FIX_ROW:
            {
                bool bIsCol = (nSlot == SID_WINDOW_FIX_COL);
                sal_Int32 nFreezeIndex = 1;
                if (const SfxInt32Item* pItem = rReq.GetArg<SfxInt32Item>(FN_PARAM_1))
                {
                    nFreezeIndex = pItem->GetValue();
                    if (nFreezeIndex < 0)
                        nFreezeIndex = 0;
                }

                if (comphelper::LibreOfficeKit::isActive())
                {
                    ScViewData& rViewData = GetViewData();
                    SCTAB nThisTab = rViewData.GetTabNo();
                    bool bChanged = rViewData.SetLOKSheetFreezeIndex(nFreezeIndex, bIsCol);
                    rReq.Done();
                    if (bChanged)
                    {
                        rBindings.Invalidate( SID_WINDOW_FIX );
                        rBindings.Invalidate(nSlot);
                        // Invalidate the slot for all views on the same tab of the document.
                        SfxLokHelper::forEachOtherView(this, [nSlot, nThisTab](ScTabViewShell* pOther) {
                            ScViewData& rOtherViewData = pOther->GetViewData();
                            if (rOtherViewData.GetTabNo() != nThisTab)
                                return;

                            SfxBindings& rOtherBind = rOtherViewData.GetBindings();
                            rOtherBind.Invalidate( SID_WINDOW_FIX );
                            rOtherBind.Invalidate(nSlot);
                        });
                        if (!GetViewData().GetDocShell().IsReadOnly())
                            GetViewData().GetDocShell().SetDocumentModified();
                    }
                }
                else
                {
                    FreezeSplitters( true, bIsCol ? SC_SPLIT_METHOD_COL : SC_SPLIT_METHOD_ROW, nFreezeIndex);
                    rReq.Done();
                    InvalidateSplit();
                }
            }
            break;

        case FID_CHG_SHOW:
            {
                sal_uInt16          nId  = ScHighlightChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = rThisFrame.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case FID_CHG_ACCEPT:
            {
                rThisFrame.ToggleChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId());
                GetViewFrame().GetBindings().Invalidate(FID_CHG_ACCEPT);
                rReq.Done ();

                /*
                sal_uInt16          nId  = ScAcceptChgDlgWrapper::GetChildWindowId();
                SfxChildWindow* pWnd = rThisFrame.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
                */
            }
            break;

        case FID_CHG_COMMENT:
            {
                ScViewData& rData = GetViewData();
                ScAddress aCursorPos( rData.GetCurX(), rData.GetCurY(), rData.GetTabNo() );
                ScDocShell& rDocSh = rData.GetDocShell();

                ScChangeAction* pAction = rDocSh.GetChangeAction( aCursorPos );
                if ( pAction )
                {
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                         pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET &&
                         dynamic_cast<const SfxStringItem*>( pItem) !=  nullptr )
                    {
                        OUString aComment = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        rDocSh.SetChangeComment( pAction, aComment );
                        rReq.Done();
                    }
                    else
                    {
                        rDocSh.ExecuteChangeCommentDialog(pAction, GetFrameWeld());
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
                GetViewData().GetDocShell().MakeDrawLayer();
                rBindings.InvalidateAll(false);
            }
            break;

        case FID_PROTECT_DOC:
            {
                ScDocument& rDoc = GetViewData().GetDocument();

                if( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_PROTECT_DOC, &pItem ) &&
                        static_cast<const SfxBoolItem*>(pItem)->GetValue() == rDoc.IsDocProtected() )
                    {
                        rReq.Ignore();
                        break;
                    }
                }

                ScDocProtection* pProtect = rDoc.GetDocProtection();
                if (pProtect && pProtect->isProtected())
                {
                    bool bCancel = false;
                    OUString aPassword;

                    if (pProtect->isProtectedWithPass())
                    {
                        OUString aText(ScResId(SCSTR_PASSWORD));

                        SfxPasswordDialog aDlg(GetFrameWeld(), &aText);
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

                    SfxPasswordDialog aDlg(GetFrameWeld(), &aText);
                    aDlg.set_title(ScResId(SCSTR_PROTECTDOC));
                    aDlg.SetMinLen( 0 );
                    aDlg.set_help_id(GetStaticInterface()->GetSlot(FID_PROTECT_DOC)->GetCommand());
                    aDlg.SetEditHelpId(HID_PASSWD_DOC);
                    aDlg.ShowExtras(SfxShowExtras::CONFIRM);
                    aDlg.SetConfirmHelpId(HID_PASSWD_DOC_CONFIRM);

                    if (aDlg.run() == RET_OK)
                    {
                        OUString aPassword = aDlg.GetPassword();
                        ProtectDoc( aPassword );
                        rReq.AppendItem( SfxBoolItem( FID_PROTECT_DOC, true ) );
                        rReq.Done();
                    }
                }
                rBindings.Invalidate( FID_PROTECT_DOC );
            }
            break;

        case FID_PROTECT_TABLE:
            ExecProtectTable( rReq );
            break;

        case SID_THEME_DIALOG:
        {
            MakeDrawLayer();
            ScViewData& rViewData = GetViewData();
            ScDocument& rDocument = rViewData.GetDocument();
            ScDrawLayer* pModel = rDocument.GetDrawLayer();
            auto const& pTheme = pModel->getTheme();
            if (pTheme)
            {
                vcl::Window* pWin = rViewData.GetActiveWin();
                auto pDialog = std::make_shared<svx::ThemeDialog>(pWin ? pWin->GetFrameWeld() : nullptr, pTheme.get());
                weld::DialogController::runAsync(pDialog, [this, pDialog](sal_uInt32 nResult) {
                    if (RET_OK != nResult)
                        return;

                    auto pColorSet = pDialog->getCurrentColorSet();
                    if (pColorSet)
                    {
                        sc::ThemeColorChanger aChanger(GetViewData().GetDocShell());
                        aChanger.apply(pColorSet);
                    }
                });
            }
            rReq.Done();
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
