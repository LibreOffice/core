/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <vcl/print.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/metric.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/undo.hxx>
#include <svtools/txtcmp.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/transfer.hxx>
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
#include "editeng/unolingu.hxx"
#include <sfx2/sfxhtml.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <srcview.hxx>
#include <viewfunc.hxx>
#include <doc.hxx>
#include <sfx2/msg.hxx>
#include <shellio.hxx>

#include <cmdid.h>          // FN_       ...
#include <helpid.h>
#include <globals.hrc>
#include <shells.hrc>
#include <popup.hrc>
#include <web.hrc>
#include <view.hrc>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <sfx2/filedlghelper.hxx>
#define SwSrcView
#define SearchSettings
#define _ExecSearch Execute
#define _StateSearch GetState
#include <svx/svxslots.hxx>
#include "swslots.hxx"

#include <unomid.h>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::sfx2;
using ::rtl::OUString;
using ::com::sun::star::util::SearchOptions;


#define SWSRCVIEWFLAGS ( SFX_VIEW_CAN_PRINT|\
                      SFX_VIEW_NO_NEWWINDOW )

#define SRC_SEARCHOPTIONS (0xFFFF & ~(SEARCH_OPTIONS_FORMAT|SEARCH_OPTIONS_FAMILIES|SEARCH_OPTIONS_SEARCH_ALL))

// Druckraender -> wie Basic - Ide
#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300

SFX_IMPL_NAMED_VIEWFACTORY(SwSrcView, "SourceView")
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}

SFX_IMPL_INTERFACE( SwSrcView, SfxViewShell, SW_RES(0) )
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_SRCVIEW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_WEBTOOLS_TOOLBOX) );
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
}

TYPEINIT1(SwSrcView, SfxViewShell)

void lcl_PrintHeader( OutputDevice &rOutDev, USHORT nPages, USHORT nCurPage, const String& rTitle )
{
    short nLeftMargin   = LMARGPRN;
    Size aSz = rOutDev.GetOutputSize();
    short nBorder = BORDERPRN;

    Color aOldFillColor( rOutDev.GetFillColor() );
    Font aOldFont( rOutDev.GetFont() );

    rOutDev.SetFillColor( Color(COL_TRANSPARENT) );

    Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlign( ALIGN_BOTTOM );
    rOutDev.SetFont( aFont );

    long nFontHeight = rOutDev.GetTextHeight();

    // 1.Border => Strich, 2+3 Border = Freiraum.
    long nYTop = TMARGPRN-3*nBorder-nFontHeight;

    long nXLeft = nLeftMargin-nBorder;
    long nXRight = aSz.Width()-RMARGPRN+nBorder;

    rOutDev.DrawRect( Rectangle(
        Point( nXLeft, nYTop ),
        Size( nXRight-nXLeft, aSz.Height() - nYTop - BMARGPRN + nBorder ) ) );


    long nY = TMARGPRN-2*nBorder;
    Point aPos( nLeftMargin, nY );
    rOutDev.DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        rOutDev.SetFont( aFont );
        String aPageStr( C2S(" [") );
        aPageStr += String( SW_RES( STR_PAGE ) );
        aPageStr += ' ';
        aPageStr += String::CreateFromInt32( nCurPage );
        aPageStr += ']';
        aPos.X() += rOutDev.GetTextWidth( rTitle );
        rOutDev.DrawText( aPos, aPageStr );
    }


    nY = TMARGPRN-nBorder;

    rOutDev.DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    rOutDev.SetFont( aOldFont );
    rOutDev.SetFillColor( aOldFillColor );
}

rtl_TextEncoding lcl_GetStreamCharSet(rtl_TextEncoding eLoadEncoding)
{
    rtl_TextEncoding eRet = eLoadEncoding;
    if(RTL_TEXTENCODING_DONTKNOW == eRet)
    {
        SvxHtmlOptions* pHtmlOptions = SvxHtmlOptions::Get();
        const sal_Char *pCharSet =
            rtl_getBestMimeCharsetFromTextEncoding( pHtmlOptions->GetTextEncoding() );
        eRet = rtl_getTextEncodingFromMimeCharset( pCharSet );
    }
    return eRet;
}

void lcl_ConvertTabsToSpaces( String& rLine )
{
    if ( rLine.Len() )
    {
        USHORT nPos = 0;
        USHORT nMax = rLine.Len();
        while ( nPos < nMax )
        {
            if ( rLine.GetChar(nPos) == '\t' )
            {
                // Nicht 4 Blanks, sondern an 4er TabPos:
                String aBlanker;
                aBlanker.Fill( ( 4 - ( nPos % 4 ) ), ' ' );
                rLine.Erase( nPos, 1 );
                rLine.Insert( aBlanker, nPos );
                nMax = rLine.Len();
            }
            nPos++; // Nicht optimal, falls Tab, aber auch nicht verkehrt...
        }
    }
}

SwSrcView::SwSrcView(SfxViewFrame* pViewFrame, SfxViewShell*) :
    SfxViewShell( pViewFrame, SWSRCVIEWFLAGS ),
    aEditWin( &pViewFrame->GetWindow(), this ),
    pSearchItem(0),
    bSourceSaved(FALSE),
    eLoadEncoding(RTL_TEXTENCODING_DONTKNOW)
{
    Init();
}

SwSrcView::~SwSrcView()
{
    SwDocShell* pDocShell = GetDocShell();
    OSL_ENSURE(PTR_CAST(SwWebDocShell, pDocShell), "Wieso keine WebDocShell?");
    const TextSelection&  rSel = aEditWin.GetTextView()->GetSelection();
    ((SwWebDocShell*)pDocShell)->SetSourcePara( static_cast< USHORT >( rSel.GetStart().GetPara() ) );

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps
        = xDPS->getDocumentProperties();
    ::rtl::OUString url = xDocProps->getAutoloadURL();
    sal_Int32 delay = xDocProps->getAutoloadSecs();
    pDocShell->SetAutoLoad(INetURLObject(url), delay,
                            (delay != 0) || !url.equalsAscii(""));
    EndListening(*pDocShell);
    delete pSearchItem;
}

void SwSrcView::SaveContentTo(SfxMedium& rMed)
{
    SvStream* pOutStream = rMed.GetOutStream();
    pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
    aEditWin.Write( *pOutStream );//, EE_FORMAT_TEXT);
}

void SwSrcView::Init()
{
    SetHelpId(SW_SRC_VIEWSHELL);
    SetName(C2S("Source"));
    SetWindow( &aEditWin );
    SwDocShell* pDocShell = GetDocShell();
    // wird das Doc noch geladen, dann muss die DocShell das Load
    // anwerfen, wenn das Laden abgeschlossen ist
    if(!pDocShell->IsLoading())
        Load(pDocShell);
    else
    {
        aEditWin.SetReadonly(TRUE);
    }

    StartListening(*pDocShell,TRUE);
}

SwDocShell*     SwSrcView::GetDocShell()
{
    SfxObjectShell* pObjShell = GetViewFrame()->GetObjectShell();
    return PTR_CAST(SwDocShell, pObjShell);
}

void SwSrcView::SaveContent(const String& rTmpFile)
{
    SfxMedium aMedium( rTmpFile,    STREAM_WRITE, TRUE);
    SvStream* pOutStream = aMedium.GetOutStream();
    pOutStream->SetStreamCharSet( lcl_GetStreamCharSet(eLoadEncoding) );
    aEditWin.Write(*pOutStream);//, EE_FORMAT_TEXT);
    aMedium.Commit();
}

void SwSrcView::Execute(SfxRequest& rReq)
{
    USHORT nSlot = rReq.GetSlot();
    TextView* pTextView = aEditWin.GetTextView();
    switch( nSlot )
    {
        case SID_SAVEASDOC:
        {
            SvtPathOptions aPathOpt;
            // filesave dialog with autoextension
            FileDialogHelper aDlgHelper(
                TemplateDescription::FILESAVE_AUTOEXTENSION,
                TemplateDescription::FILESAVE_SIMPLE );
            uno::Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();
            uno::Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

            // search for an html filter for export
            SfxFilterContainer* pFilterCont = GetObjectShell()->GetFactory().GetFilterContainer();
            const SfxFilter* pFilter =
                pFilterCont->GetFilter4Extension( C2S("html"), SFX_FILTER_EXPORT );
            if ( pFilter )
            {
                // filter found -> use its uiname and wildcard
                const String& rUIName = pFilter->GetUIName();
                const WildCard& rCard = pFilter->GetWildcard();
                xFltMgr->appendFilter( rUIName, rCard() );
                xFltMgr->setCurrentFilter( rUIName ) ;
            }
            else
            {
                // filter not found
                String sHtml(C2S("HTML"));
                xFltMgr->appendFilter( sHtml, C2S("*.html;*.htm") );
                xFltMgr->setCurrentFilter( sHtml ) ;
            }

            xFP->setDisplayDirectory( aPathOpt.GetWorkPath() );
            if( aDlgHelper.Execute() == ERRCODE_NONE)
            {
                SfxMedium aMedium( xFP->getFiles().getConstArray()[0],
                                    STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                    FALSE );
                SvStream* pOutStream = aMedium.GetOutStream();
                pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
                aEditWin.Write( *pOutStream );
                aMedium.Commit();
            }
        }
        break;
        case SID_SAVEDOC:
        {
            SwDocShell* pDocShell = GetDocShell();
            SfxMedium* pMed = 0;
            if(pDocShell->HasName())
                pMed = pDocShell->GetMedium();
            else
            {
                SfxBoolItem* pItem = (SfxBoolItem*)pDocShell->ExecuteSlot(rReq, pDocShell->GetInterface());
                if(pItem && pItem->GetValue())
                    pMed = pDocShell->GetMedium();
            }
            if(pMed)
            {
                SvStream* pOutStream = pMed->GetOutStream();
                pOutStream->Seek(0);
                pOutStream->SetStreamSize(0);
                pOutStream->SetStreamCharSet(lcl_GetStreamCharSet(eLoadEncoding));
                aEditWin.Write( *pOutStream );
                pMed->CloseOutStream();
                pMed->Commit();
                pDocShell->GetDoc()->ResetModified();
                SourceSaved();
                aEditWin.ClearModifyFlag();
            }
        }
        break;
        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pTmpArgs = rReq.GetArgs();

            USHORT nWhich = pTmpArgs->GetWhichByPos( 0 );
            OSL_ENSURE( nWhich, "Wich fuer SearchItem ?" );
            const SfxPoolItem& rItem = pTmpArgs->Get( nWhich );
            SetSearchItem( (const SvxSearchItem&)rItem);
            StartSearchAndReplace( (const SvxSearchItem&)rItem, FALSE, rReq.IsAPI() );
            if(aEditWin.IsModified())
                GetDocShell()->GetDoc()->SetModified();
        }
        break;
        case FN_REPEAT_SEARCH:
        {
            SvxSearchItem* pSrchItem = GetSearchItem();
            if(pSrchItem)
            {
                StartSearchAndReplace( *pSrchItem, FALSE, rReq.IsAPI() );
                if(aEditWin.IsModified())
                    GetDocShell()->GetDoc()->SetModified();
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
            GetViewFrame()->GetBindings().InvalidateAll(FALSE);
        break;
        case SID_REDO:
            pTextView->Redo();
            GetViewFrame()->GetBindings().InvalidateAll(FALSE);
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
            pTextView->SetSelection( TextSelection( TextPaM( 0, 0 ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
        break;
    }
    aEditWin.Invalidate();
}

void SwSrcView::GetState(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    TextView* pTextView = aEditWin.GetTextView();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_SAVEASDOC:
                rSet.Put(SfxStringItem(nWhich, String(SW_RES(STR_SAVEAS_SRC))));
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
                String aPos( SW_RES(STR_SRCVIEW_ROW) );
                TextSelection aSel = pTextView->GetSelection();
                aPos += String::CreateFromInt32( aSel.GetEnd().GetPara()+1 );
                aPos +=C2S(" : ");
                aPos += String(SW_RES(STR_SRCVIEW_COL));
                aPos += String::CreateFromInt32( aSel.GetEnd().GetIndex()+1 );
                SfxStringItem aItem( nWhich, aPos );
                rSet.Put( aItem );
            }
            break;
            case SID_SEARCH_OPTIONS:
            {
                USHORT nOpt = SRC_SEARCHOPTIONS;
                if(GetDocShell()->IsReadOnly())
                    nOpt &= ~(SEARCH_OPTIONS_REPLACE|SEARCH_OPTIONS_REPLACE_ALL);

                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS,  nOpt) );
            }
            break;
            case SID_SEARCH_ITEM:
            {
                String sSelected;
                if ( !pTextView->HasSelection() )
                {
                    const TextSelection& rSel = pTextView->GetSelection();
                    sSelected = aEditWin.GetTextEngine()->GetWord( rSel.GetStart());
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
            };
            break;
            case SID_UNDO:
            case SID_REDO:
            {
                SfxUndoManager& rMgr = pTextView->GetTextEngine()->GetUndoManager();
                USHORT nCount = 0;
                if(nWhich == SID_UNDO)
                {
                    nCount = rMgr.GetUndoActionCount();
                    if(nCount)
                    {
                        String aStr(ResId( STR_UNDO, *SFX_APP()->GetSfxResManager() ));;
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
                        String aStr(ResId( STR_REDO, *SFX_APP()->GetSfxResManager() ));;
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
                                                        &aEditWin) );
                BOOL bDisable = !aDataHelper.GetXTransferable().is() ||
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
        pSearchItem = new SvxSearchItem(SID_SEARCH_ITEM);
    }
    return pSearchItem;
}

void SwSrcView::SetSearchItem( const SvxSearchItem& rItem )
{
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rItem.Clone();
}

USHORT SwSrcView::StartSearchAndReplace(const SvxSearchItem& rSearchItem,
                                                    BOOL bFromStart,
                                                    BOOL bApi,
                                                    BOOL bRecursive)
{
    ExtTextView* pTextView = aEditWin.GetTextView();
    TextSelection aSel;
    TextPaM aPaM;

    BOOL bForward = !rSearchItem.GetBackward();
    BOOL bAtStart = pTextView->GetSelection() == TextSelection( aPaM, aPaM );

    if( !bForward )
        aPaM = TextPaM( (ULONG)-1, (USHORT)-1 );

    if( bFromStart )
    {
        aSel = pTextView->GetSelection();
        pTextView->SetSelection( TextSelection( aPaM, aPaM ));
    }

    util::SearchOptions aSearchOpt( rSearchItem.GetSearchOptions() );
    aSearchOpt.Locale = SvxCreateLocale(
        static_cast< LanguageType >( GetAppLanguage() ) );

    USHORT nFound;
    BOOL bAll = FALSE;
    switch( rSearchItem.GetCommand() )
    {
    case SVX_SEARCHCMD_FIND:
    case SVX_SEARCHCMD_FIND_ALL:
        nFound = pTextView->Search( aSearchOpt, bForward );
        break;

    case SVX_SEARCHCMD_REPLACE_ALL: bAll = TRUE;
    case SVX_SEARCHCMD_REPLACE:
        nFound = pTextView->Replace( aSearchOpt, bAll, bForward );
        break;

    default:
        nFound = 0;
    }

    if( !nFound )
    {
        BOOL bNotFoundMessage = FALSE;
        if(!bRecursive)
        {
            if(!bFromStart)
            {
                bNotFoundMessage = bAtStart;
            }
            else
            {
                bNotFoundMessage = TRUE;
                pTextView->SetSelection( aSel );
            }
        }
        else if(bAtStart)
        {
            bNotFoundMessage = TRUE;
        }


        if(!bApi)
        {
            if(bNotFoundMessage)
            {
                InfoBox( 0, SW_RES(MSG_NOT_FOUND)).Execute();
            }
            else if(!bRecursive && RET_YES ==
                QueryBox(0, SW_RES( bForward ? MSG_SEARCH_END
                                             : MSG_SEARCH_START)).Execute())
            {
                pTextView->SetSelection( TextSelection( aPaM, aPaM ) );
                StartSearchAndReplace( rSearchItem, FALSE, FALSE, TRUE );
            }
        }
    }
    return nFound;
}

USHORT SwSrcView::SetPrinter(SfxPrinter* pNew, USHORT nDiffFlags, bool )
{
    SwDocShell* pDocSh = GetDocShell();
    if ( (SFX_PRINTER_JOBSETUP | SFX_PRINTER_PRINTER) & nDiffFlags )
    {
        pDocSh->GetDoc()->setPrinter( pNew, true, true );
        if ( nDiffFlags & SFX_PRINTER_PRINTER )
            pDocSh->SetModified();
    }
    if ( nDiffFlags & SFX_PRINTER_OPTIONS )
        ::SetPrinter( pDocSh->getIDocumentDeviceAccess(), pNew, TRUE );

    const BOOL bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ? TRUE : FALSE;
    const BOOL bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE ? TRUE : FALSE;
    if ( bChgOri || bChgSize )
    {
        pDocSh->SetModified();
    }
    return 0;
}

SfxPrinter* SwSrcView::GetPrinter( BOOL bCreate )
{
    return  GetDocShell()->GetDoc()->getPrinter( bCreate );
}

sal_Int32 SwSrcView::PrintSource(
    OutputDevice *pOutDev,
    sal_Int32 nPage,
    bool bCalcNumPagesOnly )
{
    if (!pOutDev || nPage <= 0)
        return 0;

    //! this a lgorithm for printing the n-th page is very poor since it
    //! needs to go over the text of all previous pages to get to the correct one.
    //! But since HTML source code is expected to be just a small number of pages
    //! even this poor algorithm should be enough...

    pOutDev->Push();

    TextEngine* pTextEngine = aEditWin.GetTextEngine();
    pOutDev->SetMapMode( MAP_100TH_MM );
    Font aFont( aEditWin.GetOutWin()->GetFont() );
    Size aSize( aFont.GetSize() );
    aSize = aEditWin.GetOutWin()->PixelToLogic( aSize, MAP_100TH_MM );
    aFont.SetSize( aSize );
    aFont.SetColor( COL_BLACK );
    pOutDev->SetFont( aFont );

    String aTitle( GetViewFrame()->GetWindow().GetText() );

    USHORT nLineHeight = (USHORT) pOutDev->GetTextHeight(); // etwas mehr.
    USHORT nParaSpace = 10;

    Size aPaperSz = pOutDev->GetOutputSize();
    aPaperSz.Width() -= (LMARGPRN + RMARGPRN);
    aPaperSz.Height() -= (TMARGPRN + BMARGPRN);

    // nLinepPage stimmt nicht, wenn Zeilen umgebrochen werden muessen...
    USHORT nLinespPage = (USHORT) (aPaperSz.Height() / nLineHeight);
    USHORT nCharspLine = (USHORT) (aPaperSz.Width()  / pOutDev->GetTextWidth( 'X' ));
    USHORT nParas = static_cast< USHORT >( pTextEngine->GetParagraphCount() );

    USHORT nPages = (USHORT) (nParas / nLinespPage + 1 );
    USHORT nCurPage = 1;

    // Header drucken...
    if (!bCalcNumPagesOnly && nPage == nCurPage)
        lcl_PrintHeader( *pOutDev, nPages, nCurPage, aTitle );
    const Point aStartPos( LMARGPRN, TMARGPRN );
    Point aPos( aStartPos );
    for ( USHORT nPara = 0; nPara < nParas; ++nPara )
    {
        String aLine( pTextEngine->GetText( nPara ) );
        lcl_ConvertTabsToSpaces( aLine );
        USHORT nLines = aLine.Len() / nCharspLine + 1;
        for ( USHORT nLine = 0; nLine < nLines; ++nLine )
        {
            String aTmpLine( aLine, nLine * nCharspLine, nCharspLine );
            aPos.Y() += nLineHeight;
            if ( aPos.Y() > ( aPaperSz.Height() + TMARGPRN - nLineHeight/2 ) )
            {
                ++nCurPage;
                if (!bCalcNumPagesOnly && nPage == nCurPage)
                    lcl_PrintHeader( *pOutDev, nPages, nCurPage, aTitle );
                aPos = aStartPos;
            }
            if (!bCalcNumPagesOnly && nPage == nCurPage)
                pOutDev->DrawText( aPos, aTmpLine );
        }
        aPos.Y() += nParaSpace;
    }

    pOutDev->Pop();

    OSL_ENSURE( bCalcNumPagesOnly || nPage <= nCurPage, "page number out of range" );
    return nCurPage;
}

void SwSrcView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) &&
            (
                ((SfxSimpleHint&) rHint).GetId() == SFX_HINT_MODECHANGED ||
                (
                    ((SfxSimpleHint&) rHint).GetId() == SFX_HINT_TITLECHANGED &&
                    !GetDocShell()->IsReadOnly() && aEditWin.IsReadonly()
                )
            )
       )
    {
        // Broadcast kommt nur einmal!
        const SwDocShell* pDocSh = GetDocShell();
        const BOOL bReadonly = pDocSh->IsReadOnly();
        aEditWin.SetReadonly(bReadonly);
    }
    SfxViewShell::Notify(rBC, rHint);
}

void SwSrcView::Load(SwDocShell* pDocShell)
{
    SvxHtmlOptions* pHtmlOptions = SvxHtmlOptions::Get();
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( pHtmlOptions->GetTextEncoding() );
    rtl_TextEncoding eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );

    aEditWin.SetReadonly(pDocShell->IsReadOnly());
    aEditWin.SetTextEncoding(eDestEnc);
    SfxMedium* pMedium = pDocShell->GetMedium();

    const SfxFilter* pFilter = pMedium->GetFilter();
    BOOL bHtml = pFilter && pFilter->GetUserData().EqualsAscii("HTML");
    BOOL bDocModified = pDocShell->IsModified();
    if(bHtml && !bDocModified && pDocShell->HasName())
    {
        SvStream* pStream = pMedium->GetInStream();
        if(pStream && 0 == pStream->GetError() )
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
                aEditWin.SetTextEncoding(eDestEnc);
            }
            pStream->SetStreamCharSet( eDestEnc );
            pStream->Seek(0);
            TextEngine* pTextEngine = aEditWin.GetTextEngine();
            pTextEngine->EnableUndo(FALSE);
            aEditWin.Read(*pStream);//, EE_FORMAT_TEXT);
            pTextEngine->EnableUndo(TRUE);
        }
        else
        {
            Window *pTmpWindow = &GetViewFrame()->GetWindow();
            InfoBox(pTmpWindow, SW_RES(MSG_ERR_SRCSTREAM)).Execute();
        }
    }
    else
    {
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        String sFileURL( aTempFile.GetURL() );
        SvtSaveOptions aOpt;

        {
            SfxMedium aMedium( sFileURL,STREAM_READWRITE, TRUE );
            SwWriter aWriter( aMedium, *pDocShell->GetDoc() );
            WriterRef xWriter;
            ::GetHTMLWriter(aEmptyStr, aMedium.GetBaseURL( true ), xWriter);
            String sWriteName = pDocShell->HasName() ?
                                    pMedium->GetName() :
                                        (const String&) sFileURL;
            ULONG nRes = aWriter.Write(xWriter, &sWriteName);
            if(nRes)
            {
                ErrorHandler::HandleError(ErrCode(nRes));
                aEditWin.SetReadonly(TRUE);
            }
            aMedium.Commit();
            SvStream* pInStream = aMedium.GetInStream();
            pInStream->Seek(0);
            pInStream->SetStreamCharSet( eDestEnc );


            aEditWin.Read(*pInStream);//, EE_FORMAT_TEXT);
        }
    }
    aEditWin.ClearModifyFlag();

    eLoadEncoding = eDestEnc;

    if(bDocModified)
        pDocShell->SetModified();// das Flag wird zwischendurch zurueckgesetzt
    // AutoLoad abschalten
    pDocShell->SetAutoLoad(INetURLObject(), 0, FALSE);
    OSL_ENSURE(PTR_CAST(SwWebDocShell, pDocShell), "Wieso keine WebDocShell?");
    USHORT nLine = ((SwWebDocShell*)pDocShell)->GetSourcePara();
    aEditWin.SetStartLine(nLine);
    aEditWin.GetTextEngine()->ResetUndo();
    aEditWin.GetOutWin()->GrabFocus();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
