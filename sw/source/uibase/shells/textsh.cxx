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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <svl/globalnameitem.hxx>
#include <svl/ownlist.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/lnkbase.hxx>

#include <tools/errinf.hxx>
#include <svx/svdview.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <basic/sbxvar.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/pfiledlg.hxx>
#include <svtools/htmlcfg.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <comphelper/classids.hxx>
#include <editeng/acorrcfg.hxx>
#include <wdocsh.hxx>
#include <fmtinfmt.hxx>
#include <fmtclds.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <frmmgr.hxx>
#include <textsh.hxx>
#include <frmfmt.hxx>
#include <tablemgr.hxx>
#include <swundo.hxx>
#include <shellio.hxx>
#include <frmdlg.hxx>
#include <usrpref.hxx>
#include <swtable.hxx>
#include <tblafmt.hxx>
#include <caption.hxx>
#include <idxmrk.hxx>
#include <poolfmt.hxx>
#include <breakit.hxx>
#include <crsskip.hxx>
#include <modcfg.hxx>
#include <column.hxx>
#include <edtwin.hxx>
#include <shells.hrc>
#include <popup.hrc>
#include <swerror.h>
#include <unochart.hxx>

#include <chartins.hxx>

#define SwTextShell
#include <sfx2/msg.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <swslots.hxx>
#include <SwRewriter.hxx>
#include <comcore.hrc>

using namespace ::com::sun::star;

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx"
#include <misc.hrc>
#include <table.hrc>
#include <frmui.hrc>
#include <unomid.h>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>

SFX_IMPL_INTERFACE(SwTextShell, SwBaseShell)

void SwTextShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(SW_RES(MN_TEXT_POPUPMENU));

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_TEXT_TOOLBOX);

    GetStaticInterface()->RegisterChildWindow(FN_EDIT_FORMULA);
    GetStaticInterface()->RegisterChildWindow(FN_INSERT_FIELD);
    GetStaticInterface()->RegisterChildWindow(FN_INSERT_IDX_ENTRY_DLG);
    GetStaticInterface()->RegisterChildWindow(FN_INSERT_AUTH_ENTRY_DLG);
    GetStaticInterface()->RegisterChildWindow(SID_RUBY_DIALOG);
    GetStaticInterface()->RegisterChildWindow(FN_WORDCOUNT_DIALOG);
}


void SwTextShell::ExecInsert(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    OSL_ENSURE( !rSh.IsObjSelected() && !rSh.IsFrameSelected(),
            "wrong shell on dispatcher" );

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    const sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(nSlot, false, &pItem );

    switch( nSlot )
    {
    case FN_INSERT_STRING:
        if( pItem )
            rSh.InsertByWord(static_cast<const SfxStringItem *>(pItem)->GetValue());
        break;

    case FN_INSERT_SOFT_HYPHEN:
        if( CHAR_SOFTHYPHEN != rSh.SwCursorShell::GetChar() &&
            CHAR_SOFTHYPHEN != rSh.SwCursorShell::GetChar( true, -1 ))
            rSh.Insert( OUString( CHAR_SOFTHYPHEN ) );
        break;

    case FN_INSERT_HARDHYPHEN:
    case FN_INSERT_HARD_SPACE:
        {
            const sal_Unicode cIns = FN_INSERT_HARD_SPACE == nSlot ? CHAR_HARDBLANK : CHAR_HARDHYPHEN;

            SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
            SvxAutoCorrect* pACorr = rACfg.GetAutoCorrect();
            if( pACorr && rACfg.IsAutoFormatByInput()
                && pACorr->IsAutoCorrFlag(
                    CapitalStartSentence | CapitalStartWord | AddNonBrkSpace | ChgOrdinalNumber | ChgToEnEmDash | SetINetAttr | Autocorrect ) )
            {
                rSh.AutoCorrect( *pACorr, cIns );
            }
            else
            {
                rSh.Insert( OUString( cIns ) );
            }
        }
        break;

    case SID_INSERT_RLM :
    case SID_INSERT_LRM :
    case SID_INSERT_ZWNBSP :
    case SID_INSERT_ZWSP:
    {
        sal_Unicode cIns = 0;
        switch(nSlot)
        {
            case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
            case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
            case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
            case SID_INSERT_ZWNBSP: cIns = CHAR_ZWNBSP; break;
        }
        rSh.Insert( OUString( cIns ) );
    }
    break;

    case FN_INSERT_BREAK:
        {
            if( !rSh.CursorInsideInputField() )
            {
                rSh.SplitNode();
            }
            else
            {
                rSh.InsertLineBreak();
            }
        }
        rReq.Done();
        break;

    case FN_INSERT_PAGEBREAK:
        rSh.InsertPageBreak();
        rReq.Done();
        break;

    case FN_INSERT_LINEBREAK:
        rSh.InsertLineBreak();
        rReq.Done();
        break;

    case FN_INSERT_COLUMN_BREAK:
        rSh.InsertColumnBreak();
        rReq.Done();
        break;

    case SID_HYPERLINK_SETLINK:
        if (pItem)
            InsertHyperlink(*static_cast<const SvxHyperlinkItem *>(pItem));
        rReq.Done();
        break;

#if HAVE_FEATURE_AVMEDIA
    case SID_INSERT_AVMEDIA:
        rReq.SetReturnValue(SfxBoolItem(nSlot, InsertMediaDlg( rReq )));
        break;

    case  SID_INSERT_SOUND:
    case  SID_INSERT_VIDEO:
    {
        SvxPluginFileDlg aDlg( &GetView().GetViewFrame()->GetWindow(), nSlot );
        aDlg.SetContext( nSlot == SID_INSERT_SOUND? sfx2::FileDialogHelper::SW_INSERT_SOUND : sfx2::FileDialogHelper::SW_INSERT_VIDEO );

        if ( ERRCODE_NONE == aDlg.Execute() )
        {
            // Determine URL
            OUString aStrURL( aDlg.GetPath() );
            aStrURL = URIHelper::SmartRel2Abs(
                INetURLObject(), aStrURL, URIHelper::GetMaybeFileHdl() );

            INetURLObject url;
            url.SetSmartProtocol( INetProtocol::File );

            if ( url.SetURL( aStrURL ) )
            {
                OUString aName;
                comphelper::EmbeddedObjectContainer aCnt;
                svt::EmbeddedObjectRef xObj( aCnt.CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), aName ), embed::Aspects::MSOLE_CONTENT );
                if ( xObj.is() )
                {
                    svt::EmbeddedObjectRef::TryRunningState( xObj.GetObject() );

                    // set properties from dialog
                    uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                    if ( xSet.is() )
                    {
                        xSet->setPropertyValue("PluginURL",
                                uno::makeAny( OUString( url.GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                    }
                }

                rSh.InsertObject( xObj, nullptr, true, nSlot);
            }
        }
    }
    break;
#endif

    case SID_INSERT_OBJECT:
    case SID_INSERT_PLUGIN:
    {
        const SfxGlobalNameItem* pNameItem = rReq.GetArg<SfxGlobalNameItem>(SID_INSERT_OBJECT);
        SvGlobalName *pName = nullptr;
        SvGlobalName aName;
        if ( pNameItem )
        {
            aName = pNameItem->GetValue();
            pName = &aName;
        }

        const SfxStringItem* pClassLocationItem = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        const SfxStringItem* pCommandsItem = rReq.GetArg<SfxStringItem>(FN_PARAM_3);
        //TODO/LATER: recording currently not working, need code for Commandlist
        svt::EmbeddedObjectRef xObj;
        if( nSlot == SID_INSERT_PLUGIN && ( pClassLocationItem || pCommandsItem ) )
        {
            OUString sClassLocation;
            if(pClassLocationItem)
                sClassLocation = pClassLocationItem->GetValue();

            SvCommandList aCommandList;
            if(pCommandsItem)
            {
                sal_Int32 nTemp;
                aCommandList.AppendCommands( pCommandsItem->GetValue(), &nTemp );
            }

            {
                comphelper::EmbeddedObjectContainer aCnt;
                OUString sName;
                xObj.Assign( aCnt.CreateEmbeddedObject( SvGlobalName( SO3_PLUGIN_CLASSID ).GetByteSequence(), sName ),
                            embed::Aspects::MSOLE_CONTENT );
                svt::EmbeddedObjectRef::TryRunningState( xObj.GetObject() );
                uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
                if ( xSet.is() )
                {
                    try
                    {
                        if ( !sClassLocation.isEmpty() )
                            xSet->setPropertyValue("PluginURL",
                                uno::makeAny(
                                    OUString(
                                        URIHelper::SmartRel2Abs(
                                            INetURLObject(), sClassLocation,
                                            URIHelper::GetMaybeFileHdl()) ) ) );
                        uno::Sequence< beans::PropertyValue > aSeq;
                        if ( aCommandList.size() )
                        {
                            aCommandList.FillSequence( aSeq );
                            xSet->setPropertyValue("PluginCommands", uno::makeAny( aSeq ) );
                        }
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }

            if(xObj.is())
                rSh.InsertOleObject( xObj );
        }
        else
        {
            OSL_ENSURE( !pNameItem || nSlot == SID_INSERT_OBJECT, "Superfluous argument!" );
            rSh.InsertObject( xObj, pName, true, nSlot);
            rReq.Done();
        }
        break;
    }
    case SID_INSERT_FLOATINGFRAME:
    {
        svt::EmbeddedObjectRef xObj;
        const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
        const SfxStringItem* pURLItem = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
        const SvxSizeItem* pMarginItem = rReq.GetArg<SvxSizeItem>(FN_PARAM_3);
        const SfxByteItem* pScrollingItem = rReq.GetArg<SfxByteItem>(FN_PARAM_4);
        const SfxBoolItem* pBorderItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_5);

        if(pURLItem) // URL is a _must_
        {
            comphelper::EmbeddedObjectContainer aCnt;
            OUString aName;
            xObj.Assign( aCnt.CreateEmbeddedObject( SvGlobalName( SO3_IFRAME_CLASSID ).GetByteSequence(), aName ),
                        embed::Aspects::MSOLE_CONTENT );
            svt::EmbeddedObjectRef::TryRunningState( xObj.GetObject() );
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                try
                {
                    ScrollingMode eScroll = ScrollingAuto;
                    if( pScrollingItem && pScrollingItem->GetValue() <= ScrollingAuto )
                        eScroll = (ScrollingMode) pScrollingItem->GetValue();

                    Size aMargin;
                    if ( pMarginItem )
                        aMargin = pMarginItem->GetSize();

                    if ( pURLItem )
                        xSet->setPropertyValue("FrameURL", uno::makeAny( OUString( pURLItem->GetValue() ) ) );
                    if ( pNameItem )
                        xSet->setPropertyValue("FrameName", uno::makeAny( OUString( pNameItem->GetValue() ) ) );

                    if ( eScroll == ScrollingAuto )
                        xSet->setPropertyValue("FrameIsAutoScroll",
                            uno::makeAny( true ) );
                    else
                        xSet->setPropertyValue("FrameIsScrollingMode",
                            uno::makeAny( eScroll == ScrollingYes ) );

                    if ( pBorderItem )
                        xSet->setPropertyValue("FrameIsBorder",
                            uno::makeAny( pBorderItem->GetValue() ) );

                    if ( pMarginItem )
                    {
                        xSet->setPropertyValue("FrameMarginWidth",
                            uno::makeAny( sal_Int32( aMargin.Width() ) ) );

                        xSet->setPropertyValue("FrameMarginHeight",
                            uno::makeAny( sal_Int32( aMargin.Height() ) ) );
                    }
                }
                catch (const uno::Exception&)
                {
                }
            }

            rSh.InsertOleObject( xObj );
        }
        else
        {
            rSh.InsertObject( xObj, nullptr, true, nSlot);
            rReq.Done();
        }
    }
    break;
    case SID_INSERT_DIAGRAM:
        {
            SvtModuleOptions aMOpt;
            if ( !aMOpt.IsChart() )
                break;
            if(!rReq.IsAPI())
            {
                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                SwInsertChart( &GetView().GetEditWin(), &pVFrame->GetBindings() );
            }
            else
            {
                uno::Reference< chart2::data::XDataProvider > xDataProvider;
                bool bFillWithData = true;
                OUString aRangeString;
                if (!GetShell().IsTableComplexForChart())
                {
                    SwFrameFormat* pTableFormat = GetShell().GetTableFormat();
                    aRangeString = pTableFormat->GetName() + "."
                        + GetShell().GetBoxNms();

                    // get table data provider
                    xDataProvider.set( GetView().GetDocShell()->getIDocumentChartDataProviderAccess().GetChartDataProvider() );
                }
                else
                    bFillWithData = false;  // will create chart with only it's default image

                SwTableFUNC( &rSh, false ).InsertChart( xDataProvider, bFillWithData, aRangeString );
                rSh.LaunchOLEObj();

                svt::EmbeddedObjectRef& xObj = rSh.GetOLEObject();
                if(pItem && xObj.is())
                {
                    Size aSize(static_cast<const SvxSizeItem*>(pItem)->GetSize());
                    aSize = OutputDevice::LogicToLogic
                                    ( aSize, MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ) );

                    if(aSize.Width() > MINLAY&& aSize.Height()> MINLAY)
                    {
                        awt::Size aSz;
                        aSz.Width = aSize.Width();
                        aSz.Height = aSize.Height();
                        xObj->setVisualAreaSize( xObj.GetViewAspect(), aSz );
                    }
                }
            }
        }
        break;

    case FN_INSERT_SMA:
        {
            // #i34343# Inserting a math object into an autocompletion crashes
            // the suggestion has to be removed before
            GetView().GetEditWin().StopQuickHelp();
            SvGlobalName aGlobalName( SO3_SM_CLASSID );
            rSh.InsertObject( svt::EmbeddedObjectRef(), &aGlobalName );
        }
        break;

    case FN_INSERT_TABLE:
        InsertTable( rReq );
        break;

    case FN_INSERT_FRAME_INTERACT_NOCOL:
    case FN_INSERT_FRAME_INTERACT:
    {
        sal_uInt16 nCols = 1;
        bool bModifier1 = rReq.GetModifier() == KEY_MOD1;
        if(pArgs)
        {
            if(FN_INSERT_FRAME_INTERACT_NOCOL != nSlot &&
                pArgs->GetItemState(SID_ATTR_COLUMNS, false, &pItem) == SfxItemState::SET)
                nCols = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
            if(pArgs->GetItemState(SID_MODIFIER, false, &pItem) == SfxItemState::SET)
                bModifier1 |= KEY_MOD1 == static_cast<const SfxUInt16Item *>(pItem)->GetValue();
        }
        if(bModifier1 )
        {
            SwEditWin& rEdtWin = GetView().GetEditWin();
            Size aWinSize = rEdtWin.GetSizePixel();
            Point aStartPos(aWinSize.Width()/2, aWinSize.Height() / 2);
            aStartPos = rEdtWin.PixelToLogic(aStartPos);
            aStartPos.X() -= 8 * MM50;
            aStartPos.Y() -= 4 * MM50;
            Size aSize(16 * MM50, 8 * MM50);
            GetShell().LockPaint();
            GetShell().StartAllAction();
            SwFlyFrameAttrMgr aMgr( true, GetShellPtr(), FRMMGR_TYPE_TEXT );
            if(nCols > 1)
            {
                SwFormatCol aCol;
                aCol.Init( nCols, aCol.GetGutterWidth(), aCol.GetWishWidth() );
                aMgr.SetCol( aCol );
            }
            aMgr.InsertFlyFrame(FLY_AT_PARA, aStartPos, aSize);
            GetShell().EndAllAction();
            GetShell().UnlockPaint();
        }
        else
        {
            GetView().InsFrameMode(nCols);
        }
        rReq.Ignore();
    }
    break;
    case FN_INSERT_FRAME:
    {
        bool bSingleCol = false;
        if( nullptr!= dynamic_cast< SwWebDocShell*>( GetView().GetDocShell()) )
        {
            SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
            if( HTML_CFG_MSIE == rHtmlOpt.GetExportMode() )
            {
                bSingleCol = true;
            }

        }
        // Create new border
        SwFlyFrameAttrMgr aMgr( true, GetShellPtr(), FRMMGR_TYPE_TEXT );
        if(pArgs)
        {
            Size aSize(aMgr.GetSize());
            aSize.Width() = GetShell().GetAnyCurRect(RECT_PAGE_PRT).Width();
            Point aPos = aMgr.GetPos();
            RndStdIds eAnchor = FLY_AT_PARA;
            if(pArgs->GetItemState(nSlot, false, &pItem) == SfxItemState::SET)
                eAnchor = (RndStdIds)static_cast<const SfxUInt16Item *>(pItem)->GetValue();
            if(pArgs->GetItemState(FN_PARAM_1, false, &pItem)  == SfxItemState::SET)
                aPos = static_cast<const SfxPointItem *>(pItem)->GetValue();
            if(pArgs->GetItemState(FN_PARAM_2, false, &pItem)  == SfxItemState::SET)
                aSize = static_cast<const SvxSizeItem *>(pItem)->GetSize();
            if(pArgs->GetItemState(SID_ATTR_COLUMNS, false, &pItem)  == SfxItemState::SET)
            {
                const sal_uInt16 nCols = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
                if( !bSingleCol && 1 < nCols )
                {
                    SwFormatCol aFormatCol;
                    aFormatCol.Init( nCols , (rReq.IsAPI() ? 0
                                        : DEF_GUTTER_WIDTH), USHRT_MAX );
                    aMgr.SetCol(aFormatCol);
                }
            }

            GetShell().LockPaint();
            GetShell().StartAllAction();

            aMgr.InsertFlyFrame(eAnchor, aPos, aSize);

            GetShell().EndAllAction();
            GetShell().UnlockPaint();
        }
        else
        {
            SfxItemSet aSet = CreateInsertFrameItemSet(aMgr);

            FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebDocShell*>( GetView().GetDocShell()) != nullptr );
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialog creation failed!");
            std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateFrameTabDialog("FrameDialog",
                                                  GetView().GetViewFrame(),
                                                  &GetView().GetViewFrame()->GetWindow(),
                                                  aSet));
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            if(pDlg->Execute() == RET_OK && pDlg->GetOutputItemSet())
            {
                //local variable necessary at least after call of .AutoCaption() because this could be deleted at this point
                SwWrtShell& rShell = GetShell();
                rShell.LockPaint();
                rShell.StartAllAction();
                rShell.StartUndo(UNDO_INSERT);

                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                aMgr.SetAttrSet(*pOutSet);

                // At first delete the selection at the ClickToEditField.
                if( rShell.IsInClickToEdit() )
                    rShell.DelRight();

                aMgr.InsertFlyFrame();

                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        GetView().GetViewFrame()->GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    //FN_INSERT_FRAME
                    sal_uInt16 nAnchor = (sal_uInt16)aMgr.GetAnchor();
                        rReq.AppendItem(SfxUInt16Item(nSlot, nAnchor));
                        rReq.AppendItem(SfxPointItem(FN_PARAM_1, rShell.GetObjAbsPos()));
                        rReq.AppendItem(SvxSizeItem(FN_PARAM_2, rShell.GetObjSize()));
                    rReq.Done();
                }

                GetView().AutoCaption(FRAME_CAP);

                {
                    SwRewriter aRewriter;

                    aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_FRAME));

                    rShell.EndUndo(UNDO_INSERT, &aRewriter);
                }
                rShell.EndAllAction();
                rShell.UnlockPaint();
            }
        }
        break;
    }
    case FN_FORMAT_COLUMN :
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialog creation failed!");
        std::unique_ptr<VclAbstractDialog> pColDlg(pFact->CreateVclAbstractDialog( GetView().GetWindow(), rSh, DLG_COLUMN));
        OSL_ENSURE(pColDlg, "Dialog creation failed!");
        pColDlg->Execute();
    }
    break;

    default:
        OSL_ENSURE(false, "wrong  dispatcher");
        return;
    }
}

static bool lcl_IsMarkInSameSection( SwWrtShell& rWrtSh, const SwSection* pSect )
{
    rWrtSh.SwapPam();
    bool bRet = pSect == rWrtSh.GetCurrSection();
    rWrtSh.SwapPam();
    return bRet;
}

void SwTextShell::StateInsert( SfxItemSet &rSet )
{
    const bool bHtmlModeOn = ::GetHtmlMode(GetView().GetDocShell()) & HTMLMODE_ON;
    SfxWhichIter aIter( rSet );
    SwWrtShell &rSh = GetShell();
    sal_uInt16 nWhich = aIter.FirstWhich();
    SvtModuleOptions aMOpt;
    SfxObjectCreateMode eCreateMode =
                        GetView().GetDocShell()->GetCreateMode();

    bool bCursorInHidden = false;
    if( !rSh.HasMark() )
    {
        rSh.Push();
        bCursorInHidden = rSh.SelectHiddenRange();
        rSh.Pop();
    }

    while ( nWhich )
    {
        switch ( nWhich )
        {
        case SID_INSERT_AVMEDIA:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
            if ( GetShell().IsSelFrameMode()
                 || GetShell().CursorInsideInputField()
                 || SfxObjectCreateMode::EMBEDDED == eCreateMode
                 || bCursorInHidden )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case SID_INSERT_DIAGRAM:
            if( !aMOpt.IsChart()
                || GetShell().CursorInsideInputField()
                || eCreateMode == SfxObjectCreateMode::EMBEDDED
                || bCursorInHidden )
            {
                rSet.DisableItem( nWhich );
            }
            break;

            case FN_INSERT_SMA:
                if( !aMOpt.IsMath()
                    || eCreateMode == SfxObjectCreateMode::EMBEDDED
                    || bCursorInHidden
                    || rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_INSERT_FLOATINGFRAME:
            case SID_INSERT_OBJECT:
            case SID_INSERT_PLUGIN:
                {
                    if( eCreateMode == SfxObjectCreateMode::EMBEDDED || bCursorInHidden )
                    {
                        rSet.DisableItem( nWhich );
                    }
                    else if( GetShell().IsSelFrameMode()
                             || GetShell().CursorInsideInputField() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                    else if(SID_INSERT_FLOATINGFRAME == nWhich && bHtmlModeOn)
                    {
                        SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
                        const sal_uInt16 nExport = rHtmlOpt.GetExportMode();
                        if(HTML_CFG_MSIE != nExport && HTML_CFG_WRITER != nExport )
                            rSet.DisableItem(nWhich);
                    }
                }
                break;

            case FN_INSERT_FRAME_INTERACT_NOCOL :
            case FN_INSERT_FRAME_INTERACT:
                {
                    if( GetShell().IsSelFrameMode()
                        || rSh.IsTableMode()
                        || GetShell().CursorInsideInputField()
                        || bCursorInHidden )
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_HYPERLINK_GETLINK:
                {
                    SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                    rSh.GetCurAttr( aSet );

                    SvxHyperlinkItem aHLinkItem;
                    const SfxPoolItem* pItem;
                    if(SfxItemState::SET == aSet.GetItemState(RES_TXTATR_INETFMT, false, &pItem))
                    {
                        const SwFormatINetFormat* pINetFormat = static_cast<const SwFormatINetFormat*>(pItem);
                        aHLinkItem.SetURL(pINetFormat->GetValue());
                        aHLinkItem.SetTargetFrame(pINetFormat->GetTargetFrame());
                        aHLinkItem.SetIntName(pINetFormat->GetName());
                        const SvxMacro *pMacro = pINetFormat->GetMacro( SFX_EVENT_MOUSEOVER_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSEOVER_OBJECT, *pMacro);

                        pMacro = pINetFormat->GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSECLICK_OBJECT, *pMacro);

                        pMacro = pINetFormat->GetMacro( SFX_EVENT_MOUSEOUT_OBJECT );
                        if( pMacro )
                            aHLinkItem.SetMacro(HYPERDLG_EVENT_MOUSEOUT_OBJECT, *pMacro);

                        // Get the text of the Link.
                        rSh.StartAction();
                        rSh.CreateCursor();
                        rSh.SwCursorShell::SelectTextAttr(RES_TXTATR_INETFMT,true);
                        OUString sLinkName = rSh.GetSelText();
                        aHLinkItem.SetName(sLinkName);
                        aHLinkItem.SetInsertMode(HLINK_FIELD);
                        rSh.DestroyCursor();
                        rSh.EndAction();
                    }
                    else
                    {
                        OUString sReturn = rSh.GetSelText();
                        sReturn = sReturn.copy(0, std::min<sal_Int32>(255, sReturn.getLength()));
                        aHLinkItem.SetName(comphelper::string::stripEnd(sReturn, ' '));
                    }

                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        (bHtmlModeOn ? HLINK_HTMLMODE : 0)));
                    aHLinkItem.SetMacroEvents ( HYPERDLG_EVENT_MOUSEOVER_OBJECT|
                        HYPERDLG_EVENT_MOUSECLICK_OBJECT | HYPERDLG_EVENT_MOUSEOUT_OBJECT );

                    rSet.Put(aHLinkItem);
                }
                break;

            case FN_INSERT_FRAME:
                if (rSh.IsSelFrameMode() )
                {
                    const int nSel = rSh.GetSelectionType();
                    if( ((nsSelectionType::SEL_GRF | nsSelectionType::SEL_OLE ) & nSel ) || bCursorInHidden )
                        rSet.DisableItem(nWhich);
                }
                else if ( rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;

            case FN_FORMAT_COLUMN :
            {
                //#i80458# column dialog cannot work if the selection contains different page styles and different sections
                bool bDisable = true;
                if( rSh.GetFlyFrameFormat() || rSh.GetSelectedPageDescs() )
                    bDisable = false;
                if( bDisable )
                {
                    const SwSection* pCurrSection = rSh.GetCurrSection();
                    const sal_uInt16 nFullSectCnt = rSh.GetFullSelectedSectionCount();
                    if( pCurrSection && ( !rSh.HasSelection() || 0 != nFullSectCnt ))
                        bDisable = false;
                    else if(
                        rSh.HasSelection() && rSh.IsInsRegionAvailable() &&
                            ( !pCurrSection || ( 1 != nFullSectCnt &&
                                lcl_IsMarkInSameSection( rSh, pCurrSection ) )))
                        bDisable = false;
                }
                if(bDisable)
                    rSet.DisableItem(nWhich);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void  SwTextShell::ExecDelete(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    switch( rReq.GetSlot() )
    {
        case FN_DELETE_SENT:
            if( rSh.IsTableMode() )
            {
                rSh.DeleteRow();
                rSh.EnterStdMode();
            }
            else
                rSh.DelToEndOfSentence();
            break;
        case FN_DELETE_BACK_SENT:
            rSh.DelToStartOfSentence();
            break;
        case FN_DELETE_WORD:
            rSh.DelNxtWord();
            break;
        case FN_DELETE_BACK_WORD:
            rSh.DelPrvWord();
            break;
        case FN_DELETE_LINE:
            rSh.DelToEndOfLine();
            break;
        case FN_DELETE_BACK_LINE:
            rSh.DelToStartOfLine();
            break;
        case FN_DELETE_PARA:
            rSh.DelToEndOfPara();
            break;
        case FN_DELETE_BACK_PARA:
            rSh.DelToStartOfPara();
            break;
        case FN_DELETE_WHOLE_LINE:
            rSh.DelLine();
            break;
        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
    rReq.Done();
}

void SwTextShell::ExecTransliteration( SfxRequest & rReq )
{
    using namespace ::com::sun::star::i18n;
    sal_uInt32 nMode = 0;

    switch( rReq.GetSlot() )
    {
    case SID_TRANSLITERATE_SENTENCE_CASE:
        nMode = TransliterationModulesExtra::SENTENCE_CASE;
        break;
    case SID_TRANSLITERATE_TITLE_CASE:
        nMode = TransliterationModulesExtra::TITLE_CASE;
        break;
    case SID_TRANSLITERATE_TOGGLE_CASE:
        nMode = TransliterationModulesExtra::TOGGLE_CASE;
        break;
    case SID_TRANSLITERATE_UPPER:
        nMode = TransliterationModules_LOWERCASE_UPPERCASE;
        break;
    case SID_TRANSLITERATE_LOWER:
        nMode = TransliterationModules_UPPERCASE_LOWERCASE;
        break;

    case SID_TRANSLITERATE_HALFWIDTH:
        nMode = TransliterationModules_FULLWIDTH_HALFWIDTH;
        break;
    case SID_TRANSLITERATE_FULLWIDTH:
        nMode = TransliterationModules_HALFWIDTH_FULLWIDTH;
        break;

    case SID_TRANSLITERATE_HIRAGANA:
        nMode = TransliterationModules_KATAKANA_HIRAGANA;
        break;
    case SID_TRANSLITERATE_KATAGANA:
        nMode = TransliterationModules_HIRAGANA_KATAKANA;
        break;

    default:
        OSL_ENSURE(false, "wrong dispatcher");
    }

    if( nMode )
        GetShell().TransliterateText( nMode );
}

void SwTextShell::ExecRotateTransliteration( SfxRequest & rReq )
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
        GetShell().TransliterateText( m_aRotateCase.getNextMode() );
}

SwTextShell::SwTextShell(SwView &_rView) :
    SwBaseShell(_rView)
{
    SetName("Text");
    SetHelpId(SW_TEXTSHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Text));
}

SwTextShell::~SwTextShell()
{
}

SfxItemSet SwTextShell::CreateInsertFrameItemSet(SwFlyFrameAttrMgr& rMgr)
{
    static const sal_uInt16 aFrameAttrRange[] =
    {
        RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
        SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
        FN_GET_PRINT_AREA,      FN_GET_PRINT_AREA,
        SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
        FN_SET_FRM_NAME,        FN_SET_FRM_NAME,
        SID_HTML_MODE,          SID_HTML_MODE,
        SID_COLOR_TABLE,        SID_BITMAP_LIST,
        0
    };

    SfxItemSet aSet(GetPool(), aFrameAttrRange );
    aSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));

    // For the Area tab page.
    GetShell().GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->PutAreaListItems(aSet);

    const SwRect &rPg = GetShell().GetAnyCurRect(RECT_PAGE);
    SwFormatFrameSize aFrameSize(ATT_VAR_SIZE, rPg.Width(), rPg.Height());
    aFrameSize.SetWhich(GetPool().GetWhich(SID_ATTR_PAGE_SIZE));
    aSet.Put(aFrameSize);

    const SwRect &rPr = GetShell().GetAnyCurRect(RECT_PAGE_PRT);
    SwFormatFrameSize aPrtSize(ATT_VAR_SIZE, rPr.Width(), rPr.Height());
    aPrtSize.SetWhich(GetPool().GetWhich(FN_GET_PRINT_AREA));
    aSet.Put(aPrtSize);

    aSet.Put(rMgr.GetAttrSet());
    aSet.SetParent( rMgr.GetAttrSet().GetParent() );

    // Delete minimum size in columns.
    SvxBoxInfoItem aBoxInfo(static_cast<const SvxBoxInfoItem &>(aSet.Get(SID_ATTR_BORDER_INNER)));
    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX));
    aBoxInfo.SetMinDist(false);
    aBoxInfo.SetDefDist(rBox.GetDistance(SvxBoxItemLine::LEFT));
    aSet.Put(aBoxInfo);

    return aSet;
}

void SwTextShell::InsertSymbol( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    if( pArgs )
        pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), false, &pItem);

    OUString aChars, aFontName;
    if ( pItem )
    {
        aChars = static_cast<const SfxStringItem*>(pItem)->GetValue();
        const SfxPoolItem* pFtItem = nullptr;
        pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
        const SfxStringItem* pFontItem = dynamic_cast<const SfxStringItem*>( pFtItem  );
        if ( pFontItem )
            aFontName = pFontItem->GetValue();
    }

    SwWrtShell &rSh = GetShell();
    SfxItemSet aSet( GetPool(), RES_CHRATR_FONT, RES_CHRATR_FONT,
                                RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
                                RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT,
                                0 );
    rSh.GetCurAttr( aSet );
    SvtScriptType nScript = rSh.GetScriptType();

    SvxFontItem aFont( RES_CHRATR_FONT );
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, false );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aFont = *static_cast<const SvxFontItem*>(pI);
        else
            aFont = static_cast<const SvxFontItem&>(
                        aSet.Get(
                            GetWhichOfScript(
                                RES_CHRATR_FONT,
                                SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) )));
        if (aFontName.isEmpty())
            aFontName = aFont.GetFamilyName();
    }

    vcl::Font aNewFont(aFontName, Size(1,1)); // Size only because CTOR.
    if( aChars.isEmpty() )
    {
        // Set selected font as default.
        SfxAllItemSet aAllSet( rSh.GetAttrPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SwViewOption aOpt(*GetShell().GetViewOptions());
        OUString sSymbolFont = aOpt.GetSymbolFont();
        if( aFontName.isEmpty() && !sSymbolFont.isEmpty() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aFont.GetFamilyName() ) );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( GetView().GetWindow(), aAllSet,
            GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));
        if( RET_OK == pDlg->Execute() )
        {
            const SfxStringItem* pCItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
            const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);
            if ( pFontItem )
            {
                aNewFont.SetName( pFontItem->GetFamilyName() );
                aNewFont.SetStyleName( pFontItem->GetStyleName() );
                aNewFont.SetCharSet( pFontItem->GetCharSet() );
                aNewFont.SetPitch( pFontItem->GetPitch() );
            }

            if ( pCItem )
            {
                aChars  = pCItem->GetValue();
                aOpt.SetSymbolFont(aNewFont.GetFamilyName());
                SW_MOD()->ApplyUsrPref(aOpt, &GetView());
            }
        }
    }

    if( !aChars.isEmpty() )
    {
        rSh.StartAllAction();

        // Delete selected content.
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_SPECIALCHAR));

        rSh.StartUndo( UNDO_INSERT, &aRewriter );
        if ( rSh.HasSelection() )
        {
            rSh.DelRight();
            aSet.ClearItem();
            rSh.GetCurAttr( aSet );

            SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
            aSetItem.GetItemSet().Put( aSet, false );
            const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
            if( pI )
                aFont = *static_cast<const SvxFontItem*>(pI);
            else
                aFont = static_cast<const SvxFontItem&>(aSet.Get( GetWhichOfScript(
                            RES_CHRATR_FONT,
                            SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) )));
        }

        // Insert character.
        rSh.Insert( aChars );

        // #108876# a font attribute has to be set always due to a guessed script type
        if( !aNewFont.GetFamilyName().isEmpty() )
        {
            SvxFontItem aNewFontItem( aFont );
            aNewFontItem.SetFamilyName( aNewFont.GetFamilyName() );
            aNewFontItem.SetFamily(  aNewFont.GetFamily());
            aNewFontItem.SetPitch(   aNewFont.GetPitch());
            aNewFontItem.SetCharSet( aNewFont.GetCharSet() );

            SfxItemSet aRestoreSet( GetPool(), RES_CHRATR_FONT, RES_CHRATR_FONT,
                                               RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
                                               RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT, 0 );

            nScript = g_pBreakIt->GetAllScriptsOfText( aChars );
            if( SvtScriptType::LATIN & nScript )
            {
                aRestoreSet.Put( aSet.Get( RES_CHRATR_FONT ) );
                aSet.Put( aNewFontItem, RES_CHRATR_FONT);
            }
            if( SvtScriptType::ASIAN & nScript )
            {
                aRestoreSet.Put( aSet.Get( RES_CHRATR_CJK_FONT ) );
                aSet.Put( aNewFontItem, RES_CHRATR_CJK_FONT );
            }
            if( SvtScriptType::COMPLEX & nScript )
            {
                aRestoreSet.Put( aSet.Get( RES_CHRATR_CTL_FONT ) );
                aSet.Put( aNewFontItem, RES_CHRATR_CTL_FONT );
            }

            rSh.SetMark();
            rSh.ExtendSelection( false, aChars.getLength() );
            rSh.SetAttrSet( aSet, SetAttrMode::DONTEXPAND | SetAttrMode::NOFORMATATTR );
            if( !rSh.IsCursorPtAtEnd() )
                rSh.SwapPam();

            rSh.ClearMark();

            // #i75891#
            // SETATTR_DONTEXPAND does not work if there are already hard attributes.
            // Therefore we have to restore the font attributes.
            rSh.SetMark();
            rSh.SetAttrSet( aRestoreSet );
            rSh.ClearMark();

            rSh.UpdateAttr();
            aFont = aNewFontItem;
        }

        rSh.EndAllAction();
        rSh.EndUndo();

        if ( !aChars.isEmpty() )
        {
            rReq.AppendItem( SfxStringItem( GetPool().GetWhich(SID_CHARMAP), aChars ) );
            rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aNewFont.GetFamilyName() ) );
            rReq.Done();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
