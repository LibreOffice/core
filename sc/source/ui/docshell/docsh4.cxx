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

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

using namespace ::com::sun::star;

#include <math.h>

#include "scitems.hxx"
#include <sfx2/fcontnr.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/ehdl.hxx>
#include <basic/sbxcore.hxx>
#include <svtools/sfxecode.hxx>
#include <svx/ofaitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/drawitem.hxx>
#include <svx/fmshell.hxx>
#include <svtools/xwindowitem.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
#include <unotools/securityoptions.hxx>

#include <comphelper/processfactory.hxx>
#include "docuno.hxx"

#include <com/sun/star/sdbc/XResultSet.hpp>
#include "docsh.hxx"
#include "docshimp.hxx"
#include "docfunc.hxx"
#include "sc.hrc"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "appoptio.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"
#include "printfun.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "attrib.hxx"
#include "undodat.hxx"
#include "autostyl.hxx"
#include "undocell.hxx"
#include "undotab.hxx"
#include "inputhdl.hxx"
#include "dbdata.hxx"
#include "servobj.hxx"
#include "rangenam.hxx"
#include "scmod.hxx"
#include "chgviset.hxx"
#include "reffact.hxx"
#include "chartlis.hxx"
#include "chartpos.hxx"
#include "waitoff.hxx"
#include "tablink.hxx"
#include "drwlayer.hxx"
#include "docoptio.hxx"
#include "undostyl.hxx"
#include "rangeseq.hxx"
#include "chgtrack.hxx"
#include "printopt.hxx"
#include <com/sun/star/document/UpdateDocMode.hpp>
#include "scresid.hxx"
#include "scabstdlg.hxx"
#include "externalrefmgr.hxx"
#include "sharedocdlg.hxx"
#include "conditio.hxx"
#include "sheetevents.hxx"
#include <documentlinkmgr.hxx>
#include <memory>

#define IS_SHARE_HEADER(set) \
    static_cast<const SfxBoolItem&>( \
        static_cast<const SvxSetItem&>((set).Get(ATTR_PAGE_HEADERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

#define IS_SHARE_FOOTER(set) \
    static_cast<const SfxBoolItem&>( \
        static_cast<const SvxSetItem&>((set).Get(ATTR_PAGE_FOOTERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

void ScDocShell::Execute( SfxRequest& rReq )
{
    //  SID_SC_RANGE (Range),
    //  SID_SC_CELLTEXT (CellText),
    //  SID_SC_CELLS (Cells) - removed (old Basic)

    const SfxItemSet* pReqArgs = rReq.GetArgs();
    SfxBindings* pBindings = GetViewBindings();
    bool bUndo (aDocument.IsUndoEnabled());

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_SC_SETTEXT:
        {
            const SfxPoolItem* pColItem;
            const SfxPoolItem* pRowItem;
            const SfxPoolItem* pTabItem;
            const SfxPoolItem* pTextItem;
            if( pReqArgs && pReqArgs->HasItem( FN_PARAM_1, &pColItem ) &&
                            pReqArgs->HasItem( FN_PARAM_2, &pRowItem ) &&
                            pReqArgs->HasItem( FN_PARAM_3, &pTabItem ) &&
                            pReqArgs->HasItem( SID_SC_SETTEXT, &pTextItem ) )
            {
                //  Parameter sind 1-based !!!
                SCCOL nCol = static_cast<const SfxInt16Item*>(pColItem)->GetValue() - 1;
                SCROW nRow = static_cast<const SfxInt32Item*>(pRowItem)->GetValue() - 1;
                SCTAB nTab = static_cast<const SfxInt16Item*>(pTabItem)->GetValue() - 1;

                SCTAB nTabCount = aDocument.GetTableCount();
                if ( ValidCol(nCol) && ValidRow(nRow) && ValidTab(nTab,nTabCount) )
                {
                    if ( aDocument.IsBlockEditable( nTab, nCol,nRow, nCol, nRow ) )
                    {
                        OUString aVal = static_cast<const SfxStringItem*>(pTextItem)->GetValue();
                        aDocument.SetString( nCol, nRow, nTab, aVal );

                        PostPaintCell( nCol, nRow, nTab );
                        SetDocumentModified();

                        rReq.Done();
                        break;
                    }
                    else                // geschuetzte Zelle
                    {
#if HAVE_FEATURE_SCRIPTING
                        SbxBase::SetError( ERRCODE_SBX_BAD_PARAMETER );      //! welchen Fehler ?
#endif
                        break;
                    }
                }
            }
#if HAVE_FEATURE_SCRIPTING
            SbxBase::SetError( ERRCODE_SBX_NO_OBJECT );
#endif
        }
        break;

        case SID_SBA_IMPORT:
        {
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                svx::ODataAccessDescriptor aDesc;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    uno::Any aAny = static_cast<const SfxUsrAnyItem*>(pItem)->GetValue();
                    uno::Sequence<beans::PropertyValue> aProperties;
                    if ( aAny >>= aProperties )
                        aDesc.initializeFrom( aProperties );
                }

                OUString sTarget;
                if ( pReqArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
                    sTarget = static_cast<const SfxStringItem*>(pItem)->GetValue();

                bool bIsNewArea = true;         // Default sal_True (keine Nachfrage)
                if ( pReqArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SfxItemState::SET )
                    bIsNewArea = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                // bei Bedarf neuen Datenbankbereich anlegen
                bool bMakeArea = false;
                if (bIsNewArea)
                {
                    ScDBCollection* pDBColl = aDocument.GetDBCollection();
                    if ( !pDBColl || !pDBColl->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(sTarget)) )
                    {
                        ScAddress aPos;
                        if ( aPos.Parse( sTarget, &aDocument, aDocument.GetAddressConvention() ) & SCA_VALID )
                        {
                            bMakeArea = true;
                            if (bUndo)
                            {
                                OUString aStrImport = ScGlobal::GetRscString( STR_UNDO_IMPORTDATA );
                                GetUndoManager()->EnterListAction( aStrImport, aStrImport );
                            }

                            ScDBData* pDBData = GetDBData( ScRange(aPos), SC_DB_IMPORT, SC_DBSEL_KEEP );
                            OSL_ENSURE(pDBData, "kann DB-Daten nicht anlegen");
                            sTarget = pDBData->GetName();
                        }
                    }
                }

                // nachfragen, bevor alter DB-Bereich ueberschrieben wird
                bool bDo = true;
                if (!bIsNewArea)
                {
                    OUString aTemplate = ScGlobal::GetRscString( STR_IMPORT_REPLACE );
                    OUString aMessage = aTemplate.getToken( 0, '#' );
                    aMessage += sTarget;
                    aMessage += aTemplate.getToken( 1, '#' );

                    ScopedVclPtrInstance< QueryBox > aBox( nullptr, WinBits(WB_YES_NO | WB_DEF_YES), aMessage );
                    bDo = ( aBox->Execute() == RET_YES );
                }

                if (bDo)
                {
                    ScDBDocFunc(*this).UpdateImport( sTarget, aDesc );
                    rReq.Done();

                    //  UpdateImport aktualisiert auch die internen Operationen
                }
                else
                    rReq.Ignore();

                if ( bMakeArea && bUndo)
                    GetUndoManager()->LeaveListAction();
            }
            else
            {
                OSL_FAIL( "arguments expected" );
            }
        }
        break;

        case SID_CHART_SOURCE:
        case SID_CHART_ADDSOURCE:
            if (pReqArgs)
            {
                ScDocument& rDoc = GetDocument();
                const   SfxPoolItem* pItem;
                OUString  aChartName, aRangeName;

                ScRange         aSingleRange;
                ScRangeListRef  aRangeListRef;
                bool            bMultiRange = false;

                bool bColHeaders = true;
                bool bRowHeaders = true;
                bool bColInit = false;
                bool bRowInit = false;
                bool bAddRange = (nSlot == SID_CHART_ADDSOURCE);

                if( pReqArgs->HasItem( SID_CHART_NAME, &pItem ) )
                    aChartName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( SID_CHART_SOURCE, &pItem ) )
                    aRangeName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                {
                    bColHeaders = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    bColInit = true;
                }
                if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                {
                    bRowHeaders = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    bRowInit = true;
                }

                ScAddress::Details aDetails(rDoc.GetAddressConvention(), 0, 0);
                bool bValid = ( aSingleRange.ParseAny( aRangeName, &rDoc, aDetails ) & SCA_VALID ) != 0;
                if (!bValid)
                {
                    aRangeListRef = new ScRangeList;
                    aRangeListRef->Parse( aRangeName, &rDoc );
                    if ( !aRangeListRef->empty() )
                    {
                        bMultiRange = true;
                        aSingleRange = *aRangeListRef->front(); // fuer Header
                        bValid = true;
                    }
                    else
                        aRangeListRef.Clear();
                }

                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                if (pViewSh && bValid && !aChartName.isEmpty() )
                {
                    vcl::Window* pParent = pViewSh->GetDialogParent();

                    SCCOL nCol1 = aSingleRange.aStart.Col();
                    SCROW nRow1 = aSingleRange.aStart.Row();
                    SCCOL nCol2 = aSingleRange.aEnd.Col();
                    SCROW nRow2 = aSingleRange.aEnd.Row();
                    SCTAB nTab = aSingleRange.aStart.Tab();

                    //! immer oder gar nicht begrenzen ???
                    if (!bMultiRange)
                        aDocument.LimitChartArea( nTab, nCol1,nRow1, nCol2,nRow2 );

                                        // Dialog fuer Spalten/Zeilenkoepfe
                    bool bOk = true;
                    if ( !bAddRange && ( !bColInit || !bRowInit ) )
                    {
                        ScChartPositioner aChartPositioner( &aDocument, nTab, nCol1,nRow1, nCol2,nRow2 );
                        if (!bColInit)
                            bColHeaders = aChartPositioner.HasColHeaders();
                        if (!bRowInit)
                            bRowHeaders = aChartPositioner.HasRowHeaders();

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScColRowLabelDlg> pDlg(pFact->CreateScColRowLabelDlg(pParent, bRowHeaders, bColHeaders));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                        {
                            bColHeaders = pDlg->IsRow();
                            bRowHeaders = pDlg->IsCol();

                            rReq.AppendItem(SfxBoolItem(FN_PARAM_1, bColHeaders));
                            rReq.AppendItem(SfxBoolItem(FN_PARAM_2, bRowHeaders));
                        }
                        else
                            bOk = false;
                    }

                    if (bOk)            // ausfuehren
                    {
                        if (bMultiRange)
                        {
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aRangeListRef,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            aDocument.UpdateChartArea( aChartName, aRangeListRef,
                                                        bColHeaders, bRowHeaders, bAddRange );
                        }
                        else
                        {
                            ScRange aNewRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aNewRange,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            aDocument.UpdateChartArea( aChartName, aNewRange,
                                                        bColHeaders, bRowHeaders, bAddRange );
                        }
                    }
                }
                else
                {
                    OSL_FAIL("UpdateChartArea: keine ViewShell oder falsche Daten");
                }
                rReq.Done();
            }
            else
            {
                OSL_FAIL("SID_CHART_SOURCE ohne Argumente");
            }
            break;

        case FID_AUTO_CALC:
            {
                bool bNewVal;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                    bNewVal = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                    bNewVal = !aDocument.GetAutoCalc();     // Toggle fuer Menue
                aDocument.SetAutoCalc( bNewVal );
                SetDocumentModified();
                if (pBindings)
                {
                    pBindings->Invalidate( FID_AUTO_CALC );
                }
                rReq.AppendItem( SfxBoolItem( FID_AUTO_CALC, bNewVal ) );
                rReq.Done();
            }
            break;
        case FID_RECALC:
            DoRecalc( rReq.IsAPI() );
            rReq.Done();
            break;
        case FID_HARD_RECALC:
            DoHardRecalc( rReq.IsAPI() );
            rReq.Done();
            break;
        case SID_UPDATETABLINKS:
            {
                ScDocument& rDoc = GetDocument();

                ScLkUpdMode nSet = rDoc.GetLinkMode();

                sal_uInt16 nDlgRet=RET_NO;
                if(nSet==LM_UNKNOWN)
                {
                    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
                    nSet=aAppOptions.GetLinkMode();
                }

                if (nCanUpdate == css::document::UpdateDocMode::NO_UPDATE)
                    nSet = LM_NEVER;
                else if (nCanUpdate == css::document::UpdateDocMode::FULL_UPDATE)
                    nSet = LM_ALWAYS;

                if (nSet == LM_ALWAYS
                    && !(SvtSecurityOptions()
                         .isTrustedLocationUriForUpdatingLinks(
                             GetMedium() == nullptr
                             ? OUString() : GetMedium()->GetName())))
                {
                    nSet = LM_ON_DEMAND;
                }
                if (nCanUpdate == css::document::UpdateDocMode::QUIET_UPDATE
                    && nSet == LM_ON_DEMAND)
                {
                    nSet = LM_NEVER;
                }

                if(nSet==LM_ON_DEMAND)
                {
                    ScopedVclPtrInstance<QueryBox> aBox( GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                             ScGlobal::GetRscString(STR_RELOAD_TABLES) );

                    nDlgRet=aBox->Execute();
                }

                if (nDlgRet == RET_YES || nSet==LM_ALWAYS)
                {
                    ReloadTabLinks();
                    aDocument.UpdateExternalRefLinks(GetActiveDialogParent());

                    bool bAny = aDocument.GetDocLinkManager().updateDdeLinks(GetActiveDialogParent());

                    if (bAny)
                    {
                        //  Formeln berechnen und painten wie im TrackTimeHdl
                        aDocument.TrackFormulas();
                        Broadcast(SfxSimpleHint(FID_DATACHANGED));

                        //  wenn FID_DATACHANGED irgendwann mal asynchron werden sollte
                        //  (z.B. mit Invalidate am Window), muss hier ein Update erzwungen werden.
                    }

                    aDocument.UpdateAreaLinks();

                    //! Test, ob Fehler
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_REIMPORT_AFTER_LOAD:
            {
                //  wird nach dem Laden aufgerufen, wenn DB-Bereiche mit
                //  weggelassenen Daten enthalten sind

                bool bDone = false;
                ScDBCollection* pDBColl = aDocument.GetDBCollection();

                if ((nCanUpdate != css::document::UpdateDocMode::NO_UPDATE) &&
                   (nCanUpdate != css::document::UpdateDocMode::QUIET_UPDATE))
                {
                    ScRange aRange;
                    ScTabViewShell* pViewSh = GetBestViewShell();
                    OSL_ENSURE(pViewSh,"SID_REIMPORT_AFTER_LOAD: keine View");
                    if (pViewSh && pDBColl)
                    {
                        ScopedVclPtrInstance<QueryBox> aBox( GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                                ScGlobal::GetRscString(STR_REIMPORT_AFTER_LOAD) );
                        if (aBox->Execute() == RET_YES)
                        {
                            ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
                            ScDBCollection::NamedDBs::iterator itr = rDBs.begin(), itrEnd = rDBs.end();
                            for (; itr != itrEnd; ++itr)
                            {
                                ScDBData& rDBData = **itr;
                                if ( rDBData.IsStripData() &&
                                     rDBData.HasImportParam() && !rDBData.HasImportSelection() )
                                {
                                    rDBData.GetArea(aRange);
                                    pViewSh->MarkRange(aRange);

                                    //  Import und interne Operationen wie SID_REFRESH_DBAREA
                                    //  (Abfrage auf Import hier nicht noetig)

                                    ScImportParam aImportParam;
                                    rDBData.GetImportParam( aImportParam );
                                    bool bContinue = pViewSh->ImportData( aImportParam );
                                    rDBData.SetImportParam( aImportParam );

                                    //  markieren (Groesse kann sich geaendert haben)
                                    rDBData.GetArea(aRange);
                                    pViewSh->MarkRange(aRange);

                                    if ( bContinue )    // Fehler beim Import -> Abbruch
                                    {
                                        //  interne Operationen, wenn welche gespeichert

                                        if ( rDBData.HasQueryParam() || rDBData.HasSortParam() ||
                                             rDBData.HasSubTotalParam() )
                                            pViewSh->RepeatDB();

                                        //  Pivottabellen die den Bereich als Quelldaten haben

                                        RefreshPivotTables(aRange);
                                    }
                                }
                            }
                            bDone = true;
                        }
                    }
                }

                if ( !bDone && pDBColl )
                {
                    //  wenn nicht, dann aber die abhaengigen Formeln updaten
                    //! auch fuer einzelne Bereiche, die nicht aktualisiert werden koennen

                    aDocument.CalcAll();        //! nur die abhaengigen
                    PostDataChanged();
                }

                if (bDone)
                    rReq.Done();
                else
                    rReq.Ignore();
            }
            break;

        case SID_AUTO_STYLE:
            OSL_FAIL("use ScAutoStyleHint instead of SID_AUTO_STYLE");
            break;

        case SID_GET_COLORLIST:
            {
                const SvxColorListItem* pColItem = static_cast<const SvxColorListItem*>(GetItem(SID_COLOR_TABLE));
                XColorListRef pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaRefItem<XColorList>(SID_GET_COLORLIST, pList));
            }
            break;

        case FID_CHG_RECORD:
            {
                ScDocument& rDoc = GetDocument();
                // get argument (recorded macro)
                const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(FID_CHG_RECORD);
                bool bDo = true;

                // xmlsec05/06:
                // getting real parent window when called from Security-Options TP
                vcl::Window* pParent = nullptr;
                const SfxPoolItem* pParentItem;
                if( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( SID_ATTR_XWINDOW, false, &pParentItem ) )
                    pParent = static_cast<const XWindowItem*>( pParentItem )->GetWindowPtr();

                // desired state
                ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
                bool bActivateTracking = (pChangeTrack == nullptr);   // toggle
                if ( pItem )
                    bActivateTracking = pItem->GetValue();      // from argument

                if ( !bActivateTracking )
                {
                    if ( !pItem )
                    {
                        // no dialog on playing the macro
                        ScopedVclPtrInstance<WarningBox> aBox( pParent ? pParent : GetActiveDialogParent(),
                            WinBits(WB_YES_NO | WB_DEF_NO),
                            ScGlobal::GetRscString( STR_END_REDLINING ) );
                        bDo = ( aBox->Execute() == RET_YES );
                    }

                    if ( bDo )
                    {
                        if (pChangeTrack)
                        {
                            if ( pChangeTrack->IsProtected() )
                                bDo = ExecuteChangeProtectionDialog( nullptr );
                        }
                        if ( bDo )
                        {
                            rDoc.EndChangeTracking();
                            PostPaintGridAll();
                        }
                    }
                }
                else
                {
                    rDoc.StartChangeTracking();
                    ScChangeViewSettings aChangeViewSet;
                    aChangeViewSet.SetShowChanges(true);
                    rDoc.SetChangeViewSettings(aChangeViewSet);
                }

                if ( bDo )
                {
                    UpdateAcceptChangesDialog();

                    // Slots invalidieren
                    if (pBindings)
                        pBindings->InvalidateAll(false);
                    if ( !pItem )
                        rReq.AppendItem( SfxBoolItem( FID_CHG_RECORD, bActivateTracking ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_CHG_PROTECT :
            {
                vcl::Window* pParent = nullptr;
                const SfxPoolItem* pParentItem;
                if( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( SID_ATTR_XWINDOW, false, &pParentItem ) )
                    pParent = static_cast<const XWindowItem*>( pParentItem )->GetWindowPtr();
                if ( ExecuteChangeProtectionDialog( pParent ) )
                {
                    rReq.Done();
                    SetDocumentModified();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_DOCUMENT_MERGE:
        case SID_DOCUMENT_COMPARE:
            {
                bool bDo = true;
                ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
                if ( pChangeTrack && !pImpl->bIgnoreLostRedliningWarning )
                {
                    if ( nSlot == SID_DOCUMENT_COMPARE )
                    {   //! old changes trace will be lost
                        ScopedVclPtrInstance<WarningBox> aBox( GetActiveDialogParent(),
                            WinBits(WB_YES_NO | WB_DEF_NO),
                            ScGlobal::GetRscString( STR_END_REDLINING ) );
                        if( aBox->Execute() == RET_YES )
                            bDo = ExecuteChangeProtectionDialog( nullptr, true );
                        else
                            bDo = false;
                    }
                    else    // merge might reject some actions
                        bDo = ExecuteChangeProtectionDialog( nullptr, true );
                }
                if ( !bDo )
                {
                    rReq.Ignore();
                    break;
                }
                SfxApplication* pApp = SfxGetpApp();
                const SfxPoolItem* pItem;
                const SfxStringItem* pStringItem(nullptr);
                SfxMedium* pMed = nullptr;
                if (pReqArgs && pReqArgs->GetItemState(SID_FILE_NAME, true, &pItem) == SfxItemState::SET)
                {
                    pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                }
                if (pStringItem)
                {
                    OUString aFileName = pStringItem->GetValue();

                    OUString aFilterName;
                    pStringItem = nullptr;
                    if (pReqArgs->GetItemState(SID_FILTER_NAME, true, &pItem) == SfxItemState::SET)
                        pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                    if (pStringItem)
                    {
                        aFilterName = pStringItem->GetValue();
                    }
                    OUString aOptions;
                    pStringItem = nullptr;
                    if (pReqArgs->GetItemState(SID_FILE_FILTEROPTIONS, true, &pItem) == SfxItemState::SET)
                        pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                    if (pStringItem)
                    {
                        aOptions = pStringItem->GetValue();
                    }
                    short nVersion = 0;
                    const SfxInt16Item* pInt16Item(nullptr);
                    if (pReqArgs->GetItemState(SID_VERSION, true, &pItem) == SfxItemState::SET)
                        pInt16Item = dynamic_cast<const SfxInt16Item*>(pItem);
                    if (pInt16Item)
                    {
                        nVersion = pInt16Item->GetValue();
                    }

                    //  kein Filter angegeben -> Detection
                    if (aFilterName.isEmpty())
                        ScDocumentLoader::GetFilterName( aFileName, aFilterName, aOptions, true, false );

                    //  filter name from dialog contains application prefix,
                    //  GetFilter needs name without the prefix.
                    ScDocumentLoader::RemoveAppPrefix( aFilterName );

                    const SfxFilter* pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( aFilterName );
                    SfxItemSet* pSet = new SfxAllItemSet( pApp->GetPool() );
                    if (!aOptions.isEmpty())
                        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
                    if ( nVersion != 0 )
                        pSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
                    pMed = new SfxMedium( aFileName, STREAM_STD_READ, pFilter, pSet );
                }
                else
                {
                    // start file dialog asynchronous
                    pImpl->bIgnoreLostRedliningWarning = true;
                    delete pImpl->pRequest;
                    pImpl->pRequest = new SfxRequest( rReq );
                    delete pImpl->pDocInserter;
                    pImpl->pDocInserter = new ::sfx2::DocumentInserter(
                        OUString::createFromAscii( ScDocShell::Factory().GetShortName() ), false );
                    pImpl->pDocInserter->StartExecuteModal( LINK( this, ScDocShell, DialogClosedHdl ) );
                    return ;
                }

                if ( pMed )     // nun wirklich ausfuehren...
                {
                    SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

                    // pOtherDocSh->DoClose() will be called explicitly later, but it is still more safe to use SfxObjectShellLock here
                    ScDocShell* pOtherDocSh = new ScDocShell;
                    SfxObjectShellLock aDocShTablesRef = pOtherDocSh;
                    pOtherDocSh->DoLoad( pMed );
                    sal_uLong nErr = pOtherDocSh->GetErrorCode();
                    if (nErr)
                        ErrorHandler::HandleError( nErr );          // auch Warnings

                    if ( !pOtherDocSh->GetError() )                 // nur Errors
                    {
                        bool bHadTrack = ( aDocument.GetChangeTrack() != nullptr );
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                        sal_uLong nStart = 0;
                        if ( nSlot == SID_DOCUMENT_MERGE && pChangeTrack )
                        {
                            nStart = pChangeTrack->GetActionMax() + 1;
                        }
#endif
                        if ( nSlot == SID_DOCUMENT_COMPARE )
                            CompareDocument( pOtherDocSh->GetDocument() );
                        else
                            MergeDocument( pOtherDocSh->GetDocument() );

                        //  show "accept changes" dialog
                        //! get view for this document!
                        if ( !IsDocShared() )
                        {
                            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                            if ( pViewFrm )
                            {
                                pViewFrm->ShowChildWindow( ScAcceptChgDlgWrapper::GetChildWindowId() ); //@51669
                            }
                            if ( pBindings )
                            {
                                pBindings->Invalidate( FID_CHG_ACCEPT );
                            }
                        }

                        rReq.SetReturnValue( SfxInt32Item( nSlot, 0 ) );        //! ???????
                        rReq.Done();

                        if (!bHadTrack)         //  neu eingeschaltet -> auch anzeigen
                        {
                            ScChangeViewSettings* pOldSet = aDocument.GetChangeViewSettings();
                            if ( !pOldSet || !pOldSet->ShowChanges() )
                            {
                                ScChangeViewSettings aChangeViewSet;
                                aChangeViewSet.SetShowChanges(true);
                                aDocument.SetChangeViewSettings(aChangeViewSet);
                            }
                        }
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                        else if ( nSlot == SID_DOCUMENT_MERGE && IsDocShared() && pChangeTrack )
                        {
                            sal_uLong nEnd = pChangeTrack->GetActionMax();
                            if ( nEnd >= nStart )
                            {
                                // only show changes from merged document
                                ScChangeViewSettings aChangeViewSet;
                                aChangeViewSet.SetShowChanges( true );
                                aChangeViewSet.SetShowAccepted( true );
                                aChangeViewSet.SetHasActionRange();
                                aChangeViewSet.SetTheActionRange( nStart, nEnd );
                                aDocument.SetChangeViewSettings( aChangeViewSet );

                                // update view
                                PostPaintExtras();
                                PostPaintGridAll();
                            }
                        }
#endif
                    }
                    pOtherDocSh->DoClose();     // delete passiert mit der Ref
                }
            }
            break;

        case SID_DELETE_SCENARIO:
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    if (const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>(pItem))
                    {
                        OUString aName = pStringItem->GetValue();
                        SCTAB nTab;
                        if (aDocument.GetTable( aName, nTab ))
                        {
                            //  DeleteTable von viewfunc nach docfunc verschieben!

                            ScTabViewShell* pSh = GetBestViewShell();
                            if ( pSh )
                            {
                                //! SetTabNo in DeleteTable weglassen?
                                SCTAB nDispTab = pSh->GetViewData().GetTabNo();
                                pSh->DeleteTable( nTab );
                                pSh->SetTabNo(nDispTab);
                                rReq.Done();
                            }
                        }
                    }
                }
            }
            break;

        case SID_EDIT_SCENARIO:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    if (const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>(pItem))
                    {
                        OUString aName = pStringItem->GetValue();
                        SCTAB nTab;
                        if (aDocument.GetTable( aName, nTab ))
                        {
                            if (aDocument.IsScenario(nTab))
                            {
                                OUString aComment;
                                Color aColor;
                                sal_uInt16 nFlags;
                                aDocument.GetScenarioData( nTab, aComment, aColor, nFlags );

                                // Determine if the Sheet that the Scenario was created on
                                // is protected. But first we need to find that Sheet.
                                // Rewind back to the actual sheet.
                                SCTAB nActualTab = nTab;
                                do
                                {
                                    nActualTab--;
                                }
                                while(aDocument.IsScenario(nActualTab));
                                bool bSheetProtected = aDocument.IsTabProtected(nActualTab);

                                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                                std::unique_ptr<AbstractScNewScenarioDlg> pNewDlg(pFact->CreateScNewScenarioDlg(GetActiveDialogParent(), aName, true, bSheetProtected));
                                OSL_ENSURE(pNewDlg, "Dialog create fail!");
                                pNewDlg->SetScenarioData( aName, aComment, aColor, nFlags );
                                if ( pNewDlg->Execute() == RET_OK )
                                {
                                    pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                    ModifyScenario( nTab, aName, aComment, aColor, nFlags );
                                    rReq.Done();
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_ATTR_YEAR2000 :
        {
            const SfxPoolItem* pItem;
            if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
            {
                if (const SfxUInt16Item* pInt16Item = dynamic_cast<const SfxUInt16Item*>(pItem))
                {
                    sal_uInt16 nY2k = pInt16Item->GetValue();
                    // immer an den DocOptions setzen, damit das auch fuer SO50
                    // gespeichert wird (und alle Abfragen bisher auch darauf laufen).
                    // SetDocOptions propagiert das an den NumberFormatter
                    ScDocOptions aDocOpt( aDocument.GetDocOptions() );
                    aDocOpt.SetYear2000( nY2k );
                    aDocument.SetDocOptions( aDocOpt );
                    // die FormShell soll es mitbekommen
                    ScTabViewShell* pSh = GetBestViewShell();
                    if ( pSh )
                    {
                        FmFormShell* pFSh = pSh->GetFormShell();
                        if ( pFSh )
                            pFSh->SetY2KState( nY2k );
                    }
                }
            }
        }
        break;

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        case SID_SHARE_DOC:
            {
                ScViewData* pViewData = GetViewData();
                if ( !pViewData )
                {
                    rReq.Ignore();
                    break;
                }

                ScopedVclPtrInstance< ScShareDocumentDlg > aDlg( GetActiveDialogParent(), pViewData );
                if ( aDlg->Execute() == RET_OK )
                {
                    bool bSetShared = aDlg->IsShareDocumentChecked();
                    if ( bSetShared != static_cast< bool >( IsDocShared() ) )
                    {
                        if ( bSetShared )
                        {
                            bool bContinue = true;
                            if ( HasName() )
                            {
                                ScopedVclPtrInstance<QueryBox> aBox(
                                    GetActiveDialogParent(),
                                    WinBits( WB_YES_NO | WB_DEF_YES ),
                                    ScGlobal::GetRscString( STR_DOC_WILLBESAVED ) );
                                if ( aBox->Execute() == RET_NO )
                                {
                                    bContinue = false;
                                }
                            }
                            if ( bContinue )
                            {
                                EnableSharedSettings( true );

                                SC_MOD()->SetInSharedDocSaving( true );
                                if ( !SwitchToShared( true, true ) )
                                {
                                    // TODO/LATER: what should be done in case the switch has failed?
                                    // for example in case the user has cancelled the saveAs operation
                                }

                                SC_MOD()->SetInSharedDocSaving( false );

                                InvalidateName();
                                GetUndoManager()->Clear();

                                ScTabView* pTabView = dynamic_cast< ScTabView* >( pViewData->GetView() );
                                if ( pTabView )
                                {
                                    pTabView->UpdateLayerLocks();
                                }
                            }
                        }
                        else
                        {
                            uno::Reference< frame::XModel > xModel;
                            try
                            {
                                // load shared file
                                xModel.set( LoadSharedDocument(), uno::UNO_QUERY_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
                                if ( pDocObj )
                                {
                                    ScDocShell* pDocShell = dynamic_cast< ScDocShell* >( pDocObj->GetEmbeddedObject() );
                                    if ( pDocShell )
                                    {
                                        bShared = pDocShell->HasSharedXMLFlagSet();
                                    }
                                }

                                // #i87870# check if shared status was disabled and enabled again
                                bool bOwnEntry = false;
                                try
                                {
                                    ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                                    bOwnEntry = aControlFile.HasOwnEntry();
                                }
                                catch ( uno::Exception& )
                                {
                                }

                                if ( bShared && bOwnEntry )
                                {
                                    uno::Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY_THROW );
                                    if ( xStorable->isReadonly() )
                                    {
                                        xCloseable->close( sal_True );

                                        OUString aUserName( ScGlobal::GetRscString( STR_UNKNOWN_USER ) );
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            LockFileEntry aData = aLockFile.GetLockData();
                                            if ( !aData[LockFileComponent::OOOUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::OOOUSERNAME];
                                            }
                                            else if ( !aData[LockFileComponent::SYSUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::SYSUSERNAME];
                                            }
                                        }
                                        catch ( uno::Exception& )
                                        {
                                        }
                                        OUString aMessage( ScGlobal::GetRscString( STR_FILE_LOCKED_TRY_LATER ) );
                                        aMessage = aMessage.replaceFirst( "%1", aUserName );

                                        ScopedVclPtrInstance< WarningBox > aBox( GetActiveDialogParent(), WinBits( WB_OK ), aMessage );
                                        aBox->Execute();
                                    }
                                    else
                                    {
                                        ScopedVclPtrInstance<WarningBox> aBox(
                                            GetActiveDialogParent(),
                                            WinBits( WB_YES_NO | WB_DEF_YES ),
                                            ScGlobal::GetRscString( STR_DOC_DISABLESHARED ) );
                                        if ( aBox->Execute() == RET_YES )
                                        {
                                            xCloseable->close( sal_True );

                                            if ( !SwitchToShared( false, true ) )
                                            {
                                                // TODO/LATER: what should be done in case the switch has failed?
                                                // for example in case the user has cancelled the saveAs operation
                                            }

                                            EnableSharedSettings( false );

                                            if ( pBindings )
                                            {
                                                pBindings->ExecuteSynchron( SID_SAVEDOC );
                                            }

                                            ScTabView* pTabView = dynamic_cast< ScTabView* >( pViewData->GetView() );
                                            if ( pTabView )
                                            {
                                                pTabView->UpdateLayerLocks();
                                            }
                                        }
                                        else
                                        {
                                            xCloseable->close( sal_True );
                                        }
                                    }
                                }
                                else
                                {
                                    xCloseable->close( sal_True );
                                    ScopedVclPtrInstance<WarningBox> aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                        ScGlobal::GetRscString( STR_DOC_NOLONGERSHARED ) );
                                    aBox->Execute();
                                }
                            }
                            catch ( uno::Exception& )
                            {
                                OSL_FAIL( "SID_SHARE_DOC: caught exception\n" );
                                SC_MOD()->SetInSharedDocSaving( false );

                                try
                                {
                                    uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
                                    xClose->close( sal_True );
                                }
                                catch ( uno::Exception& )
                                {
                                }
                            }
                        }
                    }
                }
                rReq.Done();
            }
            break;
#endif
        case SID_OPEN_CALC:
        {
            SfxStringItem aApp(SID_DOC_SERVICE, OUString("com.sun.star.sheet.SpreadsheetDocument"));
            SfxStringItem aTarget(SID_TARGETNAME, OUString("_blank"));
            GetViewData()->GetDispatcher().Execute(
                SID_OPENDOC, SfxCallMode::API|SfxCallMode::SYNCHRON, &aApp, &aTarget, 0L);
        }
        break;
        default:
        {
            // kleiner (?) Hack -> forward der Slots an TabViewShell
            ScTabViewShell* pSh = GetBestViewShell();
            if ( pSh )
                pSh->Execute( rReq );
#if HAVE_FEATURE_SCRIPTING
            else
                SbxBase::SetError( ERRCODE_SBX_NO_ACTIVE_OBJECT );
#endif
        }
    }
}

void UpdateAcceptChangesDialog()
{
    //  update "accept changes" dialog
    //! notify all views
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->HasChildWindow( FID_CHG_ACCEPT ) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow( FID_CHG_ACCEPT );
        if ( pChild )
            static_cast<ScAcceptChgDlgWrapper*>(pChild)->ReInitDlg();
    }
}

bool ScDocShell::ExecuteChangeProtectionDialog( vcl::Window* _pParent, bool bJustQueryIfProtected )
{
    bool bDone = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if ( pChangeTrack )
    {
        bool bProtected = pChangeTrack->IsProtected();
        if ( bJustQueryIfProtected && !bProtected )
            return true;

        OUString aTitle( ScResId( bProtected ? SCSTR_CHG_UNPROTECT : SCSTR_CHG_PROTECT ) );
        OUString aText( ScResId( SCSTR_PASSWORD ) );
        OUString aPassword;

        ScopedVclPtrInstance<SfxPasswordDialog> pDlg(
            _pParent ? _pParent : GetActiveDialogParent(), &aText );
        pDlg->SetText( aTitle );
        pDlg->SetMinLen( 1 );
        pDlg->SetHelpId( GetStaticInterface()->GetSlot(SID_CHG_PROTECT)->GetCommand() );
        pDlg->SetEditHelpId( HID_CHG_PROTECT );
        if ( !bProtected )
            pDlg->ShowExtras( SfxShowExtras::CONFIRM );
        if ( pDlg->Execute() == RET_OK )
            aPassword = pDlg->GetPassword();
        pDlg.disposeAndClear();

        if (!aPassword.isEmpty())
        {
            if ( bProtected )
            {
                if ( SvPasswordHelper::CompareHashPassword(pChangeTrack->GetProtection(), aPassword) )
                {
                    if ( bJustQueryIfProtected )
                        bDone = true;
                    else
                        pChangeTrack->SetProtection(
                            css::uno::Sequence< sal_Int8 > (0) );
                }
                else
                {
                    ScopedVclPtrInstance<InfoBox> aBox( GetActiveDialogParent(),
                        OUString( ScResId( SCSTR_WRONGPASSWORD ) ) );
                    aBox->Execute();
                }
            }
            else
            {
                css::uno::Sequence< sal_Int8 > aPass;
                SvPasswordHelper::GetHashPassword( aPass, aPassword );
                pChangeTrack->SetProtection( aPass );
            }
            if ( bProtected != pChangeTrack->IsProtected() )
            {
                UpdateAcceptChangesDialog();
                bDone = true;
            }
        }
    }
    else if ( bJustQueryIfProtected )
        bDone = true;
    return bDone;
}

void ScDocShell::DoRecalc( bool bApi )
{
    bool bDone = false;
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        ScInputHandler* pHdl = SC_MOD()->GetInputHdl(pSh);
        if ( pHdl && pHdl->IsInputMode() && pHdl->IsFormulaMode() && !bApi )
        {
            pHdl->FormulaPreview();     // Teilergebnis als QuickHelp
            bDone = true;
        }
        else
        {
            ScTabView::UpdateInputLine();     // InputEnterHandler
            pSh->UpdateInputHandler();
        }
    }
    if (!bDone)                         // sonst Dokument neu berechnen
    {
        WaitObject aWaitObj( GetActiveDialogParent() );
        aDocument.CalcFormulaTree();
        if ( pSh )
            pSh->UpdateCharts(true);

        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

        //  Wenn es Charts gibt, dann alles painten, damit nicht
        //  PostDataChanged und die Charts nacheinander kommen und Teile
        //  doppelt gepainted werden.

        ScChartListenerCollection* pCharts = aDocument.GetChartListenerCollection();
        if ( pCharts && pCharts->hasListeners() )
            PostPaintGridAll();
        else
            PostDataChanged();
    }
}

void ScDocShell::DoHardRecalc( bool /* bApi */ )
{
    WaitObject aWaitObj( GetActiveDialogParent() );
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        ScTabView::UpdateInputLine();     // InputEnterHandler
        pSh->UpdateInputHandler();
    }
    aDocument.CalcAll();
    GetDocFunc().DetectiveRefresh();    // erzeugt eigenes Undo
    if ( pSh )
        pSh->UpdateCharts(true);

    // set notification flags for "calculate" event (used in SFX_HINT_DATACHANGED broadcast)
    // (might check for the presence of any formulas on each sheet)
    SCTAB nTabCount = aDocument.GetTableCount();
    SCTAB nTab;
    if (aDocument.HasAnySheetEventScript( ScSheetEventId::CALCULATE, true )) // search also for VBA handler
        for (nTab=0; nTab<nTabCount; nTab++)
            aDocument.SetCalcNotification(nTab);

    // CalcAll doesn't broadcast value changes, so SC_HINT_CALCALL is broadcasted globally
    // in addition to SFX_HINT_DATACHANGED.
    aDocument.BroadcastUno( SfxSimpleHint( SC_HINT_CALCALL ) );
    aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

    // use hard recalc also to disable stream-copying of all sheets
    // (somewhat consistent with charts)
    for (nTab=0; nTab<nTabCount; nTab++)
        if (aDocument.IsStreamValid(nTab))
            aDocument.SetStreamValid(nTab, false);

    PostPaintGridAll();
}

void ScDocShell::DoAutoStyle( const ScRange& rRange, const OUString& rStyle )
{
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    ScStyleSheet* pStyleSheet =
        pStylePool->FindCaseIns( rStyle, SFX_STYLE_FAMILY_PARA );
    if (!pStyleSheet)
        pStyleSheet = static_cast<ScStyleSheet*>(
            pStylePool->Find( ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA ));
    if (pStyleSheet)
    {
        OSL_ENSURE(rRange.aStart.Tab() == rRange.aEnd.Tab(),
                        "DoAutoStyle mit mehreren Tabellen");
        SCTAB nTab = rRange.aStart.Tab();
        SCCOL nStartCol = rRange.aStart.Col();
        SCROW nStartRow = rRange.aStart.Row();
        SCCOL nEndCol = rRange.aEnd.Col();
        SCROW nEndRow = rRange.aEnd.Row();
        aDocument.ApplyStyleAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, *pStyleSheet );
        aDocument.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        PostPaint( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab, PAINT_GRID );
    }
}

void ScDocShell::NotifyStyle( const SfxStyleSheetHint& rHint )
{
    sal_uInt16 nId = rHint.GetHint();
    const SfxStyleSheetBase* pStyle = rHint.GetStyleSheet();
    if (!pStyle)
        return;

    if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PAGE )
    {
        if ( nId == SfxStyleSheetHintId::MODIFIED )
        {
            ScDocShellModificator aModificator( *this );

            OUString aNewName = pStyle->GetName();
            OUString aOldName = aNewName;
            const SfxStyleSheetHintExtended* pExtendedHint = dynamic_cast<const SfxStyleSheetHintExtended*>(&rHint); // Name geaendert?
            if (pExtendedHint)
                aOldName = pExtendedHint->GetOldName();

            if ( aNewName != aOldName )
                aDocument.RenamePageStyleInUse( aOldName, aNewName );

            SCTAB nTabCount = aDocument.GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                if (aDocument.GetPageStyle(nTab) == aNewName)   // schon auf neu angepasst
                {
                    aDocument.PageStyleModified( nTab, aNewName );
                    ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                }

            aModificator.SetDocumentModified();

            if (pExtendedHint)
            {
                SfxBindings* pBindings = GetViewBindings();
                if (pBindings)
                {
                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                    pBindings->Invalidate( SID_STYLE_FAMILY4 );
                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                    pBindings->Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
                    pBindings->Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
                }
            }
        }
    }
    else if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PARA )
    {
        if ( nId == SfxStyleSheetHintId::MODIFIED)
        {
            OUString aNewName = pStyle->GetName();
            OUString aOldName = aNewName;
            const SfxStyleSheetHintExtended* pExtendedHint = dynamic_cast<const SfxStyleSheetHintExtended*>(&rHint);
            if (pExtendedHint)
                aOldName = pExtendedHint->GetOldName();
            if ( aNewName != aOldName )
            {
                for(SCTAB i = 0; i < aDocument.GetTableCount(); ++i)
                {
                    ScConditionalFormatList* pList = aDocument.GetCondFormList(i);
                    if (pList)
                        pList->RenameCellStyle( aOldName,aNewName );
                }
            }
        }
    }

    //  alles andere geht ueber Slots...
}

//  wie in printfun.cxx
#define ZOOM_MIN    10

void ScDocShell::SetPrintZoom( SCTAB nTab, sal_uInt16 nScale, sal_uInt16 nPages )
{
    OUString aStyleName = aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        ScDocShellModificator aModificator( *this );

        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        const bool bUndo(aDocument.IsUndoEnabled());
        if (bUndo)
        {
            sal_uInt16 nOldScale = static_cast<const SfxUInt16Item&>(rSet.Get(ATTR_PAGE_SCALE)).GetValue();
            sal_uInt16 nOldPages = static_cast<const SfxUInt16Item&>(rSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
            GetUndoManager()->AddUndoAction( new ScUndoPrintZoom(
                            this, nTab, nOldScale, nOldPages, nScale, nPages ) );
        }

        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

        ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
        aPrintFunc.UpdatePages();
        aModificator.SetDocumentModified();

        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( FID_RESET_PRINTZOOM );
    }
}

bool ScDocShell::AdjustPrintZoom( const ScRange& rRange )
{
    bool bChange = false;
    SCTAB nTab = rRange.aStart.Tab();

    OUString aStyleName = aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bool bHeaders = static_cast<const SfxBoolItem&>(rSet.Get(ATTR_PAGE_HEADERS)).GetValue();
        sal_uInt16 nOldScale = static_cast<const SfxUInt16Item&>(rSet.Get(ATTR_PAGE_SCALE)).GetValue();
        sal_uInt16 nOldPages = static_cast<const SfxUInt16Item&>(rSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
        const ScRange* pRepeatCol = aDocument.GetRepeatColRange( nTab );
        const ScRange* pRepeatRow = aDocument.GetRepeatRowRange( nTab );

        //  benoetigte Skalierung fuer Selektion ausrechnen

        sal_uInt16 nNewScale = nOldScale;

        long nBlkTwipsX = 0;
        if (bHeaders)
            nBlkTwipsX += (long) PRINT_HEADER_WIDTH;
        SCCOL nStartCol = rRange.aStart.Col();
        SCCOL nEndCol = rRange.aEnd.Col();
        if ( pRepeatCol && nStartCol >= pRepeatCol->aStart.Col() )
        {
            for (SCCOL i=pRepeatCol->aStart.Col(); i<=pRepeatCol->aEnd.Col(); i++ )
                nBlkTwipsX += aDocument.GetColWidth( i, nTab );
            if ( nStartCol <= pRepeatCol->aEnd.Col() )
                nStartCol = pRepeatCol->aEnd.Col() + 1;
        }
        // legacy compilers' own scope for i
        {
            for ( SCCOL i=nStartCol; i<=nEndCol; i++ )
                nBlkTwipsX += aDocument.GetColWidth( i, nTab );
        }

        long nBlkTwipsY = 0;
        if (bHeaders)
            nBlkTwipsY += (long) PRINT_HEADER_HEIGHT;
        SCROW nStartRow = rRange.aStart.Row();
        SCROW nEndRow = rRange.aEnd.Row();
        if ( pRepeatRow && nStartRow >= pRepeatRow->aStart.Row() )
        {
            nBlkTwipsY += aDocument.GetRowHeight( pRepeatRow->aStart.Row(),
                    pRepeatRow->aEnd.Row(), nTab );
            if ( nStartRow <= pRepeatRow->aEnd.Row() )
                nStartRow = pRepeatRow->aEnd.Row() + 1;
        }
        nBlkTwipsY += aDocument.GetRowHeight( nStartRow, nEndRow, nTab );

        Size aPhysPage;
        long nHdr, nFtr;
        ScPrintFunc aOldPrFunc( this, GetPrinter(), nTab );
        aOldPrFunc.GetScaleData( aPhysPage, nHdr, nFtr );
        nBlkTwipsY += nHdr + nFtr;

        if ( nBlkTwipsX == 0 )      // hidden columns/rows may lead to 0
            nBlkTwipsX = 1;
        if ( nBlkTwipsY == 0 )
            nBlkTwipsY = 1;

        long nNeeded = std::min( aPhysPage.Width()  * 100 / nBlkTwipsX,
                            aPhysPage.Height() * 100 / nBlkTwipsY );
        if ( nNeeded < ZOOM_MIN )
            nNeeded = ZOOM_MIN;         // Begrenzung
        if ( nNeeded < (long) nNewScale )
            nNewScale = (sal_uInt16) nNeeded;

        bChange = ( nNewScale != nOldScale || nOldPages != 0 );
        if ( bChange )
            SetPrintZoom( nTab, nNewScale, 0 );
    }
    return bChange;
}

void ScDocShell::PageStyleModified( const OUString& rStyleName, bool bApi )
{
    ScDocShellModificator aModificator( *this );

    SCTAB nTabCount = aDocument.GetTableCount();
    SCTAB nUseTab = MAXTAB+1;
    for (SCTAB nTab=0; nTab<nTabCount && nUseTab>MAXTAB; nTab++)
        if ( aDocument.GetPageStyle(nTab) == rStyleName &&
                ( !bApi || aDocument.GetPageSize(nTab).Width() ) )
            nUseTab = nTab;
                                // bei bApi nur, wenn Umbrueche schon angezeigt

    if (ValidTab(nUseTab))      // nicht verwendet -> nichts zu tun
    {
        bool bWarn = false;

        ScPrintFunc aPrintFunc( this, GetPrinter(), nUseTab );  //! ohne CountPages auskommen
        if (!aPrintFunc.UpdatePages())                          //  setzt Umbrueche auf allen Tabs
            bWarn = true;

        if (bWarn && !bApi)
        {
            ScWaitCursorOff aWaitOff( GetActiveDialogParent() );
            ScopedVclPtrInstance<InfoBox> aInfoBox(GetActiveDialogParent(),
                                                   ScGlobal::GetRscString(STR_PRINT_INVALID_AREA));
            aInfoBox->Execute();
        }
    }

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_RESET_PRINTZOOM );
        pBindings->Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
        pBindings->Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    }
}

void ScDocShell::ExecutePageStyle( SfxViewShell& rCaller,
                                   SfxRequest&   rReq,
                                   SCTAB         nCurTab )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_STATUS_PAGESTYLE:  // Click auf StatusBar-Control
        case SID_FORMATPAGE:
            {
                if ( pReqArgs != nullptr )
                {
                }
                else if ( pReqArgs == nullptr )
                {
                    OUString aOldName = aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool* pStylePool = aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aOldName, SFX_STYLE_FAMILY_PAGE );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        ScStyleSaveData aOldData;
                        const bool bUndo(aDocument.IsUndoEnabled());
                        if (bUndo)
                            aOldData.InitFromStyle( pStyleSheet );

                        SfxItemSet&     rStyleSet = pStyleSheet->GetItemSet();

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateScStyleDlg( GetActiveDialogParent(), *pStyleSheet, RID_SCDLG_STYLES_PAGE, RID_SCDLG_STYLES_PAGE ));
                        OSL_ENSURE(pDlg, "Dialog create fail!");

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            WaitObject aWait( GetActiveDialogParent() );

                            OUString aNewName = pStyleSheet->GetName();
                            if ( aNewName != aOldName &&
                                aDocument.RenamePageStyleInUse( aOldName, aNewName ) )
                            {
                                SfxBindings* pBindings = GetViewBindings();
                                if (pBindings)
                                {
                                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                                }
                            }

                            if ( pOutSet )
                                aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            // merken fuer GetState():
                            GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, bHeaderOn, bFooterOn );
                            rCaller.GetViewFrame()->GetBindings().Invalidate( SID_HFEDIT );

                            ScStyleSaveData aNewData;
                            aNewData.InitFromStyle( pStyleSheet );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                        new ScUndoModifyStyle( this, SFX_STYLE_FAMILY_PAGE,
                                                    aOldData, aNewData ) );
                            }

                            PageStyleModified( aNewName, false );
                            rReq.Done();
                        }
                        pDlg.reset();

                        rStyleSet.ClearItem( ATTR_PAGE_PAPERTRAY );
                    }
                }
            }
            break;

        case SID_HFEDIT:
            {
                if ( pReqArgs != nullptr )
                {
                }
                else if ( pReqArgs == nullptr )
                {
                    OUString aStr( aDocument.GetPageStyle( nCurTab ) );

                    ScStyleSheetPool* pStylePool
                        = aDocument.GetStyleSheetPool();

                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aStr, SFX_STYLE_FAMILY_PAGE );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet&  rStyleSet = pStyleSheet->GetItemSet();

                        SvxPageUsage eUsage =
                            SvxPageUsage( static_cast<const SvxPageItem&>(
                                            rStyleSet.Get( ATTR_PAGE )).
                                                GetPageUsage() );
                        bool bShareHeader = IS_SHARE_HEADER(rStyleSet);
                        bool bShareFooter = IS_SHARE_FOOTER(rStyleSet);
                        sal_uInt16 nResId = 0;

                        switch ( eUsage )
                        {
                            case SVX_PAGE_LEFT:
                            case SVX_PAGE_RIGHT:
                            {
                                if ( bHeaderOn && bFooterOn )
                                    nResId = RID_SCDLG_HFEDIT;
                                else if ( SVX_PAGE_RIGHT == eUsage )
                                {
                                    if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                                else
                                {
                                    //  #69193a# respect "shared" setting
                                    if ( !bHeaderOn && bFooterOn )
                                        nResId = bShareFooter ?
                                                    RID_SCDLG_HFEDIT_RIGHTFOOTER :
                                                    RID_SCDLG_HFEDIT_LEFTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = bShareHeader ?
                                                    RID_SCDLG_HFEDIT_RIGHTHEADER :
                                                    RID_SCDLG_HFEDIT_LEFTHEADER;
                                }
                            }
                            break;

                            case SVX_PAGE_MIRROR:
                            case SVX_PAGE_ALL:
                            default:
                            {
                                if ( !bShareHeader && !bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_ALL;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if ( bShareHeader && bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT;
                                    else
                                    {
                                        if ( !bHeaderOn && bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                        else if ( bHeaderOn && !bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                    }
                                }
                                else if ( !bShareHeader &&  bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SFTR;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if (  bShareHeader && !bShareFooter )
                                {
                                    if ( bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SHDR;
                                    else if ( !bHeaderOn && bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( bHeaderOn && !bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                            }
                        }

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
                                                                                GetActiveDialogParent(),
                                                                                rStyleSet,
                                                                                aStr,
                                                                                nResId));
                        OSL_ENSURE(pDlg, "Dialog create fail!");
                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            if ( pOutSet )
                                aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            SetDocumentModified();
                            rReq.Done();
                        }
                    }
                }
            }
            break;

        default:
        break;
    }
}

void ScDocShell::GetStatePageStyle( SfxViewShell&   /* rCaller */,
                                    SfxItemSet&     rSet,
                                    SCTAB           nCurTab )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_STATUS_PAGESTYLE:
                rSet.Put( SfxStringItem( nWhich, aDocument.GetPageStyle( nCurTab ) ) );
                break;

            case SID_HFEDIT:
                {
                    OUString            aStr        = aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool*   pStylePool  = aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( aStr, SFX_STYLE_FAMILY_PAGE );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                        GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, bHeaderOn, bFooterOn );

                        if ( !bHeaderOn && !bFooterOn )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScDocShell::GetState( SfxItemSet &rSet )
{
    bool bTabView = GetBestViewShell() != nullptr;

    SfxWhichIter aIter(rSet);
    for (sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich())
    {
        if (!bTabView)
        {
            rSet.DisableItem(nWhich);
            continue;
        }

        switch (nWhich)
        {
            case FID_AUTO_CALC:
                if ( aDocument.GetHardRecalcState() != ScDocument::HARDRECALCSTATE_OFF )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, aDocument.GetAutoCalc() ) );
                break;

            case FID_CHG_RECORD:
                if ( IsDocShared() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich,
                        aDocument.GetChangeTrack() != nullptr ) );
                break;

            case SID_CHG_PROTECT:
                {
                    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
                    if ( pChangeTrack && !IsDocShared() )
                        rSet.Put( SfxBoolItem( nWhich,
                            pChangeTrack->IsProtected() ) );
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DOCUMENT_COMPARE:
                {
                    if ( IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            //  Wenn eine Formel editiert wird, muss FID_RECALC auf jeden Fall enabled sein.
            //  Recalc fuer das Doc war mal wegen eines Bugs disabled, wenn AutoCalc an war,
            //  ist jetzt wegen eines anderen Bugs aber auch immer enabled.

            case SID_TABLES_COUNT:
                rSet.Put( SfxInt16Item( nWhich, aDocument.GetTableCount() ) );
                break;

            case SID_ATTR_YEAR2000 :
                rSet.Put( SfxUInt16Item( nWhich,
                    aDocument.GetDocOptions().GetYear2000() ) );
            break;

            case SID_SHARE_DOC:
                {
                    if ( IsReadOnly() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            default:
                {
                }
                break;
        }
    }
}

void ScDocShell::GetSbxState( SfxItemSet &rSet )
{
    //  SID_SC_SELECTION (Selection),
    //  SID_SC_ACTIVECELL (ActiveCell),
    //  SID_SC_ACTIVETAB (ActiveTable),
    //  SID_TABLES_GET (Tables),
    //  SID_PIVOT_GET (DataPilotTables) - removed (old Basic)

        //  Wenn hier Slots von der View-Shell executed werden, muss auch der
        //  GetState weitergeleitet werden!

    ScTabViewShell* pVisibleSh = GetBestViewShell();        // sichtbare View
    if ( pVisibleSh )
        pVisibleSh->GetState( rSet );
}

void ScDocShell::Draw( OutputDevice* pDev, const JobSetup & /* rSetup */, sal_uInt16 nAspect )
{

    SCTAB nVisTab = aDocument.GetVisibleTab();
    if (!aDocument.HasTable(nVisTab))
        return;

    ComplexTextLayoutMode nOldLayoutMode = pDev->GetLayoutMode();
    pDev->SetLayoutMode( TEXT_LAYOUT_DEFAULT );     // even if it's the same, to get the metafile action

    if ( nAspect == ASPECT_THUMBNAIL )
    {
        Rectangle aBoundRect = GetVisArea( ASPECT_THUMBNAIL );
        ScViewData aTmpData( this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &aDocument, pDev, 1.0, aBoundRect, &aTmpData, true );
    }
    else
    {
        Rectangle aBoundRect = SfxObjectShell::GetVisArea();
        ScViewData aTmpData( this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &aDocument, pDev, 1.0, aBoundRect, &aTmpData, true );
    }

    pDev->SetLayoutMode( nOldLayoutMode );
}

Rectangle ScDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    SfxObjectCreateMode eShellMode = GetCreateMode();
    if ( eShellMode == SfxObjectCreateMode::ORGANIZER )
    {
        //  ohne Inhalte wissen wir auch nicht, wie gross die Inhalte sind
        //  leeres Rechteck zurueckgeben, das wird dann nach dem Laden berechnet
        return Rectangle();
    }

    if( nAspect == ASPECT_THUMBNAIL )
    {
        SCTAB nVisTab = aDocument.GetVisibleTab();
        if (!aDocument.HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->aDocument.SetVisibleTab(nVisTab);
        }
        Size aSize = aDocument.GetPageSize(nVisTab);
        const long SC_PREVIEW_SIZE_X = 10000;
        const long SC_PREVIEW_SIZE_Y = 12400;
        Rectangle aArea( 0,0, SC_PREVIEW_SIZE_X, SC_PREVIEW_SIZE_Y);
        if (aSize.Width() > aSize.Height())
        {
            aArea.Right() = SC_PREVIEW_SIZE_Y;
            aArea.Bottom() = SC_PREVIEW_SIZE_X;
        }

        bool bNegativePage = aDocument.IsNegativePage( aDocument.GetVisibleTab() );
        if ( bNegativePage )
            ScDrawLayer::MirrorRectRTL( aArea );
        SnapVisArea( aArea );
        return aArea;
    }
    else if( nAspect == ASPECT_CONTENT && eShellMode != SfxObjectCreateMode::EMBEDDED )
    {
        //  Visarea holen wie nach Load

        SCTAB nVisTab = aDocument.GetVisibleTab();
        if (!aDocument.HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->aDocument.SetVisibleTab(nVisTab);
        }
        SCCOL nStartCol;
        SCROW nStartRow;
        aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
        SCCOL nEndCol;
        SCROW nEndRow;
        aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
        if (nStartCol>nEndCol)
            nStartCol = nEndCol;
        if (nStartRow>nEndRow)
            nStartRow = nEndRow;
        Rectangle aNewArea = ((ScDocument&)aDocument)
                                .GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab );
        //TODO/LATER: different methods for setting VisArea?!
        const_cast<ScDocShell*>(this)->SfxObjectShell::SetVisArea( aNewArea );
        return aNewArea;
    }
    else
        return SfxObjectShell::GetVisArea( nAspect );
}

namespace {

void SnapHor( const ScDocument& rDoc, SCTAB nTab, long& rVal, SCCOL& rStartCol )
{
    SCCOL nCol = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;
    while ( nCol<MAXCOL )
    {
        long nAdd = rDoc.GetColWidth(nCol, nTab);
        if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
        {
            nSnap += nAdd;
            ++nCol;
        }
        else
            break;
    }
    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartCol = nCol;
}

void SnapVer( const ScDocument& rDoc, SCTAB nTab, long& rVal, SCROW& rStartRow )
{
    SCROW nRow = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;

    bool bFound = false;
    for (SCROW i = nRow; i <= MAXROW; ++i)
    {
        SCROW nLastRow;
        if (rDoc.RowHidden(i, nTab, nullptr, &nLastRow))
        {
            i = nLastRow;
            continue;
        }

        nRow = i;
        long nAdd = rDoc.GetRowHeight(i, nTab);
        if ( nSnap + nAdd/2 < nTwips || nRow < rStartRow )
        {
            nSnap += nAdd;
            ++nRow;
        }
        else
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)
        nRow = MAXROW;  // all hidden down to the bottom

    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartRow = nRow;
}

}

void ScDocShell::SnapVisArea( Rectangle& rRect ) const
{
    SCTAB nTab = aDocument.GetVisibleTab();
    bool bNegativePage = aDocument.IsNegativePage( nTab );
    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // calculate with positive (LTR) values

    SCCOL nCol = 0;
    SnapHor( aDocument, nTab, rRect.Left(), nCol );
    ++nCol;                                         // mindestens eine Spalte
    SnapHor( aDocument, nTab, rRect.Right(), nCol );

    SCROW nRow = 0;
    SnapVer( aDocument, nTab, rRect.Top(), nRow );
    ++nRow;                                         // mindestens eine Zeile
    SnapVer( aDocument, nTab, rRect.Bottom(), nRow );

    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // back to real rectangle
}

void ScDocShell::GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                            SCTAB             nCurTab,
                                            bool&             rbHeader,
                                            bool&             rbFooter )
{
    if ( !pStyleSet )
    {
        ScStyleSheetPool*  pStylePool  = aDocument.GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStylePool->
                                            Find( aDocument.GetPageStyle( nCurTab ),
                                                  SFX_STYLE_FAMILY_PAGE );

        OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

        if ( pStyleSheet )
            pStyleSet = &pStyleSheet->GetItemSet();
        else
            rbHeader = rbFooter = false;
    }

    OSL_ENSURE( pStyleSet, "PageStyle-Set not found! :-(" );
    if (!pStyleSet)
        return;

    const SvxSetItem*   pSetItem = nullptr;
    const SfxItemSet*   pSet     = nullptr;

    pSetItem = static_cast<const SvxSetItem*>( &pStyleSet->Get( ATTR_PAGE_HEADERSET ) );
    pSet     = &pSetItem->GetItemSet();
    rbHeader = static_cast<const SfxBoolItem&>(pSet->Get(ATTR_PAGE_ON)).GetValue();

    pSetItem = static_cast<const SvxSetItem*>( &pStyleSet->Get( ATTR_PAGE_FOOTERSET ) );
    pSet     = &pSetItem->GetItemSet();
    rbFooter = static_cast<const SfxBoolItem&>(pSet->Get(ATTR_PAGE_ON)).GetValue();
}

#if defined WNT
bool ScDocShell::DdeGetData( const OUString& rItem,
                             const OUString& rMimeType,
                             css::uno::Any & rValue )
{
    if( SotClipboardFormatId::STRING == SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
        if( rItem.equalsIgnoreAsciiCase( "Format" ) )
        {
            OString aFmtByte(OUStringToOString(aDdeTextFmt,
                osl_getThreadTextEncoding()));
            rValue <<= css::uno::Sequence< sal_Int8 >(
                                        reinterpret_cast<const sal_Int8*>(aFmtByte.getStr()),
                                        aFmtByte.getLength() + 1 );
            return true;
        }
        ScImportExport aObj( &aDocument, rItem );
        if ( !aObj.IsRef() )
            return false;                           // ungueltiger Bereich

        if( aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( aDdeTextFmt == "SYLK" ||
            aDdeTextFmt == "FSYLK" )
        {
            OString aData;
            if( aObj.ExportByteString( aData, osl_getThreadTextEncoding(),
                                        SotClipboardFormatId::SYLK ) )
            {
                rValue <<= css::uno::Sequence< sal_Int8 >(
                                            reinterpret_cast<const sal_Int8*>(aData.getStr()),
                                            aData.getLength() + 1 );
                return true;
            }
            else
                return false;
        }
        if( aDdeTextFmt == "CSV" ||
            aDdeTextFmt == "FCSV" )
            aObj.SetSeparator( ',' );
        aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, 0, false ) );
        return aObj.ExportData( rMimeType, rValue );
    }

    ScImportExport aObj( &aDocument, rItem );
    aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, 0, false ) );
    return aObj.IsRef() && aObj.ExportData( rMimeType, rValue );
}

bool ScDocShell::DdeSetData( const OUString& rItem,
                             const OUString& rMimeType,
                             const css::uno::Any & rValue )
{
    if( SotClipboardFormatId::STRING == SotExchange::GetFormatIdFromMimeType( rMimeType ))
    {
        if( rItem.equalsIgnoreAsciiCase( "Format" ) )
        {
            if ( ScByteSequenceToString::GetString( aDdeTextFmt, rValue, osl_getThreadTextEncoding() ) )
            {
                aDdeTextFmt = aDdeTextFmt.toAsciiUpperCase();
                return true;
            }
            return false;
        }
        ScImportExport aObj( &aDocument, rItem );
        if( aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( aDdeTextFmt == "SYLK" ||
            aDdeTextFmt == "FSYLK" )
        {
            OUString aData;
            if ( ScByteSequenceToString::GetString( aData, rValue, osl_getThreadTextEncoding() ) )
            {
                return aObj.ImportString( aData, SotClipboardFormatId::SYLK );
            }
            return false;
        }
        if( aDdeTextFmt == "CSV" ||
            aDdeTextFmt == "FCSV" )
            aObj.SetSeparator( ',' );
        return aObj.ImportData( rMimeType, rValue );
    }
    ScImportExport aObj( &aDocument, rItem );
    return aObj.IsRef() && aObj.ImportData( rMimeType, rValue );
}
#endif

::sfx2::SvLinkSource* ScDocShell::DdeCreateLinkSource( const OUString& rItem )
{
    //  only check for valid item string - range is parsed again in ScServerObject ctor

    //  named range?
    OUString aPos = rItem;
    ScRangeName* pRange = aDocument.GetRangeName();
    if( pRange )
    {
        const ScRangeData* pData = pRange->findByUpperName(ScGlobal::pCharClass->uppercase(aPos));
        if (pData)
        {
            if( pData->HasType( ScRangeData::Type::RefArea    )
                || pData->HasType( ScRangeData::Type::AbsArea )
                || pData->HasType( ScRangeData::Type::AbsPos  ) )
                pData->GetSymbol( aPos );           // continue with the name's contents
        }
    }

    // Address in DDE function must be always parsed as CONV_OOO so that it
    // would always work regardless of current address conversion.  We do this
    // because the address item in a DDE entry is *not* normalized when saved
    // into ODF.
    ScRange aRange;
    bool bValid = ( (aRange.Parse(aPos, &aDocument, formula::FormulaGrammar::CONV_OOO ) & SCA_VALID) ||
                    (aRange.aStart.Parse(aPos, &aDocument, formula::FormulaGrammar::CONV_OOO) & SCA_VALID) );

    ScServerObject* pObj = nullptr;            // NULL = error
    if ( bValid )
        pObj = new ScServerObject( this, rItem );

    //  GetLinkManager()->InsertServer() is in the ScServerObject ctor

    return pObj;
}

ScViewData* ScDocShell::GetViewData()
{
    SfxViewShell* pCur = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( pCur );
    return pViewSh ? &pViewSh->GetViewData() : nullptr;
}

SCTAB ScDocShell::GetCurTab()
{
    //! this must be made non-static and use a ViewShell from this document!

    ScViewData* pViewData = GetViewData();

    return pViewData ? pViewData->GetTabNo() : static_cast<SCTAB>(0);
}

ScTabViewShell* ScDocShell::GetBestViewShell( bool bOnlyVisible )
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    // falsches Doc?
    if( pViewSh && pViewSh->GetViewData().GetDocShell() != this )
        pViewSh = nullptr;
    if( !pViewSh )
    {
        // 1. ViewShell suchen
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this, bOnlyVisible );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            pViewSh = dynamic_cast< ScTabViewShell *>( p );
        }
    }
    return pViewSh;
}

SfxBindings* ScDocShell::GetViewBindings()
{
    //  used to invalidate slots after changes to this document

    SfxViewShell* pViewSh = GetBestViewShell();
    if (pViewSh)
        return &pViewSh->GetViewFrame()->GetBindings();
    else
        return nullptr;
}

ScDocShell* ScDocShell::GetShellByNum( sal_uInt16 nDocNo )      // static
{
    ScDocShell* pFound = nullptr;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    sal_uInt16 nShellCnt = 0;

    while ( pShell && !pFound )
    {
        if ( dynamic_cast<const ScDocShell*>(pShell) != nullptr )
        {
            if ( nShellCnt == nDocNo )
                pFound = static_cast<ScDocShell*>(pShell);
            else
                ++nShellCnt;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }

    return pFound;
}

IMPL_LINK_TYPED( ScDocShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    OSL_ENSURE( _pFileDlg, "ScDocShell::DialogClosedHdl(): no file dialog" );
    OSL_ENSURE( pImpl->pDocInserter, "ScDocShell::DialogClosedHdl(): no document inserter" );

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        sal_uInt16 nSlot = pImpl->pRequest->GetSlot();
        SfxMedium* pMed = pImpl->pDocInserter->CreateMedium();
        // #i87094# If a .odt was selected pMed is NULL.
        if (pMed)
        {
            pImpl->pRequest->AppendItem( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
            if ( SID_DOCUMENT_COMPARE == nSlot )
            {
                if ( pMed->GetFilter() )
                    pImpl->pRequest->AppendItem(
                            SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                OUString sOptions = ScDocumentLoader::GetOptions( *pMed );
                if ( !sOptions.isEmpty() )
                    pImpl->pRequest->AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, sOptions ) );
            }
            const SfxPoolItem* pItem = nullptr;
            const SfxInt16Item* pInt16Item(nullptr);
            SfxItemSet* pSet = pMed->GetItemSet();
            if (pSet && pSet->GetItemState(SID_VERSION, true, &pItem) == SfxItemState::SET)
            {
                pInt16Item = dynamic_cast<const SfxInt16Item*>(pItem);
            }
            if (pInt16Item)
            {
                pImpl->pRequest->AppendItem( *pItem );
            }

            Execute( *(pImpl->pRequest) );
        }
    }

    pImpl->bIgnoreLostRedliningWarning = false;
}

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT

void ScDocShell::EnableSharedSettings( bool bEnable )
{
    SetDocumentModified();

    if ( bEnable )
    {
        aDocument.EndChangeTracking();
        aDocument.StartChangeTracking();

        // hide accept or reject changes dialog
        sal_uInt16 nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        if ( pViewFrame && pViewFrame->HasChildWindow( nId ) )
        {
            pViewFrame->ToggleChildWindow( nId );
            SfxBindings* pBindings = GetViewBindings();
            if ( pBindings )
            {
                pBindings->Invalidate( FID_CHG_ACCEPT );
            }
        }
    }
    else
    {
        aDocument.EndChangeTracking();
    }

    ScChangeViewSettings aChangeViewSet;
    aChangeViewSet.SetShowChanges( false );
    aDocument.SetChangeViewSettings( aChangeViewSet );
}

uno::Reference< frame::XModel > ScDocShell::LoadSharedDocument()
{
    uno::Reference< frame::XModel > xModel;
    try
    {
        SC_MOD()->SetInSharedDocLoading( true );
        uno::Reference< frame::XDesktop2 > xLoader = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Sequence < beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name = "Hidden";
        aArgs[0].Value <<= sal_True;

        if ( GetMedium() )
        {
            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_PASSWORD, false);
            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
            {
                aArgs.realloc( 2 );
                aArgs[1].Name = "Password";
                aArgs[1].Value <<= pPasswordItem->GetValue();
            }
        }

        xModel.set(
            xLoader->loadComponentFromURL( GetSharedFileURL(), "_blank", 0, aArgs ),
            uno::UNO_QUERY_THROW );
        SC_MOD()->SetInSharedDocLoading( false );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "ScDocShell::LoadSharedDocument(): caught exception\n" );
        SC_MOD()->SetInSharedDocLoading( false );
        try
        {
            uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
            xClose->close( sal_True );
            return uno::Reference< frame::XModel >();
        }
        catch ( uno::Exception& )
        {
            return uno::Reference< frame::XModel >();
        }
    }
    return xModel;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
