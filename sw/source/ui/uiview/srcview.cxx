/*************************************************************************
 *
 *  $RCSfile: srcview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-15 14:55:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif


#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif
#ifndef _EXTATTR_HXX //autogen
#include <svtools/extattr.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _IODLG_HXX //autogen
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_PRNMON_HXX //autogen
#include <sfx2/prnmon.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _SRCVCFG_HXX
#include <srcvcfg.hxx>
#endif
#ifndef _VIEWFUNC_HXX
#include <viewfunc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DATAEX_HXX
#include <dataex.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>          // FN_       ...
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _WEB_HRC
#include <web.hrc>
#endif
#ifndef _VIEW_HRC
#include <view.hrc>
#endif


#define SwSrcView
#define SearchSettings
#define _ExecSearch Execute
#define _StateSearch GetState
#include <svx/svxslots.hxx>
#include "swslots.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;

#define C2S(cChar) UniString::CreateFromAscii(cChar)

#define SWSRCVIEWFLAGS ( SFX_VIEW_MAXIMIZE_FIRST|           \
                      SFX_VIEW_OBJECTSIZE_EMBEDDED|     \
                      SFX_VIEW_CAN_PRINT|\
                      SFX_VIEW_NO_NEWWINDOW )

#define SRC_SEARCHOPTIONS (0xFFFF & ~(SEARCH_OPTIONS_FORMAT|SEARCH_OPTIONS_FAMILIES|SEARCH_OPTIONS_SEARCH_ALL))

// Druckraender -> wie Basic - Ide
#define LMARGPRN        1700
#define RMARGPRN         900
#define TMARGPRN        2000
#define BMARGPRN        1000
#define BORDERPRN       300



SFX_IMPL_VIEWFACTORY(SwSrcView, SW_RES(STR_NONAME))
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}


SFX_IMPL_INTERFACE( SwSrcView, SfxViewShell, SW_RES(0) )
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_SRCVIEW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_SRCVIEW_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_WEBTOOLS_TOOLBOX) );
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
}

TYPEINIT1(SwSrcView, SfxViewShell)

/*-----------------18.11.96 08.05-------------------

--------------------------------------------------*/


void lcl_PrintHeader( Printer* pPrinter, USHORT nPages, USHORT nCurPage, const String& rTitle )
{
    short nLeftMargin   = LMARGPRN;
    Size aSz = pPrinter->GetOutputSize();
    short nBorder = BORDERPRN;

    Color aOldFillColor( pPrinter->GetFillColor() );
    Font aOldFont( pPrinter->GetFont() );

    pPrinter->SetFillColor( Color(COL_TRANSPARENT) );

    Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );
    aFont.SetAlign( ALIGN_BOTTOM );
    pPrinter->SetFont( aFont );

    long nFontHeight = pPrinter->GetTextHeight();

    // 1.Border => Strich, 2+3 Border = Freiraum.
    long nYTop = TMARGPRN-3*nBorder-nFontHeight;

    long nXLeft = nLeftMargin-nBorder;
    long nXRight = aSz.Width()-RMARGPRN+nBorder;

    pPrinter->DrawRect( Rectangle(
        Point( nXLeft, nYTop ),
        Size( nXRight-nXLeft, aSz.Height() - nYTop - BMARGPRN + nBorder ) ) );


    long nY = TMARGPRN-2*nBorder;
    Point aPos( nLeftMargin, nY );
    pPrinter->DrawText( aPos, rTitle );
    if ( nPages != 1 )
    {
        aFont.SetWeight( WEIGHT_NORMAL );
        pPrinter->SetFont( aFont );
        String aPageStr( C2S(" [") );
        aPageStr += String( SW_RES( STR_PAGE ) );
        aPageStr += ' ';
        aPageStr += String::CreateFromInt32( nCurPage );
        aPageStr += ']';
        aPos.X() += pPrinter->GetTextWidth( rTitle );
        pPrinter->DrawText( aPos, aPageStr );
    }


    nY = TMARGPRN-nBorder;

    pPrinter->DrawLine( Point( nXLeft, nY ), Point( nXRight, nY ) );

    pPrinter->SetFont( aOldFont );
    pPrinter->SetFillColor( aOldFillColor );
}

/*-----------------18.11.96 08.21-------------------

--------------------------------------------------*/

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

/*-----------------18.11.96 12.12-------------------

--------------------------------------------------*/

void lcl_SetFont(SwSrcView* pView, const String& rStyle)
{
    const SvxFontListItem* pFontListItem = (const SvxFontListItem* )pView->GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST );
    const FontList*  pList = pFontListItem->GetFontList();

    FontInfo aInfo = pList->Get(rStyle,WEIGHT_NORMAL, ITALIC_NONE);

    SwSrcEditWindow& rEditWin = pView->GetEditWin();
    const Font& rFont = rEditWin.GetTextEngine()->GetFont();
    Font aFont(aInfo);
    aFont.SetSize(rFont.GetSize());
    rEditWin.GetTextEngine()->SetFont( aFont );
    rEditWin.GetOutWin()->SetFont(aFont);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwSrcView::SwSrcView(SfxViewFrame* pFrame, const SwSrcView&) :
    SfxViewShell( pFrame, SWSRCVIEWFLAGS ),
    aEditWin( &pFrame->GetWindow(), this ),
    pSearchItem(0),
    pSrcViewConfig(0),
    bSourceSaved(FALSE)
{
    Init();
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwSrcView::SwSrcView(SfxViewFrame* pFrame, SfxViewShell*) :
    SfxViewShell( pFrame, SWSRCVIEWFLAGS ),
    aEditWin( &pFrame->GetWindow(), this ),
    pSearchItem(0),
    bSourceSaved(FALSE)
{
    Init();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwSrcView::~SwSrcView()
{
    SwDocShell* pDocShell = GetDocShell();
    DBG_ASSERT(PTR_CAST(SwWebDocShell, pDocShell), "Wieso keine WebDocShell?")
    TextEngine* pTextEngine = aEditWin.GetTextEngine();
    const TextSelection&  rSel = aEditWin.GetTextView()->GetSelection();
    ((SwWebDocShell*)pDocShell)->SetSourcePara(rSel.GetStart().GetPara());


    SfxDocumentInfo& rDocInfo = pDocShell->GetDocInfo();
    pDocShell->SetAutoLoad(rDocInfo.GetReloadURL(),
                                rDocInfo.GetReloadDelay(),
                                rDocInfo.IsReloadEnabled() );
//  EndListening(*GetViewFrame());
    EndListening(*pDocShell);
    delete pSearchItem;
}

/*-----------------24.04.97 10:37-------------------

--------------------------------------------------*/
void SwSrcView::SaveContentTo(SfxMedium& rMed)
{
    aEditWin.Write( *rMed.GetInStream());//, EE_FORMAT_TEXT);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcView::Init()
{
    SetHelpId(SW_SRC_VIEWSHELL);
    SetName(C2S("Source"));
    SetWindow( &aEditWin );
    pSrcViewConfig = SW_MOD()->GetSourceViewConfig();
    if(pSrcViewConfig->GetFontName().getLength())
        lcl_SetFont( this, pSrcViewConfig->GetFontName());
    TextViewOutWin* pOutWin = aEditWin.GetOutWin();
    Font aFont(aEditWin.GetTextEngine()->GetFont());
    Size aSize(aFont.GetSize());
    aSize.Height() = pSrcViewConfig->GetFontHeight();
    aFont.SetSize(pOutWin->LogicToPixel(aSize, MAP_TWIP));
    aEditWin.GetTextEngine()->SetFont(aFont);
    pOutWin->SetFont(aFont);
    SwDocShell* pDocShell = GetDocShell();
    // wird das Doc noch geladen, dann muss die DocShell das Load
    // anwerfen, wenn das Laden abgeschlossen ist
    if(!pDocShell->IsLoading())
        Load(pDocShell);
    else
    {
        aEditWin.SetReadonly(TRUE);
    }

//  StartListening(*GetViewFrame());
    StartListening(*pDocShell);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwDocShell*     SwSrcView::GetDocShell()
{
    SfxObjectShell* pObjShell = GetViewFrame()->GetObjectShell();
    return PTR_CAST(SwDocShell, pObjShell);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcView::SaveContent(const String& rTmpFile)
{
    SfxMedium aMedium( rTmpFile,    STREAM_WRITE, TRUE);
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( gsl_getSystemTextEncoding() );
    rtl_TextEncoding eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
    SvStream* pOutStream = aMedium.GetOutStream();
    pOutStream->SetStreamCharSet( eDestEnc );
    aEditWin.Write(*pOutStream);//, EE_FORMAT_TEXT);
    aMedium.Commit();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcView::Execute(SfxRequest& rReq)
{
    USHORT nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    TextView* pTextView = aEditWin.GetTextView();
    switch( nSlot )
    {
        case SID_ATTR_CHAR_FONT:
        {
            const SvxFontItem& rFontItem = (const SvxFontItem&)pArgs->Get(RES_CHRATR_FONT);
            String sStyle = rFontItem.GetFamilyName();
            lcl_SetFont( this, sStyle );
            pSrcViewConfig->SetFontName(sStyle);
        }
        break;
        case SID_ATTR_CHAR_FONTHEIGHT:
        {
            TextViewOutWin* pOutWin = aEditWin.GetOutWin();

            const SvxFontHeightItem& rFontHeight = (const SvxFontHeightItem&)pArgs->Get(RES_CHRATR_FONTSIZE);
            Font aFont(aEditWin.GetTextEngine()->GetFont());
            Size aSize(aFont.GetSize());
            aSize.Height() = rFontHeight.GetHeight();
            aFont.SetSize(pOutWin->LogicToPixel(aSize, MAP_TWIP));
            aEditWin.GetTextEngine()->SetFont(aFont);
            pSrcViewConfig->SetFontHeight(USHORT(rFontHeight.GetHeight()));
        }
        break;
        case SID_SAVEASDOC:
        {
            SvtPathOptions aPathOpt;
            Window* pParent = &GetViewFrame()->GetWindow();
            SfxFileDialog* pFileDlg = new SfxFileDialog(pParent, WB_SAVEAS|WB_3DLOOK);
            pFileDlg->DisableSaveLastDirectory();
            pFileDlg->SetHelpId(HID_FILEDLG_SRCVIEW);
            String sHtml(C2S("HTML"));
            pFileDlg->AddFilter( sHtml, C2S("*.html;*.htm") );
            pFileDlg->SetCurFilter( sHtml );
            pFileDlg->SetPath( aPathOpt.GetWorkPath() );
            if( RET_OK == pFileDlg->Execute())
            {
                SfxMedium aMedium( pFileDlg->GetPath(),
                                    STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                    FALSE );
#ifdef USED
                // set the filter for the correct handling of
                // extended attributes
                SfxObjectFactory& rFac = GetDocShell()->GetFactory();
                for( USHORT i = 0; i < rFac.GetFilterCount(); i++ )
                {
                    const SfxFilter* pFlt = rFac.GetFilter( i );
                    if( pFlt && pFlt->GetFilterName() == sHtml )
                    {
                        aMedium.SetFilter( pFlt );
                        break;
                    }
                }
#endif
                aEditWin.Write( *aMedium.GetOutStream() );
                aMedium.Commit();
            }
            delete pFileDlg;
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
                SvStream* pInStream = pMed->GetInStream();
                pInStream->Seek(0);
                pInStream->SetStreamSize(0);
                aEditWin.Write( *pInStream );
                pMed->Commit();
                pDocShell->GetDoc()->ResetModified();
                SourceSaved();
                aEditWin.ClearModifyFlag();
            }
        }
        break;
        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            USHORT nWhich = pArgs->GetWhichByPos( 0 );
            DBG_ASSERT( nWhich, "Wich fuer SearchItem ?" );
            const SfxPoolItem& rItem = pArgs->Get( nWhich );
            SetSearchItem( (const SvxSearchItem&)rItem);
            StartSearchAndReplace( (const SvxSearchItem&)rItem, FALSE, rReq.IsAPI() );
            if(aEditWin.IsModified())
                GetDocShell()->GetDoc()->SetModified();
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
        break;
        case SID_REDO:
            pTextView->Redo();
        break;
        case SID_REPEAT:
        break;
        case SID_CUT:
            pTextView->Cut();
        break;
        case SID_COPY:
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


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
            case RES_CHRATR_FONT:
            {
                Font aFont = aEditWin.GetTextEngine()->GetFont();
                rSet.Put(SvxFontItem(
                            aFont.GetFamily(),
                            aFont.GetName(),
                            aFont.GetStyleName()));
            }
            break;
            case RES_CHRATR_FONTSIZE:
            {
                TextViewOutWin* pOutWin = aEditWin.GetOutWin();
                SvxFontHeightItem aFontHeight;
                Size aTemp(aEditWin.GetTextEngine()->GetFont().GetSize());
                aTemp = pOutWin->PixelToLogic(aTemp, MAP_TWIP);
                aFontHeight.SetHeight(aTemp.Height());
                aFontHeight.SetWhich(RES_CHRATR_FONTSIZE);
                rSet.Put(aFontHeight);
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
                        String aStr(ResId( STR_UNDO, SFX_APP()->GetSfxResManager() ));;
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
                        String aStr(ResId( STR_REDO, SFX_APP()->GetSfxResManager() ));;
                        aStr += rMgr.GetRedoActionComment(--nCount);
                        rSet.Put(SfxStringItem(nWhich,aStr));
                    }
                    else
                        rSet.DisableItem(nWhich);
                }
            }
            break;
            case SID_REPEAT:
                rSet.DisableItem(nWhich);
            break;
            case SID_CUT:
            case SID_COPY:
                if(!pTextView->HasSelection())
                    rSet.DisableItem(nWhich);
            break;
            case SID_PASTE:
            {
                SvDataObjectRef xObj;
                if ( Clipboard::GetFormatCount() )
                {
                    SwModule* pMod = SW_MOD();
                    if ( pMod->pClipboard )
                        xObj = pMod->pClipboard ;
                    else
                        xObj = SvDataObject::PasteClipboard();
                }
                if(!xObj.Is())
                    rSet.DisableItem(nWhich);
            }
            break;

        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SvxSearchItem* SwSrcView::GetSearchItem()
{
    if(!pSearchItem)
    {
        pSearchItem = new SvxSearchItem(SID_SEARCH_ITEM);
    }
    return pSearchItem;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcView::SetSearchItem( const SvxSearchItem& rItem )
{
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rItem.Clone();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


USHORT SwSrcView::StartSearchAndReplace(const SvxSearchItem& rSearchItem,
                                                    BOOL bFromStart,
                                                    BOOL bApi,
                                                    BOOL bRecursive)
{
    ExtTextView* pTextView = aEditWin.GetTextView();
    TextSelection aSel;
    BOOL bForward = !rSearchItem.GetBackward();
    BOOL bAtStart = pTextView->GetSelection() == TextSelection( TextPaM( 0x0, 0x0 ), TextPaM( 0x0, 0x0 ) );
    if ( bFromStart )
    {
        aSel = pTextView->GetSelection();
        if ( !rSearchItem.GetBackward() )
            pTextView->SetSelection( TextSelection( TextPaM( 0x0, 0x0 ), TextPaM( 0x0, 0x0 ) ) );
        else
            pTextView->SetSelection( TextSelection( TextPaM( 0xFFFFFFFF, 0xFFFF ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
    }

    SearchParam aSearchParam( rSearchItem.GetSearchString(),
                        SearchParam::SRCH_NORMAL, rSearchItem.GetExact(),
                        rSearchItem.GetWordOnly(), rSearchItem.GetSelection() );
    if ( rSearchItem.GetRegExp() )
        aSearchParam.SetSrchType( SearchParam::SRCH_REGEXP );
    else if ( rSearchItem.IsLevenshtein() )
    {
        aSearchParam.SetSrchType( SearchParam::SRCH_LEVDIST );
        aSearchParam.SetSrchRelaxed( rSearchItem.IsLEVRelaxed() ? TRUE : FALSE );
        aSearchParam.SetLEVOther( rSearchItem.GetLEVOther() );
        aSearchParam.SetLEVShorter( rSearchItem.GetLEVShorter() );
        aSearchParam.SetLEVLonger( rSearchItem.GetLEVLonger() );
    }
    USHORT nFound = 0;
    if ( ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND ) ||
         ( rSearchItem.GetCommand() == SVX_SEARCHCMD_FIND_ALL ) )
    {
        nFound = pTextView->Search( aSearchParam, bForward );
    }
    else if ( ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE ) ||
              ( rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL ) )
    {
        aSearchParam.SetReplaceStr( rSearchItem.GetReplaceString() );
        BOOL bAll = rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL;
        nFound = pTextView->Replace( aSearchParam, bAll, bForward );
    }
    if(!nFound)
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
            if(bNotFoundMessage)
            {
                InfoBox( 0, SW_RES(MSG_NOT_FOUND)).Execute();
            }
            else if(!bRecursive && RET_YES ==
                QueryBox(0, SW_RES( bForward ? MSG_SEARCH_END
                                             : MSG_SEARCH_START)).Execute())
            {
                if ( bForward )
                    pTextView->SetSelection( TextSelection( TextPaM( 0x0, 0x0 ), TextPaM( 0x0, 0x0 ) ) );
                else
                    pTextView->SetSelection( TextSelection( TextPaM( 0xFFFFFFFF, 0xFFFF ), TextPaM( 0xFFFFFFFF, 0xFFFF ) ) );
                StartSearchAndReplace(rSearchItem, FALSE, FALSE, TRUE);
            }
    }
    return nFound;
}

/*-----------------02.07.97 09:29-------------------

--------------------------------------------------*/
USHORT SwSrcView::SetPrinter(SfxPrinter* pNew, USHORT nDiffFlags )
{
    SwDocShell* pDocSh = GetDocShell();
    if ( (SFX_PRINTER_JOBSETUP | SFX_PRINTER_PRINTER) & nDiffFlags )
    {
        pDocSh->GetDoc()->SetPrt( pNew );
        if ( nDiffFlags & SFX_PRINTER_PRINTER )
            pDocSh->SetModified();
    }
    if ( nDiffFlags & SFX_PRINTER_OPTIONS )
        ::SetPrinter( pNew, TRUE );

    const BOOL bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ? TRUE : FALSE;
    const BOOL bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE ? TRUE : FALSE;
    if ( bChgOri || bChgSize )
    {
        pDocSh->SetModified();
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ErrCode SwSrcView::DoPrint( SfxPrinter *pPrinter, PrintDialog *pDlg,
                            BOOL bSilent )
{
    SfxPrintProgress *pProgress = new SfxPrintProgress( this, !bSilent );
    SfxPrinter *pDocPrinter = GetPrinter(TRUE);
    if ( !pPrinter )
        pPrinter = pDocPrinter;
    else if ( pDocPrinter != pPrinter )
    {
        pProgress->RestoreOnEndPrint( pDocPrinter->Clone() );
        SetPrinter( pPrinter, SFX_PRINTER_PRINTER );
    }
    pProgress->SetWaitMode(FALSE);

    // Drucker starten
    PreparePrint( pDlg );
    SfxObjectShell *pObjShell = GetViewFrame()->GetObjectShell();

    SfxViewShell::Print(*pProgress, pDlg); //???

    MapMode eOldMapMode( pPrinter->GetMapMode() );
    Font aOldFont( pPrinter->Printer::GetFont() );

    TextEngine* pTextEngine = aEditWin.GetTextEngine();
    pPrinter->SetMapMode(MAP_100TH_MM);
    Font aFont(aEditWin.GetOutWin()->GetFont());
    Size aSize(aFont.GetSize());
    aSize = aEditWin.GetOutWin()->PixelToLogic(aSize, MAP_100TH_MM);
    aFont.SetSize(aSize);
    aFont.SetColor(COL_BLACK);
    pPrinter->SetFont( aFont );

    String aTitle( GetViewFrame()->GetWindow().GetText() );

    USHORT nLineHeight = (USHORT) pPrinter->GetTextHeight(); // etwas mehr.
    USHORT nParaSpace = 10;

    Size aPaperSz = pPrinter->GetOutputSize();
    aPaperSz.Width() -= (LMARGPRN+RMARGPRN);
    aPaperSz.Height() -= (TMARGPRN+BMARGPRN);

    // nLinepPage stimmt nicht, wenn Zeilen umgebrochen werden muessen...
    USHORT nLinespPage = (USHORT) (aPaperSz.Height()/nLineHeight);
    USHORT nCharspLine = (USHORT) (aPaperSz.Width() / pPrinter->GetTextWidth( 'X' ));
    USHORT nParas = pTextEngine->GetParagraphCount();

    USHORT nPages = (USHORT) (nParas/nLinespPage+1 );
    USHORT nCurPage = 1;

    BOOL bStartJob = pPrinter->StartJob( aTitle );
    if( bStartJob )
    {
        pPrinter->StartPage();
        // Header drucken...
        lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle );
        Point aPos( LMARGPRN, TMARGPRN );
        for ( USHORT nPara = 0; nPara < nParas; nPara++ )
        {
            String aLine( pTextEngine->GetText( nPara ) );
            lcl_ConvertTabsToSpaces( aLine );
            USHORT nLines = aLine.Len()/nCharspLine+1;
            for ( USHORT nLine = 0; nLine < nLines; nLine++ )
            {
                String aTmpLine( aLine, nLine*nCharspLine, nCharspLine );
                aPos.Y() += nLineHeight;
                if ( aPos.Y() > ( aPaperSz.Height()+TMARGPRN ) )
                {
                    nCurPage++;
                    pPrinter->EndPage();
                    pPrinter->StartPage();
                    lcl_PrintHeader( pPrinter, nPages, nCurPage, aTitle );
                    aPos = Point( LMARGPRN, TMARGPRN+nLineHeight );
                }
                pPrinter->DrawText( aPos, aTmpLine );
            }
            aPos.Y() += nParaSpace;
        }
        pPrinter->EndPage();
    }

    pPrinter->SetFont( aOldFont );
    pPrinter->SetMapMode( eOldMapMode );

    if ( !bStartJob )
    {
        // Printer konnte nicht gestartet werden
        delete pProgress;
        return ERRCODE_IO_ABORT;
    }

    pProgress->Stop();
    pProgress->DeleteOnEndPrint();
    pPrinter->EndJob();
    return pPrinter->GetError();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SfxPrinter* SwSrcView::GetPrinter( BOOL bCreate )
{
    return  GetDocShell()->GetDoc()->GetPrt( bCreate );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwSrcView::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA(SfxSimpleHint) &&
            (((SfxSimpleHint&) rHint).GetId() == SFX_HINT_MODECHANGED) ||
            (((SfxSimpleHint&) rHint).GetId() == SFX_HINT_TITLECHANGED &&
               !GetDocShell()->IsReadOnly() && aEditWin.IsReadonly()))
    {
        // Broadcast kommt nur einmal!
        const SwDocShell* pDocSh = GetDocShell();
        const BOOL bReadonly = pDocSh->IsReadOnly();
        aEditWin.SetReadonly(bReadonly);
    }
    SfxViewShell::Notify(rBC, rHint);
}

/*-----------------19.04.97 10:19-------------------

--------------------------------------------------*/
void SwSrcView::Load(SwDocShell* pDocShell)
{
    const sal_Char *pCharSet =
        rtl_getBestMimeCharsetFromTextEncoding( gsl_getSystemTextEncoding() );
    rtl_TextEncoding eDestEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
    aEditWin.SetReadonly(pDocShell->IsReadOnly());
    SfxMedium* pMedium = pDocShell->GetMedium();

    const SfxFilter* pFilter = pMedium->GetFilter();
    BOOL bHtml = pFilter && C2S("HTML") == pFilter->GetFilterName();
    BOOL bDocModified = pDocShell->IsModified();
    if(bHtml && !bDocModified && pDocShell->HasName())
    {
        SvStream* pStream = pMedium->GetInStream();
        if(pStream && 0 == pStream->GetError() )
        {
            pStream->SetStreamCharSet( eDestEnc );
            pStream->Seek(0);

            aEditWin.Read(*pStream);//, EE_FORMAT_TEXT);
        }
        else
        {
            Window *pWindow = &GetViewFrame()->GetWindow();
            InfoBox(pWindow, SW_RES(MSG_ERR_SRCSTREAM)).Execute();
        }
    }
    else
    {
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        String sFileURL( aTempFile.GetFileName() ),
               sBaseURL( INetURLObject::GetBaseURL() );
        BOOL bIsRemote = pMedium->IsRemote();
        SvtSaveOptions aOpt;

        if( bIsRemote ? aOpt.IsSaveRelINet() : aOpt.IsSaveRelFSys() )
            INetURLObject::SetBaseURL( pMedium->GetName() );
        else
            INetURLObject::SetBaseURL( aEmptyStr );

        {
            SfxMedium aMedium( sFileURL,STREAM_READWRITE, TRUE );
            SwWriter aWriter( aMedium, *pDocShell->GetDoc() );
            WriterRef xWriter;
            ::GetHTMLWriter(aEmptyStr, xWriter);
            String sWriteName = pDocShell->HasName() ?
                                    pMedium->GetName() :
                                        (const String&) sFileURL;
            ULONG nRes = aWriter.Write(xWriter, &sWriteName);
            if(nRes)
            {
                ErrorHandler::HandleError(ErrCode(nRes));
                aEditWin.SetReadonly(TRUE);
            }
            SvStream* pInStream = aMedium.GetInStream();
            pInStream->Seek(0);
            pInStream->SetStreamCharSet( eDestEnc );

            INetURLObject::SetBaseURL(sBaseURL);

            aEditWin.Read(*pInStream);//, EE_FORMAT_TEXT);
        }
    }
    aEditWin.ClearModifyFlag();

    if(bDocModified)
        pDocShell->SetModified();// das Flag wird zwischendurch zurueckgesetzt
    // AutoLoad abschalten
    pDocShell->SetAutoLoad(INetURLObject(), 0, FALSE);
    DBG_ASSERT(PTR_CAST(SwWebDocShell, pDocShell), "Wieso keine WebDocShell?")
    USHORT nLine = ((SwWebDocShell*)pDocShell)->GetSourcePara();
    aEditWin.SetStartLine(nLine);
    aEditWin.GetTextEngine()->ResetUndo();
    aEditWin.GetOutWin()->GrabFocus();
}


