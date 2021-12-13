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
#include <config_fuzzers.h>

#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <o3tl/any.hxx>
#include <vcl/graphicfilter.hxx>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <officecfg/Office/Common.hxx>
#include <SwCapObjType.hxx>
#include <SwStyleNameMapper.hxx>
#include <docary.hxx>
#include <hintids.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/filedlghelper.hxx>
#include <editeng/langitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/linkwarn.hxx>
#include <sfx2/htmlmode.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/lrspitem.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/weld.hxx>
#include <editeng/tstpitem.hxx>
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <svl/ptitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/errinf.hxx>
#include <tools/urlobj.hxx>
#include <svx/svdview.hxx>
#include <swtypes.hxx>
#include <swwait.hxx>
#include <redlndlg.hxx>
#include <gotodlg.hxx>
#include <view.hxx>
#include <uivwimp.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <printdata.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <DocumentRedlineManager.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <basesh.hxx>
#include <swmodule.hxx>
#include <uitool.hxx>
#include <shellio.hxx>
#include <fmtinfmt.hxx>
#include <mdiexp.hxx>
#include <drawbase.hxx>
#include <frmatr.hxx>
#include <frmmgr.hxx>
#include <pagedesc.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <edtwin.hxx>
#include <wview.hxx>
#include <cmdid.h>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <strings.hrc>
#include <swerror.h>
#include <globals.hrc>
#include <fmtclds.hxx>
#include <sfx2/templatedlg.hxx>
#include <dbconfig.hxx>
#include <dbmgr.hxx>
#include <reffld.hxx>
#include <comphelper/lok.hxx>

#include <PostItMgr.hxx>

#include <comphelper/processfactory.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <svx/svxdlg.hxx>
#include <swabstdlg.hxx>
#include <fmthdft.hxx>
#include <unotextrange.hxx>
#include <docstat.hxx>
#include <wordcountdialog.hxx>
#include <sfx2/sidebar/Sidebar.hxx>

#include <vcl/GraphicNativeTransform.hxx>
#include <vcl/GraphicNativeMetadata.hxx>
#include <vcl/settings.hxx>
#include <i18nutil/searchopt.hxx>
#include <osl/diagnose.h>
#include <paratr.hxx>
#include <rootfrm.hxx>
#include <frameformats.hxx>

#include <viewimp.hxx>
#include <pagefrm.hxx>

#include <memory>
#include <string_view>
#include <svl/slstitm.hxx>

#include <basegfx/utils/zoomtools.hxx>

#include <ndtxt.hxx>

const char sStatusDelim[] = " : ";

using namespace sfx2;
using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui::dialogs;

static void lcl_SetAllTextToDefaultLanguage( SwWrtShell &rWrtSh, sal_uInt16 nWhichId )
{
    if (!(nWhichId == RES_CHRATR_LANGUAGE ||
          nWhichId == RES_CHRATR_CJK_LANGUAGE ||
          nWhichId == RES_CHRATR_CTL_LANGUAGE))
        return;

    rWrtSh.StartAction();
    rWrtSh.LockView( true );
    rWrtSh.Push();

    // prepare to apply new language to all text in document
    rWrtSh.SelAll();
    rWrtSh.ExtendedSelectAll();

    // set language attribute to default for all text
    rWrtSh.ResetAttr({ nWhichId });

    rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
    rWrtSh.LockView( false );
    rWrtSh.EndAction();

}

/**
 * Create string for showing the page number in the statusbar
 *
 * @param nPhyNum  The physical page number
 * @param nVirtNum The logical page number (user-assigned)
 * @param rPgStr   User-defined page name (will be shown if different from logical page number)
 *
 * @return OUString Formatted string: Page 1 of 10 (Page 1 of 8 to print OR Page nVirtNumv/rPgStr)
 **/
OUString SwView::GetPageStr(sal_uInt16 nPhyNum, sal_uInt16 nVirtNum, const OUString& rPgStr)
{
    // Show user-defined page number in brackets if any.
    OUString extra;
    if (!rPgStr.isEmpty() && std::u16string_view(OUString::number(nPhyNum)) != rPgStr)
        extra = rPgStr;
    else if (nPhyNum != nVirtNum)
        extra = OUString::number(nVirtNum);

    sal_uInt16 nPageCount = GetWrtShell().GetPageCnt();
    sal_uInt16 nPrintedPhyNum = nPhyNum;
    sal_uInt16 nPrintedPageCount = nPageCount;
    if (!GetWrtShell().getIDocumentDeviceAccess().getPrintData().IsPrintEmptyPages())
        SwDoc::CalculateNonBlankPages(*m_pWrtShell->GetLayout(), nPrintedPageCount, nPrintedPhyNum);
    // Show printed page numbers only, when they are different
    OUString aStr( nPageCount != nPrintedPageCount
                    ? SwResId(STR_PAGE_COUNT_PRINTED)
                    : (extra.isEmpty() ? SwResId(STR_PAGE_COUNT) : SwResId(STR_PAGE_COUNT_CUSTOM)));
    aStr = aStr.replaceFirst("%1", OUString::number(nPhyNum));
    aStr = aStr.replaceFirst("%2", OUString::number(nPageCount));
    if (nPageCount != nPrintedPageCount)
    {
        aStr = aStr.replaceFirst("%3", OUString::number(nPrintedPhyNum));
        aStr = aStr.replaceFirst("%4", OUString::number(nPrintedPageCount));
    }
    else
        aStr = aStr.replaceFirst("%3", extra);

    return aStr;
}

ErrCode SwView::InsertGraphic( const OUString &rPath, const OUString &rFilter,
                                bool bLink, GraphicFilter *pFilter )
{
    SwWait aWait( *GetDocShell(), true );

    Graphic aGraphic;
    ErrCode aResult = ERRCODE_NONE;
    if( !pFilter )
    {
        pFilter = &GraphicFilter::GetGraphicFilter();
    }
    aResult = GraphicFilter::LoadGraphic( rPath, rFilter, aGraphic, pFilter );

    if( ERRCODE_NONE == aResult )
    {
        GraphicNativeMetadata aMetadata;
        if ( aMetadata.read(aGraphic) )
        {
            const Degree10 aRotation = aMetadata.getRotation();
            if (aRotation)
            {
                GraphicNativeTransform aTransform( aGraphic );
                aTransform.rotate( aRotation );
            }
        }

        SwFlyFrameAttrMgr aFrameManager( true, GetWrtShellPtr(), Frmmgr_Type::GRF, nullptr );
        SwWrtShell& rShell = GetWrtShell();

        // #i123922# determine if we really want to insert or replace the graphic at a selected object
        const bool bReplaceMode(rShell.HasSelection() && SelectionType::Frame == rShell.GetSelectionType());

        if(bReplaceMode)
        {
            // #i123922# Do same as in D&D, ReRead graphic and all is done
            rShell.ReRead(
                bLink ? rPath : OUString(),
                bLink ? rFilter : OUString(),
                &aGraphic);
        }
        else
        {
            rShell.StartAction();
            if( bLink )
            {
                SwDocShell* pDocSh = GetDocShell();
                INetURLObject aTemp(
                    pDocSh->HasName() ?
                        pDocSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ) :
                        OUString());

                OUString sURL = URIHelper::SmartRel2Abs(
                    aTemp, rPath, URIHelper::GetMaybeFileHdl() );
                aGraphic.setOriginURL(sURL);
                rShell.Insert( sURL, rFilter, aGraphic, &aFrameManager );
            }
            else
            {
                rShell.Insert( OUString(), OUString(), aGraphic, &aFrameManager );
            }

            // it is too late after "EndAction" because the Shell can already be destroyed.
            rShell.EndAction();
        }
    }
    return aResult;
}

bool SwView::InsertGraphicDlg( SfxRequest& rReq )
{
    bool bReturn = false;
    SwDocShell* pDocShell = GetDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();

    OUString sGraphicFormat = SwResId(STR_POOLFRM_GRAPHIC);

// No file pickers in a non-desktop (mobile app) build.

#if HAVE_FEATURE_DESKTOP
    // when in HTML mode insert only as a link
    const sal_uInt16 nHtmlMode = ::GetHtmlMode(pDocShell);
    std::unique_ptr<FileDialogHelper> pFileDlg(new FileDialogHelper(
        ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE,
        FileDialogFlags::Graphic, GetFrameWeld()));
    pFileDlg->SetTitle(SwResId(STR_INSERT_GRAPHIC ));
    pFileDlg->SetContext( FileDialogHelper::WriterInsertImage );

    uno::Reference < XFilePicker3 > xFP = pFileDlg->GetFilePicker();
    uno::Reference < XFilePickerControlAccess > xCtrlAcc(xFP, UNO_QUERY);
    if(nHtmlMode & HTMLMODE_ON)
    {
        xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, makeAny(true));
        xCtrlAcc->enableControl( ExtendedFilePickerElementIds::CHECKBOX_LINK, false);
    }

    std::vector<OUString> aFormats;
    const size_t nArrLen = pDoc->GetFrameFormats()->size();
    for( size_t i = 0; i < nArrLen; ++i )
    {
        const SwFrameFormat* pFormat = (*pDoc->GetFrameFormats())[ i ];
        if(pFormat->IsDefault() || pFormat->IsAuto())
            continue;
        aFormats.push_back(pFormat->GetName());
    }

    // pool formats

    const std::vector<OUString>& rFramePoolArr(
            SwStyleNameMapper::GetFrameFormatUINameArray());
    for(const auto & i : rFramePoolArr)
    {
        aFormats.push_back(i);
    }

    std::sort(aFormats.begin(), aFormats.end());
    aFormats.erase(std::unique(aFormats.begin(), aFormats.end()), aFormats.end());

    Sequence<OUString> aListBoxEntries(aFormats.size());
    OUString* pEntries = aListBoxEntries.getArray();
    sal_Int16 nSelect = 0;
    for( size_t i = 0; i < aFormats.size(); ++i )
    {
        pEntries[i] = aFormats[i];
        if(pEntries[i] == sGraphicFormat)
            nSelect = i;
    }
    try
    {
        Any aTemplates(&aListBoxEntries, cppu::UnoType<decltype(aListBoxEntries)>::get());

        xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
            ListboxControlActions::ADD_ITEMS , aTemplates );

        Any aSelectPos(&nSelect, cppu::UnoType<decltype(nSelect)>::get());
        xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
            ListboxControlActions::SET_SELECT_ITEM, aSelectPos );
    }
    catch (const Exception&)
    {
        OSL_FAIL("control access failed");
    }
#endif

    const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(SID_INSERT_GRAPHIC);
    bool bShowError = !pName;
    if( pName
#if HAVE_FEATURE_DESKTOP
        || ERRCODE_NONE == pFileDlg->Execute()
#endif
        )
    {

        OUString aFileName, aFilterName;
        if ( pName )
        {
            aFileName = pName->GetValue();
            const SfxStringItem* pFilter = rReq.GetArg<SfxStringItem>(FN_PARAM_FILTER);
            if ( pFilter )
                aFilterName = pFilter->GetValue();
        }
#if HAVE_FEATURE_DESKTOP
        else
        {
            aFileName = pFileDlg->GetPath();
            aFilterName = pFileDlg->GetCurrentFilter();
            rReq.AppendItem( SfxStringItem( SID_INSERT_GRAPHIC, aFileName ) );
            rReq.AppendItem( SfxStringItem( FN_PARAM_FILTER, aFilterName ) );

            bool bAsLink = false;
            if(nHtmlMode & HTMLMODE_ON)
                bAsLink = true;
            else
            {
                try
                {
                    Any aVal = xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
                    OSL_ENSURE(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found");
                    bAsLink = !aVal.hasValue() || *o3tl::doAccess<bool>(aVal);
                    Any aTemplateValue = xCtrlAcc->getValue(
                        ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
                        ListboxControlActions::GET_SELECTED_ITEM );
                    OUString sTmpl;
                    aTemplateValue >>= sTmpl;
                    rReq.AppendItem( SfxStringItem( FN_PARAM_2, sTmpl) );
                }
                catch (const Exception&)
                {
                    OSL_FAIL("control access failed");
                }
            }
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
        }
        const SfxBoolItem* pAsLink = rReq.GetArg<SfxBoolItem>(FN_PARAM_1);
        const SfxStringItem* pStyle = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
#endif

        bool bAsLink = false;

#if HAVE_FEATURE_DESKTOP
        if( nHtmlMode & HTMLMODE_ON )
            bAsLink = true;
        else
        {
            if ( rReq.GetArgs() )
            {
                if ( pAsLink )
                    bAsLink = pAsLink->GetValue();
                if ( pStyle && !pStyle->GetValue().isEmpty() )
                    sGraphicFormat = pStyle->GetValue();
            }
            else
            {
                Any aVal = xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
                OSL_ENSURE(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found");
                bAsLink = !aVal.hasValue() || *o3tl::doAccess<bool>(aVal);
                Any aTemplateValue = xCtrlAcc->getValue(
                    ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
                    ListboxControlActions::GET_SELECTED_ITEM );
                OUString sTmpl;
                aTemplateValue >>= sTmpl;
                if( !sTmpl.isEmpty() )
                    sGraphicFormat = sTmpl;
                rReq.AppendItem( SfxStringItem( FN_PARAM_2, sGraphicFormat ) );
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
            }

            // really store as link only?
            if( bAsLink && officecfg::Office::Common::Misc::ShowLinkWarningDialog::get() )
            {
                SvxLinkWarningDialog aWarnDlg(GetFrameWeld(), pFileDlg->GetPath());
                if (aWarnDlg.run() != RET_OK)
                    bAsLink=false; // don't store as link
            }
        }
#endif

        SwWrtShell& rSh = GetWrtShell();
        rSh.LockPaint();
        rSh.StartAction();

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, SwResId(STR_GRAPHIC_DEFNAME));

        // #i123922# determine if we really want to insert or replace the graphic at a selected object
        const bool bReplaceMode(rSh.HasSelection() && SelectionType::Frame == rSh.GetSelectionType());

        rSh.StartUndo(SwUndoId::INSERT, &aRewriter);

        ErrCode nError = InsertGraphic( aFileName, aFilterName, bAsLink, &GraphicFilter::GetGraphicFilter() );

        // format not equal to current filter (with autodetection)
        if( nError == ERRCODE_GRFILTER_FORMATERROR )
            nError = InsertGraphic( aFileName, OUString(), bAsLink, &GraphicFilter::GetGraphicFilter() );

        // #i123922# no new FrameFormat for replace mode, only when new object was created,
        // else this would reset the current setting for the frame holding the graphic
        if ( !bReplaceMode && rSh.IsFrameSelected() )
        {
            SwFrameFormat* pFormat = pDoc->FindFrameFormatByName( sGraphicFormat );
            if(!pFormat)
                pFormat = pDoc->MakeFrameFormat(sGraphicFormat,
                                        pDocShell->GetDoc()->GetDfltFrameFormat(),
                                        true, false);
            rSh.SetFrameFormat( pFormat );
        }

        TranslateId pResId;
        if( nError == ERRCODE_GRFILTER_OPENERROR )
            pResId = STR_GRFILTER_OPENERROR;
        else if( nError == ERRCODE_GRFILTER_IOERROR )
            pResId = STR_GRFILTER_IOERROR;
        else if( nError ==ERRCODE_GRFILTER_FORMATERROR )
            pResId = STR_GRFILTER_FORMATERROR;
        else if( nError ==ERRCODE_GRFILTER_VERSIONERROR )
            pResId = STR_GRFILTER_VERSIONERROR;
        else if( nError ==ERRCODE_GRFILTER_FILTERERROR )
            pResId = STR_GRFILTER_FILTERERROR;
        else if( nError ==ERRCODE_GRFILTER_TOOBIG )
            pResId = STR_GRFILTER_TOOBIG;

        rSh.EndAction();
        rSh.UnlockPaint();
        if (pResId)
        {
            if( bShowError )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SwResId(pResId)));
                xInfoBox->run();
            }
            rReq.Ignore();
        }
        else
        {
            // set the specific graphic attributes to the graphic
            bReturn = true;
            AutoCaption( GRAPHIC_CAP );
            rReq.Done();
        }

        rSh.EndUndo(); // due to possible change of Shell
    }

    return bReturn;
}

void SwView::Execute(SfxRequest &rReq)
{
    const sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    bool bIgnore = false;
    switch( nSlot )
    {
        case SID_CREATE_SW_DRAWVIEW:
            m_pWrtShell->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
            break;

        case FN_LINE_NUMBERING_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclSwViewDialog(*this));
            VclAbstractDialog::AsyncContext aContext;
            aContext.maEndDialogFn = [](sal_Int32){};
            pDlg->StartExecuteAsync(aContext);
            break;
        }
        case FN_EDIT_LINK_DLG:
            EditLinkDlg();
            break;
        case SID_REFRESH_VIEW:
            GetEditWin().Invalidate();
            m_pWrtShell->Reformat();
            break;
        case FN_PAGEUP:
        case FN_PAGEUP_SEL:
        case FN_PAGEDOWN:
        case FN_PAGEDOWN_SEL:
        {
            tools::Rectangle aVis( GetVisArea() );
            SwEditWin& rTmpWin = GetEditWin();
            if ( FN_PAGEUP == nSlot || FN_PAGEUP_SEL == nSlot )
                PageUpCursor(FN_PAGEUP_SEL == nSlot);
            else
                PageDownCursor(FN_PAGEDOWN_SEL == nSlot);

            rReq.SetReturnValue(SfxBoolItem(nSlot,
                                                aVis != GetVisArea()));
            //#i42732# - notify the edit window that from now on we do not use the input language
            rTmpWin.SetUseInputLanguage( false );
        }
        break;
        case SID_ZOOM_IN:
        case SID_ZOOM_OUT:
        {
            tools::Long nFact = m_pWrtShell->GetViewOptions()->GetZoom();
            if (SID_ZOOM_IN == nSlot)
                nFact = basegfx::zoomtools::zoomIn(nFact);
            else
                nFact = basegfx::zoomtools::zoomOut(nFact);
            SetZoom(SvxZoomType::PERCENT, nFact);
        }
        break;
        case FN_TO_PREV_PAGE:
        case FN_TO_NEXT_PAGE:
        {
            sal_uInt16 nPage = 0;
            if (m_pWrtShell->IsCursorVisible())
                nPage = m_pWrtShell->GetCursor()->GetPageNum();
            else
            {
                SwFrame* pPageFrame = m_pWrtShell->Imp()->GetFirstVisPage(m_pWrtShell->GetOut());
                if (pPageFrame)
                    nPage = pPageFrame->GetPhyPageNum();
            }
            if (nPage != 0)
            {
                sal_uInt16 nOldPage(nPage);
                if (FN_TO_PREV_PAGE == nSlot && nPage > 1)
                    nPage--;
                else if (FN_TO_NEXT_PAGE == nSlot && nPage < m_pWrtShell->GetPageCount())
                    nPage++;
                if (nPage != nOldPage)
                {
                    m_pWrtShell->LockPaint();
                    if (IsDrawMode())
                        LeaveDrawCreate();
                    m_pWrtShell->EnterStdMode();
                    m_pWrtShell->GotoPage(nPage, true);
                    // set visible area (borrowed from SwView::PhyPageUp/Down)
                    const Point aPt(m_aVisArea.Left(), m_pWrtShell->GetPagePos(nPage).Y());
                    Point aAlPt(AlignToPixel(aPt));
                    if(aPt.Y() != aAlPt.Y())
                        aAlPt.AdjustY(3 * GetEditWin().PixelToLogic(Size(0, 1)).Height());
                    SetVisArea(aAlPt);
                    m_pWrtShell->UnlockPaint();
                }
            }
        }
        break;
        case FN_SELECTION_CYCLE:
        {
            if (m_pWrtShell->IsSelFrameMode())
                break;
            if (!m_pWrtShell->IsStdMode())
                m_pWrtShell->EnterStdMode();
            SwShellCursor *pCursor = m_pWrtShell->SwCursorShell::GetCursor_();
            Point CurrMarkPt = pCursor->GetMkPos();
            Point CurrPointPt = pCursor->GetPtPos();
            sal_uInt16 nStep = m_aSelectCycle.nStep;
            if (nStep && (CurrMarkPt != m_aSelectCycle.m_MarkPt || CurrPointPt != m_aSelectCycle.m_PointPt))
                nStep = 0;
            switch(nStep)
            {
                case 0:
                    m_aSelectCycle.m_pInitialCursor = CurrPointPt;
                    m_pWrtShell->SwCursorShell::ClearMark();
                    m_pWrtShell->SelWrd(&CurrPointPt);
                    break;
                case 1:
                    m_pWrtShell->SelSentence(&CurrPointPt);
                    break;
                case 2:
                    m_pWrtShell->SelPara(&CurrPointPt);
                    break;
                case 3:
                    m_pWrtShell->SwCursorShell::ClearMark();
                    m_pWrtShell->SwCursorShell::SetCursor(m_aSelectCycle.m_pInitialCursor);
                    break;
            }
            nStep++;
            nStep %= 4;
            pCursor = m_pWrtShell->SwCursorShell::GetCursor_();
            m_aSelectCycle.m_MarkPt = pCursor->GetMkPos();
            m_aSelectCycle.m_PointPt = pCursor->GetPtPos();
            m_aSelectCycle.nStep = nStep;
        }
        break;
        case FN_REDLINE_ON:
        {
            if( pArgs &&
                SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem ))
            {
                IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
                Sequence <sal_Int8> aPasswd = rIDRA.GetRedlinePassword();
                if( aPasswd.hasElements() )
                {
                    OSL_ENSURE( !static_cast<const SfxBoolItem*>(pItem)->GetValue(), "SwView::Execute(): password set and redlining off doesn't match!" );

                    // dummy password from OOXML import: only confirmation dialog
                    if (aPasswd.getLength() == 1 && aPasswd[0] == 1)
                    {
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(m_pWrtShell->GetView().GetFrameWeld(),
                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                   SfxResId(RID_SVXSTR_END_REDLINING_WARNING)));
                        xWarn->set_default_response(RET_NO);
                        if (xWarn->run() == RET_YES)
                            rIDRA.SetRedlinePassword(Sequence <sal_Int8> ());
                        else
                            break;
                    }
                    else
                    {
                        // xmlsec05:    new password dialog
                        SfxPasswordDialog aPasswdDlg(GetFrameWeld());
                        aPasswdDlg.SetMinLen(1);
                        //#i69751# the result of Execute() can be ignored
                        (void)aPasswdDlg.run();
                        OUString sNewPasswd(aPasswdDlg.GetPassword());
                        Sequence <sal_Int8> aNewPasswd = rIDRA.GetRedlinePassword();
                        SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                        if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                            rIDRA.SetRedlinePassword(Sequence <sal_Int8> ());
                        else
                        {   // xmlsec05: message box for wrong password
                            break;
                        }
                    }
                }

                SwDocShell* pDocShell = GetDocShell();
                pDocShell->SetChangeRecording( static_cast<const SfxBoolItem*>(pItem)->GetValue(), /*bLockAllViews=*/true );

                // Notify all view shells of this document, as the track changes mode is document-global.
                for (SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst(pDocShell); pViewFrame; pViewFrame = SfxViewFrame::GetNext(*pViewFrame, pDocShell))
                {
                    pViewFrame->GetBindings().Invalidate(FN_REDLINE_ON);
                    pViewFrame->GetBindings().Update(FN_REDLINE_ON);
                }
            }
        }
        break;
        case FN_REDLINE_PROTECT :
        {
            IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
            Sequence <sal_Int8> aPasswd = rIDRA.GetRedlinePassword();
            if( pArgs && SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem )
                && static_cast<const SfxBoolItem*>(pItem)->GetValue() == aPasswd.hasElements() )
                break;

            // xmlsec05:    new password dialog
            //              message box for wrong password
            SfxPasswordDialog aPasswdDlg(GetFrameWeld());
            aPasswdDlg.SetMinLen(1);
            if (!aPasswd.hasElements())
                aPasswdDlg.ShowExtras(SfxShowExtras::CONFIRM);
            if (aPasswdDlg.run())
            {
                RedlineFlags nOn = RedlineFlags::On;
                OUString sNewPasswd(aPasswdDlg.GetPassword());
                Sequence <sal_Int8> aNewPasswd =
                        rIDRA.GetRedlinePassword();
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if(!aPasswd.hasElements())
                {
                    rIDRA.SetRedlinePassword(aNewPasswd);
                }
                else if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                {
                    rIDRA.SetRedlinePassword(Sequence <sal_Int8> ());
                    nOn = RedlineFlags::NONE;
                }
                const RedlineFlags nMode = rIDRA.GetRedlineFlags();
                m_pWrtShell->SetRedlineFlagsAndCheckInsMode( (nMode & ~RedlineFlags::On) | nOn);
                rReq.AppendItem( SfxBoolItem( FN_REDLINE_PROTECT, !(nMode&RedlineFlags::On) ) );
            }
            else
                bIgnore = true;
        }
        break;
        case FN_REDLINE_SHOW:

            if( pArgs &&
                SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem))
            {
                // tdf#125754 avoid recursive layout
                // because all views share the layout, have to use AllAction
                const bool bShow = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                m_pWrtShell->StartAllAction();
                // always show redline insertions in Hide Changes mode
                if ( m_pWrtShell->GetViewOptions()->IsShowChangesInMargin() &&
                     m_pWrtShell->GetViewOptions()->IsShowChangesInMargin2() )
                {
                    GetDocShell()->GetDoc()->GetDocumentRedlineManager().HideAll(/*bDeletion=*/!bShow);
                }
                m_pWrtShell->GetLayout()->SetHideRedlines( !bShow );
                m_pWrtShell->EndAllAction();
                if (m_pWrtShell->IsRedlineOn())
                    m_pWrtShell->SetInsMode();
            }
            break;
        case FN_MAILMERGE_SENDMAIL_CHILDWINDOW:
        case FN_REDLINE_ACCEPT:
            GetViewFrame()->ToggleChildWindow(nSlot);
        break;
        case FN_REDLINE_ACCEPT_DIRECT:
        case FN_REDLINE_REJECT_DIRECT:
        case FN_REDLINE_ACCEPT_TONEXT:
        case FN_REDLINE_REJECT_TONEXT:
        {
            SwDoc *pDoc = m_pWrtShell->GetDoc();
            SwPaM *pCursor = m_pWrtShell->GetCursor();
            const SwRedlineTable& rRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
            SwRedlineTable::size_type nRedline = SwRedlineTable::npos;
            if (pArgs && pArgs->GetItemState(nSlot, false, &pItem) == SfxItemState::SET)
            {
                const sal_Int64 nChangeId = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                for (SwRedlineTable::size_type i = 0; i < rRedlineTable.size(); ++i)
                {
                    if (nChangeId == rRedlineTable[i]->GetId())
                        nRedline = i;
                }
            }

            if( pCursor->HasMark() && nRedline == SwRedlineTable::npos)
            {
                if (FN_REDLINE_ACCEPT_DIRECT == nSlot || FN_REDLINE_ACCEPT_TONEXT == nSlot)
                    m_pWrtShell->AcceptRedlinesInSelection();
                else
                    m_pWrtShell->RejectRedlinesInSelection();
            }
            else
            {
                // We check for a redline at the start of the selection/cursor, not the point.
                // This ensures we work properly with FN_REDLINE_NEXT_CHANGE, which leaves the
                // point at the *end* of the redline and the mark at the start (so GetRedline
                // would return NULL if called on the point)
                const SwRangeRedline* pRedline = nullptr;
                if (nRedline != SwRedlineTable::npos)
                {
                    // A redline was explicitly requested by specifying an
                    // index, don't guess based on the cursor position.

                    if (nRedline < rRedlineTable.size())
                        pRedline = rRedlineTable[nRedline];
                }
                else
                    pRedline = pDoc->getIDocumentRedlineAccess().GetRedline(*pCursor->Start(), &nRedline);

                // accept or reject table row deletion or insertion
                bool bTableChange = false;
                if ( !pRedline && m_pWrtShell->IsCursorInTable() )
                {
                    nRedline = 0;
                    auto pTabLine = pCursor->Start()->nNode.GetNode().GetTableBox()->GetUpper();

                    if ( RedlineType::None != pTabLine->GetRedlineType() )
                    {
                        nRedline = pTabLine->UpdateTextChangesOnly(nRedline);

                        if ( nRedline != SwRedlineTable::npos )
                        {
                            bTableChange = true;

                            SwWrtShell& rSh = GetWrtShell();
                            SwRewriter aRewriter;

                            aRewriter.AddRule(UndoArg1, SwResId(
                                rRedlineTable[nRedline]->GetType() == RedlineType::Delete
                                    ? STR_REDLINE_TABLE_ROW_DELETE
                                    : STR_REDLINE_TABLE_ROW_INSERT ));

                            SwUndoId eUndoId =
                                (FN_REDLINE_ACCEPT_DIRECT == nSlot || FN_REDLINE_ACCEPT_TONEXT == nSlot)
                                    ? SwUndoId::ACCEPT_REDLINE
                                    : SwUndoId::REJECT_REDLINE;

                            rSh.StartUndo( eUndoId, &aRewriter);
                            while ( nRedline != SwRedlineTable::npos && nRedline < rRedlineTable.size() )
                            {
                                pRedline = rRedlineTable[nRedline];

                                // until next redline is not in the same row
                                SwTableBox* pTableBox = pRedline->Start()->nNode.GetNode().GetTableBox();
                                if ( !pTableBox || pTableBox->GetUpper() != pTabLine )
                                    break;

                                if (FN_REDLINE_ACCEPT_DIRECT == nSlot || FN_REDLINE_ACCEPT_TONEXT == nSlot)
                                    m_pWrtShell->AcceptRedline(nRedline);
                                else
                                    m_pWrtShell->RejectRedline(nRedline);
                            }
                            rSh.EndUndo( eUndoId, &aRewriter);
                        }
                    }
                }
                else
                {
                    assert(pRedline != nullptr);
                }

                if (pRedline && !bTableChange)
                {
                    if (FN_REDLINE_ACCEPT_DIRECT == nSlot || FN_REDLINE_ACCEPT_TONEXT == nSlot)
                        m_pWrtShell->AcceptRedline(nRedline);
                    else
                        m_pWrtShell->RejectRedline(nRedline);
                }
            }
            if (FN_REDLINE_ACCEPT_TONEXT == nSlot || FN_REDLINE_REJECT_TONEXT == nSlot)
            {
                // Go to next change after accepting or rejecting one (tdf#101977)
                GetViewFrame()->GetDispatcher()->Execute(FN_REDLINE_NEXT_CHANGE, SfxCallMode::ASYNCHRON);
            }
        }
        break;

        case FN_REDLINE_NEXT_CHANGE:
        {
            // If a parameter is provided, try going to the nth change, not to
            // the next one.
            SwDoc* pDoc = m_pWrtShell->GetDoc();
            const SwRedlineTable& rRedlineTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
            SwRedlineTable::size_type nRedline = SwRedlineTable::npos;
            if (pArgs && pArgs->GetItemState(nSlot, false, &pItem) == SfxItemState::SET)
            {
                const sal_uInt32 nChangeId = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                for (SwRedlineTable::size_type i = 0; i < rRedlineTable.size(); ++i)
                {
                    if (nChangeId == rRedlineTable[i]->GetId())
                        nRedline = i;
                }
            }

            const SwRangeRedline *pNext = nullptr;
            if (nRedline < rRedlineTable.size())
                pNext = m_pWrtShell->GotoRedline(nRedline, true);
            else
                pNext = m_pWrtShell->SelNextRedline();

            if (pNext)
            {
                if (comphelper::LibreOfficeKit::isActive())
                {
                    OString aPayload(".uno:CurrentTrackedChangeId=");
                    sal_uInt32 nRedlineId = pNext->GetId();
                    aPayload += OString::number(nRedlineId);
                    libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, aPayload.getStr());
                }

                m_pWrtShell->SetInSelect();
            }

        }
        break;

        case FN_REDLINE_PREV_CHANGE:
        {
            const SwRangeRedline *pPrev = m_pWrtShell->SelPrevRedline();

            if (pPrev)
            {
                if (comphelper::LibreOfficeKit::isActive())
                {
                    OString aPayload(".uno:CurrentTrackedChangeId=");
                    sal_uInt32 nRedlineId = pPrev->GetId();
                    aPayload += OString::number(nRedlineId);
                    libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, aPayload.getStr());
                }

                m_pWrtShell->SetInSelect();
            }
        }
        break;

        case SID_DOCUMENT_COMPARE:
        case SID_DOCUMENT_MERGE:
            {
                OUString sFileName, sFilterName;
                sal_Int16 nVersion = 0;
                bool bHasFileName = false;
                m_pViewImpl->SetParam( 0 );
                bool bNoAcceptDialog = false;

                if( pArgs )
                {
                    if( SfxItemState::SET == pArgs->GetItemState( SID_FILE_NAME, false, &pItem ))
                        sFileName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    bHasFileName = !sFileName.isEmpty();

                    if( SfxItemState::SET == pArgs->GetItemState( SID_FILTER_NAME, false, &pItem ))
                        sFilterName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                    if( SfxItemState::SET == pArgs->GetItemState( SID_VERSION, false, &pItem ))
                    {
                        nVersion = static_cast<const SfxInt16Item *>(pItem)->GetValue();
                        m_pViewImpl->SetParam( nVersion );
                    }
                    if( SfxItemState::SET == pArgs->GetItemState( SID_NO_ACCEPT_DIALOG, false, &pItem ))
                    {
                        bNoAcceptDialog = static_cast<const SfxBoolItem *>(pItem)->GetValue();
                    }
                }

                m_pViewImpl->InitRequest( rReq );
                tools::Long nFound = InsertDoc( nSlot, sFileName, sFilterName, nVersion );

                if ( bHasFileName )
                {
                    rReq.SetReturnValue( SfxInt32Item( nSlot, nFound ));

                    if (nFound > 0 && !bNoAcceptDialog) // show Redline browser
                    {
                        SfxViewFrame* pVFrame = GetViewFrame();
                        pVFrame->ShowChildWindow(FN_REDLINE_ACCEPT);

                        // re-initialize the Redline dialog
                        const sal_uInt16 nId = SwRedlineAcceptChild::GetChildWindowId();
                        SwRedlineAcceptChild *pRed = static_cast<SwRedlineAcceptChild*>(
                                                pVFrame->GetChildWindow(nId));
                        if (pRed)
                            pRed->ReInitDlg(GetDocShell());
                    }
                }
                else
                    bIgnore = true;
            }
        break;
        case FN_SYNC_LABELS:
            GetViewFrame()->ShowChildWindow(nSlot);
        break;
        case FN_ESCAPE:
        {
            if ( m_pWrtShell->HasDrawViewDrag() )
            {
                m_pWrtShell->BreakDrag();
                m_pWrtShell->EnterSelFrameMode();
            }
            else if ( m_pWrtShell->IsDrawCreate() )
            {
                GetDrawFuncPtr()->BreakCreate();
                AttrChangedNotify(nullptr); // shell change if needed
            }
            else if ( m_pWrtShell->HasSelection() || IsDrawMode() )
            {
                SdrView *pSdrView = m_pWrtShell->HasDrawView() ? m_pWrtShell->GetDrawView() : nullptr;
                if(pSdrView && pSdrView->AreObjectsMarked() &&
                    pSdrView->GetHdlList().GetFocusHdl())
                {
                    const_cast<SdrHdlList&>(pSdrView->GetHdlList()).ResetFocusHdl();
                }
                else
                {
                    if(pSdrView)
                    {
                        LeaveDrawCreate();
                        Point aPt(LONG_MIN, LONG_MIN);
                        //go out of the frame
                        m_pWrtShell->SelectObj(aPt, SW_LEAVE_FRAME);
                        SfxBindings& rBind = GetViewFrame()->GetBindings();
                        rBind.Invalidate( SID_ATTR_SIZE );
                    }
                    m_pWrtShell->EnterStdMode();
                    AttrChangedNotify(nullptr); // shell change if necessary
                }
            }
            else if ( GetEditWin().GetApplyTemplate() )
            {
                GetEditWin().SetApplyTemplate(SwApplyTemplate());
            }
            else if( static_cast<SfxObjectShell*>(GetDocShell())->IsInPlaceActive() )
            {
                Escape();
            }
            else if ( GetEditWin().IsChainMode() )
            {
                GetEditWin().SetChainMode( false );
            }
            else if( m_pWrtShell->GetFlyFrameFormat() )
            {
                const SwFrameFormat* pFormat = m_pWrtShell->GetFlyFrameFormat();
                if(m_pWrtShell->GotoFly( pFormat->GetName(), FLYCNTTYPE_FRM ))
                {
                    m_pWrtShell->HideCursor();
                    m_pWrtShell->EnterSelFrameMode();
                }
            }
            else
            {
                SfxBoolItem aItem( SID_WIN_FULLSCREEN, false );
                GetViewFrame()->GetDispatcher()->ExecuteList(SID_WIN_FULLSCREEN,
                        SfxCallMode::RECORD, { &aItem });
                bIgnore = true;
            }
        }
        break;
        case SID_ATTR_BORDER_INNER:
        case SID_ATTR_BORDER_OUTER:
        case SID_ATTR_BORDER_SHADOW:
            if(pArgs)
                m_pWrtShell->SetAttrSet(*pArgs);
            break;

        case SID_ATTR_PAGE:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE_MAXSIZE:
        case SID_ATTR_PAGE_PAPERBIN:
        case SID_ATTR_PAGE_EXT1:
        case FN_PARAM_FTN_INFO:
        {
            if(pArgs)
            {
                const size_t nCurIdx = m_pWrtShell->GetCurPageDesc();
                SwPageDesc aPageDesc( m_pWrtShell->GetPageDesc( nCurIdx ) );
                ::ItemSetToPageDesc( *pArgs, aPageDesc );
                // change the descriptor of the core
                m_pWrtShell->ChgPageDesc( nCurIdx, aPageDesc );
            }
        }
        break;
        case FN_GOTO_PAGE:
        {
            SwGotoPageDlg aDlg(GetViewFrame()->GetFrameWeld(), &GetViewFrame()->GetBindings());
            if (aDlg.run() == RET_OK)
                GetWrtShell().GotoPage(aDlg.GetPageSelection(), true);
        }
        break;
        case  FN_EDIT_CURRENT_TOX:
        {
            GetViewFrame()->GetDispatcher()->Execute(
                                FN_INSERT_MULTI_TOX, SfxCallMode::ASYNCHRON);
        }
        break;
        case FN_UPDATE_CUR_TOX:
        {
            const SwTOXBase* pBase = m_pWrtShell->GetCurTOX();
            if(pBase)
            {
                // tdf#106374: don't jump view on the update
                const bool bWasLocked = m_pWrtShell->IsViewLocked();
                m_pWrtShell->LockView(true);
                m_pWrtShell->StartAction();
                if(TOX_INDEX == pBase->GetType())
                    m_pWrtShell->ApplyAutoMark();
                m_pWrtShell->UpdateTableOf( *pBase );
                m_pWrtShell->EndAction();
                if (!bWasLocked)
                    m_pWrtShell->LockView(false);
            }
        }
        break;
        case FN_UPDATE_TOX:
        {
            m_pWrtShell->StartAction();
            m_pWrtShell->EnterStdMode();
            bool bOldCursorInReadOnly = m_pWrtShell->IsReadOnlyAvailable();
            m_pWrtShell->SetReadOnlyAvailable( true );

            for( int i = 0; i < 2; ++i )
            {
                if( m_pWrtShell->GetTOXCount() == 1 )
                    ++i;

                while( m_pWrtShell->GotoPrevTOXBase() )
                    ;   // jump to the first "table of ..."

                // if we are not in one, jump to next
                const SwTOXBase* pBase = m_pWrtShell->GetCurTOX();
                if( !pBase )
                {
                    m_pWrtShell->GotoNextTOXBase();
                    pBase = m_pWrtShell->GetCurTOX();
                }

                bool bAutoMarkApplied = false;
                while( pBase )
                {
                    if(TOX_INDEX == pBase->GetType() && !bAutoMarkApplied)
                    {
                        m_pWrtShell->ApplyAutoMark();
                        bAutoMarkApplied = true;
                    }
                    // pBase is needed only for the interface. Should be changed in future! (JP 1996)
                    m_pWrtShell->UpdateTableOf( *pBase );

                    if( m_pWrtShell->GotoNextTOXBase() )
                        pBase = m_pWrtShell->GetCurTOX();
                    else
                        pBase = nullptr;
                }
            }
            m_pWrtShell->SetReadOnlyAvailable( bOldCursorInReadOnly );
            m_pWrtShell->EndAction();
        }
        break;
        case SID_ATTR_BRUSH:
        {
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(RES_BACKGROUND, false, &pItem))
            {
                const size_t nCurIdx = m_pWrtShell->GetCurPageDesc();
                SwPageDesc aDesc( m_pWrtShell->GetPageDesc( nCurIdx ));
                SwFrameFormat& rMaster = aDesc.GetMaster();
                rMaster.SetFormatAttr(*pItem);
                m_pWrtShell->ChgPageDesc( nCurIdx, aDesc);
            }
        }
        break;
        case SID_CLEARHISTORY:
        {
            m_pWrtShell->DelAllUndoObj();
        }
        break;
        case SID_UNDO:
        {
            m_pShell->ExecuteSlot(rReq);
        }
        break;
#if defined(_WIN32) || defined UNX
        case SID_TWAIN_SELECT:
        case SID_TWAIN_TRANSFER:
            GetViewImpl()->ExecuteScan( rReq );
        break;
#endif

        case SID_ATTR_DEFTABSTOP:
        {
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
            {
                SvxTabStopItem aDefTabs( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
                const sal_uInt16 nTab = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                MakeDefTabs( nTab, aDefTabs );
                m_pWrtShell->SetDefault( aDefTabs );
            }
        }
        break;
        case SID_ATTR_LANGUAGE  :
        if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_ATTR_LANGUAGE, false, &pItem))
        {
            SvxLanguageItem aLang(static_cast<const SvxLanguageItem*>(pItem)->GetLanguage(), RES_CHRATR_LANGUAGE);
            m_pWrtShell->SetDefault( aLang );
            lcl_SetAllTextToDefaultLanguage( *m_pWrtShell, RES_CHRATR_LANGUAGE );
        }
        break;
        case  SID_ATTR_CHAR_CTL_LANGUAGE:
        if(pArgs && SfxItemState::SET == pArgs->GetItemState(RES_CHRATR_CTL_LANGUAGE, false, &pItem))
        {
            m_pWrtShell->SetDefault( *pItem );
            lcl_SetAllTextToDefaultLanguage( *m_pWrtShell, RES_CHRATR_CTL_LANGUAGE );
        }
        break;
        case  SID_ATTR_CHAR_CJK_LANGUAGE:
        if(pArgs && SfxItemState::SET == pArgs->GetItemState(RES_CHRATR_CJK_LANGUAGE, false, &pItem))
        {
            m_pWrtShell->SetDefault( *pItem );
            lcl_SetAllTextToDefaultLanguage( *m_pWrtShell, RES_CHRATR_CJK_LANGUAGE );
        }
        break;
        case FN_TOGGLE_OUTLINE_CONTENT_VISIBILITY:
        {
            m_pWrtShell->EnterStdMode();
            size_t nPos(m_pWrtShell->GetOutlinePos());
            if (nPos != SwOutlineNodes::npos)
            {
                SwNode* pNode = m_pWrtShell->GetNodes().GetOutLineNds()[nPos];
                pNode->GetTextNode()->SetAttrOutlineContentVisible(
                            !m_pWrtShell->GetAttrOutlineContentVisible(nPos));
                m_pWrtShell->InvalidateOutlineContentVisibility();
                m_pWrtShell->GotoOutline(nPos);
            }
        }
        break;
        case FN_NAV_ELEMENT:
        {
            // nothing here on purpose - if removed only the listbox that changed is changed
        }
        break;
        case FN_SCROLL_PREV:
        case FN_SCROLL_NEXT:
        {
            bool *pbNext = new bool(true);
            if (nSlot == FN_SCROLL_PREV)
                *pbNext = false;
            MoveNavigationHdl(pbNext);
        }
        break;
        case SID_JUMPTOMARK:
            if( pArgs && SfxItemState::SET == pArgs->GetItemState(SID_JUMPTOMARK, false, &pItem))
                JumpToSwMark( static_cast<const SfxStringItem*>(pItem)->GetValue() );
        break;
        case SID_GALLERY :
            // First make sure that the sidebar is visible
            GetViewFrame()->ShowChildWindow(SID_SIDEBAR);

            ::sfx2::sidebar::Sidebar::ShowPanel(
                u"GalleryPanel",
                GetViewFrame()->GetFrame().GetFrameInterface());
        break;
        case SID_AVMEDIA_PLAYER :
            GetViewFrame()->ChildWindowExecute(rReq);
        break;
        case SID_VIEW_DATA_SOURCE_BROWSER:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            pVFrame->ChildWindowExecute(rReq);
            if(pVFrame->HasChildWindow(SID_BROWSER))
            {
                const SwDBData& rData = GetWrtShell().GetDBData();
                SwModule::ShowDBObj(*this, rData);
            }
        }
        break;
        case FN_INSERT_FIELD_DATA_ONLY:
        {
            bool bShow = false;
            if( pArgs &&
                SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem ))
                bShow = static_cast<const SfxBoolItem*>(pItem)->GetValue();
            if((bShow && m_bInMailMerge) != GetViewFrame()->HasChildWindow(nSlot))
                GetViewFrame()->ToggleChildWindow(nSlot);
            //if fields have been successfully inserted call the "real"
            //mail merge dialog
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
            SwWrtShell &rSh = GetWrtShell();
            if(m_bInMailMerge && rSh.IsAnyDatabaseFieldInDoc())
            {
                SwDBManager* pDBManager = rSh.GetDBManager();
                if (pDBManager)
                {
                    SwDBData aData = rSh.GetDBData();
                    rSh.EnterStdMode(); // force change in text shell; necessary for mixing DB fields
                    AttrChangedNotify(nullptr);

                    Sequence<PropertyValue> aProperties(3);
                    PropertyValue* pValues = aProperties.getArray();
                    pValues[0].Name = "DataSourceName";
                    pValues[1].Name = "Command";
                    pValues[2].Name = "CommandType";
                    pValues[0].Value <<= aData.sDataSource;
                    pValues[1].Value <<= aData.sCommand;
                    pValues[2].Value <<= aData.nCommandType;
                    pDBManager->ExecuteFormLetter(rSh, aProperties);
                }
            }
#endif
            m_bInMailMerge &= bShow;
            GetViewFrame()->GetBindings().Invalidate(FN_INSERT_FIELD);
        }
        break;
        case FN_QRY_MERGE:
        {
            bool bUseCurrentDocument = true;
            bool bQuery = !pArgs || SfxItemState::SET != pArgs->GetItemState(nSlot);
            if(bQuery)
            {
                SfxViewFrame* pTmpFrame = GetViewFrame();
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractMailMergeCreateFromDlg> pDlg(pFact->CreateMailMergeCreateFromDlg(pTmpFrame->GetFrameWeld()));
                if (RET_OK == pDlg->Execute())
                    bUseCurrentDocument = pDlg->IsThisDocument();
                else
                    break;
            }
            GenerateFormLetter(bUseCurrentDocument);
        }
        break;
        case SID_RECHECK_DOCUMENT:
        {
            SwDocShell* pDocShell = GetDocShell();
            SwDoc* pDoc = pDocShell->GetDoc();
            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( pDoc->GetGCIterator() );
            if( xGCIterator.is() )
            {
                xGCIterator->resetIgnoreRules();
            }
            // reset ignore lists
            pDoc->SpellItAgainSam( true, false, false );
            // clear ignore dictionary
            uno::Reference< linguistic2::XDictionary > xDictionary = LinguMgr::GetIgnoreAllList();
            if( xDictionary.is() )
                xDictionary->clear();
            // put cursor to the start of the document
            m_pWrtShell->StartOfSection();
            [[fallthrough]]; // call spell/grammar dialog
        }
        case FN_SPELL_GRAMMAR_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != nullptr)
                pViewFrame->SetChildWindow (FN_SPELL_GRAMMAR_DIALOG,
                    static_cast<const SfxBoolItem&>( (rReq.GetArgs()->
                        Get(FN_SPELL_GRAMMAR_DIALOG))).GetValue());
            else
                pViewFrame->ToggleChildWindow(FN_SPELL_GRAMMAR_DIALOG);

            pViewFrame->GetBindings().Invalidate(FN_SPELL_GRAMMAR_DIALOG);
            rReq.Ignore ();
        }
        break;
        case SID_ALIGN_ANY_LEFT :
        case SID_ALIGN_ANY_HCENTER  :
        case SID_ALIGN_ANY_RIGHT    :
        case SID_ALIGN_ANY_JUSTIFIED:
        case SID_ALIGN_ANY_TOP      :
        case SID_ALIGN_ANY_VCENTER  :
        case SID_ALIGN_ANY_BOTTOM   :
        case SID_ALIGN_ANY_HDEFAULT :
        case SID_ALIGN_ANY_VDEFAULT :
        {
            sal_uInt16 nAlias = 0;
            if( m_nSelectionType & (SelectionType::DrawObjectEditMode|SelectionType::Text) )
            {
                switch( nSlot )
                {
                    case SID_ALIGN_ANY_LEFT :       nAlias = SID_ATTR_PARA_ADJUST_LEFT; break;
                    case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_ATTR_PARA_ADJUST_CENTER; break;
                    case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_ATTR_PARA_ADJUST_RIGHT; break;
                    case SID_ALIGN_ANY_JUSTIFIED:   nAlias = SID_ATTR_PARA_ADJUST_BLOCK; break;
                    case SID_ALIGN_ANY_TOP      :   nAlias = SID_TABLE_VERT_NONE; break;
                    case SID_ALIGN_ANY_VCENTER  :   nAlias = SID_TABLE_VERT_CENTER; break;
                    case SID_ALIGN_ANY_BOTTOM   :   nAlias = SID_TABLE_VERT_BOTTOM; break;
                }
            }
            else
            {
                switch( nSlot )
                {
                    case SID_ALIGN_ANY_LEFT :       nAlias = SID_OBJECT_ALIGN_LEFT    ; break;
                    case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_OBJECT_ALIGN_CENTER ; break;
                    case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_OBJECT_ALIGN_RIGHT  ; break;
                    case SID_ALIGN_ANY_TOP      :   nAlias = SID_OBJECT_ALIGN_UP     ;  break;
                    case SID_ALIGN_ANY_VCENTER  :   nAlias = SID_OBJECT_ALIGN_MIDDLE ;  break;
                    case SID_ALIGN_ANY_BOTTOM   :   nAlias = SID_OBJECT_ALIGN_DOWN    ; break;
                }
            }
            //these slots are either re-mapped to text or object alignment
            if (nAlias)
                GetViewFrame()->GetDispatcher()->Execute(
                                nAlias, SfxCallMode::ASYNCHRON);
        }
        break;
        case SID_RESTORE_EDITING_VIEW:
        {
            //#i33307# restore editing position
            Point aCursorPos;
            bool bSelectObj;
            if(m_pViewImpl->GetRestorePosition(aCursorPos, bSelectObj))
            {
                m_pWrtShell->SwCursorShell::SetCursor( aCursorPos, !bSelectObj );
                if( bSelectObj )
                {
                    m_pWrtShell->SelectObj( aCursorPos );
                    m_pWrtShell->EnterSelFrameMode( &aCursorPos );
                }
            }
        }
        break;
        case SID_INSERT_GRAPHIC:
        {
            rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg( rReq )));
        }
        break;
        case SID_MOVE_SHAPE_HANDLE:
        {
            if (pArgs && pArgs->Count() >= 3)
            {
                SdrView *pSdrView = m_pWrtShell->HasDrawView() ? m_pWrtShell->GetDrawView() : nullptr;
                if (pSdrView == nullptr)
                    break;
                const SfxUInt32Item* handleNumItem = rReq.GetArg<SfxUInt32Item>(FN_PARAM_1);
                const SfxUInt32Item* newPosXTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_2);
                const SfxUInt32Item* newPosYTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_3);
                const SfxInt32Item* OrdNum = rReq.GetArg<SfxInt32Item>(FN_PARAM_4);

                const sal_uLong handleNum = handleNumItem->GetValue();
                const sal_uLong newPosX = newPosXTwips->GetValue();
                const sal_uLong newPosY = newPosYTwips->GetValue();
                const Point mPoint(newPosX, newPosY);
                const SdrHdl* handle = pSdrView->GetHdlList().GetHdl(handleNum);
                if (!handle)
                {
                    break;
                }

                if (handle->GetKind() == SdrHdlKind::Anchor || handle->GetKind() == SdrHdlKind::Anchor_TR)
                    m_pWrtShell->FindAnchorPos(mPoint, /*bMoveIt=*/true);
                else
                    pSdrView->MoveShapeHandle(handleNum, mPoint, OrdNum ? OrdNum->GetValue() : -1);
            }
            break;
        }

        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
    if(!bIgnore)
        rReq.Done();
}

bool SwView::IsConditionalFastCall( const SfxRequest &rReq )
{
    sal_uInt16 nId = rReq.GetSlot();
    bool bRet = false;

    if (nId == FN_REDLINE_ACCEPT_DIRECT || nId == FN_REDLINE_REJECT_DIRECT)
    {
        if (comphelper::LibreOfficeKit::isActive())
            bRet = true;
    }
    return bRet || SfxShell::IsConditionalFastCall(rReq);

}

/// invalidate page numbering field
void SwView::UpdatePageNums()
{
    SfxBindings &rBnd = GetViewFrame()->GetBindings();
    rBnd.Invalidate(FN_STAT_PAGE);
}

void SwView::UpdateDocStats()
{
    SfxBindings &rBnd = GetViewFrame()->GetBindings();
    rBnd.Invalidate( FN_STAT_WORDCOUNT );
    rBnd.Update( FN_STAT_WORDCOUNT );
}

/// get status of the status line
void SwView::StateStatusLine(SfxItemSet &rSet)
{
    SwWrtShell& rShell = GetWrtShell();

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    OSL_ENSURE( nWhich, "empty set");

    //get section change event
    const SwSection* CurrSect = rShell.GetCurrSection();
    if( CurrSect )
    {
        const OUString& sCurrentSectionName = CurrSect->GetSectionName();
        if(sCurrentSectionName != m_sOldSectionName)
        {
            SwCursorShell::FireSectionChangeEvent(2, 1);
        }
        m_sOldSectionName = sCurrentSectionName;
    }
    else if (!m_sOldSectionName.isEmpty())
    {
        SwCursorShell::FireSectionChangeEvent(2, 1);
        m_sOldSectionName= OUString();
    }
    //get column change event
    if(rShell.bColumnChange())
    {
        SwCursorShell::FireColumnChangeEvent(2, 1);
    }

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_STAT_PAGE: {
                // number of pages, log. page number
                sal_uInt16 nPage, nLogPage;
                OUString sDisplay;
                rShell.GetPageNumber( -1, rShell.IsCursorVisible(), nPage, nLogPage, sDisplay );
                bool bExtendedTooltip(!sDisplay.isEmpty() &&
                                      std::u16string_view(OUString::number(nPage)) != sDisplay &&
                                      nPage != nLogPage);
                OUString aTooltip = bExtendedTooltip ? SwResId(STR_BOOKCTRL_HINT_EXTENDED)
                                                     : SwResId(STR_BOOKCTRL_HINT);
                std::vector<OUString> aStringList
                {
                    GetPageStr(nPage, nLogPage, sDisplay),
                    aTooltip
                };
                rSet.Put(SfxStringListItem(FN_STAT_PAGE, &aStringList));
                //if existing page number is not equal to old page number, send out this event.
                if (m_nOldPageNum != nLogPage )
                {
                    if (m_nOldPageNum != 0)
                        SwCursorShell::FirePageChangeEvent(m_nOldPageNum, nLogPage);
                    m_nOldPageNum = nLogPage;
                }
                const sal_uInt16 nCnt = GetWrtShell().GetPageCnt();
                if (m_nPageCnt != nCnt)   // notify Basic
                {
                    m_nPageCnt = nCnt;
                    SfxGetpApp()->NotifyEvent(SfxEventHint(SfxEventHintId::SwEventPageCount, SwDocShell::GetEventName(STR_SW_EVENT_PAGE_COUNT), GetViewFrame()->GetObjectShell()), false);
                }
            }
            break;

            case FN_STAT_WORDCOUNT:
            {
                SwDocStat selectionStats;
                SwDocStat documentStats;
                rShell.CountWords(selectionStats);
                documentStats = rShell.GetDoc()->getIDocumentStatistics().GetUpdatedDocStat( true /* complete-async */, false /* don't update fields */ );

                sal_uLong nWord = selectionStats.nWord ? selectionStats.nWord : documentStats.nWord;
                sal_uLong nChar = selectionStats.nChar ? selectionStats.nChar : documentStats.nChar;
                TranslateId pResId = selectionStats.nWord ? STR_WORDCOUNT : STR_WORDCOUNT_NO_SELECTION;
                TranslateNId pWordResId = selectionStats.nWord ? STR_WORDCOUNT_WORDARG : STR_WORDCOUNT_WORDARG_NO_SELECTION;
                TranslateNId pCharResId = selectionStats.nWord ? STR_WORDCOUNT_CHARARG : STR_WORDCOUNT_CHARARG_NO_SELECTION;

                const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetUILocaleDataWrapper();
                OUString aWordArg = SwResId(pWordResId, nWord).replaceAll("$1", rLocaleData.getNum(nWord, 0));
                OUString aCharArg = SwResId(pCharResId, nChar).replaceAll("$1", rLocaleData.getNum(nChar, 0));
                OUString aWordCount(SwResId(pResId));
                aWordCount = aWordCount.replaceAll("$1", aWordArg);
                aWordCount = aWordCount.replaceAll("$2", aCharArg);

                rSet.Put( SfxStringItem( FN_STAT_WORDCOUNT, aWordCount ) );

                SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
                if (pWrdCnt)
                    pWrdCnt->SetCounts(selectionStats, documentStats);
            }
            break;

            case FN_STAT_TEMPLATE:
            {
                rSet.Put(SfxStringItem( FN_STAT_TEMPLATE,
                                        rShell.GetCurPageStyle()));

            }
            break;
            case SID_ATTR_ZOOM:
            {
                if ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    SvxZoomType eZoom = pVOpt->GetZoomType();
                    SvxZoomItem aZoom(eZoom,
                                        pVOpt->GetZoom());
                    if( pVOpt->getBrowseMode() )
                    {
                        aZoom.SetValueSet(
                                SvxZoomEnableFlags::N50|
                                SvxZoomEnableFlags::N75|
                                SvxZoomEnableFlags::N100|
                                SvxZoomEnableFlags::N150|
                                SvxZoomEnableFlags::N200);
                    }
                    rSet.Put( aZoom );
                }
                else
                    rSet.DisableItem( SID_ATTR_ZOOM );
            }
            break;
            case SID_ATTR_VIEWLAYOUT:
            {
                if ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    const sal_uInt16 nColumns  = pVOpt->GetViewLayoutColumns();
                    const bool  bBookMode = pVOpt->IsViewLayoutBookMode();
                    SvxViewLayoutItem aViewLayout(nColumns, bBookMode);
                    rSet.Put( aViewLayout );
                }
                else
                    rSet.DisableItem( SID_ATTR_VIEWLAYOUT );
            }
            break;
            case SID_ATTR_ZOOMSLIDER:
            {
                if ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    const sal_uInt16 nCurrentZoom = pVOpt->GetZoom();
                    SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM );
                    aZoomSliderItem.AddSnappingPoint( 100 );

                    if ( !m_pWrtShell->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE) )
                    {
                        const sal_uInt16 nColumns = pVOpt->GetViewLayoutColumns();
                        const bool bAutomaticViewLayout = 0 == nColumns;
                        const SwPostItMgr* pMgr = GetPostItMgr();

                        // snapping points:
                        // automatic mode: 1 Page, 2 Pages, 100%
                        // n Columns mode: n Pages, 100%
                        // n Columns book mode: nPages without gaps, 100%
                        const SwRect aPageRect( m_pWrtShell->GetAnyCurRect( CurRectType::PageCalc ) );
                        const SwRect aRootRect( m_pWrtShell->GetAnyCurRect( CurRectType::PagesArea ) ); // width of columns
                        Size aPageSize( aPageRect.SSize() );
                        aPageSize.AdjustWidth(pMgr->HasNotes() && pMgr->ShowNotes() ?
                                             pMgr->GetSidebarWidth() + pMgr->GetSidebarBorderWidth() :
                                             0 );

                        Size aRootSize( aRootRect.SSize() );

                        const MapMode aTmpMap( MapUnit::MapTwip );
                        const Size& rEditSize = GetEditWin().GetOutputSizePixel();
                        const Size aWindowSize( GetEditWin().PixelToLogic( rEditSize, aTmpMap ) );

                        const tools::Long nOf = pVOpt->GetDocumentBorder() * 2;
                        tools::Long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
                        nTmpWidth += nOf;
                        aPageSize.AdjustHeight(nOf );
                        tools::Long nFac = aWindowSize.Width() * 100 / nTmpWidth;

                        tools::Long nVisPercent = aWindowSize.Height() * 100 / aPageSize.Height();
                        nFac = std::min( nFac, nVisPercent );

                        if (nFac >= MINZOOM)
                        {
                            aZoomSliderItem.AddSnappingPoint( nFac );
                        }

                        if ( bAutomaticViewLayout )
                        {
                            nTmpWidth += aPageSize.Width() + pVOpt->GetGapBetweenPages();
                            nFac = aWindowSize.Width() * 100 / nTmpWidth;
                            nFac = std::min( nFac, nVisPercent );
                            if (nFac >= MINZOOM)
                            {
                                aZoomSliderItem.AddSnappingPoint( nFac );
                            }
                        }
                    }

                    rSet.Put( aZoomSliderItem );
                }
                else
                    rSet.DisableItem( SID_ATTR_ZOOMSLIDER );
            }
            break;
            case SID_ATTR_POSITION:
            case SID_ATTR_SIZE:
            {
                if( !rShell.IsFrameSelected() && !rShell.IsObjSelected() )
                    SwBaseShell::SetFrameMode_( FLY_DRAG_END );
                else
                {
                    FlyMode eFrameMode = SwBaseShell::GetFrameMode();
                    if ( eFrameMode == FLY_DRAG_START || eFrameMode == FLY_DRAG )
                    {
                        if ( nWhich == SID_ATTR_POSITION )
                            rSet.Put( SfxPointItem( SID_ATTR_POSITION,
                                                    rShell.GetAnchorObjDiff()));
                        else
                            rSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                                                   rShell.GetObjSize()));
                    }
                }
            }
            break;
            case SID_TABLE_CELL:

            if( rShell.IsFrameSelected() || rShell.IsObjSelected() )
            {
                // #i39171# Don't put a SvxSizeItem into a slot which is defined as SfxStringItem.
                // SvxPosSizeStatusBarControl no longer resets to empty display if only one slot
                // has no item, so SID_TABLE_CELL can remain empty (the SvxSizeItem is supplied
                // in SID_ATTR_SIZE).
            }
            else
            {
                OUString sStr;
                if( rShell.IsCursorInTable() )
                {
                    // table name + cell coordinate
                    sStr = rShell.GetTableFormat()->GetName() + ":";
                    sStr += rShell.GetBoxNms();
                }
                else
                {
                    const SwSection* pCurrSect = rShell.GetCurrSection();
                    if( pCurrSect )
                    {
                        switch( pCurrSect->GetType() )
                        {
                        case SectionType::ToxHeader:
                        case SectionType::ToxContent:
                            {
                                const SwTOXBase* pTOX = m_pWrtShell->GetCurTOX();
                                if( pTOX )
                                    sStr = pTOX->GetTOXName();
                                else
                                {
                                    OSL_ENSURE( false,
                                        "Unknown kind of section" );
                                    sStr = pCurrSect->GetSectionName();
                                }
                            }
                            break;
                        default:
                            sStr = pCurrSect->GetSectionName();
                            break;
                        }
                    }
                }

                const SwNumRule* pNumRule = rShell.GetNumRuleAtCurrCursorPos();
                const bool bOutlineNum = pNumRule && pNumRule->IsOutlineRule();

                if (pNumRule && !bOutlineNum )  // cursor in numbering
                {
                    sal_uInt8 nNumLevel = rShell.GetNumLevel();
                    if ( nNumLevel < MAXLEVEL )
                    {
                        if(!pNumRule->IsAutoRule())
                        {
                            SfxItemSetFixed<RES_PARATR_NUMRULE, RES_PARATR_NUMRULE> aSet(GetPool());
                            rShell.GetCurAttr(aSet);
                            if(SfxItemState::DEFAULT <=
                               aSet.GetItemState(RES_PARATR_NUMRULE))
                            {
                                const OUString& rNumStyle =
                                    aSet.Get(RES_PARATR_NUMRULE).GetValue();
                                if(!rNumStyle.isEmpty())
                                {
                                    if(!sStr.isEmpty())
                                        sStr += sStatusDelim;
                                    sStr += rNumStyle;
                                }
                            }
                        }
                        if (!sStr.isEmpty())
                            sStr += sStatusDelim;
                        sStr += SwResId(STR_NUM_LEVEL) + OUString::number( nNumLevel + 1 );

                    }
                }
                const int nOutlineLevel = rShell.GetCurrentParaOutlineLevel();
                if( nOutlineLevel != 0 )
                {
                    if (!sStr.isEmpty())
                        sStr += " , ";
                    if( bOutlineNum )
                    {
                        sStr += SwResId(STR_OUTLINE_NUMBERING) +
                            sStatusDelim + SwResId(STR_NUM_LEVEL);
                    }
                    else
                        sStr += SwResId(STR_NUM_OUTLINE);
                    sStr += OUString::number( nOutlineLevel);
                }

                if( rShell.HasReadonlySel() )
                {
                    if (!sStr.isEmpty())
                        sStr = sStatusDelim + sStr;
                    sStr = SwResId(SW_STR_READONLY) + sStr;
                }
                if (!sStr.isEmpty())
                    rSet.Put( SfxStringItem( SID_TABLE_CELL, sStr ));
            }
            break;
            case FN_STAT_SELMODE:
            {
                if(rShell.IsStdMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 0));
                else if(rShell.IsAddMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 2));
                else if(rShell.IsBlockMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 3));
                else
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 1));
                break;
            }
            case SID_ATTR_INSERT:
                if( rShell.IsRedlineOn() )
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxBoolItem(SID_ATTR_INSERT,rShell.IsInsMode()));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/** execute method for the status line
 *
 * @param rReq ???
 */
void SwView::ExecuteStatusLine(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetWrtShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem=nullptr;
    bool bUp = false;
    sal_uInt16 nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_STAT_PAGE:
        {
            GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_PAGE,
                                      SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
        }
        break;

        case FN_STAT_WORDCOUNT:
        {
            GetViewFrame()->GetDispatcher()->Execute(FN_WORDCOUNT_DIALOG,
                                      SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
        }
        break;

        case FN_STAT_BOOKMARK:
        if ( pArgs )
        {
            if (SfxItemState::SET == pArgs->GetItemState( nWhich, true, &pItem))
            {
                const IDocumentMarkAccess* pMarkAccess = rSh.getIDocumentMarkAccess();
                const sal_Int32 nIdx = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                if(nIdx < pMarkAccess->getBookmarksCount())
                {
                    const IDocumentMarkAccess::const_iterator_t ppBookmark = rSh.getIDocumentMarkAccess()->getBookmarksBegin() + nIdx;
                    rSh.EnterStdMode();
                    rSh.GotoMark( *ppBookmark );
                }
                else
                    OSL_FAIL("SwView::ExecuteStatusLine(..)"
                        " - Ignoring out of range bookmark index");
            }
        }
        break;

        case FN_STAT_TEMPLATE:
        {
            GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_PAGE_DLG,
                                        SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
        }
        break;
        case SID_ATTR_ZOOM:
        {
            if ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() )
            {
                const SfxItemSet *pSet = nullptr;
                ScopedVclPtr<AbstractSvxZoomDialog> pDlg;
                if ( pArgs )
                    pSet = pArgs;
                else
                {
                    const SwViewOption& rViewOptions = *rSh.GetViewOptions();
                    SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM, SID_ATTR_VIEWLAYOUT, SID_ATTR_VIEWLAYOUT> aCoreSet(m_pShell->GetPool());
                    SvxZoomItem aZoom( rViewOptions.GetZoomType(), rViewOptions.GetZoom() );

                    const bool bBrowseMode = rSh.GetViewOptions()->getBrowseMode();
                    if( bBrowseMode )
                    {
                        aZoom.SetValueSet(
                                SvxZoomEnableFlags::N50|
                                SvxZoomEnableFlags::N75|
                                SvxZoomEnableFlags::N100|
                                SvxZoomEnableFlags::N150|
                                SvxZoomEnableFlags::N200);
                    }
                    aCoreSet.Put( aZoom );

                    if ( !bBrowseMode )
                    {
                        const SvxViewLayoutItem aViewLayout( rViewOptions.GetViewLayoutColumns(), rViewOptions.IsViewLayoutBookMode() );
                        aCoreSet.Put( aViewLayout );
                    }

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    pDlg.disposeAndReset(pFact->CreateSvxZoomDialog(GetViewFrame()->GetFrameWeld(), aCoreSet));
                    pDlg->SetLimits( MINZOOM, MAXZOOM );
                    if( pDlg->Execute() != RET_CANCEL )
                        pSet = pDlg->GetOutputItemSet();
                }

                const SfxPoolItem* pViewLayoutItem = nullptr;
                if ( pSet && SfxItemState::SET == pSet->GetItemState(SID_ATTR_VIEWLAYOUT, true, &pViewLayoutItem))
                {
                    const sal_uInt16 nColumns = static_cast<const SvxViewLayoutItem *>(pViewLayoutItem)->GetValue();
                    const bool bBookMode  = static_cast<const SvxViewLayoutItem *>(pViewLayoutItem)->IsBookMode();
                    SetViewLayout( nColumns, bBookMode );
                }

                if ( pSet && SfxItemState::SET == pSet->GetItemState(SID_ATTR_ZOOM, true, &pItem))
                {
                    SvxZoomType eType = static_cast<const SvxZoomItem *>(pItem)->GetType();
                    SetZoom( eType, static_cast<const SvxZoomItem *>(pItem)->GetValue() );
                }
                bUp = true;
                if ( pItem )
                    rReq.AppendItem( *pItem );
                rReq.Done();
            }
        }
        break;

        case SID_ATTR_VIEWLAYOUT:
        {
            if ( pArgs && !rSh.getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE) &&
                ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() ) )
            {
                if ( SfxItemState::SET == pArgs->GetItemState(SID_ATTR_VIEWLAYOUT, true, &pItem ))
                {
                    const sal_uInt16 nColumns = static_cast<const SvxViewLayoutItem *>(pItem)->GetValue();
                    const bool bBookMode  = (0 != nColumns && 0 == (nColumns % 2)) &&
                                            static_cast<const SvxViewLayoutItem *>(pItem)->IsBookMode();

                    SetViewLayout( nColumns, bBookMode );
                }

                bUp = true;
                rReq.Done();

                InvalidateRulerPos();
            }
        }
        break;

        case SID_ATTR_ZOOMSLIDER:
        {
            if ( pArgs && ( ( GetDocShell()->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) || !GetDocShell()->IsInPlaceActive() ) )
            {
                if ( SfxItemState::SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, true, &pItem ))
                {
                    const sal_uInt16 nCurrentZoom = static_cast<const SvxZoomSliderItem *>(pItem)->GetValue();
                    SetZoom( SvxZoomType::PERCENT, nCurrentZoom );
                }

                bUp = true;
                rReq.Done();
            }
        }
        break;

        case SID_ATTR_SIZE:
        {
            sal_uInt16 nId = 0;
            if( rSh.IsCursorInTable() )
                nId = FN_FORMAT_TABLE_DLG;
            else if( rSh.GetCurTOX() )
                nId = FN_INSERT_MULTI_TOX;
            else if( rSh.GetCurrSection() )
                nId = FN_EDIT_REGION;
            else
            {
                const SwNumRule* pNumRule = rSh.GetNumRuleAtCurrCursorPos();
                if( pNumRule )  // cursor in numbering
                {
                    if( pNumRule->IsAutoRule() )
                        nId = FN_NUMBER_BULLETS;
                    else
                    {
                        // start dialog of the painter
                        nId = 0;
                    }
                }
                else if( rSh.IsFrameSelected() )
                    nId = FN_FORMAT_FRAME_DLG;
                else if( rSh.IsObjSelected() )
                    nId = SID_ATTR_TRANSFORM;
            }
            if( nId )
                GetViewFrame()->GetDispatcher()->Execute(nId,
                    SfxCallMode::SYNCHRON | SfxCallMode::RECORD );
        }
        break;

        case FN_STAT_SELMODE:
        {
            if ( pArgs )
            {
                if (SfxItemState::SET == pArgs->GetItemState( nWhich, true, &pItem))
                {
                    switch ( static_cast<const SfxUInt16Item *>(pItem)->GetValue() )
                    {
                        case 0: rSh.EnterStdMode(); break;
                        case 1: rSh.EnterExtMode(); break;
                        case 2: rSh.EnterAddMode(); break;
                        case 3: rSh.EnterBlockMode(); break;
                    }
                }
            }
            bUp = true;
            break;
        }
        case FN_SET_ADD_MODE:
            rSh.ToggleAddMode();
            nWhich = FN_STAT_SELMODE;
            bUp = true;
        break;
        case FN_SET_BLOCK_MODE:
            rSh.ToggleBlockMode();
            nWhich = FN_STAT_SELMODE;
            bUp = true;
        break;
        case FN_SET_EXT_MODE:
            rSh.ToggleExtMode();
            nWhich = FN_STAT_SELMODE;
            bUp = true;
        break;
        case SID_ATTR_INSERT:
            SwPostItMgr* pMgr = GetPostItMgr();
            if ( pMgr && pMgr->HasActiveSidebarWin() )
            {
                pMgr->ToggleInsModeOnActiveSidebarWin();
            }
            else
                rSh.ToggleInsMode();
            bUp = true;
        break;

    }
    if ( bUp )
    {
        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(nWhich);
        rBnd.Update(nWhich);
    }
}

void SwView::InsFrameMode(sal_uInt16 nCols)
{
    if ( m_pWrtShell->HasWholeTabSelection() )
    {
        SwFlyFrameAttrMgr aMgr( true, m_pWrtShell.get(), Frmmgr_Type::TEXT, nullptr );

        const SwFrameFormat &rPageFormat =
                m_pWrtShell->GetPageDesc(m_pWrtShell->GetCurPageDesc()).GetMaster();
        SwTwips lWidth = rPageFormat.GetFrameSize().GetWidth();
        const SvxLRSpaceItem &rLR = rPageFormat.GetLRSpace();
        lWidth -= rLR.GetLeft() + rLR.GetRight();
        aMgr.SetSize(Size(lWidth, aMgr.GetSize().Height()));
        if(nCols > 1)
        {
            SwFormatCol aCol;
            aCol.Init( nCols, aCol.GetGutterWidth(), aCol.GetWishWidth() );
            aMgr.SetCol( aCol );
        }
        aMgr.InsertFlyFrame();
    }
    else
        GetEditWin().InsFrame(nCols);
}

/// show "edit link" dialog
void SwView::EditLinkDlg()
{
    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(GetViewFrame()->GetFrameWeld(), &GetWrtShell().GetLinkManager(), bWeb));
    pDlg->Execute();
}

namespace sw {

auto PrepareJumpToTOXMark(SwDoc const& rDoc, OUString const& rName)
    -> std::optional<std::pair<SwTOXMark, sal_Int32>>
{
    sal_Int32 const first(rName.indexOf(toxMarkSeparator));
    if (first == -1)
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: missing separator");
        return std::optional<std::pair<SwTOXMark, sal_Int32>>();
    }
    sal_Int32 const counter(rName.copy(0, first).toInt32());
    if (counter <= 0)
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: invalid counter");
        return std::optional<std::pair<SwTOXMark, sal_Int32>>();
    }
    sal_Int32 const second(rName.indexOf(toxMarkSeparator, first + 1));
    if (second == -1)
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: missing separator");
        return std::optional<std::pair<SwTOXMark, sal_Int32>>();
    }
    OUString const entry(rName.copy(first + 1, second - (first + 1)));
    if (rName.getLength() < second + 2)
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: invalid tox");
        return std::optional<std::pair<SwTOXMark, sal_Int32>>();
    }
    sal_uInt16 const indexType(rName[second + 1]);
    OUString const indexName(rName.copy(second + 2));
    SwTOXType const* pType(nullptr);
    switch (indexType)
    {
        case 'A':
            pType = rDoc.GetTOXType(TOX_INDEX, 0);
            assert(pType);
            break;
        case 'C':
            pType = rDoc.GetTOXType(TOX_CONTENT, 0);
            assert(pType);
            break;
        case 'U':
            for (auto i = rDoc.GetTOXTypeCount(TOX_USER); 0 < i; )
            {
                --i;
                auto const pTmp(rDoc.GetTOXType(TOX_USER, i));
                if (pTmp->GetTypeName() == indexName)
                {
                    pType = pTmp;
                    break;
                }
            }
            break;
    }
    if (!pType)
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: tox doesn't exist");
        return std::optional<std::pair<SwTOXMark, sal_Int32>>();
    }
    // type and alt text are the search keys
    SwTOXMark tmp(pType);
    tmp.SetAlternativeText(entry);
    return std::optional<std::pair<SwTOXMark, sal_Int32>>(std::pair<SwTOXMark, sal_Int32>(tmp, counter));
}

} // namespace sw

static auto JumpToTOXMark(SwWrtShell & rSh, OUString const& rName) -> bool
{
    std::optional<std::pair<SwTOXMark, sal_Int32>> const tmp(
        sw::PrepareJumpToTOXMark(*rSh.GetDoc(), rName));
    if (!tmp)
    {
        return false;
    }
    SwTOXMark const* pMark(&tmp->first);
    // hack: check first if one exists
    if (&tmp->first != &rSh.GetDoc()->GotoTOXMark(tmp->first, TOX_SAME_NXT, rSh.IsReadOnlyAvailable()))
    {
        for (sal_Int32 i = 0; i < tmp->second; ++i)
        {
            pMark = &rSh.GotoTOXMark(*pMark, TOX_SAME_NXT);
        }
        return true;
    }
    else
    {
        SAL_WARN("sw.ui", "JumpToTOXMark: tox mark doesn't exist");
        return false;
    }
}

bool SwView::JumpToSwMark( std::u16string_view rMark )
{
    bool bRet = false;
    if( !rMark.empty() )
    {
        // place bookmark at top-center
        bool bSaveCC = m_bCenterCursor;
        bool bSaveCT = m_bTopCursor;
        SetCursorAtTop( true );

        // For scrolling the FrameSet, the corresponding shell needs to have the focus.
        bool bHasShFocus = m_pWrtShell->HasShellFocus();
        if( !bHasShFocus )
            m_pWrtShell->ShellGetFocus();

        const SwFormatINetFormat* pINet;
        OUString sCmp;
        OUString  sMark( INetURLObject::decode( rMark,
                                           INetURLObject::DecodeMechanism::WithCharset ));

        sal_Int32 nLastPos, nPos = sMark.indexOf( cMarkSeparator );
        if( -1 != nPos )
            while( -1 != ( nLastPos = sMark.indexOf( cMarkSeparator, nPos + 1 )) )
                nPos = nLastPos;

        IDocumentMarkAccess::const_iterator_t ppMark;
        IDocumentMarkAccess* const pMarkAccess = m_pWrtShell->getIDocumentMarkAccess();
        if( -1 != nPos )
            sCmp = sMark.copy(nPos + 1).replaceAll(" ", "");

        if( !sCmp.isEmpty() )
        {
            OUString sName( sMark.copy( 0, nPos ) );
            sCmp = sCmp.toAsciiLowerCase();
            FlyCntType eFlyType = FLYCNTTYPE_ALL;

            if (sCmp == "drawingobject")
                bRet = m_pWrtShell->GotoDrawingObject(sName);
            else if( sCmp == "region" )
            {
                m_pWrtShell->EnterStdMode();
                bRet = m_pWrtShell->GotoRegion( sName );
            }
            else if( sCmp == "outline" )
            {
                m_pWrtShell->EnterStdMode();
                bRet = m_pWrtShell->GotoOutline( sName );
            }
            else if( sCmp == "frame" )
                eFlyType = FLYCNTTYPE_FRM;
            else if( sCmp == "graphic" )
                eFlyType = FLYCNTTYPE_GRF;
            else if( sCmp == "ole" )
                eFlyType = FLYCNTTYPE_OLE;
            else if( sCmp == "table" )
            {
                m_pWrtShell->EnterStdMode();
                bRet = m_pWrtShell->GotoTable( sName );
            }
            else if( sCmp == "sequence" )
            {
                m_pWrtShell->EnterStdMode();
                sal_Int32 nNoPos = sName.indexOf( cSequenceMarkSeparator );
                if ( nNoPos != -1 )
                {
                    sal_uInt16 nSeqNo = sName.copy( nNoPos + 1 ).toInt32();
                    sName = sName.copy( 0, nNoPos );
                    bRet = m_pWrtShell->GotoRefMark(sName, REF_SEQUENCEFLD, nSeqNo);
                }
            }
            else if (sCmp == "toxmark")
            {
                bRet = JumpToTOXMark(*m_pWrtShell, sName);
            }
            else if( sCmp == "text" )
            {
                // normal text search
                m_pWrtShell->EnterStdMode();

                i18nutil::SearchOptions2 aSearchOpt(
                                    SearchAlgorithms_ABSOLUTE, 0,
                                    sName, OUString(),
                                    SvtSysLocale().GetLanguageTag().getLocale(),
                                    0,0,0,
                                    TransliterationFlags::IGNORE_CASE,
                                    SearchAlgorithms2::ABSOLUTE,
                                    '\\' );

                //todo/mba: assuming that notes shouldn't be searched
                if( m_pWrtShell->SearchPattern( aSearchOpt, false/*bSearchInNotes*/, SwDocPositions::Start, SwDocPositions::End ))
                {
                    m_pWrtShell->EnterStdMode(); // remove the selection
                    bRet = true;
                }
            }
            else if( pMarkAccess->getAllMarksEnd() != (ppMark = pMarkAccess->findMark(sMark)) )
            {
                bRet = m_pWrtShell->GotoMark( *ppMark, false );
            }
            else if( nullptr != ( pINet = m_pWrtShell->FindINetAttr( sMark ) )) {
                m_pWrtShell->addCurrentPosition();
                bRet = m_pWrtShell->GotoINetAttr( *pINet->GetTextINetFormat() );
            }

            // for all types of Flys
            if( FLYCNTTYPE_ALL != eFlyType && m_pWrtShell->GotoFly( sName, eFlyType ))
            {
                bRet = true;
                if( FLYCNTTYPE_FRM == eFlyType )
                {
                    // TextFrames: set Cursor in the frame
                    m_pWrtShell->UnSelectFrame();
                    m_pWrtShell->LeaveSelFrameMode();
                }
                else
                {
                    m_pWrtShell->HideCursor();
                    m_pWrtShell->EnterSelFrameMode();
                }
            }
        }
        else if( pMarkAccess->getAllMarksEnd() != (ppMark = pMarkAccess->findMark(sMark)))
        {
            bRet = m_pWrtShell->GotoMark( *ppMark, false );
        }
        else if( nullptr != ( pINet = m_pWrtShell->FindINetAttr( sMark ) ))
            bRet = m_pWrtShell->GotoINetAttr( *pINet->GetTextINetFormat() );

        // make selection visible later
        if ( m_aVisArea.IsEmpty() )
            m_bMakeSelectionVisible = true;

        // reset ViewStatus
        SetCursorAtTop( bSaveCT, bSaveCC );

        if(!m_pWrtShell->IsFrameSelected() && !m_pWrtShell->IsObjSelected())
            m_pWrtShell->ShowCursor();

        if( !bHasShFocus )
            m_pWrtShell->ShellLoseFocus();
    }
    return bRet;
}

// #i67305# Undo after insert from file:
// Undo "Insert form file" crashes with documents imported from binary filter (.sdw) => disabled
// Undo "Insert form file" crashes with (.odt) documents crashes if these documents contains
// page styles with active header/footer => disabled for those documents
static size_t lcl_PageDescWithHeader( const SwDoc& rDoc )
{
    size_t nRet = 0;
    size_t nCnt = rDoc.GetPageDescCnt();
    for( size_t i = 0; i < nCnt; ++i )
    {
        const SwPageDesc& rPageDesc = rDoc.GetPageDesc( i );
        const SwFrameFormat& rMaster = rPageDesc.GetMaster();
        const SfxPoolItem* pItem;
        if( ( SfxItemState::SET == rMaster.GetAttrSet().GetItemState( RES_HEADER, false, &pItem ) &&
              static_cast<const SwFormatHeader*>(pItem)->IsActive() ) ||
            ( SfxItemState::SET == rMaster.GetAttrSet().GetItemState( RES_FOOTER, false, &pItem )  &&
              static_cast<const SwFormatFooter*>(pItem)->IsActive()) )
            ++nRet;
    }
    return nRet; // number of page styles with active header/footer
}

void SwView::ExecuteInsertDoc( SfxRequest& rRequest, const SfxPoolItem* pItem )
{
    m_pViewImpl->InitRequest( rRequest );
    m_pViewImpl->SetParam( pItem ? 1 : 0 );
    const sal_uInt16 nSlot = rRequest.GetSlot();

    if ( !pItem )
    {
        InsertDoc( nSlot, "", "" );
    }
    else
    {
        OUString sFile, sFilter;
        sFile = static_cast<const SfxStringItem *>( pItem )->GetValue();
        if ( SfxItemState::SET == rRequest.GetArgs()->GetItemState( FN_PARAM_1, true, &pItem ) )
            sFilter = static_cast<const SfxStringItem *>(pItem )->GetValue();

        bool bHasFileName = !sFile.isEmpty();
        tools::Long nFound = InsertDoc( nSlot, sFile, sFilter );

        if ( bHasFileName )
        {
            rRequest.SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
            rRequest.Done();
        }
    }
}

tools::Long SwView::InsertDoc( sal_uInt16 nSlotId, const OUString& rFileName, const OUString& rFilterName, sal_Int16 nVersion )
{
    std::unique_ptr<SfxMedium> pMed;
    SwDocShell* pDocSh = GetDocShell();

    if( !rFileName.isEmpty() )
    {
        SfxObjectFactory& rFact = pDocSh->GetFactory();
        std::shared_ptr<const SfxFilter> pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( rFilterName );
        if ( !pFilter )
        {
            pMed.reset(new SfxMedium(rFileName, StreamMode::READ, nullptr, nullptr ));
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
            pMed->UseInteractionHandler( true );
            ErrCode nErr = aMatcher.GuessFilter(*pMed, pFilter, SfxFilterFlags::NONE);
            if ( nErr )
                pMed.reset();
            else
                pMed->SetFilter( pFilter );
        }
        else
            pMed.reset(new SfxMedium(rFileName, StreamMode::READ, pFilter, nullptr));
    }
    else
    {
        m_pViewImpl->StartDocumentInserter(
            // tdf#118578 allow inserting any Writer document except GlobalDoc
            SwDocShell::Factory().GetFactoryName(),
            LINK( this, SwView, DialogClosedHdl ),
            nSlotId
        );
        return -1;
    }

    if( !pMed )
        return -1;

    return InsertMedium( nSlotId, std::move(pMed), nVersion );
}

tools::Long SwView::InsertMedium( sal_uInt16 nSlotId, std::unique_ptr<SfxMedium> pMedium, sal_Int16 nVersion )
{
    bool bInsert = false, bCompare = false;
    tools::Long nFound = 0;
    SwDocShell* pDocSh = GetDocShell();

    switch( nSlotId )
    {
        case SID_DOCUMENT_MERGE:                        break;
        case SID_DOCUMENT_COMPARE: bCompare = true; break;
        case SID_INSERTDOC:        bInsert = true;  break;

        default:
            OSL_ENSURE( false, "unknown SlotId!" );
            bInsert = true;
            break;
    }

    if( bInsert )
    {
        uno::Reference< frame::XDispatchRecorder > xRecorder =
                GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            SfxRequest aRequest(GetViewFrame(), SID_INSERTDOC);
            aRequest.AppendItem(SfxStringItem(SID_INSERTDOC, pMedium->GetOrigURL()));
            if(pMedium->GetFilter())
                aRequest.AppendItem(SfxStringItem(FN_PARAM_1, pMedium->GetFilter()->GetName()));
            aRequest.Done();
        }

        SfxObjectShellRef aRef( pDocSh );

        ErrCode nError = SfxObjectShell::HandleFilter( pMedium.get(), pDocSh );
        // #i16722# aborted?
        if(nError != ERRCODE_NONE)
        {
            return -1;
        }

        pMedium->Download();    // start download if needed
        if( aRef.is() && 1 < aRef->GetRefCount() )  // still a valid ref?
        {
            SwReaderPtr pRdr;
            Reader *pRead = pDocSh->StartConvertFrom(*pMedium, pRdr, m_pWrtShell.get());
            if( pRead ||
                (pMedium->GetFilter()->GetFilterFlags() & SfxFilterFlags::STARONEFILTER) )
            {
                size_t nUndoCheck = 0;
                SwDoc *pDoc = pDocSh->GetDoc();
                if( pRead && pDocSh->GetDoc() )
                    nUndoCheck = lcl_PageDescWithHeader( *pDoc );
                ErrCode nErrno;
                {   //Scope for SwWait-Object, to be able to execute slots
                    //outside this scope.
                    SwWait aWait( *GetDocShell(), true );
                    m_pWrtShell->StartAllAction();
                    if ( m_pWrtShell->HasSelection() )
                        m_pWrtShell->DelRight();      // delete selections
                    if( pRead )
                    {
                        nErrno = pRdr->Read( *pRead );  // and insert document
                        pRdr.reset();
                    }
                    else
                    {
                        ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                        uno::Reference<text::XTextRange> const xInsertPosition(
                            SwXTextRange::CreateXTextRange(*pDoc,
                                *m_pWrtShell->GetCursor()->GetPoint(), nullptr));
                        nErrno = pDocSh->ImportFrom(*pMedium, xInsertPosition)
                                    ? ERRCODE_NONE : ERR_SWG_READ_ERROR;
                    }

                }

                // update all "table of ..." sections if needed
                if( m_pWrtShell->IsUpdateTOX() )
                {
                    SfxRequest aReq( FN_UPDATE_TOX, SfxCallMode::SLOT, GetPool() );
                    Execute( aReq );
                    m_pWrtShell->SetUpdateTOX( false ); // reset
                }

                if( pDoc )
                { // Disable Undo for .sdw or
                  // if the number of page styles with header/footer has changed
                    if( !pRead || nUndoCheck != lcl_PageDescWithHeader( *pDoc ) )
                    {
                        pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
                    }
                }

                m_pWrtShell->EndAllAction();
                if( nErrno )
                {
                    ErrorHandler::HandleError( nErrno );
                    nFound = nErrno.IsError() ? -1 : 0;
                }
                else
                    nFound = 0;
            }
        }
    }
    else
    {
        SfxObjectShellRef xDocSh;
        SfxObjectShellLock xLockRef;

        const int nRet = SwFindDocShell( xDocSh, xLockRef, pMedium->GetName(), OUString(),
                                    OUString(), nVersion, pDocSh );
        if( nRet )
        {
            SwWait aWait( *GetDocShell(), true );
            m_pWrtShell->StartAllAction();

            m_pWrtShell->EnterStdMode(); // delete selections

            if( bCompare )
                nFound = m_pWrtShell->CompareDoc( *static_cast<SwDocShell*>( xDocSh.get() )->GetDoc() );
            else
                nFound = m_pWrtShell->MergeDoc( *static_cast<SwDocShell*>( xDocSh.get() )->GetDoc() );

            m_pWrtShell->EndAllAction();

            if (!bCompare && !nFound)
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetEditWin().GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SwResId(STR_NO_MERGE_ENTRY)));
                xInfoBox->run();
            }
            if( nRet==2 && xDocSh.is() )
                xDocSh->DoClose();
        }
    }

    return nFound;
}

void SwView::EnableMailMerge()
{
    m_bInMailMerge = true;
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate(FN_INSERT_FIELD_DATA_ONLY);
    rBind.Update(FN_INSERT_FIELD_DATA_ONLY);
}

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS

namespace
{
    bool lcl_NeedAdditionalDataSource( const uno::Reference< XDatabaseContext >& _rDatasourceContext )
    {
        Sequence < OUString > aNames = _rDatasourceContext->getElementNames();

        return  (   !aNames.hasElements()
                ||  (   ( 1 == aNames.getLength() )
                    &&  aNames.getConstArray()[0] == SW_MOD()->GetDBConfig()->GetBibliographySource().sDataSource
                    )
                );
    }
}

#endif

void SwView::GenerateFormLetter(bool bUseCurrentDocument)
{
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) bUseCurrentDocument;
#else
    if(bUseCurrentDocument)
    {
        if(!GetWrtShell().IsAnyDatabaseFieldInDoc())
        {
            //check availability of data sources (except biblio source)
            uno::Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference<XDatabaseContext>  xDBContext = DatabaseContext::create(xContext);
            bool bCallAddressPilot = false;
            if ( lcl_NeedAdditionalDataSource( xDBContext ) )
            {
                // no data sources are available - create a new one
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "modules/swriter/ui/datasourcesunavailabledialog.ui"));
                std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("DataSourcesUnavailableDialog"));
                // no cancel allowed
                if (RET_OK != xQuery->run())
                    return;
                bCallAddressPilot = true;
            }
            else
            {
                //take an existing data source or create a new one?
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractMailMergeFieldConnectionsDlg> pConnectionsDlg(pFact->CreateMailMergeFieldConnectionsDlg(GetFrameWeld()));
                if(RET_OK == pConnectionsDlg->Execute())
                    bCallAddressPilot = !pConnectionsDlg->IsUseExistingConnections();
                else
                    return;

            }
            if(bCallAddressPilot)
            {
                GetViewFrame()->GetDispatcher()->Execute(
                                SID_ADDRESS_DATA_SOURCE, SfxCallMode::SYNCHRON);
                if ( lcl_NeedAdditionalDataSource( xDBContext ) )
                    // no additional data source has been created
                    // -> assume that the user has cancelled the pilot
                    return;
            }

            //call insert fields with database field page available, only
            SfxViewFrame* pVFrame = GetViewFrame();
            //at first hide the default field dialog if currently visible
            pVFrame->SetChildWindow(FN_INSERT_FIELD, false);
            //enable the status of the db field dialog - it is disabled in the status method
            //to prevent creation of the dialog without mail merge active
            EnableMailMerge();
            //then show the "Data base only" field dialog
            SfxBoolItem aOn(FN_INSERT_FIELD_DATA_ONLY, true);
            pVFrame->GetDispatcher()->ExecuteList(FN_INSERT_FIELD_DATA_ONLY,
                    SfxCallMode::SYNCHRON, { &aOn });
            return;
        }
        else
        {
            OUString sSource;
            if(!GetWrtShell().IsFieldDataSourceAvailable(sSource))
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "modules/swriter/ui/warndatasourcedialog.ui"));
                std::unique_ptr<weld::MessageDialog> xWarning(xBuilder->weld_message_dialog("WarnDataSourceDialog"));
                OUString sTmp(xWarning->get_primary_text());
                xWarning->set_primary_text(sTmp.replaceFirst("%1", sSource));
                if (RET_OK == xWarning->run())
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( nullptr, SID_OPTIONS_DATABASES ));
                    pDlg->Execute();
                }
                return ;
            }
        }
        SwDBManager* pDBManager = GetWrtShell().GetDBManager();

        SwDBData aData;
        SwWrtShell &rSh = GetWrtShell();

        std::vector<OUString> aDBNameList;
        std::vector<OUString> aAllDBNames;
        rSh.GetAllUsedDB( aDBNameList, &aAllDBNames );
        if(!aDBNameList.empty())
        {
            OUString sDBName(aDBNameList[0]);
            sal_Int32 nIdx {0};
            aData.sDataSource = sDBName.getToken(0, DB_DELIM, nIdx);
            aData.sCommand = sDBName.getToken(0, DB_DELIM, nIdx);
            aData.nCommandType = sDBName.getToken(0, DB_DELIM, nIdx).toInt32();
        }
        rSh.EnterStdMode(); // force change in text shell; necessary for mixing DB fields
        AttrChangedNotify(nullptr);

        if (pDBManager)
        {
            Sequence<PropertyValue> aProperties(3);
            PropertyValue* pValues = aProperties.getArray();
            pValues[0].Name = "DataSourceName";
            pValues[1].Name = "Command";
            pValues[2].Name = "CommandType";
            pValues[0].Value <<= aData.sDataSource;
            pValues[1].Value <<= aData.sCommand;
            pValues[2].Value <<= aData.nCommandType;
            pDBManager->ExecuteFormLetter(GetWrtShell(), aProperties);
        }
    }
    else
    {
        // call documents and template dialog
        SfxApplication* pSfxApp = SfxGetpApp();
        weld::Window* pTopWin = pSfxApp->GetTopWindow();

        SfxTemplateManagerDlg aDocTemplDlg(GetFrameWeld());
        int nRet = aDocTemplDlg.run();
        bool bNewWin = false;
        if ( nRet == RET_OK )
        {
            if ( pTopWin != pSfxApp->GetTopWindow() )
            {
                // the dialogue opens a document -> a new TopWindow appears
                pTopWin = pSfxApp->GetTopWindow();
                bNewWin = true;
            }
        }

        if (bNewWin)
        {
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->present();
        }
    }
#endif
}

IMPL_LINK( SwView, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( ERRCODE_NONE != _pFileDlg->GetError() )
        return;

    std::unique_ptr<SfxMedium> pMed = m_pViewImpl->CreateMedium();
    if ( !pMed )
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetEditWin().GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(RID_SVXSTR_TXTFILTER_FILTERERROR)));
        xInfoBox->run();
        return;
    }

    const sal_uInt16 nSlot = m_pViewImpl->GetRequest()->GetSlot();
    tools::Long nFound = InsertMedium( nSlot, std::move(pMed), m_pViewImpl->GetParam() );

    if ( SID_INSERTDOC == nSlot )
    {
        if ( m_pViewImpl->GetParam() == 0 )
        {
            m_pViewImpl->GetRequest()->SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
            m_pViewImpl->GetRequest()->Ignore();
        }
        else
        {
            m_pViewImpl->GetRequest()->SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
            m_pViewImpl->GetRequest()->Done();
        }
    }
    else if ( SID_DOCUMENT_COMPARE == nSlot || SID_DOCUMENT_MERGE == nSlot )
    {
        m_pViewImpl->GetRequest()->SetReturnValue( SfxInt32Item( nSlot, nFound ) );

        if ( nFound > 0 ) // show Redline browser
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            pVFrame->ShowChildWindow(FN_REDLINE_ACCEPT);

            // re-initialize Redline dialog
            sal_uInt16 nId = SwRedlineAcceptChild::GetChildWindowId();
            SwRedlineAcceptChild* pRed = static_cast<SwRedlineAcceptChild*>(pVFrame->GetChildWindow( nId ));
            if ( pRed )
                pRed->ReInitDlg( GetDocShell() );
        }
    }
}

void SwView::ExecuteScan( SfxRequest& rReq )
{
    if (m_pViewImpl)
        m_pViewImpl->ExecuteScan(rReq) ;
}

const OUString& SwView::GetOldGrfCat()
{
    return GetCachedString(OldGrfCat);
}

void SwView::SetOldGrfCat(const OUString& sStr)
{
    SetCachedString(OldGrfCat, sStr);
}

const OUString& SwView::GetOldTabCat()
{
    return GetCachedString(OldTabCat);
}

void SwView::SetOldTabCat(const OUString& sStr)
{
    SetCachedString(OldTabCat, sStr);
}

const OUString& SwView::GetOldFrameCat()
{
    return GetCachedString(OldFrameCat);
}

void SwView::SetOldFrameCat(const OUString& sStr)
{
    SetCachedString(OldFrameCat, sStr);
}

const OUString& SwView::GetOldDrwCat()
{
    return GetCachedString(OldDrwCat);
}

void SwView::SetOldDrwCat(const OUString& sStr)
{
    SwView::SetCachedString(OldDrwCat, sStr);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
