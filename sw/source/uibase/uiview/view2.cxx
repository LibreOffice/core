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

#include <com/sun/star/util/SearchOptions2.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <o3tl/any.hxx>
#include <vcl/graphicfilter.hxx>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/aeitem.hxx>
#include <SwStyleNameMapper.hxx>
#include <docary.hxx>
#include <hintids.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <caption.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/urihelper.hxx>
#include <svtools/miscopt.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxhelp.hxx>
#include <editeng/langitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/linkwarn.hxx>
#include <sfx2/htmlmode.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/lrspitem.hxx>
#include <unotools/textsearch.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/tstpitem.hxx>
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <svl/ptitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/errinf.hxx>
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
#include <frmmgr.hxx>
#include <pagedesc.hxx>
#include <section.hxx>
#include <usrpref.hxx>
#include <IMark.hxx>
#include <navipi.hxx>
#include <tox.hxx>
#include <workctrl.hxx>
#include <scroll.hxx>
#include <edtwin.hxx>
#include <wview.hxx>
#include <textsh.hxx>
#include <tabsh.hxx>
#include <listsh.hxx>
#include <cmdid.h>
#include <comcore.hrc>
#include <poolfmt.hrc>
#include <statstr.hrc>
#include <swerror.h>
#include <globals.hrc>
#include <shells.hrc>
#include <web.hrc>
#include <view.hrc>
#include <app.hrc>
#include <fmtclds.hxx>
#include <helpid.h>
#include <sfx2/templatedlg.hxx>
#include <dbconfig.hxx>
#include <dbmgr.hxx>
#include <reffld.hxx>

#include <PostItMgr.hxx>

#include <ndtxt.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx"
#include <envelp.hrc>
#include <fmthdft.hxx>
#include <svx/ofaitem.hxx>
#include <unomid.h>
#include <unotextrange.hxx>
#include <docstat.hxx>
#include <wordcountdialog.hxx>
#include <sfx2/sidebar/Sidebar.hxx>

#include <vcl/GraphicNativeTransform.hxx>
#include <vcl/GraphicNativeMetadata.hxx>
#include <vcl/settings.hxx>

#include <memory>

const char sStatusDelim[] = " : ";
const char sStatusComma[] = " , ";

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
    if (nWhichId == RES_CHRATR_LANGUAGE ||
        nWhichId == RES_CHRATR_CJK_LANGUAGE ||
        nWhichId == RES_CHRATR_CTL_LANGUAGE)
    {
        rWrtSh.StartAction();
        rWrtSh.LockView( true );
        rWrtSh.Push();

        // prepare to apply new language to all text in document
        rWrtSh.SelAll();
        rWrtSh.ExtendedSelectAll();

        // set language attribute to default for all text
        std::set<sal_uInt16> aAttribs;
        aAttribs.insert( nWhichId );
        rWrtSh.ResetAttr( aAttribs );

        rWrtSh.Pop( false );
        rWrtSh.LockView( false );
        rWrtSh.EndAction();
    }
}

/**
 * Create string for showing the page number in the statusbar
 *
 * @param nPhyNum  The physical page number
 * @param nVirtNum The logical page number (user-assigned)
 * @param rPgStr   User-defined page name (will be shown if different from logical page number)
 *
 * @return OUString Formatted string: Page 1/10 (Page nVirtNumv/rPgStr)
 **/
OUString SwView::GetPageStr(sal_uInt16 nPhyNum, sal_uInt16 nVirtNum, const OUString& rPgStr)
{
    // Show user-defined page number in brackets if any.
    OUString extra;
    if (!rPgStr.isEmpty() && OUString::number(nPhyNum) != rPgStr)
        extra = rPgStr;
    else if (nPhyNum != nVirtNum)
        extra = OUString::number(nVirtNum);

    OUString aStr(extra.isEmpty() ? SW_RES(STR_PAGE_COUNT) : SW_RES(STR_PAGE_COUNT_CUSTOM));
    aStr = aStr.replaceFirst("%1", OUString::number(nPhyNum));
    aStr = aStr.replaceFirst("%2", OUString::number(GetWrtShell().GetPageCnt()));
    aStr = aStr.replaceFirst("%3", extra);
    return aStr;
}

int SwView::InsertGraphic( const OUString &rPath, const OUString &rFilter,
                                bool bLink, GraphicFilter *pFilter,
                                Graphic* pPreviewGrf )
{
    SwWait aWait( *GetDocShell(), true );

    Graphic aGraphic;
    int aResult = GRFILTER_OK;
    if ( pPreviewGrf )
        aGraphic = *pPreviewGrf;
    else
    {
        if( !pFilter )
        {
            pFilter = &GraphicFilter::GetGraphicFilter();
        }
        aResult = GraphicFilter::LoadGraphic( rPath, rFilter, aGraphic, pFilter );
    }

    if( GRFILTER_OK == aResult )
    {
        GraphicNativeMetadata aMetadata;
        if ( aMetadata.read(aGraphic) )
        {
            const sal_uInt16 aRotation = aMetadata.getRotation();
            if (aRotation != 0)
            {
                ScopedVclPtrInstance< MessageDialog > aQueryBox( GetWindow(),"QueryRotateIntoStandardOrientationDialog","modules/swriter/ui/queryrotateintostandarddialog.ui");
                if (aQueryBox->Execute() == RET_YES)
                {
                    GraphicNativeTransform aTransform( aGraphic );
                    aTransform.rotate( aRotation );
                }
            }
        }

        SwFlyFrameAttrMgr aFrameManager( true, GetWrtShellPtr(), Frmmgr_Type::GRF );
        SwWrtShell& rShell = GetWrtShell();

        // #i123922# determine if we really want to insert or replace the graphic at a selected object
        const bool bReplaceMode(rShell.HasSelection() && nsSelectionType::SEL_FRM == rShell.GetSelectionType());

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
                        pDocSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) :
                        OUString());

                OUString sURL = URIHelper::SmartRel2Abs(
                    aTemp, rPath, URIHelper::GetMaybeFileHdl() );

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
    const sal_uInt16 nHtmlMode = ::GetHtmlMode(pDocShell);
    // when in HTML mode insert only as a link
    std::unique_ptr<FileDialogHelper> pFileDlg(new FileDialogHelper(
        ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE,
        FileDialogFlags::Graphic ));
    pFileDlg->SetTitle(SW_RESSTR(STR_INSERT_GRAPHIC ));
    pFileDlg->SetContext( FileDialogHelper::SW_INSERT_GRAPHIC );
    uno::Reference < XFilePicker2 > xFP = pFileDlg->GetFilePicker();
    uno::Reference < XFilePickerControlAccess > xCtrlAcc(xFP, UNO_QUERY);
    if(nHtmlMode & HTMLMODE_ON)
    {
        xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, makeAny(true));
        xCtrlAcc->enableControl( ExtendedFilePickerElementIds::CHECKBOX_LINK, false);
    }

    std::vector<OUString> aFormats;
    SwDoc* pDoc = pDocShell->GetDoc();
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
    OUString sGraphicFormat = SW_RESSTR(STR_POOLFRM_GRAPHIC);
    for( size_t i = 0; i < aFormats.size(); ++i )
    {
        pEntries[i] = aFormats[i];
        if(pEntries[i].equals(sGraphicFormat))
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

    const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(SID_INSERT_GRAPHIC);
    bool bShowError = !pName;
    if( pName || ERRCODE_NONE == pFileDlg->Execute() )
    {

        OUString aFileName, aFilterName;
        if ( pName )
        {
            aFileName = pName->GetValue();
            const SfxStringItem* pFilter = rReq.GetArg<SfxStringItem>(FN_PARAM_FILTER);
            if ( pFilter )
                aFilterName = pFilter->GetValue();
        }
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

        bool bAsLink = false;
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
            if( bAsLink && SvtMiscOptions().ShowLinkWarningDialog() )
            {
                ScopedVclPtrInstance< SvxLinkWarningDialog > aWarnDlg(GetWindow(),pFileDlg->GetPath());
                if( aWarnDlg->Execute() != RET_OK )
                    bAsLink=false; // don't store as link
            }
        }

        SwWrtShell& rSh = GetWrtShell();
        rSh.LockPaint();
        rSh.StartAction();

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_GRAPHIC_DEFNAME));

        // #i123922# determine if we really want to insert or replace the graphic at a selected object
        const bool bReplaceMode(rSh.HasSelection() && nsSelectionType::SEL_FRM == rSh.GetSelectionType());

        rSh.StartUndo(UNDO_INSERT, &aRewriter);

        int nError = InsertGraphic( aFileName, aFilterName, bAsLink, &GraphicFilter::GetGraphicFilter() );

        // format not equal to current filter (with autodetection)
        if( nError == GRFILTER_FORMATERROR )
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

        RESOURCE_TYPE nResId = 0;
        switch( nError )
        {
            case GRFILTER_OPENERROR:
                nResId = STR_GRFILTER_OPENERROR;
                break;
            case GRFILTER_IOERROR:
                nResId = STR_GRFILTER_IOERROR;
                break;
            case GRFILTER_FORMATERROR:
                nResId = STR_GRFILTER_FORMATERROR;
                break;
            case GRFILTER_VERSIONERROR:
                nResId = STR_GRFILTER_VERSIONERROR;
                break;
            case GRFILTER_FILTERERROR:
                nResId = STR_GRFILTER_FILTERERROR;
                break;
            case GRFILTER_TOOBIG:
                nResId = STR_GRFILTER_TOOBIG;
                break;
        }

        rSh.EndAction();
        rSh.UnlockPaint();
        if( nResId )
        {
            if( bShowError )
            {
                ScopedVclPtrInstance< MessageDialog > aInfoBox( GetWindow(), SW_RESSTR( nResId ), VclMessageType::Info);
                aInfoBox->Execute();
            }
            rReq.Ignore();
        }
        else
        {
            // set the specific graphic attrbutes to the graphic
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
            OSL_ENSURE(pFact, "Dialog creation failed!");
            std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateVclSwViewDialog(*this));
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            pDlg->Execute();
            break;
        }
        case FN_EDIT_LINK_DLG:
            EditLinkDlg();
            break;
        case FN_REFRESH_VIEW:
            GetEditWin().Invalidate();
            break;
        case FN_PAGEUP:
        case FN_PAGEUP_SEL:
        case FN_PAGEDOWN:
        case FN_PAGEDOWN_SEL:
        {
            Rectangle aVis( GetVisArea() );
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
        case FN_REDLINE_ON:
        {
            if( pArgs &&
                SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem ))
            {
                IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
                Sequence <sal_Int8> aPasswd = rIDRA.GetRedlinePassword();
                if( aPasswd.getLength() )
                {
                    OSL_ENSURE( !static_cast<const SfxBoolItem*>(pItem)->GetValue(), "SwView::Execute(): password set an redlining off doesn't match!" );
                    // xmlsec05:    new password dialog
                    vcl::Window* pParent = &GetViewFrame()->GetWindow();
                    ScopedVclPtrInstance< SfxPasswordDialog > aPasswdDlg( pParent );
                    aPasswdDlg->SetMinLen( 1 );
                    //#i69751# the result of Execute() can be ignored
                    (void)aPasswdDlg->Execute();
                    OUString sNewPasswd( aPasswdDlg->GetPassword() );
                    Sequence <sal_Int8> aNewPasswd = rIDRA.GetRedlinePassword();
                    SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                    if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                        rIDRA.SetRedlinePassword(Sequence <sal_Int8> ());
                    else
                    {   // xmlsec05: message box for wrong password
                        break;
                    }
                }

                const sal_uInt16 nOn = static_cast<const SfxBoolItem*>(pItem)->GetValue()
                    ? nsRedlineMode_t::REDLINE_ON : 0;
                const sal_uInt16 nMode = m_pWrtShell->GetRedlineMode();
                m_pWrtShell->SetRedlineModeAndCheckInsMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
            }
        }
        break;
        case FN_REDLINE_PROTECT :
        {
            IDocumentRedlineAccess& rIDRA = m_pWrtShell->getIDocumentRedlineAccess();
            Sequence <sal_Int8> aPasswd = rIDRA.GetRedlinePassword();
            if( pArgs && SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem )
                && static_cast<const SfxBoolItem*>(pItem)->GetValue() == ( aPasswd.getLength() != 0 ) )
                break;

            // xmlsec05:    new password dialog
            //              message box for wrong password
            vcl::Window* pParent = &GetViewFrame()->GetWindow();
            ScopedVclPtrInstance< SfxPasswordDialog > aPasswdDlg( pParent );
            aPasswdDlg->SetMinLen( 1 );
            if(!aPasswd.getLength())
                aPasswdDlg->ShowExtras(SfxShowExtras::CONFIRM);
            if (aPasswdDlg->Execute())
            {
                sal_uInt16 nOn = nsRedlineMode_t::REDLINE_ON;
                OUString sNewPasswd( aPasswdDlg->GetPassword() );
                Sequence <sal_Int8> aNewPasswd =
                        rIDRA.GetRedlinePassword();
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if(!aPasswd.getLength())
                {
                    rIDRA.SetRedlinePassword(aNewPasswd);
                }
                else if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                {
                    rIDRA.SetRedlinePassword(Sequence <sal_Int8> ());
                    nOn = 0;
                }
                const sal_uInt16 nMode = rIDRA.GetRedlineMode();
                m_pWrtShell->SetRedlineModeAndCheckInsMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
                rReq.AppendItem( SfxBoolItem( FN_REDLINE_PROTECT, ((nMode&nsRedlineMode_t::REDLINE_ON)==0) ) );
            }
            else
                bIgnore = true;
        }
        break;
        case FN_REDLINE_SHOW:

            if( pArgs &&
                SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem))
            {
                sal_uInt16 nMode = ( ~(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)
                        & m_pWrtShell->GetRedlineMode() ) | nsRedlineMode_t::REDLINE_SHOW_INSERT;
                if( static_cast<const SfxBoolItem*>(pItem)->GetValue() )
                    nMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;

                m_pWrtShell->SetRedlineModeAndCheckInsMode( nMode );
            }
            break;
        case FN_MAILMERGE_SENDMAIL_CHILDWINDOW:
        case FN_REDLINE_ACCEPT:
            GetViewFrame()->ToggleChildWindow(nSlot);
        break;
        case FN_REDLINE_ACCEPT_DIRECT:
        case FN_REDLINE_REJECT_DIRECT:
        {
            SwDoc *pDoc = m_pWrtShell->GetDoc();
            SwPaM *pCursor = m_pWrtShell->GetCursor();
            if( pCursor->HasMark())
            {
                if (FN_REDLINE_ACCEPT_DIRECT == nSlot)
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
                sal_uInt16 nRedline = 0;
                const SwRangeRedline *pRedline = pDoc->getIDocumentRedlineAccess().GetRedline(*pCursor->Start(), &nRedline);
                assert(pRedline != nullptr);
                if (pRedline)
                {
                    if (FN_REDLINE_ACCEPT_DIRECT == nSlot)
                        m_pWrtShell->AcceptRedline(nRedline);
                    else
                        m_pWrtShell->RejectRedline(nRedline);
                }
            }
            // Go to next change after accepting or rejecting one (fdo#83953)
            GetViewFrame()->GetDispatcher()->Execute(FN_REDLINE_NEXT_CHANGE, SfxCallMode::ASYNCHRON);
        }
        break;

        case FN_REDLINE_NEXT_CHANGE:
        {
            const SwRangeRedline *pCurrent = m_pWrtShell->GetCurrRedline();
            const SwRangeRedline *pNext = m_pWrtShell->SelNextRedline();

            // FN_REDLINE_PREV_CHANGE leaves the selection point at the start of the redline.
            // In such cases, SelNextRedline (which starts searching from the selection point)
            // immediately finds the current redline and advances the selection point to its end.

            // This behavior means that PREV_CHANGE followed by NEXT_CHANGE would not change
            // the current redline, so we detect it and select the next redline again.
            if (pCurrent && pCurrent == pNext)
                pNext = m_pWrtShell->SelNextRedline();

            if (pNext)
                m_pWrtShell->SetInSelect();
        }
        break;

        case FN_REDLINE_PREV_CHANGE:
        {
            const SwPaM *pCursor = m_pWrtShell->GetCursor();
            const SwPosition initialCursorStart = *pCursor->Start();
            const SwRangeRedline *pPrev = m_pWrtShell->SelPrevRedline();

            if (pPrev)
            {
                // FN_REDLINE_NEXT_CHANGE leaves the selection point at the end of the redline.
                // In such cases, SelPrevRedline (which starts searching from the selection point)
                // immediately finds the current redline and advances the selection point to its
                // start.

                // This behavior means that NEXT_CHANGE followed by PREV_CHANGE would not change
                // the current redline, so we detect it and move to the previous redline again.
                if (initialCursorStart == *pPrev->Start())
                    pPrev = m_pWrtShell->SelPrevRedline();
            }

            if (pPrev)
                m_pWrtShell->SetInSelect();
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
                long nFound = InsertDoc( nSlot, sFileName, sFilterName, nVersion );

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
            if ( m_pWrtShell->HasDrawView() && m_pWrtShell->GetDrawView()->IsDragObj() )
            {
                m_pWrtShell->BreakDrag();
                m_pWrtShell->EnterSelFrameMode();
            }
            else if ( m_pWrtShell->IsDrawCreate() )
            {
                GetDrawFuncPtr()->BreakCreate();
                AttrChangedNotify(m_pWrtShell); // shell change if needed
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
                    AttrChangedNotify(m_pWrtShell); // shell change if necessary
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
            ScopedVclPtrInstance< SwGotoPageDlg > aDlg (&GetViewFrame()->GetWindow(), &GetViewFrame()->GetBindings());
            if(aDlg->Execute() == RET_OK)
                GetWrtShell().GotoPage((sal_uInt16)aDlg->GetPageSelection(), true);
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
                m_pWrtShell->StartAction();
                if(TOX_INDEX == pBase->GetType())
                    m_pWrtShell->ApplyAutoMark();
                m_pWrtShell->UpdateTableOf( *pBase );
                m_pWrtShell->EndAction();
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
                SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
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
        case FN_SCROLL_PREV:
        case FN_SCROLL_NEXT:
        {
            // call the handlers of PageUp/DownButtons, only
            bool *pbNext = new bool(true); // FN_SCROLL_NEXT
            if (nSlot == FN_SCROLL_PREV)
                *pbNext = false;
            // #i75416# move the execution of the search to an asynchronously called static link
            Application::PostUserEvent( LINK(this, SwView, MoveNavigationHdl), pbNext );
        }
        break;
        case FN_NAVIGATION_POPUP:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            SfxChildWindow* pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );
            if (!pCh)
            {
                pVFrame->ToggleChildWindow( SID_NAVIGATOR );
                pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );
            }
            if (pCh)
            {
                static_cast<SwNavigationPI*>( pCh->GetContextWindow(SW_MOD()))->CreateNavigationTool(
                                GetVisArea(), true, &pVFrame->GetWindow());
            }
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
                "GalleryPanel",
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
                const SwDBData& rData = GetWrtShell().GetDBDesc();
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
#if HAVE_FEATURE_DBCONNECTIVITY
            SwWrtShell &rSh = GetWrtShell();
            if(m_bInMailMerge && rSh.IsAnyDatabaseFieldInDoc())
            {
                SwDBManager* pDBManager = rSh.GetDBManager();
                if (pDBManager)
                {
                    SwDBData aData;
                    aData = rSh.GetDBData();
                    rSh.EnterStdMode(); // force change in text shell; necessary for mixing DB fields
                    AttrChangedNotify( &rSh );

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
                OSL_ENSURE(pFact, "Dialog creation failed!");
                AbstractMailMergeCreateFromDlg* pDlg = pFact->CreateMailMergeCreateFromDlg(&pTmpFrame->GetWindow());
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if(RET_OK == pDlg->Execute())
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
            uno::Reference< linguistic2::XDictionary > xDictionary( LinguMgr::GetIgnoreAllList(), uno::UNO_QUERY );
            if( xDictionary.is() )
                xDictionary->clear();
            // put cursor to the start of the document
            m_pWrtShell->SttDoc();
            SAL_FALLTHROUGH; // call spell/grammar dialog
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
            if( m_nSelectionType & (nsSelectionType::SEL_DRW_TXT|nsSelectionType::SEL_TXT) )
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
            //special handling for the draw shell
            if(nAlias && (m_nSelectionType & (nsSelectionType::SEL_DRW)))
            {
                SfxAllEnumItem aEnumItem(SID_OBJECT_ALIGN, nAlias - SID_OBJECT_ALIGN_LEFT);
                GetViewFrame()->GetDispatcher()->ExecuteList(SID_OBJECT_ALIGN,
                        SfxCallMode::ASYNCHRON, { &aEnumItem });
            }
            else if(nAlias)
            //these slots are either re-mapped to text or object alignment
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

        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
    if(!bIgnore)
        rReq.Done();
}

/// invalidate page numbering field
void SwView::UpdatePageNums(sal_uInt16 nPhyNum, sal_uInt16 nVirtNum, const OUString& rPgStr)
{
    OUString sTemp(GetPageStr( nPhyNum, nVirtNum, rPgStr ));
    const SfxStringItem aTmp( FN_STAT_PAGE, sTemp );
    // Used to distinguish which tooltip to show
    const SfxBoolItem bExtendedTooltip( FN_STAT_PAGE,
                                        !rPgStr.isEmpty() && OUString::number(nPhyNum) != rPgStr
                                        && nPhyNum != nVirtNum );

    SfxBindings &rBnd = GetViewFrame()->GetBindings();
    rBnd.SetState( aTmp );
    rBnd.Update( FN_STAT_PAGE );
    rBnd.SetState( bExtendedTooltip );
    rBnd.Update( FN_STAT_PAGE );
}

void SwView::UpdateDocStats()
{
    SfxBindings &rBnd = GetViewFrame()->GetBindings();
    rBnd.Invalidate( FN_STAT_WORDCOUNT );
    rBnd.Update( FN_STAT_WORDCOUNT );
}

void SwView::SetAnnotationMode(bool bMode)
{
    m_bAnnotationMode = bMode;
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
        OUString sCurrentSectionName = CurrSect->GetSectionName();
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
                rSet.Put( SfxStringItem( FN_STAT_PAGE,
                            GetPageStr( nPage, nLogPage, sDisplay) ));
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
                    SfxGetpApp()->NotifyEvent(SfxEventHint(SW_EVENT_PAGE_COUNT, SwDocShell::GetEventName(STR_SW_EVENT_PAGE_COUNT), GetViewFrame()->GetObjectShell()), false);
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
                OUString aWordCount( SW_RES( selectionStats.nWord ? STR_STATUSBAR_WORDCOUNT : STR_STATUSBAR_WORDCOUNT_NO_SELECTION ) );
                const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetUILocaleDataWrapper();
                aWordCount = aWordCount.replaceFirst( "%1", rLocaleData.getNum( nWord, 0 ) );
                aWordCount = aWordCount.replaceFirst( "%2", rLocaleData.getNum( nChar, 0 ) );
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
                    SvxZoomType eZoom = (SvxZoomType) pVOpt->GetZoomType();
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
                        const SwRect aPageRect( m_pWrtShell->GetAnyCurRect( RECT_PAGE_CALC ) );
                        const SwRect aRootRect( m_pWrtShell->GetAnyCurRect( RECT_PAGES_AREA ) ); // width of columns
                        Size aPageSize( aPageRect.SSize() );
                        aPageSize.Width() += pMgr->HasNotes() && pMgr->ShowNotes() ?
                                             pMgr->GetSidebarWidth() + pMgr->GetSidebarBorderWidth() :
                                             0;

                        Size aRootSize( aRootRect.SSize() );

                        const MapMode aTmpMap( MAP_TWIP );
                        const Size& rEditSize = GetEditWin().GetOutputSizePixel();
                        const Size aWindowSize( GetEditWin().PixelToLogic( rEditSize, aTmpMap ) );

                        const long nOf = pVOpt->GetDocumentBorder() * 2L;
                        long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
                        nTmpWidth += nOf;
                        aPageSize.Height() += nOf;
                        long nFac = aWindowSize.Width() * 100 / nTmpWidth;

                        long nVisPercent = aWindowSize.Height() * 100 / aPageSize.Height();
                        nFac = std::min( nFac, nVisPercent );

                        aZoomSliderItem.AddSnappingPoint( nFac );

                        if ( bAutomaticViewLayout )
                        {
                            nTmpWidth += aPageSize.Width() + pVOpt->GetGapBetweenPages();
                            nFac = aWindowSize.Width() * 100 / nTmpWidth;
                            nFac = std::min( nFac, nVisPercent );
                            aZoomSliderItem.AddSnappingPoint( nFac );
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
                    sStr = rShell.GetTableFormat()->GetName();
                    sStr += ":";
                    sStr += rShell.GetBoxNms();
                }
                else
                {
                    const SwSection* pCurrSect = rShell.GetCurrSection();
                    if( pCurrSect )
                    {
                        switch( pCurrSect->GetType() )
                        {
                        case TOX_HEADER_SECTION:
                        case TOX_CONTENT_SECTION:
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
                            SfxItemSet aSet(GetPool(),
                                    RES_PARATR_NUMRULE, RES_PARATR_NUMRULE);
                            rShell.GetCurAttr(aSet);
                            if(SfxItemState::DEFAULT <=
                               aSet.GetItemState(RES_PARATR_NUMRULE))
                            {
                                const OUString& rNumStyle =
                                    static_cast<const SfxStringItem &>(
                                     aSet.Get(RES_PARATR_NUMRULE)).GetValue();
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
                        sStr += SW_RESSTR(STR_NUM_LEVEL);
                        sStr += OUString::number( nNumLevel + 1 );

                    }
                }
                const int nOutlineLevel = rShell.GetCurrentParaOutlineLevel();
                if( nOutlineLevel != 0 )
                {
                    if (!sStr.isEmpty())
                        sStr += sStatusComma;
                    if( bOutlineNum )
                    {
                        sStr += SW_RESSTR(STR_OUTLINE_NUMBERING);
                        sStr += sStatusDelim;
                        sStr += SW_RESSTR(STR_NUM_LEVEL);
                    }
                    else
                        sStr += SW_RESSTR(STR_NUM_OUTLINE);
                    sStr += OUString::number( nOutlineLevel);
                }

                if( rShell.HasReadonlySel() )
                {
                    if (!sStr.isEmpty())
                        sStr = sStatusDelim + sStr;
                    sStr = SW_RESSTR(SW_STR_READONLY) + sStr;
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
            GetViewFrame()->GetDispatcher()->Execute( SID_NAVIGATOR,
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
                    rSh.GotoMark( ppBookmark->get() );
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
                std::unique_ptr<AbstractSvxZoomDialog> pDlg;
                if ( pArgs )
                    pSet = pArgs;
                else
                {
                    const SwViewOption& rViewOptions = *rSh.GetViewOptions();
                    SfxItemSet aCoreSet(m_pShell->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM, SID_ATTR_VIEWLAYOUT, SID_ATTR_VIEWLAYOUT, 0 );
                    SvxZoomItem aZoom( (SvxZoomType)rViewOptions.GetZoomType(), rViewOptions.GetZoom() );

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
                    if(pFact)
                    {
                        pDlg.reset(pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aCoreSet));
                        OSL_ENSURE(pDlg, "Zooming fail!");
                        if (pDlg)
                        {
                            pDlg->SetLimits( MINZOOM, MAXZOOM );
                            if( pDlg->Execute() != RET_CANCEL )
                                pSet = pDlg->GetOutputItemSet();
                        }
                    }
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
                    const bool bBookMode  = !(0 == nColumns || 0 != (nColumns % 2)) &&
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
            else
            {

                if( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                    rSh.ToggleExtMode();
                else if ( rSh.IsExtMode() )
                {
                    rSh.ToggleExtMode();
                    rSh.ToggleAddMode();
                }
                else if ( rSh.IsAddMode() )
                {
                    rSh.ToggleAddMode();
                    rSh.ToggleBlockMode();
                }
                else
                    rSh.ToggleBlockMode();
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
        SwFlyFrameAttrMgr aMgr( true, m_pWrtShell, Frmmgr_Type::TEXT );

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
    std::unique_ptr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog( &GetViewFrame()->GetWindow(), &GetWrtShell().GetLinkManager(), bWeb ));
    if ( pDlg )
    {
        pDlg->Execute();
    }
}

bool SwView::JumpToSwMark( const OUString& rMark )
{
    bool bRet = false;
    if( !rMark.isEmpty() )
    {
        // place bookmark at top-center
        bool bSaveCC = IsCursorAtCenter();
        bool bSaveCT = IsCursorAtTop();
        SetCursorAtTop( true );

        // For scrolling the FrameSet, the corresponding shell needs to have the focus.
        bool bHasShFocus = m_pWrtShell->HasShellFocus();
        if( !bHasShFocus )
            m_pWrtShell->ShellGetFocus();

        const SwFormatINetFormat* pINet;
        OUString sCmp;
        OUString  sMark( INetURLObject::decode( rMark,
                                           INetURLObject::DECODE_WITH_CHARSET ));

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

            if( sCmp == "region" )
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
                    m_pWrtShell->GotoRefMark( sName, REF_SEQUENCEFLD, nSeqNo );
                }
            }
            else if( sCmp == "text" )
            {
                // normal text search
                m_pWrtShell->EnterStdMode();

                SearchOptions2 aSearchOpt(
                                    SearchAlgorithms_ABSOLUTE, 0,
                                    sName, OUString(),
                                    SvtSysLocale().GetLanguageTag().getLocale(),
                                    0,0,0,
                                    TransliterationModules_IGNORE_CASE,
                                    SearchAlgorithms2::ABSOLUTE,
                                    '\\' );

                //todo/mba: assuming that notes shouldn't be searched
                bool bSearchInNotes = false;
                if( m_pWrtShell->SearchPattern( aSearchOpt, bSearchInNotes, DOCPOS_START, DOCPOS_END ))
                {
                    m_pWrtShell->EnterStdMode(); // remove the selection
                    bRet = true;
                }
            }
            else if( pMarkAccess->getAllMarksEnd() != (ppMark = pMarkAccess->findMark(sMark)) )
            {
                m_pWrtShell->GotoMark( ppMark->get(), false );
                bRet = true;
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
            m_pWrtShell->GotoMark( ppMark->get(), false );
            bRet = true;
        }
        else if( nullptr != ( pINet = m_pWrtShell->FindINetAttr( sMark ) ))
            bRet = m_pWrtShell->GotoINetAttr( *pINet->GetTextINetFormat() );

        // make selection visible later
        if ( m_aVisArea.IsEmpty() )
            m_bMakeSelectionVisible = true;

        // reset ViewStatus
        SetCursorAtTop( bSaveCT, bSaveCC );

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
        long nFound = InsertDoc( nSlot, sFile, sFilter );

        if ( bHasFileName )
        {
            rRequest.SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
            rRequest.Done();
        }
    }
}

long SwView::InsertDoc( sal_uInt16 nSlotId, const OUString& rFileName, const OUString& rFilterName, sal_Int16 nVersion )
{
    SfxMedium* pMed = nullptr;
    SwDocShell* pDocSh = GetDocShell();

    if( !rFileName.isEmpty() )
    {
        SfxObjectFactory& rFact = pDocSh->GetFactory();
        std::shared_ptr<const SfxFilter> pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( rFilterName );
        if ( !pFilter )
        {
            pMed = new SfxMedium(rFileName, StreamMode::READ, nullptr, nullptr );
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
            pMed->UseInteractionHandler( true );
            ErrCode nErr = aMatcher.GuessFilter(*pMed, pFilter, SfxFilterFlags::NONE);
            if ( nErr )
                DELETEZ(pMed);
            else
                pMed->SetFilter( pFilter );
        }
        else
            pMed = new SfxMedium(rFileName, StreamMode::READ, pFilter, nullptr);
    }
    else
    {
        OUString sFactory = OUString::createFromAscii( pDocSh->GetFactory().GetShortName() );
        m_pViewImpl->StartDocumentInserter( sFactory, LINK( this, SwView, DialogClosedHdl ) );
        return -1;
    }

    if( !pMed )
        return -1;

    return InsertMedium( nSlotId, pMed, nVersion );
}

long SwView::InsertMedium( sal_uInt16 nSlotId, SfxMedium* pMedium, sal_Int16 nVersion )
{
    bool bInsert = false, bCompare = false;
    long nFound = 0;
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

        sal_uInt32 nError = SfxObjectShell::HandleFilter( pMedium, pDocSh );
        // #i16722# aborted?
        if(nError != ERRCODE_NONE)
        {
            delete pMedium;
            return -1;
        }

        pMedium->Download();    // start download if needed
        if( aRef.Is() && 1 < aRef->GetRefCount() )  // still a valid ref?
        {
            SwReader* pRdr;
            Reader *pRead = pDocSh->StartConvertFrom( *pMedium, &pRdr, m_pWrtShell );
            if( pRead ||
                (pMedium->GetFilter()->GetFilterFlags() & SfxFilterFlags::STARONEFILTER) )
            {
                size_t nUndoCheck = 0;
                SwDoc *pDoc = pDocSh->GetDoc();
                if( pRead && pDocSh->GetDoc() )
                    nUndoCheck = lcl_PageDescWithHeader( *pDoc );
                sal_uLong nErrno;
                {   //Scope for SwWait-Object, to be able to execute slots
                    //outside this scope.
                    SwWait aWait( *GetDocShell(), true );
                    m_pWrtShell->StartAllAction();
                    if ( m_pWrtShell->HasSelection() )
                        m_pWrtShell->DelRight();      // delete selections
                    if( pRead )
                    {
                        nErrno = pRdr->Read( *pRead );  // and insert document
                        delete pRdr;
                    }
                    else
                    {
                        ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                        uno::Reference<text::XTextRange> const xInsertPosition(
                            SwXTextRange::CreateXTextRange(*pDoc,
                                *m_pWrtShell->GetCursor()->GetPoint(), nullptr));
                        nErrno = pDocSh->ImportFrom(*pMedium, xInsertPosition)
                                    ? 0 : ERR_SWG_READ_ERROR;
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
                    nFound = IsError( nErrno ) ? -1 : 0;
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

        OUString sFltNm;
        const int nRet = SwFindDocShell( xDocSh, xLockRef, pMedium->GetName(), OUString(),
                                    sFltNm, nVersion, pDocSh );
        if( nRet )
        {
            SwWait aWait( *GetDocShell(), true );
            m_pWrtShell->StartAllAction();

            m_pWrtShell->EnterStdMode(); // delete selections

            if( bCompare )
                nFound = m_pWrtShell->CompareDoc( *static_cast<SwDocShell*>(&xDocSh)->GetDoc() );
            else
                nFound = m_pWrtShell->MergeDoc( *static_cast<SwDocShell*>(&xDocSh)->GetDoc() );

            m_pWrtShell->EndAllAction();

            if (!bCompare && !nFound)
            {
                vcl::Window* pWin = &GetEditWin();
                ScopedVclPtrInstance<MessageDialog>(pWin, SW_RES(STR_NO_MERGE_ENTRY), VclMessageType::Info)->Execute();
            }
            if( nRet==2 && xDocSh.Is() )
                xDocSh->DoClose();
        }
    }

    delete pMedium;
    return nFound;
}

void SwView::EnableMailMerge()
{
    m_bInMailMerge = true;
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate(FN_INSERT_FIELD_DATA_ONLY);
    rBind.Update(FN_INSERT_FIELD_DATA_ONLY);
}

#if HAVE_FEATURE_DBCONNECTIVITY

namespace
{
    bool lcl_NeedAdditionalDataSource( const uno::Reference< XDatabaseContext >& _rDatasourceContext )
    {
        Sequence < OUString > aNames = _rDatasourceContext->getElementNames();

        return  (   !aNames.getLength()
                ||  (   ( 1 == aNames.getLength() )
                    &&  aNames.getConstArray()[0] == SW_MOD()->GetDBConfig()->GetBibliographySource().sDataSource
                    )
                );
    }
}

#endif

void SwView::GenerateFormLetter(bool bUseCurrentDocument)
{
#if !HAVE_FEATURE_DBCONNECTIVITY
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
                ScopedVclPtrInstance<MessageDialog> aQuery(&GetViewFrame()->GetWindow(),
                    "DataSourcesUnavailableDialog",
                    "modules/swriter/ui/datasourcesunavailabledialog.ui");
                // no cancel allowed
                if (RET_OK != aQuery->Execute())
                    return;
                bCallAddressPilot = true;
            }
            else
            {
                //take an existing data source or create a new one?
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialog creation failed!");
                AbstractMailMergeFieldConnectionsDlg* pConnectionsDlg = pFact->CreateMailMergeFieldConnectionsDlg(&GetViewFrame()->GetWindow());
                OSL_ENSURE(pConnectionsDlg, "Dialog creation failed!");
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
                ScopedVclPtrInstance<MessageDialog> aWarning(&GetViewFrame()->GetWindow(),
                    "WarnDataSourceDialog",
                    "modules/swriter/ui/warndatasourcedialog.ui");
                OUString sTmp(aWarning->get_primary_text());
                aWarning->set_primary_text(sTmp.replaceFirst("%1", sSource));
                if (RET_OK == aWarning->Execute())
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    if ( pFact )
                    {
                        std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( nullptr, SID_OPTIONS_DATABASES ));
                        pDlg->Execute();
                    }
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
            aData.sDataSource = sDBName.getToken(0, DB_DELIM);
            aData.sCommand = sDBName.getToken(1, DB_DELIM);
            aData.nCommandType = sDBName.getToken(2, DB_DELIM ).toInt32();
        }
        rSh.EnterStdMode(); // force change in text shell; necessary for mixing DB fields
        AttrChangedNotify( &rSh );

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
        vcl::Window* pTopWin = pSfxApp->GetTopWindow();

        ScopedVclPtrInstance< SfxTemplateManagerDlg > aDocTemplDlg;
        int nRet = aDocTemplDlg->Execute();
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

        if ( bNewWin )
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->ToTop();
    }
#endif
}

IMPL_LINK_TYPED( SwView, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( ERRCODE_NONE != _pFileDlg->GetError() )
        return;

    SfxMedium* pMed = m_pViewImpl->CreateMedium();
    if ( !pMed )
        return;

    const sal_uInt16 nSlot = m_pViewImpl->GetRequest()->GetSlot();
    long nFound = InsertMedium( nSlot, pMed, m_pViewImpl->GetParam() );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
