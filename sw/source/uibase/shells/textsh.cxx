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
#include <sfx2/bindings.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>

#include <i18nutil/transliteration.hxx>
#include <svl/eitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svtools/htmlcfg.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <osl/diagnose.h>

#include <comphelper/classids.hxx>
#include <editeng/acorrcfg.hxx>
#include <wdocsh.hxx>
#include <fmtinfmt.hxx>
#include <fmtclds.hxx>
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
#include <breakit.hxx>
#include <edtwin.hxx>
#include <strings.hrc>
#include <unochart.hxx>
#include <chartins.hxx>
#include <viewopt.hxx>

#define ShellClass_SwTextShell
#include <sfx2/msg.hxx>
#include <vcl/EnumContext.hxx>
#include <swslots.hxx>
#include <SwRewriter.hxx>
#include <SwCapObjType.hxx>

using namespace ::com::sun::star;

#include <svx/svxdlg.hxx>
#include <swabstdlg.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svtools/embedhlp.hxx>
#include <sfx2/event.hxx>
#include <com/sun/star/ui/dialogs/DialogClosedEvent.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <IDocumentUndoRedo.hxx>
#include <formatcontentcontrol.hxx>

SFX_IMPL_INTERFACE(SwTextShell, SwBaseShell)

IMPL_STATIC_LINK( SwTextShell, DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, pEvent, void )
{
    if (SwView* pView = GetActiveView())
    {
        SwWrtShell& rWrtShell = pView->GetWrtShell();

        sal_Int16 nDialogRet = pEvent->DialogResult;
        if( nDialogRet == ui::dialogs::ExecutableDialogResults::CANCEL )
        {
            rWrtShell.Undo();
            rWrtShell.GetIDocumentUndoRedo().ClearRedo();
        }
        else
        {
            OSL_ENSURE( nDialogRet == ui::dialogs::ExecutableDialogResults::OK,
                "dialog execution failed" );
        }
    }
}

void SwTextShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("text");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Text_Toolbox_Sw);

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
                    ACFlags::CapitalStartSentence | ACFlags::CapitalStartWord |
                    ACFlags::AddNonBrkSpace | ACFlags::ChgOrdinalNumber | ACFlags::TransliterateRTL |
                    ACFlags::ChgToEnEmDash | ACFlags::SetINetAttr | ACFlags::Autocorrect |
                    ACFlags::SetDOIAttr ) )
            {
                rSh.AutoCorrect( *pACorr, cIns );
            }
            else
            {
                rSh.Insert( OUString( cIns ) );
            }
        }
        break;

    case FN_INSERT_NNBSP: // shift+mod2/alt+space inserts some other character w/o going through SwEditWin::KeyInput(), at least on macOS
    case SID_INSERT_RLM :
    case SID_INSERT_LRM :
    case SID_INSERT_WJ :
    case SID_INSERT_ZWSP:
    {
        sal_Unicode cIns = 0;
        switch(nSlot)
        {
            case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
            case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
            case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
            case SID_INSERT_WJ: cIns = CHAR_WJ; break;
            case FN_INSERT_NNBSP: cIns = CHAR_NNBSP; break;
        }
        rSh.Insert( OUString( cIns ) );
    }
    break;

    case FN_INSERT_BREAK:
        {
            if (!rSh.CursorInsideInputField() && !rSh.CursorInsideContentControl())
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

    case FN_INSERT_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::RICH_TEXT);
        rReq.Done();
        break;

    case FN_INSERT_CHECKBOX_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::CHECKBOX);
        rReq.Done();
        break;

    case FN_INSERT_DROPDOWN_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::DROP_DOWN_LIST);
        rReq.Done();
        break;

    case FN_INSERT_PICTURE_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::PICTURE);
        rReq.Done();
        break;

    case FN_INSERT_DATE_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::DATE);
        rReq.Done();
        break;

    case FN_INSERT_PLAIN_TEXT_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::PLAIN_TEXT);
        rReq.Done();
        break;

    case FN_INSERT_COMBO_BOX_CONTENT_CONTROL:
        rSh.InsertContentControl(SwContentControlType::COMBO_BOX);
        rReq.Done();
        break;

    case FN_CONTENT_CONTROL_PROPERTIES:
    {
        SwWrtShell& rWrtSh = GetShell();
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwContentControlDlg(GetView().GetFrameWeld(), rWrtSh));
        VclAbstractDialog::AsyncContext aContext;
        aContext.maEndDialogFn = [](sal_Int32){};
        pDlg->StartExecuteAsync(aContext);
        rReq.Done();
        break;
    }

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
#endif

    case SID_INSERT_OBJECT:
    {
        const SfxGlobalNameItem* pNameItem = rReq.GetArg<SfxGlobalNameItem>(SID_INSERT_OBJECT);
        SvGlobalName *pName = nullptr;
        SvGlobalName aName;
        if ( pNameItem )
        {
            aName = pNameItem->GetValue();
            pName = &aName;
        }

        svt::EmbeddedObjectRef xObj;
        rSh.InsertObject( xObj, pName, nSlot);
        rReq.Done();
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
                    ScrollingMode eScroll = ScrollingMode::Auto;
                    if( pScrollingItem && pScrollingItem->GetValue() <= int(ScrollingMode::Auto) )
                        eScroll = static_cast<ScrollingMode>(pScrollingItem->GetValue());

                    Size aMargin;
                    if ( pMarginItem )
                        aMargin = pMarginItem->GetSize();

                    xSet->setPropertyValue("FrameURL", uno::Any( pURLItem->GetValue() ) );
                    if ( pNameItem )
                        xSet->setPropertyValue("FrameName", uno::Any( pNameItem->GetValue() ) );

                    if ( eScroll == ScrollingMode::Auto )
                        xSet->setPropertyValue("FrameIsAutoScroll",
                            uno::Any( true ) );
                    else
                        xSet->setPropertyValue("FrameIsScrollingMode",
                            uno::Any( eScroll == ScrollingMode::Yes ) );

                    if ( pBorderItem )
                        xSet->setPropertyValue("FrameIsBorder",
                            uno::Any( pBorderItem->GetValue() ) );

                    if ( pMarginItem )
                    {
                        xSet->setPropertyValue("FrameMarginWidth",
                            uno::Any( sal_Int32( aMargin.Width() ) ) );

                        xSet->setPropertyValue("FrameMarginHeight",
                            uno::Any( sal_Int32( aMargin.Height() ) ) );
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
            rSh.InsertObject( xObj, nullptr, nSlot);
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
                SwInsertChart( LINK( this, SwTextShell, DialogClosedHdl ) );
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

                SwTableFUNC( &rSh ).InsertChart( xDataProvider, bFillWithData, aRangeString );
                rSh.LaunchOLEObj();

                svt::EmbeddedObjectRef& xObj = rSh.GetOLEObject();
                if(pItem && xObj.is())
                {
                    Size aSize(static_cast<const SvxSizeItem*>(pItem)->GetSize());
                    aSize = o3tl::convert(aSize, o3tl::Length::twip, o3tl::Length::mm100);

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
            const SfxUInt16Item* pColsItem = nullptr;
            if(FN_INSERT_FRAME_INTERACT_NOCOL != nSlot &&
                (pColsItem = pArgs->GetItemIfSet(SID_ATTR_COLUMNS, false)))
                nCols = pColsItem->GetValue();
            if(const SfxUInt16Item* pModifierItem = pArgs->GetItemIfSet(SID_MODIFIER, false))
                bModifier1 |= KEY_MOD1 == pModifierItem->GetValue();
        }
        if(bModifier1 )
        {
            SwEditWin& rEdtWin = GetView().GetEditWin();
            Size aWinSize = rEdtWin.GetSizePixel();
            Point aStartPos(aWinSize.Width()/2, aWinSize.Height() / 2);
            aStartPos = rEdtWin.PixelToLogic(aStartPos);
            constexpr tools::Long constTwips_2cm = o3tl::toTwips(2, o3tl::Length::cm);
            constexpr tools::Long constTwips_4cm = o3tl::toTwips(4, o3tl::Length::cm);
            aStartPos.AdjustX(-constTwips_4cm);
            aStartPos.AdjustY(-constTwips_2cm);
            Size aSize(2 * constTwips_4cm, 2 * constTwips_2cm);
            GetShell().LockPaint(LockPaintReason::InsertFrame);
            GetShell().StartAllAction();
            SwFlyFrameAttrMgr aMgr( true, GetShellPtr(), Frmmgr_Type::TEXT, nullptr );
            if(nCols > 1)
            {
                SwFormatCol aCol;
                aCol.Init( nCols, aCol.GetGutterWidth(), aCol.GetWishWidth() );
                aMgr.SetCol( aCol );
            }
            aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aStartPos, aSize);
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
            if( HTML_CFG_MSIE == SvxHtmlOptions::GetExportMode() )
            {
                bSingleCol = true;
            }

        }
        // Create new border
        SwFlyFrameAttrMgr aMgr( true, GetShellPtr(), Frmmgr_Type::TEXT, nullptr );
        if(pArgs)
        {
            Size aSize(aMgr.GetSize());
            aSize.setWidth( GetShell().GetAnyCurRect(CurRectType::PagePrt).Width() );
            Point aPos = aMgr.GetPos();
            RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
            if(pArgs->GetItemState(nSlot, false, &pItem) == SfxItemState::SET)
                eAnchor = static_cast<RndStdIds>(static_cast<const SfxUInt16Item *>(pItem)->GetValue());
            if(pArgs->GetItemState(FN_PARAM_1, false, &pItem)  == SfxItemState::SET)
                aPos = static_cast<const SfxPointItem *>(pItem)->GetValue();
            if(pArgs->GetItemState(FN_PARAM_2, false, &pItem)  == SfxItemState::SET)
                aSize = static_cast<const SvxSizeItem *>(pItem)->GetSize();
            if(const SfxUInt16Item* pColsItem = pArgs->GetItemIfSet(SID_ATTR_COLUMNS, false))
            {
                const sal_uInt16 nCols = pColsItem->GetValue();
                if( !bSingleCol && 1 < nCols )
                {
                    SwFormatCol aFormatCol;
                    aFormatCol.Init( nCols , (rReq.IsAPI() ? 0
                                        : DEF_GUTTER_WIDTH), USHRT_MAX );
                    aMgr.SetCol(aFormatCol);
                }
            }

            GetShell().LockPaint(LockPaintReason::InsertFrame);
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
            ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateFrameTabDialog("FrameDialog",
                                                  GetView().GetViewFrame(),
                                                  GetView().GetFrameWeld(),
                                                  aSet));
            if(pDlg->Execute() == RET_OK && pDlg->GetOutputItemSet())
            {
                //local variable necessary at least after call of .AutoCaption() because this could be deleted at this point
                SwWrtShell& rShell = GetShell();
                rShell.LockPaint(LockPaintReason::InsertFrame);
                rShell.StartAllAction();
                rShell.StartUndo(SwUndoId::INSERT);

                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                aMgr.SetAttrSet(*pOutSet);

                // At first delete the selection at the ClickToEditField.
                if( rShell.IsInClickToEdit() )
                    rShell.DelRight();

                aMgr.InsertFlyFrame();

                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        GetView().GetViewFrame().GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    //FN_INSERT_FRAME
                    sal_uInt16 nAnchor = static_cast<sal_uInt16>(aMgr.GetAnchor());
                    rReq.AppendItem(SfxUInt16Item(nSlot, nAnchor));
                    rReq.AppendItem(SfxPointItem(FN_PARAM_1, rShell.GetObjAbsPos()));
                    rReq.AppendItem(SvxSizeItem(FN_PARAM_2, rShell.GetObjSize()));
                    rReq.Done();
                }

                GetView().AutoCaption(FRAME_CAP);

                {
                    SwRewriter aRewriter;

                    aRewriter.AddRule(UndoArg1, SwResId(STR_FRAME));

                    rShell.EndUndo(SwUndoId::INSERT, &aRewriter);
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
        VclPtr<VclAbstractDialog> pColDlg(pFact->CreateSwColumnDialog(GetView().GetFrameWeld(), rSh));
        pColDlg->StartExecuteAsync([=](sal_Int32 /*nResult*/){
            pColDlg->disposeOnce();
        });
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
    const bool bCursorInHidden = rSh.IsInHiddenRange(/*bSelect=*/false);

    while ( nWhich )
    {
        switch ( nWhich )
        {
        case SID_INSERT_AVMEDIA:
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
                        const sal_uInt16 nExport = SvxHtmlOptions::GetExportMode();
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
                    SfxItemSetFixed<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT> aSet(GetPool());
                    rSh.GetCurAttr( aSet );

                    SvxHyperlinkItem aHLinkItem;
                    if(const SwFormatINetFormat* pINetFormat = aSet.GetItemIfSet(RES_TXTATR_INETFMT, false))
                    {
                        aHLinkItem.SetURL(pINetFormat->GetValue());
                        aHLinkItem.SetTargetFrame(pINetFormat->GetTargetFrame());
                        aHLinkItem.SetIntName(pINetFormat->GetName());
                        const SvxMacro *pMacro = pINetFormat->GetMacro( SvMacroItemId::OnMouseOver );
                        if( pMacro )
                            aHLinkItem.SetMacro(HyperDialogEvent::MouseOverObject, *pMacro);

                        pMacro = pINetFormat->GetMacro( SvMacroItemId::OnClick );
                        if( pMacro )
                            aHLinkItem.SetMacro(HyperDialogEvent::MouseClickObject, *pMacro);

                        pMacro = pINetFormat->GetMacro( SvMacroItemId::OnMouseOut );
                        if( pMacro )
                            aHLinkItem.SetMacro(HyperDialogEvent::MouseOutObject, *pMacro);

                        // Get the text of the Link.
                        rSh.StartAction();
                        const bool bAtEnd(rSh.IsCursorPtAtEnd());
                        if(!bAtEnd) // tdf#91832: ensure forward selection
                            rSh.SwapPam();
                        rSh.CreateCursor();
                        if(!bAtEnd)
                            rSh.SwapPam();
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

                    aHLinkItem.SetInsertMode(static_cast<SvxLinkInsertMode>(aHLinkItem.GetInsertMode() |
                        (bHtmlModeOn ? HLINK_HTMLMODE : 0)));
                    aHLinkItem.SetMacroEvents ( HyperDialogEvent::MouseOverObject|
                        HyperDialogEvent::MouseClickObject | HyperDialogEvent::MouseOutObject );

                    rSet.Put(aHLinkItem);
                }
                break;

            case FN_INSERT_FRAME:
                if (rSh.IsSelFrameMode() )
                {
                    const SelectionType nSel = rSh.GetSelectionType();
                    if( ((SelectionType::Graphic | SelectionType::Ole ) & nSel ) || bCursorInHidden )
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

void SwTextShell::ExecTransliteration( SfxRequest const & rReq )
{
    using namespace ::com::sun::star::i18n;
    TransliterationFlags nMode = TransliterationFlags::NONE;

    switch( rReq.GetSlot() )
    {
    case SID_TRANSLITERATE_SENTENCE_CASE:
        nMode = TransliterationFlags::SENTENCE_CASE;
        break;
    case SID_TRANSLITERATE_TITLE_CASE:
        nMode = TransliterationFlags::TITLE_CASE;
        break;
    case SID_TRANSLITERATE_TOGGLE_CASE:
        nMode = TransliterationFlags::TOGGLE_CASE;
        break;
    case SID_TRANSLITERATE_UPPER:
        nMode = TransliterationFlags::LOWERCASE_UPPERCASE;
        break;
    case SID_TRANSLITERATE_LOWER:
        nMode = TransliterationFlags::UPPERCASE_LOWERCASE;
        break;

    case SID_TRANSLITERATE_HALFWIDTH:
        nMode = TransliterationFlags::FULLWIDTH_HALFWIDTH;
        break;
    case SID_TRANSLITERATE_FULLWIDTH:
        nMode = TransliterationFlags::HALFWIDTH_FULLWIDTH;
        break;

    case SID_TRANSLITERATE_HIRAGANA:
        nMode = TransliterationFlags::KATAKANA_HIRAGANA;
        break;
    case SID_TRANSLITERATE_KATAKANA:
        nMode = TransliterationFlags::HIRAGANA_KATAKANA;
        break;

    default:
        OSL_ENSURE(false, "wrong dispatcher");
    }

    if( nMode != TransliterationFlags::NONE )
        GetShell().TransliterateText( nMode );
}

void SwTextShell::ExecRotateTransliteration( SfxRequest const & rReq )
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
    {
        SwWrtShell& rSh = GetShell();
        if (rSh.HasSelection())
        {
            rSh.TransliterateText(m_aRotateCase.getNextMode());
        }
        else
        {
            if (rSh.IsEndSentence())
            {
                rSh.BwdSentence(true);
                rSh.TransliterateText(m_aRotateCase.getNextMode());
            }
            else if ((rSh.IsEndWrd() || rSh.IsStartWord() || rSh.IsInWord()) && rSh.SelWrd())
                rSh.TransliterateText(m_aRotateCase.getNextMode());
        }
    }
}

SwTextShell::SwTextShell(SwView &_rView) :
    SwBaseShell(_rView)
{
    SetName("Text");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Text));
}

SwTextShell::~SwTextShell()
{
}

SfxItemSet SwTextShell::CreateInsertFrameItemSet(SwFlyFrameAttrMgr& rMgr)
{
    SfxItemSet aSet(GetPool(), svl::Items<
        RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
        XATTR_FILL_FIRST,       XATTR_FILL_LAST, // tdf#95003
        SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
        SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
        SID_COLOR_TABLE,        SID_PATTERN_LIST,
        SID_HTML_MODE,          SID_HTML_MODE,
        FN_GET_PRINT_AREA,      FN_GET_PRINT_AREA,
        FN_SET_FRM_NAME,        FN_SET_FRM_NAME>);
    aSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));

    // For the Area tab page.
    GetShell().GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->PutAreaListItems(aSet);

    const SwRect &rPg = GetShell().GetAnyCurRect(CurRectType::Page);
    SwFormatFrameSize aFrameSize(SwFrameSize::Variable, rPg.Width(), rPg.Height());
    aFrameSize.SetWhich(GetPool().GetWhich(SID_ATTR_PAGE_SIZE));
    aSet.Put(aFrameSize);

    const SwRect &rPr = GetShell().GetAnyCurRect(CurRectType::PagePrt);
    SwFormatFrameSize aPrtSize(SwFrameSize::Variable, rPr.Width(), rPr.Height());
    aPrtSize.SetWhich(GetPool().GetWhich(FN_GET_PRINT_AREA));
    aSet.Put(aPrtSize);

    aSet.Put(rMgr.GetAttrSet());
    aSet.SetParent( rMgr.GetAttrSet().GetParent() );

    // Delete minimum size in columns.
    SvxBoxInfoItem aBoxInfo(aSet.Get(SID_ATTR_BORDER_INNER));
    const SvxBoxItem& rBox = aSet.Get(RES_BOX);
    aBoxInfo.SetMinDist(false);
    aBoxInfo.SetDefDist(rBox.GetDistance(SvxBoxItemLine::LEFT));
    aSet.Put(aBoxInfo);

    return aSet;
}

void SwTextShell::InsertSymbol( SfxRequest& rReq )
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxStringItem* pItem = nullptr;
    if( pArgs )
        pItem = pArgs->GetItemIfSet(SID_CHARMAP, false);

    OUString aChars, aFontName;
    if ( pItem )
    {
        aChars = pItem->GetValue();
        const SfxStringItem* pFontItem = pArgs->GetItemIfSet( SID_ATTR_SPECIALCHAR, false );
        if ( pFontItem )
            aFontName = pFontItem->GetValue();
    }

    SwWrtShell &rSh = GetShell();
    SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONT,
                    RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
                    RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT>  aSet( GetPool() );
    rSh.GetCurAttr( aSet );
    SvtScriptType nScript = rSh.GetScriptType();

    std::shared_ptr<SvxFontItem> aFont(std::make_shared<SvxFontItem>(RES_CHRATR_FONT));
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, false );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
        {
            aFont.reset(static_cast<SvxFontItem*>(pI->Clone()));
        }
        else
        {
            TypedWhichId<SvxFontItem> nFontWhich =
                            GetWhichOfScript(
                                RES_CHRATR_FONT,
                                SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) );
            aFont.reset(aSet.Get(nFontWhich).Clone());
        }

        if (aFontName.isEmpty())
            aFontName = aFont->GetFamilyName();
    }

    vcl::Font aNewFont(aFontName, Size(1,1)); // Size only because CTOR.
    if( aChars.isEmpty() )
    {
        // Set selected font as default.
        SfxAllItemSet aAllSet( rSh.GetAttrPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SwViewOption aOpt(*GetShell().GetViewOptions());
        const OUString& sSymbolFont = aOpt.GetSymbolFont();
        if( aFontName.isEmpty() && !sSymbolFont.isEmpty() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aFont->GetFamilyName() ) );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        auto xFrame = GetView().GetViewFrame().GetFrame().GetFrameInterface();
        ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateCharMapDialog(GetView().GetFrameWeld(), aAllSet, xFrame));
        pDlg->Execute();
        return;
    }

    if( aChars.isEmpty() )
        return;

    rSh.StartAllAction();

    // Delete selected content.
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_SPECIALCHAR));

    rSh.StartUndo( SwUndoId::INSERT, &aRewriter );
    if ( rSh.HasSelection() )
    {
        rSh.DelRight();
        aSet.ClearItem();
        rSh.GetCurAttr( aSet );

        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, false );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
        {
            aFont.reset(static_cast<SvxFontItem*>(pI->Clone()));
        }
        else
        {
            TypedWhichId<SvxFontItem> nFontWhich =
                GetWhichOfScript(
                        RES_CHRATR_FONT,
                        SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) );
            aFont.reset(aSet.Get( nFontWhich ).Clone());
        }
    }

    // Insert character.
    rSh.Insert( aChars );

    // #108876# a font attribute has to be set always due to a guessed script type
    if( !aNewFont.GetFamilyName().isEmpty() )
    {
        std::unique_ptr<SvxFontItem> aNewFontItem(aFont->Clone());
        aNewFontItem->SetFamilyName( aNewFont.GetFamilyName() );
        aNewFontItem->SetFamily(  aNewFont.GetFamilyType());
        aNewFontItem->SetPitch(   aNewFont.GetPitch());
        aNewFontItem->SetCharSet( aNewFont.GetCharSet() );

        SfxItemSetFixed<RES_CHRATR_FONT, RES_CHRATR_FONT,
                       RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
                       RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT>  aRestoreSet( GetPool() );

        nScript = g_pBreakIt->GetAllScriptsOfText( aChars );
        if( SvtScriptType::LATIN & nScript )
        {
            aRestoreSet.Put( aSet.Get( RES_CHRATR_FONT ) );
            aNewFontItem->SetWhich(RES_CHRATR_FONT);
            aSet.Put( *aNewFontItem );
        }
        if( SvtScriptType::ASIAN & nScript )
        {
            aRestoreSet.Put( aSet.Get( RES_CHRATR_CJK_FONT ) );
            aNewFontItem->SetWhich(RES_CHRATR_CJK_FONT);
            aSet.Put( *aNewFontItem );
        }
        if( SvtScriptType::COMPLEX & nScript )
        {
            aRestoreSet.Put( aSet.Get( RES_CHRATR_CTL_FONT ) );
            aNewFontItem->SetWhich(RES_CHRATR_CTL_FONT);
            aSet.Put( *aNewFontItem );
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

        // Why was this done? aFont is not used anymore below, we are not
        // in a loop and it's a local variable...?
        // aFont = aNewFontItem;
    }

    rSh.EndAllAction();
    rSh.EndUndo();

    if ( !aChars.isEmpty() )
    {
        rReq.AppendItem( SfxStringItem( SID_CHARMAP, aChars ) );
        rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aNewFont.GetFamilyName() ) );
        rReq.Done();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
