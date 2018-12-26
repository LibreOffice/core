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

#include <rtl/tencinfo.h>
#include <hintids.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <vcl/errinf.hxx>
#include <vcl/weld.hxx>
#include <vcl/print.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/metric.hxx>
#include <vcl/textview.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/undo.hxx>
#include <unotools/textsearch.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/transfer.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svx/svxids.hrc>
#include <svtools/htmlcfg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/prnmon.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/srchdlg.hxx>
#include <svl/srchitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/sfxhtml.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <srcview.hxx>
#include "viewfunc.hxx"
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentState.hxx>
#include <sfx2/msg.hxx>
#include <shellio.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <strings.hrc>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <sfx2/filedlghelper.hxx>
#define ShellClass_SwSrcView
#include <swslots.hxx>

#include <unomid.h>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

#define SWSRCVIEWFLAGS SfxViewShellFlags::NO_NEWWINDOW

#define SRC_SEARCHOPTIONS (SearchOptionFlags::ALL & ~SearchOptionFlags(SearchOptionFlags::FORMAT|SearchOptionFlags::FAMILIES|SearchOptionFlags::SEARCHALL))

// Printing margins -> like Basic - Ide
#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300

SFX_IMPL_NAMED_VIEWFACTORY(SwSrcView, "SourceView")
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}

SFX_IMPL_SUPERCLASS_INTERFACE(SwSrcView, SfxViewShell)

void SwSrcView::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("source");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS,
                                            SfxVisibilityFlags::Standard|SfxVisibilityFlags::Server,
                                            ToolbarId::Webtools_Toolbox);

    GetStaticInterface()->RegisterChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
}


static void lcl_PrintHeader( vcl::RenderContext &rOutDev, sal_Int32 nPages, sal_Int32 nCurPage, const OUString& rTitle )
{
    short nLeftMargin   = LMARGPRN;
    Size aSz = rOutDev.GetOutputSize();
    short nBorder = BORDERPRN;

    Color aOldFillColor( rOutDev.GetFillColor() );
    vcl::Font aOldFont( rOutDev.GetFont() );

    rOutDev.SetFillColor( COL_TRANSPARENT );

    vcl::Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlignment( ALIGN_BOTTOM );
    rOutDev.SetFont( aFont );

    long nFontHeight = rOutDev.GetTextHeight();

    // 1.Border => Line, 2+3 Border = Space.
    long nYTop = TMARGPRN-3*nBorder-nFontHeight;

    long nXLeft = nLeftMargin-nBorder;
    long nXRight = aSz.Width()-RMARGPRN+nBorder;

    rOutDev.DrawRect( tools::Rectangle(
        Point( nXLeft, nYTop ),
        Size( nXRight-nXLeft, aSz.Height() - nYTop - BMARGPRN + nBorder ) ) );

    long nY = TMARGPRN-2*nBorder;
    Point aPos( nLeftMargin, nY );
    rOutDev.DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        rOutDev.SetFont( aFont );
        OUString aPageStr( " [" );
        aPageStr += SwResId( STR_PAGE );
        aPageStr += " ";
        aPageStr += OUString::number( nCurPage );
        aPageStr += "]";
        aPos.AdjustX(rOutDev.GetTextWidth( rTitle ) );
        rOutDev.DrawText( aPos, aPageStr );
    }

    nY = TMARGPRN-nBorder;

    rOutDev.DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    rOutDev.SetFont( aOldFont );
    rOutDev.SetFillColor( aOldFillColor );
}

static rtl_TextEncoding lcl_GetStreamCharSet(rtl_TextEncoding eLoadEncoding)
{
    rtl_TextEncoding eRet = eLoadEncoding;
    if(RTL_TEXTENCODING_DONTKNOW == eRet)
    {
        SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
        const sal_Char *pCharSet =
            rtl_getBestMimeCharsetFromTextEncoding( rHtmlOptions.GetTextEncoding() );
        eRet = rtl_getTextEncodingFromMimeCharset( pCharSet );
    }
    return eRet;
}

static OUString lcl_ConvertTabsToSpaces( const OUString& sLine )
{
    if (sLine.isEmpty())
        return sLine;

    OUString aRet = sLine;
    const sal_Unicode aPadSpaces[4] = {' ', ' ', ' ', ' '};
    sal_Int32 nPos = 0;
    for (;;)
    {
        nPos = aRet.indexOf('\t', nPos);
        if (nPos<0)
        {
            break;
        }
        // Not 4 blanks, but on 4th TabPos:
        const sal_Int32 nPadLen = 4 - (nPos % 4);
        aRet = aRet.replaceAt(nPos, 1, OUString(aPadSpaces, nPadLen));
        nPos += nPadLen;
    }
    return aRet;
}

SwSrcView::SwSrcView(SfxViewFrame* pViewFrame, SfxViewShell*) :
    SfxViewShell( pViewFrame, SWSRCVIEWFLAGS ),
    aEditWin( VclPtr<SwSrcEditWindow>::Create( &pViewFrame->GetWindow(), this ) ),
    bSourceSaved(false),
    eLoadEncoding(RTL_TEXTENCODING_DONTKNOW)
{
    Init();
}

SwSrcView::~SwSrcView()
{
    SwDocShell* pDocShell = GetDocShell();
    OSL_ENSURE(dynamic_cast<SwWebDocShell*>( pDocShell), "Why no WebDocShell?" );
    const TextSelection&  rSel = aEditWin->GetTextView()->GetSelection();
    static_cast<SwWebDocShell*>(pDocShell)->SetSourcePara( static_cast< sal_uInt16 >( rSel.GetStart().GetPara() ) );

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps
        = xDPS->getDocumentProperties();
    OUString url = xDocProps->getAutoloadURL();
    sal_Int32 delay = xDocProps->getAutoloadSecs();
    pDocShell->SetAutoLoad(INetURLObject(url), delay,
                            (delay != 0) || !url.isEmpty());
    EndListening(*pDocShell);
    pSearchItem.reset();

    aEditWin.disposeAndClear();
}

void SwSrcView::SaveContentTo(SfxMedium& rMed)
{
    SvStream* pOutStream = rMed.GetOutStream();
    pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
    aEditWin->Write( *pOutStream );
}

void SwSrcView::Init()
{
    SetName("Source");
    SetWindow( aEditWin.get() );
    SwDocShell* pDocShell = GetDocShell();
    // If the doc is still loading, then the DocShell must fire up
    // the Load if the loading is completed.
    if(!pDocShell->IsLoading())
        Load(pDocShell);
    else
    {
        aEditWin->SetReadonly(true);
    }

    SetNewWindowAllowed( false );
    StartListening(*pDocShell, DuplicateHandling::Prevent);
}

SwDocShell*     SwSrcView::GetDocShell()
{
    SfxObjectShell* pObjShell = GetViewFrame()->GetObjectShell();
    return dynamic_cast<SwDocShell*>( pObjShell );
}

void SwSrcView::SaveContent(const OUString& rTmpFile)
{
    SfxMedium aMedium( rTmpFile, StreamMode::WRITE);
    SvStream* pOutStream = aMedium.GetOutStream();
    pOutStream->SetStreamCharSet( lcl_GetStreamCharSet(eLoadEncoding) );
    aEditWin->Write(*pOutStream);
    aMedium.Commit();
}

void SwSrcView::Execute(SfxRequest& rReq)
{
    TextView* pTextView = aEditWin->GetTextView();
    switch( rReq.GetSlot() )
    {
        case SID_SAVEACOPY:
        case SID_SAVEASDOC:
        {
            SvtPathOptions aPathOpt;
            // filesave dialog with autoextension
            FileDialogHelper aDlgHelper(
                TemplateDescription::FILESAVE_AUTOEXTENSION,
                FileDialogFlags::NONE, aEditWin->GetFrameWeld());
            uno::Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();
            uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

            // search for an html filter for export
            SfxFilterContainer* pFilterCont = GetObjectShell()->GetFactory().GetFilterContainer();
            std::shared_ptr<const SfxFilter> pFilter =
                pFilterCont->GetFilter4Extension( "html", SfxFilterFlags::EXPORT );
            if ( pFilter )
            {
                // filter found -> use its uiname and wildcard
                const OUString& rUIName = pFilter->GetUIName();
                const WildCard& rCard = pFilter->GetWildcard();
                xFltMgr->appendFilter( rUIName, rCard.getGlob() );
                xFltMgr->setCurrentFilter( rUIName ) ;
            }
            else
            {
                // filter not found
                OUString sHtml("HTML");
                xFltMgr->appendFilter( sHtml, "*.html;*.htm" );
                xFltMgr->setCurrentFilter( sHtml ) ;
            }

            xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );
            if( aDlgHelper.Execute() == ERRCODE_NONE)
            {
                SfxMedium aMedium( xFP->getSelectedFiles().getConstArray()[0],
                                    StreamMode::WRITE | StreamMode::SHARE_DENYNONE );
                SvStream* pOutStream = aMedium.GetOutStream();
                pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
                aEditWin->Write( *pOutStream );
                aMedium.Commit();
            }
        }
        break;
        case SID_SAVEDOC:
        {
            SwDocShell* pDocShell = GetDocShell();
            SfxMedium* pMed = nullptr;
            if(pDocShell->HasName())
                pMed = pDocShell->GetMedium();
            else
            {
                const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pDocShell->ExecuteSlot(rReq, pDocShell->GetInterface()));
                if(pItem && pItem->GetValue())
                    pMed = pDocShell->GetMedium();
            }
            if(pMed)
            {
                SvStream* pOutStream = pMed->GetOutStream();
                pOutStream->Seek(0);
                pOutStream->SetStreamSize(0);
                pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
                aEditWin->Write( *pOutStream );
                pMed->CloseOutStream();
                pMed->Commit();
                pDocShell->GetDoc()->getIDocumentState().ResetModified();
                bSourceSaved = true;
                aEditWin->ClearModifyFlag();
            }
        }
        break;
        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pTmpArgs = rReq.GetArgs();

            const sal_uInt16 nWhich = pTmpArgs->GetWhichByPos( 0 );
            OSL_ENSURE( nWhich, "Which for SearchItem ?" );
            const SfxPoolItem& rItem = pTmpArgs->Get( nWhich );
            SetSearchItem( static_cast<const SvxSearchItem&>(rItem));
            StartSearchAndReplace( static_cast<const SvxSearchItem&>(rItem), rReq.IsAPI() );
            if(aEditWin->IsModified())
                GetDocShell()->GetDoc()->getIDocumentState().SetModified();
        }
        break;
        case FN_REPEAT_SEARCH:
        {
            SvxSearchItem* pSrchItem = GetSearchItem();
            if(pSrchItem)
            {
                StartSearchAndReplace( *pSrchItem, rReq.IsAPI() );
                if(aEditWin->IsModified())
                    GetDocShell()->GetDoc()->getIDocumentState().SetModified();
            }
        }
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
        {
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
        }
        break;
        case SID_UNDO:
            pTextView->Undo();
            GetViewFrame()->GetBindings().InvalidateAll(false);
        break;
        case SID_REDO:
            pTextView->Redo();
            GetViewFrame()->GetBindings().InvalidateAll(false);
        break;
        case SID_REPEAT:
        break;
        case SID_CUT:
            if(pTextView->HasSelection())
                pTextView->Cut();
        break;
        case SID_COPY:
            if(pTextView->HasSelection())
                pTextView->Copy();
        break;
        case SID_PASTE:
            pTextView->Paste();
        break;
        case SID_SELECTALL:
            pTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( TEXT_PARA_ALL, TEXT_INDEX_ALL ) ) );
        break;
    }
    aEditWin->Invalidate();
}

void SwSrcView::GetState(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    TextView* pTextView = aEditWin->GetTextView();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_SAVEASDOC:
                rSet.Put(SfxStringItem(nWhich, SwResId(STR_SAVEAS_SRC)));
            break;
            case SID_SAVEACOPY:
                rSet.Put(SfxStringItem(nWhich, SwResId(STR_SAVEACOPY_SRC)));
            break;
            case SID_SAVEDOC:
            {
                SwDocShell* pDocShell = GetDocShell();
                if(!pDocShell->IsModified())
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
            break;
            case SID_TABLE_CELL:
            {
                OUString aPos( SwResId(STR_SRCVIEW_ROW) );
                TextSelection aSel = pTextView->GetSelection();
                aPos += OUString::number( aSel.GetEnd().GetPara()+1 );
                aPos += " : ";
                aPos += SwResId(STR_SRCVIEW_COL);
                aPos += OUString::number( aSel.GetEnd().GetIndex()+1 );
                SfxStringItem aItem( nWhich, aPos );
                rSet.Put( aItem );
            }
            break;
            case SID_SEARCH_OPTIONS:
            {
                SearchOptionFlags nOpt = SRC_SEARCHOPTIONS;
                if(GetDocShell()->IsReadOnly())
                    nOpt &= ~SearchOptionFlags(SearchOptionFlags::REPLACE|SearchOptionFlags::REPLACE_ALL);

                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS,  static_cast<sal_uInt16>(nOpt) ) );
            }
            break;
            case SID_SEARCH_ITEM:
            {
                OUString sSelected;
                if ( !pTextView->HasSelection() )
                {
                    const TextSelection& rSel = pTextView->GetSelection();
                    sSelected = aEditWin->GetTextEngine()->GetWord( rSel.GetStart());
                }
                else
                {
                    sSelected = pTextView->GetSelected();
                }
                SvxSearchItem * pSrchItem = GetSearchItem();
                pSrchItem->SetSearchString( sSelected );
                rSet.Put( *pSrchItem );
            }
            break;
            case FN_REPEAT_SEARCH:
            {
                if(!GetSearchItem())
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_UNDO:
            case SID_REDO:
            {
                SfxUndoManager& rMgr = pTextView->GetTextEngine()->GetUndoManager();
                sal_uInt16 nCount = 0;
                if(nWhich == SID_UNDO)
                {
                    nCount = rMgr.GetUndoActionCount();
                    if(nCount)
                    {
                        OUString aStr(SvtResId( STR_UNDO));
                        aStr += rMgr.GetUndoActionComment(--nCount);
                        rSet.Put(SfxStringItem(nWhich, aStr));
                    }
                    else
                        rSet.DisableItem(nWhich);
                }
                else
                {
                    nCount = rMgr.GetRedoActionCount();
                    if(nCount)
                    {
                        OUString aStr(SvtResId( STR_REDO));
                        aStr += rMgr.GetRedoActionComment(--nCount);
                        rSet.Put(SfxStringItem(nWhich,aStr));
                    }
                    else
                        rSet.DisableItem(nWhich);
                }
            }
            break;
            case SID_MAIL_SENDDOCASPDF:
            case SID_MAIL_SENDDOC :
            case SID_EXPORTDOCASPDF:
            case SID_DIRECTEXPORTDOCASPDF:
            case SID_EXPORTDOC:
            case SID_REPEAT:
            case SID_BROWSER_MODE:
            case FN_PRINT_LAYOUT:
                rSet.DisableItem(nWhich);
            break;
            case SID_CUT:
            case SID_COPY:
                if(!pTextView->HasSelection())
                    rSet.DisableItem(nWhich);
            break;
            case SID_PASTE:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard(
                                                        aEditWin.get()) );
                bool bDisable = !aDataHelper.GetXTransferable().is() ||
                            0 == aDataHelper.GetFormatCount();
                if( bDisable )
                    rSet.DisableItem(nWhich);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

SvxSearchItem* SwSrcView::GetSearchItem()
{
    if(!pSearchItem)
    {
        pSearchItem.reset(new SvxSearchItem(SID_SEARCH_ITEM));
    }
    return pSearchItem.get();
}

void SwSrcView::SetSearchItem( const SvxSearchItem& rItem )
{
    pSearchItem.reset(static_cast<SvxSearchItem*>(rItem.Clone()));
}

void SwSrcView::StartSearchAndReplace(const SvxSearchItem& rSearchItem,
                                                  bool bApi,
                                                  bool bRecursive)
{
    TextView* pTextView = aEditWin->GetTextView();
    TextSelection aSel;
    TextPaM aPaM;

    bool bForward = !rSearchItem.GetBackward();
    bool bAtStart = pTextView->GetSelection() == TextSelection( aPaM, aPaM );

    if( !bForward )
        aPaM = TextPaM( TEXT_PARA_ALL, TEXT_INDEX_ALL );

    i18nutil::SearchOptions2 aSearchOpt( rSearchItem.GetSearchOptions() );
    aSearchOpt.Locale = GetAppLanguageTag().getLocale();

    sal_uInt16 nFound;
    bool bAll = false;
    switch( rSearchItem.GetCommand() )
    {
    case SvxSearchCmd::FIND:
    case SvxSearchCmd::FIND_ALL:
        nFound = pTextView->Search( aSearchOpt, bForward ) ? 1 : 0;
        break;

    case SvxSearchCmd::REPLACE_ALL: bAll = true;
        [[fallthrough]];
    case SvxSearchCmd::REPLACE:
        nFound = pTextView->Replace( aSearchOpt, bAll, bForward );
        break;

    default:
        nFound = 0;
    }

    if( !nFound )
    {
        bool bNotFoundMessage = false;
        if(!bRecursive)
        {
            bNotFoundMessage = bAtStart;
        }
        else if(bAtStart)
        {
            bNotFoundMessage = true;
        }

        if(!bApi)
        {
            if(bNotFoundMessage)
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/infonotfounddialog.ui"));
                std::unique_ptr<weld::MessageDialog> xInfoBox(xBuilder->weld_message_dialog("InfoNotFoundDialog"));
                xInfoBox->run();
            }
            else if(!bRecursive)
            {
                int nRet;

                if (!bForward)
                {
                    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/querycontinueenddialog.ui"));
                    std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("QueryContinueEndDialog"));
                    nRet = xQueryBox->run();
                }
                else
                {
                    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/querycontinuebegindialog.ui"));
                    std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("QueryContinueBeginDialog"));
                    nRet = xQueryBox->run();
                }

                if (nRet == RET_YES)
                {
                    pTextView->SetSelection( TextSelection( aPaM, aPaM ) );
                    StartSearchAndReplace( rSearchItem, false, true );
                }
            }
        }
    }
}

sal_uInt16 SwSrcView::SetPrinter(SfxPrinter* pNew, SfxPrinterChangeFlags nDiffFlags )
{
    SwDocShell* pDocSh = GetDocShell();
    if ( (SfxPrinterChangeFlags::JOBSETUP | SfxPrinterChangeFlags::PRINTER) & nDiffFlags )
    {
        pDocSh->GetDoc()->getIDocumentDeviceAccess().setPrinter( pNew, true, true );
        if ( nDiffFlags & SfxPrinterChangeFlags::PRINTER )
            pDocSh->SetModified();
    }
    if ( nDiffFlags & SfxPrinterChangeFlags::OPTIONS )
        ::SetPrinter( &pDocSh->getIDocumentDeviceAccess(), pNew, true );

    const bool bChgOri  = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION);
    const bool bChgSize = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE);
    if ( bChgOri || bChgSize )
    {
        pDocSh->SetModified();
    }
    return 0;
}

SfxPrinter* SwSrcView::GetPrinter( bool bCreate )
{
    return  GetDocShell()->GetDoc()->getIDocumentDeviceAccess().getPrinter( bCreate );
}

sal_Int32 SwSrcView::PrintSource(
    OutputDevice *pOutDev,
    sal_Int32 nPage,
    bool bCalcNumPagesOnly )
{
    if (!pOutDev || nPage <= 0)
        return 0;

    //! This algorithm for printing the n-th page is very poor since it
    //! needs to go over the text of all previous pages to get to the correct one.
    //! But since HTML source code is expected to be just a small number of pages
    //! even this poor algorithm should be enough...

    pOutDev->Push();

    TextEngine* pTextEngine = aEditWin->GetTextEngine();
    pOutDev->SetMapMode(MapMode(MapUnit::Map100thMM));
    vcl::Font aFont( aEditWin->GetOutWin()->GetFont() );
    Size aSize( aFont.GetFontSize() );
    aSize = aEditWin->GetOutWin()->PixelToLogic(aSize, MapMode(MapUnit::Map100thMM));
    aFont.SetFontSize( aSize );
    aFont.SetColor( COL_BLACK );
    pOutDev->SetFont( aFont );

    OUString aTitle( GetViewFrame()->GetWindow().GetText() );

    const long nLineHeight = pOutDev->GetTextHeight(); // slightly more
    const long nParaSpace = 10;

    Size aPaperSz = pOutDev->GetOutputSize();
    aPaperSz.AdjustWidth( -(LMARGPRN + RMARGPRN) );
    aPaperSz.AdjustHeight( -(TMARGPRN + BMARGPRN) );

    // nLinepPage is not true, if lines have to be wrapped...
    const long nLinespPage = nLineHeight ? aPaperSz.Height() / nLineHeight : 1;
    const long nCharWidth = pOutDev->GetTextWidth("X");
    const sal_Int32 nCharspLine = nCharWidth ? static_cast<sal_Int32>(aPaperSz.Width() / nCharWidth) : 1;
    const sal_uInt32 nParas = pTextEngine->GetParagraphCount();

    const sal_Int32 nPages = static_cast<sal_Int32>(nParas / nLinespPage + 1 );
    sal_Int32 nCurPage = 1;

    // Print header...
    if (!bCalcNumPagesOnly && nPage == nCurPage)
        lcl_PrintHeader( *pOutDev, nPages, nCurPage, aTitle );
    const Point aStartPos( LMARGPRN, TMARGPRN );
    Point aPos( aStartPos );
    for ( sal_uInt32 nPara = 0; nPara < nParas; ++nPara )
    {
        const OUString aLine( lcl_ConvertTabsToSpaces(pTextEngine->GetText( nPara )) );
        const sal_Int32 nLineLen = aLine.getLength();
        const sal_Int32 nLines = (nLineLen+nCharspLine-1) / nCharspLine;
        for ( sal_Int32 nLine = 0; nLine < nLines; ++nLine )
        {
            aPos.AdjustY(nLineHeight );
            if ( aPos.Y() > ( aPaperSz.Height() + TMARGPRN - nLineHeight/2 ) )
            {
                ++nCurPage;
                if (!bCalcNumPagesOnly && nPage == nCurPage)
                    lcl_PrintHeader( *pOutDev, nPages, nCurPage, aTitle );
                aPos = aStartPos;
            }
            if (!bCalcNumPagesOnly && nPage == nCurPage)
            {
                const sal_Int32 nStart = nLine * nCharspLine;
                const sal_Int32 nLen = std::min(nLineLen-nStart, nCharspLine);
                pOutDev->DrawText( aPos, aLine.copy(nStart, nLen) );
            }
        }
        aPos.AdjustY(nParaSpace );
    }

    pOutDev->Pop();

    OSL_ENSURE( bCalcNumPagesOnly || nPage <= nCurPage, "page number out of range" );
    return nCurPage;
}

void SwSrcView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::ModeChanged ||
            (
             rHint.GetId() == SfxHintId::TitleChanged &&
             !GetDocShell()->IsReadOnly() && aEditWin->IsReadonly()
            )
       )
    {
        // Broadcast only comes once!
        const SwDocShell* pDocSh = GetDocShell();
        const bool bReadonly = pDocSh->IsReadOnly();
        aEditWin->SetReadonly(bReadonly);
    }
    SfxViewShell::Notify(rBC, rHint);
}

void SwSrcView::Load(SwDocShell* pDocShell)
{
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( rHtmlOptions.GetTextEncoding() );
    rtl_TextEncoding eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    aEditWin->SetReadonly(pDocShell->IsReadOnly());
    aEditWin->SetTextEncoding(eDestEnc);
    SfxMedium* pMedium = pDocShell->GetMedium();

    std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
    bool bHtml = pFilter && pFilter->GetUserData() == "HTML";
    bool bDocModified = pDocShell->IsModified();
    if(bHtml && !bDocModified && pDocShell->HasName())
    {
        SvStream* pStream = pMedium->GetInStream();
        if(pStream && ERRCODE_NONE == pStream->GetError() )
        {
            rtl_TextEncoding eHeaderEnc =
                SfxHTMLParser::GetEncodingByHttpHeader(
                                            pDocShell->GetHeaderAttributes() );
            if( RTL_TEXTENCODING_DONTKNOW == eHeaderEnc )
            {
                const sal_Char *pTmpCharSet =
                    rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_ISO_8859_1 );
                eHeaderEnc = rtl_getTextEncodingFromMimeCharset( pTmpCharSet );
            }
            if( RTL_TEXTENCODING_DONTKNOW != eHeaderEnc &&
                 eDestEnc != eHeaderEnc )
            {
                eDestEnc = eHeaderEnc;
                aEditWin->SetTextEncoding(eDestEnc);
            }
            pStream->SetStreamCharSet( eDestEnc );
            pStream->Seek(0);
            TextEngine* pTextEngine = aEditWin->GetTextEngine();
            pTextEngine->EnableUndo(false);
            aEditWin->Read(*pStream);
            pTextEngine->EnableUndo(true);
        }
        else
        {
            vcl::Window& rTmpWindow = GetViewFrame()->GetWindow();
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(rTmpWindow.GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(STR_ERR_SRCSTREAM)));
            xBox->run();
        }
    }
    else
    {
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        const OUString sFileURL( aTempFile.GetURL() );
        SvtSaveOptions aOpt;

        {
            SfxMedium aMedium( sFileURL,StreamMode::READWRITE );
            SwWriter aWriter( aMedium, *pDocShell->GetDoc() );
            WriterRef xWriter;
            ::GetHTMLWriter(OUString(), aMedium.GetBaseURL( true ), xWriter);
            const OUString sWriteName = pDocShell->HasName()
                ? pMedium->GetName()
                : sFileURL;
            ErrCode nRes = aWriter.Write(xWriter, &sWriteName);
            if(nRes)
            {
                ErrorHandler::HandleError(nRes);
                aEditWin->SetReadonly(true);
            }
            aMedium.Commit();
            SvStream* pInStream = aMedium.GetInStream();
            pInStream->Seek(0);
            pInStream->SetStreamCharSet( eDestEnc );

            aEditWin->Read(*pInStream);
        }
    }
    aEditWin->ClearModifyFlag();

    eLoadEncoding = eDestEnc;

    if(bDocModified)
        pDocShell->SetModified();// The flag will be reset in between times.
    // Disable AutoLoad
    pDocShell->SetAutoLoad(INetURLObject(), 0, false);
    OSL_ENSURE(dynamic_cast<SwWebDocShell*>( pDocShell), "Why no WebDocShell?" );
    sal_uInt16 nLine = static_cast<SwWebDocShell*>(pDocShell)->GetSourcePara();
    aEditWin->SetStartLine(nLine);
    aEditWin->GetTextEngine()->ResetUndo();
    aEditWin->GetOutWin()->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
