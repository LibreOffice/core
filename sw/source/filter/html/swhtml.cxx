/*************************************************************************
 *
 *  $RCSfile: swhtml.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-13 10:42:42 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef PRODUCT
#include <stdlib.h>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif
#ifndef _FSETOBJSH_HXX //autogen
#include <sfx2/fsetobsh.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif

#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif


#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark ...
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>       // fuer Start/EndAction
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // fuer SwObjectDying
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _SWCSS1_HXX
#include <swcss1.hxx>
#endif
#ifndef _HTMLVSH_HXX
#include <htmlvsh.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _HTMLNUM_HXX
#include <htmlnum.hxx>
#endif
#ifndef _SWHTML_HXX
#include <swhtml.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

#define FONTSIZE_MASK           7
#define FONTCOLOR_MASK          (1<<15)
#define FONT_MASK               (1<<14)

#define HTML_ESC_PROP 80
#define HTML_ESC_SUPER DFLT_ESC_SUPER
#define HTML_ESC_SUB DFLT_ESC_SUB

#define HTML_SPTYPE_NONE 0
#define HTML_SPTYPE_BLOCK 1
#define HTML_SPTYPE_HORI 2
#define HTML_SPTYPE_VERT 3

#ifndef TOOLS_CONSTASCII_STRINGPARAM
#define TOOLS_CONSTASCII_STRINGPARAM( constAsciiStr ) constAsciiStr, sizeof( constAsciiStr )-1
#endif

// <P ALIGN=xxx>, <Hn ALIGN=xxx>, <TD ALIGN=xxx> usw.
HTMLOptionEnum __FAR_DATA aHTMLPAlignTable[] =
{
    { sHTML_AL_left,    SVX_ADJUST_LEFT     },
    { sHTML_AL_center,  SVX_ADJUST_CENTER   },
    { sHTML_AL_middle,  SVX_ADJUST_CENTER   }, // Netscape
    { sHTML_AL_right,   SVX_ADJUST_RIGHT    },
    { sHTML_AL_justify, SVX_ADJUST_BLOCK    },
    { sHTML_AL_char,    SVX_ADJUST_LEFT     },
    { 0,                0                   }
};

// <SPACER TYPE=...>
static HTMLOptionEnum __FAR_DATA aHTMLSpacerTypeTable[] =
{
    { sHTML_SPTYPE_block,       HTML_SPTYPE_BLOCK       },
    { sHTML_SPTYPE_horizontal,  HTML_SPTYPE_HORI        },
    { sHTML_SPTYPE_vertical,    HTML_SPTYPE_VERT        },
    { 0,                    0                       }
};

SV_IMPL_PTRARR( _HTMLAttrs, _HTMLAttrPtr )

HTMLReader::HTMLReader()
{
    bTmplBrowseMode = TRUE;

    String sTemplate(
            String::CreateFromAscii(TOOLS_CONSTASCII_STRINGPARAM("internal")) );
    sTemplate += INET_PATH_TOKEN;
    sTemplate.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM("html") );
#ifndef MAC_WITHOUT_EXT
    sTemplate.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM(".vor") );
#endif

    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE ))
        SetTemplateName( sTemplate );
#ifndef PRODUCT
    else
        ASSERT( !this,
            "Die html.vor befindet sich nicht mehr im definierten Directory!");
#endif
}

int HTMLReader::SetStrmStgPtr()
{
    ASSERT( pMedium, "Wo ist das Medium??" );

    if( pMedium->IsRemote() || !pMedium->IsStorage() )
    {
        pStrm = pMedium->GetInStream();
        return TRUE;
    }
    return FALSE;

}

    // Aufruf fuer die allg. Reader-Schnittstelle
ULONG HTMLReader::Read( SwDoc &rDoc,SwPaM &rPam, const String & rName )
{
    if( !pStrm )
    {
        ASSERT( pStrm, "HTML-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    if( !bInsertMode )
    {
        Reader::SetNoOutlineNum( rDoc );
        Reader::ResetFrmFmts( rDoc );

        rDoc.SetBrowseMode( TRUE );

        // Die HTML-Seitenvorlage setzen, wenn des kein HTML-Dokument ist,
        // sonst ist sie schon gesetzt.
        if( !rDoc.IsHTMLMode() )
        {
            rDoc.Insert( rPam, SwFmtPageDesc(
                rDoc.GetPageDescFromPool( RES_POOLPAGE_HTML )) );
        }
    }

    // damit keiner das Doc klaut!
    rDoc.AddLink();
    ULONG nRet = 0;
    SvParserRef xParser = new SwHTMLParser( &rDoc, rPam, *pStrm,
                                            rName, !bInsertMode, pMedium,
                                            IsReadUTF8() );

    SvParserState eState = xParser->CallParser();

    if( SVPAR_PENDING == eState )
        pStrm->ResetError();
    else if( SVPAR_ACCEPTED != eState )
    {
        String sErr( String::CreateFromInt32((sal_Int32)xParser->GetLineNr()));
        sErr += ',';
        sErr += String::CreateFromInt32((sal_Int32)xParser->GetLinePos());

        // den Stream als Fehlernummer Transporter benutzen
        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }


    return nRet;
}


/*  */

SwHTMLParser::SwHTMLParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                            const String& rPath, int bReadNewDoc,
                            SfxMedium* pMed, BOOL bReadUTF8 )
    : SfxHTMLParser( rIn, bReadNewDoc, pMed ),
    SwClient( 0 ),
    aPathToFile( rPath ),
    pDoc( pD ),
    pTable(0), pFormImpl( 0 ), pMarquee( 0 ),
    pField( 0 ),
    pCSS1Parser( 0 ), pNumRuleInfo( new SwHTMLNumRuleInfo ),
    eParaAdjust( SVX_ADJUST_END ),
    nScriptStartLineNr( 0 ),
    nBaseFontStMin( 0 ), nFontStMin( 0 ),
    nDefListDeep( 0 ),
    nFontStHeadStart( 0 ),
    nSBModuleCnt( 0 ),
    nMissingImgMaps( 0 ),
    nOpenParaToken( 0 ),
    pImageMap( 0 ),
    pImageMaps( 0 ),
    pSttNdIdx( 0 ),
    pPendStack( 0 ),
    nParaCnt( 5 ),
    pActionViewShell( 0 ),
    pAppletImpl( 0 ),
    pFootEndNoteImpl( 0 ),
    nContextStMin( 0 ),
    nContextStAttrMin( 0 ),
    bInFloatingFrame( FALSE ),
    bInNoEmbed( FALSE ),
    bInField( FALSE ), bInTitle( FALSE ),
    bInFootEndNoteAnchor( FALSE ), bInFootEndNoteSymbol( FALSE ),
    bNoParSpace( FALSE ), bDocInitalized( FALSE ), bCallNextToken( FALSE ),
    bDataAvailableLinkSet( FALSE ),
    bIgnoreRawData( FALSE ),
    bUpdateDocStat( FALSE ),
    bFixSelectWidth( FALSE ),
    bFixSelectHeight( FALSE ),
    bTextArea( FALSE ), bSelect( FALSE ),
    bSetModEnabled( FALSE ),
    eJumpTo( JUMPTO_NONE )
#ifndef PRODUCT
    ,nContinue( 0 )
#endif
{

    bUpperSpace = bViewCreated = bChkJumpMark =
    bSetCrsr = FALSE;

    eScriptLang = HTML_SL_UNKNOWN;
    bAnyStarBasic = TRUE;

    pPam = new SwPaM( *rCrsr.GetPoint() );
    memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));

    // Die Font-Groessen 1-7 aus der INI-Datei lesen
    OfaHtmlOptions* pHtmlOptions = OFF_APP()->GetHtmlOptions();
    aFontHeights[0] = pHtmlOptions->GetFontSize( 0 ) * 20;
    aFontHeights[1] = pHtmlOptions->GetFontSize( 1 ) * 20;
    aFontHeights[2] = pHtmlOptions->GetFontSize( 2 ) * 20;
    aFontHeights[3] = pHtmlOptions->GetFontSize( 3 ) * 20;
    aFontHeights[4] = pHtmlOptions->GetFontSize( 4 ) * 20;
    aFontHeights[5] = pHtmlOptions->GetFontSize( 5 ) * 20;
    aFontHeights[6] = pHtmlOptions->GetFontSize( 6 ) * 20;

    bKeepUnknown = pHtmlOptions->IsImportUnknown();

    pDoc->SetDefault( SvxFontHeightItem(aFontHeights[2]) );

    // Waehrend des Imports in den HTML-Modus schalten, damit die
    // richrigen Vorlagen angelegt werden
    bOldIsHTMLMode = pDoc->IsHTMLMode();
    pDoc->SetHTMLMode( TRUE );

    pCSS1Parser = new SwCSS1Parser( pDoc, aFontHeights, IsNewDoc() );
    pCSS1Parser->SetIgnoreFontFamily( pHtmlOptions->IsIgnoreFontFamily() );

    if( bReadUTF8 )
    {
        SetSrcEncoding( RTL_TEXTENCODING_UTF8 );
    }
    else
    {
        SwDocShell *pDocSh = pDoc->GetDocShell();
        SvKeyValueIterator *pHeaderAttrs =
            pDocSh->GetHeaderAttributes();
        if( pHeaderAttrs )
            SetEncodingByHTTPHeader( pHeaderAttrs );
    }
    pCSS1Parser->SetDfltEncoding( gsl_getSystemTextEncoding() );
    sBaseURL = INetURLObject::GetBaseURL();

    // Timer nur bei ganz normalen Dokumenten aufsetzen!
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if( pDocSh )
    {
        if( SFX_CREATE_MODE_INTERNAL == pDocSh->GetCreateMode() ||
            SFX_CREATE_MODE_PREVIEW == pDocSh->GetCreateMode() )
            bViewCreated = TRUE;            // dann nicht, synchron laden

        // es ist ein Sprungziel vorgegeben.

        if( pMed )
        {
            sJmpMark = pMed->GetURLObject().GetMark();
            if( sJmpMark.Len() )
            {
                eJumpTo = JUMPTO_MARK;
                String sCmp;
                xub_StrLen nLastPos, nPos = 0;
                while( STRING_NOTFOUND != ( nLastPos =
                        sJmpMark.Search( cMarkSeperator, nPos + 1 )) )
                    nPos = nLastPos;

                if( nPos && ( sCmp = sJmpMark.Copy( nPos + 1 ) ).
                                                EraseAllChars().Len() )
                {
                    sCmp.ToLowerAscii();
                    if( sCmp.EqualsAscii( pMarkToRegion ) )
                        eJumpTo = JUMPTO_REGION;
                    else if( sCmp.EqualsAscii( pMarkToTable ) )
                        eJumpTo = JUMPTO_TABLE;
                    else if( sCmp.EqualsAscii( pMarkToGraphic ) )
                        eJumpTo = JUMPTO_GRAPHIC;
                    else if( sCmp.EqualsAscii( pMarkToOutline ) ||
                            sCmp.EqualsAscii( pMarkToText ) ||
                            sCmp.EqualsAscii( pMarkToFrame ) )
                        eJumpTo = JUMPTO_NONE;  // das ist nichts gueltiges!
                    else
                        // ansonsten ist das ein normaler (Book)Mark
                        nPos = STRING_LEN;
                }
                else
                    nPos = STRING_LEN;

                sJmpMark.Erase( nPos );
                if( !sJmpMark.Len() )
                    eJumpTo = JUMPTO_NONE;
            }
        }
    }
}

__EXPORT SwHTMLParser::~SwHTMLParser()
{
#ifndef PRODUCT
    ASSERT( !nContinue, "DTOR im Continue - Das geht schief!!!" );
#endif
    BOOL bAsync = pDoc->IsInLoadAsynchron();
    pDoc->SetInLoadAsynchron( FALSE );
    pDoc->SetHTMLMode( bOldIsHTMLMode );

    if( pDoc->GetDocShell() && aLoadEnv.Is() )
    {
        // Den Link vor dem DocumentDetected austragen, damit
        // wir waehrend des DocumentDetected nicht noch irgendwelche
        // Daten bekommen, die zum nochmaligen Aufruf von
        // DocumentDetected (in NextToken) fuehren.
        ((SfxLoadEnvironment*)&aLoadEnv)->SetDataAvailableLink( Link() );
        bDataAvailableLinkSet = FALSE;

        // wenn noch nichts entschieden ist, dann aber hier
        if( !bDocInitalized )
        {
            ErrCode eError = ((SfxLoadEnvironment*)&aLoadEnv)->
                                DocumentDetected( pDoc->GetDocShell(), 0 );
        }
    }

    // das DocumentDetected kann ggfs. die DocShells loeschen, darum nochmals
    // abfragen
    if( pDoc->GetDocShell() )
    {
        // Gelinkte Bereiche updaten
        USHORT nLinkMode = pDoc->GetLinkUpdMode();
        if( nLinkMode != NEVER && bAsync &&
            SFX_CREATE_MODE_INTERNAL!=pDoc->GetDocShell()->GetCreateMode() )
            pDoc->GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL,
                                                   TRUE, FALSE );

        if( pDoc->GetDocShell()->IsLoading() )
            pDoc->GetDocShell()->StartLoadFinishedTimer();
    }

    delete pSttNdIdx;

    if( aSetAttrTab.Count() )
    {
        ASSERT( !aSetAttrTab.Count(),"Es stehen noch Attribute auf dem Stack" );
        aSetAttrTab.DeleteAndDestroy( 0, aSetAttrTab.Count() );
    }

    delete pPam;
    delete pCSS1Parser;
    delete pNumRuleInfo;
    DeleteFormImpl();
    DeleteFootEndNoteImpl();

    ASSERT( !pTable, "Es existiert noch eine offene Tabelle" );
    delete pImageMaps;
    //delete pTable;

    ASSERT( !pPendStack,
            "SwHTMLParser::~SwHTMLParser: Hier sollte es keinen Pending-Stack mehr geben" );
    while( pPendStack )
    {
        SwPendingStack* pTmp = pPendStack;
        pPendStack = pPendStack->pNext;
        delete pTmp->pData;
        delete pTmp;
    }

    if( !pDoc->RemoveLink() )
    {
        // keiner will mehr das Doc haben, also weg damit
        delete pDoc;
    }
}

SvParserState __EXPORT SwHTMLParser::CallParser()
{
    // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
    pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
    if( !IsNewDoc() )       // in ein Dokument einfuegen ?
    {
        const SwPosition* pPos = pPam->GetPoint();
        SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();

        pDoc->SplitNode( *pPos );

        *pSttNdIdx = pPos->nNode.GetIndex()-1;
        pDoc->SplitNode( *pPos );

        pPam->Move( fnMoveBackward );
        pDoc->SetTxtFmtColl( *pPam,
                pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    }

    if( GetMedium() )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == GetMedium()->GetItemSet()->GetItemState(
                SID_LOADENVIRONMENT, FALSE, &pItem ))
        {
            aLoadEnv = &((SfxRefItem*)pItem)->GetValue();
        }

        if( !bViewCreated && aLoadEnv.Is() )
        {
            ((SfxLoadEnvironment*)&aLoadEnv)->
                SetDataAvailableLink( GetAsynchCallLink() );
            bDataAvailableLinkSet = TRUE;
        }
        else
            bViewCreated = TRUE;
    }

    // Laufbalken anzeigen
    else if( !GetMedium() || !GetMedium()->IsRemote() )
    {
        rInput.Seek(STREAM_SEEK_TO_END);
        rInput.ResetError();
        ::StartProgress( STR_STATSTR_W4WREAD, 0, rInput.Tell(),
                         pDoc->GetDocShell() );
        rInput.Seek(STREAM_SEEK_TO_BEGIN);
        rInput.ResetError();
    }

    SwPageDesc& rDesc = pDoc->_GetPageDesc( 0 );
    rDesc.Add( this );

    SvParserState eRet = HTMLParser::CallParser();

    return eRet;
}

void __EXPORT SwHTMLParser::Continue( int nToken )
{
#ifndef PRODUCT
    ASSERT( !nContinue, "Continue im Continue - Das sollte doch nicht sein, oder?" );
    nContinue++;
#endif

    // Wenn der Import (vom SFX) abgebrochen wurde, wird ein Fehler
    // gesetzt aber trotzdem noch weiter gemacht, damit vernuenftig
    // aufgeraeumt wird.
    ASSERT( SVPAR_ERROR!=eState,
            "SwHTMLParser::Continue: bereits ein Fehler gesetzt" );
    if( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        eState = SVPAR_ERROR;

    // Die ViewShell vom Dokument holen, merken und als aktuelle setzen.
    ViewShell *pInitVSh = CallStartAction();

    if( SVPAR_ERROR != eState && !pInitVSh && GetMedium() && !bViewCreated )
    {
        // Beim ersten Aufruf erstmal returnen, Doc anzeigen
        // und auf Timer Callback warten.
        // An dieser Stelle wurde im CallParser gerade mal ein Zeichen
        // gelesen und ein SaveState(0) gerufen.
        eState = SVPAR_PENDING;
        bViewCreated = TRUE;
        pDoc->SetInLoadAsynchron( TRUE );

#ifndef PRODUCT
        nContinue--;
#endif

        return;
    }

    bSetModEnabled = FALSE;
    if( pDoc->GetDocShell() &&
        0 != (bSetModEnabled = pDoc->GetDocShell()->IsEnableSetModified()) )
    {
        pDoc->GetDocShell()->EnableSetModified( FALSE );
    }

    // waehrend des einlesens kein OLE-Modified rufen
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bModified = pDoc->IsModified();
    BOOL bWasUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    GetSaveAndSetOwnBaseURL();

    // Wenn der Import abgebrochen wird, kein Continue mehr rufen.
    // Falls ein Pending-Stack existiert aber durch einen Aufruf
    // von NextToken dafuer sorgen, dass der Pending-Stack noch
    // beendet wird.
    if( SVPAR_ERROR == eState )
    {
        ASSERT( !pPendStack || pPendStack->nToken,
                "SwHTMLParser::Continue: Pending-Stack ohne Token" );
        if( pPendStack && pPendStack->nToken )
            NextToken( pPendStack->nToken );
        ASSERT( !pPendStack,
                "SwHTMLParser::Continue: Es gibt wieder einen Pend-Stack" );
    }
    else
    {
        HTMLParser::Continue( pPendStack ? pPendStack->nToken : nToken );
    }

    SetSaveBaseURL();

    // Laufbalken wieder abschalten
    EndProgress( pDoc->GetDocShell() );

    BOOL bLFStripped = FALSE;
    if( SVPAR_PENDING != GetStatus() )
    {
        // noch die letzten Attribute setzen
        {
            if( aScriptSource.Len() )
            {
                SwScriptFieldType *pType =
                    (SwScriptFieldType*)pDoc->GetSysFldType( RES_SCRIPTFLD );

                SwScriptField aFld( pType, aScriptType, aScriptSource,
                                    FALSE );
                InsertAttr( SwFmtFld( aFld ) );
            }

            if( pAppletImpl )
                EndApplet();

            // ggf. ein noch vorhandes LF hinter dem letzen Absatz entfernen
            if( IsNewDoc() )
                bLFStripped = StripTrailingLF() > 0;

            // noch offene Nummerierungen beenden.
            while( GetNumInfo().GetNumRule() )
                EndNumBulList();

            ASSERT( !nContextStMin, "Es gibt geschuetzte Kontexte" );
            nContextStMin = 0;
            while( aContexts.Count() )
            {
                _HTMLAttrContext *pCntxt = PopContext();
                if( pCntxt )
                {
                    EndContext( pCntxt );
                    delete pCntxt;
                }
            }

            if( aParaAttrs.Count() )
                aParaAttrs.Remove( 0, aParaAttrs.Count() );

            SetAttr( FALSE );

            // Noch die erst verzoegert gesetzten Styles setzen
            pCSS1Parser->SetDelayedStyles();
        }

        // den Start wieder korrigieren
        if( !IsNewDoc() && pSttNdIdx->GetIndex() )
        {
            SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
            SwNodeIndex aNxtIdx( *pSttNdIdx );
            if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
            {
                xub_StrLen nStt = pTxtNode->GetTxt().Len();
                // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
                if( pPam->GetPoint()->nNode == aNxtIdx )
                {
                    pPam->GetPoint()->nNode = *pSttNdIdx;
                    pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
                }

#ifndef PRODUCT
// !!! sollte nicht moeglich sein, oder ??
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( TRUE ).nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( FALSE ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( TRUE ).nNode.GetIndex() )
{
    register xub_StrLen nCntPos = pPam->GetBound( TRUE ).nContent.GetIndex();
    pPam->GetBound( TRUE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( FALSE ).nNode.GetIndex() )
{
    register xub_StrLen nCntPos = pPam->GetBound( FALSE ).nContent.GetIndex();
    pPam->GetBound( FALSE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
#endif
                // Zeichen Attribute beibehalten!
                SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
                if( pTxtNode->GetTxt().Len() )
                    pDelNd->FmtToTxtAttr( pTxtNode );
                else
                    pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
                pTxtNode->JoinNext();
            }
        }
    }

    if( SVPAR_ACCEPTED == eState )
    {
        if( nMissingImgMaps )
        {
            // es fehlen noch ein paar Image-Map zuordungen.
            // vielleicht sind die Image-Maps ja jetzt da?
            ConnectImageMaps();
        }

        // jetzt noch den letzten ueberfluessigen Absatz loeschen
        SwPosition* pPos = pPam->GetPoint();
        if( !pPos->nContent.GetIndex() && !bLFStripped )
        {
            SwTxtNode* pAktNd;
            ULONG nNodeIdx = pPos->nNode.GetIndex();

            BOOL bHasFlysOrMarks =
                HasCurrentParaFlys() || HasCurrentParaBookmarks( TRUE );

            if( IsNewDoc() )
            {
                if( pDoc->GetNodes()[ nNodeIdx -1 ]->IsCntntNode() &&
                    !pPam->GetPoint()->nContent.GetIndex() )
                {
                    nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
                    SwCntntNode* pCNd = pPam->GetCntntNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() && !bHasFlysOrMarks )
                    {
                        pPam->GetBound(TRUE).nContent.Assign( 0, 0 );
                        pPam->GetBound(FALSE).nContent.Assign( 0, 0 );
                        pDoc->GetNodes().Delete( pPam->GetPoint()->nNode );
                    }
                }
            }
            else if( 0 != ( pAktNd = pDoc->GetNodes()[ nNodeIdx ]->GetTxtNode()) && !bHasFlysOrMarks )
            {
                if( pAktNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if( !pAktNd->GetTxt().Len() )
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPam->Move( fnMoveBackward );
                }
            }
        }

        // nun noch das SplitNode vom Anfang aufheben
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     // dann gabs am Ende kein <P>,
                pPam->Move( fnMoveForward, fnGoNode );  // als zum naechsten Node
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                // eigentlich muss hier ein JoinNext erfolgen, aber alle Cursor
                // usw. sind im pTxtNode angemeldet, so dass der bestehen
                // bleiben MUSS.

                // Absatz in Zeichen-Attribute umwandeln, aus dem Prev die
                // Absatzattribute und die Vorlage uebernehmen!
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->SwCntntNode::ResetAllAttr();

                if( pPrev->GetpSwAttrSet() )
                    pTxtNode->SwCntntNode::SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(TRUE).nNode.GetNode() == pPrev )
                    pPam->GetBound(TRUE).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(FALSE).nNode.GetNode() == pPrev )
                    pPam->GetBound(FALSE).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }

        // und noch die DocumentInfo aufbereiten
        if( IsNewDoc() )
        {
            const SfxDocumentInfo *pInfo = pDoc->GetInfo();
            if( pInfo->IsReloadEnabled() &&
                !pInfo->GetReloadURL().Len() )
            {
                SfxDocumentInfo aInfo( *pDoc->GetInfo() );
                aInfo.SetReloadURL( aPathToFile );
                pDoc->SetInfo( aInfo );
            }
        }

        if( bUpdateDocStat )
        {
            SwDocStat aStat( pDoc->GetDocStat() );
            ViewShell *pTmpVSh = 0;
            pDoc->GetEditShell( &pTmpVSh );
            USHORT nPages = pTmpVSh ? pTmpVSh->GetNumPages() : 1;
            pDoc->UpdateDocStat( aStat, nPages );
        }
    }

    if( SVPAR_PENDING != GetStatus() )
        delete pSttNdIdx, pSttNdIdx = 0;

    // sollte der Parser der Letzte sein, der das Doc haelt, dann braucht
    // man hier auch nichts mehr tun, Doc wird gleich zerstoert!
    if( 1 < pDoc->GetLinkCnt() )
    {
        if( bWasUndo )
        {
            pDoc->DelAllUndoObj();
            pDoc->DoUndo( TRUE );
        }
        else if( !pInitVSh )
        {
            // Wenn zu Beginn des Continue keine Shell vorhanden war,
            // kann trotzdem mitlerweile eine angelegt worden sein.
            // In dieses Fall stimmt das bWasUndo-Flag nicht und
            // wir muessen das Undo noch anschalten.
            ViewShell *pTmpVSh = CheckActionViewShell();
            if( pTmpVSh )
                pDoc->DoUndo( TRUE );
        }

        pDoc->SetOle2Link( aOLELink );
        if( !bModified )
            pDoc->ResetModified();
        if( bSetModEnabled && pDoc->GetDocShell() )
        {
            pDoc->GetDocShell()->EnableSetModified( TRUE );
            bSetModEnabled = FALSE; // this is unnecessary here
        }
    }


    // Wenn die Dokuemnt-ViewShell noch existiert und eine Action
    // offen ist (muss bei Abbruch nicht sein), die Action beenden,
    // uns von der Shell abmelden und schliesslich die alte Shell
    // wieder rekonstruieren.
    CallEndAction( TRUE );

#ifndef PRODUCT
    nContinue--;
#endif
}

void SwHTMLParser::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_OBJECTDYING:
        if( ((SwPtrMsgPoolItem *)pOld)->pObject == pRegisteredIn )
        {
            // dann uns selbst beenden
            pRegisteredIn->Remove( this );
            ReleaseRef();                   // ansonsten sind wir fertig!
        }
        break;
    }
}

void SwHTMLParser::DocumentDetected()
{
    ASSERT( !bDocInitalized, "DocumentDetected mehrfach aufgerufen" );
    bDocInitalized = TRUE;
    if( IsNewDoc() )
    {
        if( IsInHeader() )
            FinishHeader( TRUE );

        CallEndAction( TRUE, TRUE );

        SetSaveBaseURL();

#if 0
        ViewShell *pTmpVSh = 0;
        pDoc->GetEditShell( &pTmpVSh );
        ASSERT( pTmpVSh==0,
                "Dok-ViewShell existiert schon vor DocDetected" );
#endif
        ErrCode eError = ((SfxLoadEnvironment*)&aLoadEnv)->
                        DocumentDetected( pDoc->GetDocShell(), 0 );
        if( eError )
        {
            eState = SVPAR_ERROR;
            return;
        }
        GetSaveAndSetOwnBaseURL();

        pDoc->DoUndo( FALSE );
        // Durch das DocumentDetected wurde im allgemeinen eine
        // ViewShell angelegt. Es kann aber auch sein, dass sie
        // erst spaeter angelegt wird, naemlich dann, wenn die UI
        // gecaptured ist.
        CallStartAction();
    }
}

// wird fuer jedes Token gerufen, das in CallParser erkannt wird
void __EXPORT SwHTMLParser::NextToken( int nToken )
{
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->GetLinkCnt() )
    {
        // wurde der Import vom SFX abgebrochen? Wenn ein Pending-Stack
        // existiert den noch aufraumen
        eState = SVPAR_ERROR;
        ASSERT( !pPendStack || pPendStack->nToken,
                "SwHTMLParser::NextToken: Pending-Stack ohne Token" );
        if( 1 == pDoc->GetLinkCnt() || !pPendStack )
            return ;
    }

#ifndef PRODUCT
    if( pPendStack )
    {
        switch( nToken )
        {
            // Tabellen werden ueber rekusive Methodenaufrufe gelesen
        case HTML_TABLE_ON:
            // Bei CSS-Deklarationen muss evtl. noch auf das
            // Ende eines File-Downloads gewartet werden.
        case HTML_LINK:
            // Bei Controls muss evtl. noch die Groesse gesetzt werden.
        case HTML_INPUT:
        case HTML_TEXTAREA_ON:
        case HTML_SELECT_ON:
        case HTML_SELECT_OFF:
            break;
        default:
            ASSERT( !pPendStack, "Unbekanntes Token fuer Pending-Stack" );
            break;
        }
    }
#endif

    // Die folgeneden Spezialfaelle muessen vor der Filter-Detection behandelt
    // werden, denn der Inhalt des Titels, etc. wird auch in Netcape nicht
    // zur Filter-Detection herangezogen.
    if( !pPendStack )
    {
        if( bInTitle )
        {
            switch( nToken )
            {
            case HTML_TITLE_OFF:
                if( IsNewDoc() && sTitle.Len() )
                {
                    SfxDocumentInfo aInfo( *pDoc->GetInfo() );
                    aInfo.SetTitle( sTitle );
                    pDoc->SetInfo( aInfo );

                    if( pDoc->GetDocShell() )
                        pDoc->GetDocShell()->SetTitle( sTitle );
                }
                bInTitle = FALSE;
                sTitle.Erase();
                break;

            case HTML_NONBREAKSPACE:
                sTitle += ' ';
                break;

            case HTML_SOFTHYPH:
                sTitle += '-';
                break;

            case HTML_TEXTTOKEN:
                sTitle += aToken;
                break;

            default:
                sTitle += '<';
                if( (HTML_TOKEN_ONOFF & nToken) && (1 & nToken) )
                    sTitle += '/';
                sTitle += sSaveToken;
                if( aToken.Len() )
                {
                    sTitle += ' ';
                    sTitle += aToken;
                }
                sTitle += '>';
                break;
            }

            return;
        }
    }

    // Wenn wir noch nicht wissen, was fuer ein Dokument wir vor uns haben,
    // versuchen wir das erstmal rauszufinden. Das muss fuer Controls in
    // Fall vor dem Einfuegen des Controls passieren, weil beim Einfuegen
    // bereits eine View benoetigt wird.
    if( !bDocInitalized && aLoadEnv.Is() )
        DocumentDetected();

    BOOL bGetIDOption = FALSE, bInsertUnknown = FALSE;
    BOOL bUpperSpaceSave = bUpperSpace;
    bUpperSpace = FALSE;

    // Die folgenden Speziallfaelle muessen oder koennen nach der
    // Filter-Detection erfolgen.
    if( !pPendStack )
    {
        if( bInFloatingFrame )
        {
            // <SCRIPT> wird hier (von uns) ignoriert, weil es auch in
            // Applets ignoriert wird!
            if( HTML_IFRAME_OFF == nToken )
            {
                bCallNextToken = FALSE;
                EndFloatingFrame();
            }

            return;
        }
        else if( bInNoEmbed )
        {
            switch( nToken )
            {
            case HTML_NOEMBED_OFF:
                aContents.ConvertLineEnd();
                InsertComment( aContents, sHTML_noembed );
                aContents.Erase();
                bCallNextToken = FALSE;
                bInNoEmbed = FALSE;
                break;

            case HTML_RAWDATA:
                InsertCommentText( sHTML_noembed );
                break;

            default:
                ASSERT( !this, "SwHTMLParser::NextToken: ungueltiges Tag" );
                break;
            }

            return;
        }
        else if( pAppletImpl )
        {
            // in einem Applet interessieren uns (erstmal) nur <PARAM>-Tags
            // und das </APPLET>.
            // <SCRIPT> wird hier (von Netscape) ignoriert!

            switch( nToken )
            {
            case HTML_APPLET_OFF:
                bCallNextToken = FALSE;
                EndApplet();
                break;

            case HTML_PARAM:
                InsertParam();
                break;
            }

            return;
        }
        else if( bTextArea )
        {
            // in einer TextArea wird alles bis zum </TEXTAREA> als Text
            // eingefuegt
            // <SCRIPT> wird hier (von Netscape) ignoriert!

            switch( nToken )
            {
            case HTML_TEXTAREA_OFF:
                bCallNextToken = FALSE;
                EndTextArea();
                break;

            default:
                InsertTextAreaText( nToken );
                break;
            }

            return;
        }
        else if( bSelect )
        {
            // MUSS nach bNoScript kommen!
            switch( nToken )
            {
            case HTML_SELECT_OFF:
                bCallNextToken = FALSE;
                EndSelect();
                return;

            case HTML_OPTION:
                InsertSelectOption();
                return;

            case HTML_TEXTTOKEN:
                InsertSelectText();
                return;

            case HTML_INPUT:
            case HTML_SCRIPT_ON:
            case HTML_SCRIPT_OFF:
            case HTML_NOSCRIPT_ON:
            case HTML_NOSCRIPT_OFF:
            case HTML_RAWDATA:
                // im normalen switch bahandeln
                break;

            default:
                // ignorieren
                return;
            }
        }
        else if( pMarquee )
        {
            // in einer TextArea wird alles bis zum </TEXTAREA> als Text
            // eingefuegt
            // Die <SCRIPT>-Tags werden vom MS-IE ignoriert, von uns das
            // geasmte Script
            switch( nToken )
            {
            case HTML_MARQUEE_OFF:
                bCallNextToken = FALSE;
                EndMarquee();
                break;

            case HTML_TEXTTOKEN:
                InsertMarqueeText();
                break;
            }

            return;
        }
        else if( bInField )
        {
            switch( nToken )
            {
            case HTML_SDFIELD_OFF:
                bCallNextToken = FALSE;
                EndField();
                break;

            case HTML_TEXTTOKEN:
                InsertFieldText();
                break;
            }

            return;
        }
        else if( bInFootEndNoteAnchor || bInFootEndNoteSymbol )
        {
            switch( nToken )
            {
            case HTML_ANCHOR_OFF:
                EndAnchor();
                bCallNextToken = FALSE;
                break;

            case HTML_TEXTTOKEN:
                InsertFootEndNoteText();
                break;
            }
            return;
        }
        else if( aUnknownToken.Len() )
        {
            // Unbekannte Token im Header werden nur durch ein passendes
            // End-Token, </HEAD> oder <BODY> wieder beendet. Darin wird Text
            // ignoriert.
            switch( nToken )
            {
            case HTML_UNKNOWNCONTROL_OFF:
                if( aUnknownToken.CompareTo(sSaveToken) != COMPARE_EQUAL )
                    return;
            case HTML_FRAMESET_ON:
            case HTML_HEAD_OFF:
            case HTML_BODY_ON:
            case HTML_IMAGE:        // Warum auch immer Netscape das tut.
                aUnknownToken.Erase();
                break;
            case HTML_TEXTTOKEN:
                return;
            default:
                break;
            }
        }
    }

    switch( nToken )
    {
    case HTML_BODY_ON:
        if( aStyleSource.Len() )
        {
            pCSS1Parser->ParseStyleSheet( aStyleSource );
            aStyleSource.Erase();
        }
        if( IsNewDoc() )
        {
            InsertBodyOptions();
            // Falls es eine Vorlage fuer die erste oder rechte Seite gibt,
            // setzen wir die hier.
            const SwPageDesc *pPageDesc = 0;
            if( pCSS1Parser->IsSetFirstPageDesc() )
                pPageDesc = pCSS1Parser->GetFirstPageDesc();
            else if( pCSS1Parser->IsSetRightPageDesc() )
                pPageDesc = pCSS1Parser->GetRightPageDesc();

            if( pPageDesc )
                pDoc->Insert( *pPam, SwFmtPageDesc( pPageDesc ) );
        }
        break;

    case HTML_LINK:
        InsertLink();
        break;

    case HTML_BASE:
        {
            const HTMLOptions *pOptions = GetOptions();
            for( USHORT i = pOptions->Count(); i; )
            {
                const HTMLOption *pOption = (*pOptions)[ --i ];
                switch( pOption->GetToken() )
                {
                case HTML_O_HREF:
                    sBaseURL = pOption->GetString();
                    INetURLObject::SetBaseURL( sBaseURL );
                    break;
                case HTML_O_TARGET:
                    if( IsNewDoc() )
                    {
                        SfxDocumentInfo aInfo( *pDoc->GetInfo() );
                        aInfo.SetDefaultTarget( pOption->GetString() );
                        pDoc->SetInfo( aInfo );
                    }
                    break;
                }
            }
        }
        break;

    case HTML_META:
        if( IsNewDoc() )
        {
            SwDocShell *pDocSh = pDoc->GetDocShell();
            SvKeyValueIterator *pHTTPHeader =
                pDocSh ? pDocSh->GetHeaderAttributes() : 0;
            SfxDocumentInfo aInfo( *pDoc->GetInfo() );

            if( ParseMetaOptions( &aInfo, pHTTPHeader ) )
            {
                pDoc->SetInfo( aInfo );
            }
            else
            {
                ParseMoreMetaOptions();
            }
        }
        break;

    case HTML_TITLE_ON:
        bInTitle = TRUE;
        break;

    case HTML_SCRIPT_ON:
        NewScript();
        break;

    case HTML_SCRIPT_OFF:
        EndScript();
        break;

    case HTML_NOSCRIPT_ON:
    case HTML_NOSCRIPT_OFF:
        bInsertUnknown = TRUE;
        break;

    case HTML_STYLE_ON:
        NewStyle();
        break;

    case HTML_STYLE_OFF:
        EndStyle();
        break;

    case HTML_RAWDATA:
        if( !bIgnoreRawData )
        {
            if( IsReadScript() )
            {
                AddScriptSource();
            }
            else if( IsReadStyle() )
            {
                if( aStyleSource.Len() )
                    aStyleSource += '\n';
                aStyleSource += aToken;
            }
        }
        break;

    case HTML_APPLET_ON:
#ifdef SOLAR_JAVA
        InsertApplet();
        bCallNextToken = pAppletImpl!=0 && pTable!=0;
#endif
        break;

    case HTML_IFRAME_ON:
        InsertFloatingFrame();
        bCallNextToken = bInFloatingFrame && pTable!=0;
        break;

    case HTML_LINEBREAK:
        if( !IsReadPRE() )
        {
            InsertLineBreak();
            break;
        }
        else
            bGetIDOption = TRUE;
            // <BR>s in <PRE> aehneln echten LFs, deshalb kein break

    case HTML_NEWPARA:
        // CR in PRE/LISTING/XMP
        {
            if( HTML_NEWPARA==nToken ||
                pPam->GetPoint()->nContent.GetIndex() )
            {
                AppendTxtNode(); // lf gibts hier nicht, deshalb unkritisch
                SetTxtCollAttrs();
            }
            // Laufbalkenanzeige
            if( !GetMedium() || !GetMedium()->IsRemote() )
                ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        }
        break;

    case HTML_NONBREAKSPACE:
        pDoc->Insert( *pPam, CHAR_HARDBLANK );
        break;

    case HTML_SOFTHYPH:
        pDoc->Insert( *pPam, CHAR_SOFTHYPHEN );
        break;

    case HTML_LINEFEEDCHAR:
        if( pPam->GetPoint()->nContent.GetIndex() )
            AppendTxtNode();
        if( !pTable && !pDoc->IsInHeaderFooter( pPam->GetPoint()->nNode ) )
        {
            NewAttr( &aAttrTab.pBreak, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE) );
            EndAttr( aAttrTab.pBreak, 0, FALSE );
        }
        break;

    case HTML_TEXTTOKEN:
        // dann fuege den String ein, ohne das Attribute am Ende
        // aufgespannt werden.
        if( aToken.Len() && ' '==aToken.GetChar(0) && !IsReadPRE() )
        {
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
            if( nPos )
            {
                const String& rText =
                    pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->GetTxtNode()
                                                               ->GetTxt();
                register sal_Unicode cLast = rText.GetChar(--nPos);
                if( ' ' == cLast || '\x0a' == cLast)
                    aToken.Erase(0,1);
            }
            else
                aToken.Erase(0,1);

            if( !aToken.Len() )
            {
                bUpperSpace = bUpperSpaceSave;
                break;
            }
        }

        if( aToken.Len() )
        {
            if( !bDocInitalized && aLoadEnv.Is() )
                DocumentDetected();
            pDoc->Insert( *pPam, aToken );

            // wenn es noch vorlaefige Absatz-Attribute gibt, der Absatz aber
            // nicht leer ist, dann sind die Absatz-Attribute entgueltig.
            if( aParaAttrs.Count() )
                aParaAttrs.Remove( 0, aParaAttrs.Count() );

            SetAttr();
        }
        break;

    case HTML_HORZRULE:
        InsertHorzRule();
        break;

    case HTML_IMAGE:
        InsertImage();
        // sollte der Parser der Letzte sein, der das Doc haelt, dann kann
        // man hier abbrechen und einen Fehler setzen.
        if( 1 == pDoc->GetLinkCnt() )
        {
            eState = SVPAR_ERROR;
        }
        break;

    case HTML_SPACER:
        InsertSpacer();
        break;

    case HTML_EMBED:
        InsertEmbed();
        break;

    case HTML_NOEMBED_ON:
        bInNoEmbed = TRUE;
        bCallNextToken = pTable!=0;
        ReadRawData( sHTML_noembed );
        break;

    case HTML_DEFLIST_ON:
        if( nOpenParaToken )
            EndPara();
        NewDefList();
        break;
    case HTML_DEFLIST_OFF:
        if( nOpenParaToken )
            EndPara();
        EndDefListItem( 0, FALSE, 1==nDefListDeep );
        EndDefList();
        break;

    case HTML_DD_ON:
    case HTML_DT_ON:
        if( nOpenParaToken )
            EndPara();
        EndDefListItem( 0, FALSE );// <DD>/<DT> beenden und keine Vorl. setzen
        NewDefListItem( nToken );
        break;

    case HTML_DD_OFF:
    case HTML_DT_OFF:
        // siehe HTML_LI_OFF
        // eigentlich muesste man ein DD/DT jetzt beenden. Da aber sowhl
        // Netscape als auch Microsoft das nicht tun, machen wir das eben
        // auch nicht.
        EndDefListItem( nToken, FALSE );
        break;

    // Bereiche
    case HTML_DIVISION_ON:
    case HTML_CENTER_ON:
        if( nOpenParaToken )
        {
            if( IsReadPRE() )
                nOpenParaToken = 0;
            else
                EndPara();
        }
        NewDivision( nToken );
        break;

    case HTML_DIVISION_OFF:
    case HTML_CENTER_OFF:
        if( nOpenParaToken )
        {
            if( IsReadPRE() )
                nOpenParaToken = 0;
            else
                EndPara();
        }
        EndDivision( nToken );
        break;

    case HTML_MULTICOL_ON:
        if( nOpenParaToken )
            EndPara();
        NewMultiCol();
        break;

    case HTML_MULTICOL_OFF:
        if( nOpenParaToken )
            EndPara();
        EndTag( HTML_MULTICOL_ON );
        break;

    case HTML_MARQUEE_ON:
        NewMarquee();
        bCallNextToken = pMarquee!=0 && pTable!=0;
        break;

    case HTML_FORM_ON:
        NewForm();
        break;
    case HTML_FORM_OFF:
        EndForm();
        break;

    // Vorlagen:
    case HTML_PARABREAK_ON:
        if( nOpenParaToken )
            EndPara( TRUE );
        NewPara();
        break;

    case HTML_PARABREAK_OFF:
        EndPara( TRUE );
        break;

    case HTML_ADDRESS_ON:
        if( nOpenParaToken )
            EndPara();
        NewTxtFmtColl( HTML_ADDRESS_ON, RES_POOLCOLL_SENDADRESS );
        break;

    case HTML_ADDRESS_OFF:
        if( nOpenParaToken )
            EndPara();
        EndTxtFmtColl( HTML_ADDRESS_OFF );
        break;

    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE30_ON:
        if( nOpenParaToken )
            EndPara();
        NewTxtFmtColl( HTML_BLOCKQUOTE_ON, RES_POOLCOLL_HTML_BLOCKQUOTE );
        break;

    case HTML_BLOCKQUOTE_OFF:
    case HTML_BLOCKQUOTE30_OFF:
        if( nOpenParaToken )
            EndPara();
        EndTxtFmtColl( HTML_BLOCKQUOTE_ON );
        break;

    case HTML_PREFORMTXT_ON:
    case HTML_LISTING_ON:
    case HTML_XMP_ON:
        if( nOpenParaToken )
            EndPara();
        NewTxtFmtColl( nToken, RES_POOLCOLL_HTML_PRE );
        break;

    case HTML_PREFORMTXT_OFF:
        bNoParSpace = TRUE; // der letzte PRE-Absatz muss einen Zeilenabstand bekommen
        EndTxtFmtColl( HTML_PREFORMTXT_OFF );
        break;

    case HTML_LISTING_OFF:
    case HTML_XMP_OFF:
        EndTxtFmtColl( nToken );
        break;

    case HTML_HEAD1_ON:
    case HTML_HEAD2_ON:
    case HTML_HEAD3_ON:
    case HTML_HEAD4_ON:
    case HTML_HEAD5_ON:
    case HTML_HEAD6_ON:
        if( nOpenParaToken )
        {
            if( IsReadPRE() )
                nOpenParaToken = 0;
            else
                EndPara();
        }
        NewHeading( nToken );
        break;

    case HTML_HEAD1_OFF:
    case HTML_HEAD2_OFF:
    case HTML_HEAD3_OFF:
    case HTML_HEAD4_OFF:
    case HTML_HEAD5_OFF:
    case HTML_HEAD6_OFF:
        EndHeading();
        break;

    case HTML_TABLE_ON:
        if( pPendStack )
            BuildTable( SVX_ADJUST_END );
        else
        {
            if( nOpenParaToken )
                EndPara();
            ASSERT( !pTable, "Tabelle in Tabelle darf hier nicht vorkommen" );
            if( !pTable && (IsNewDoc() || !pPam->GetNode()->FindTableNode()) &&
                (pPam->GetPoint()->nNode.GetIndex() >
                            pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                !pPam->GetNode()->FindFootnoteStartNode() ) )
            {
                if ( nParaCnt < 5 )
                    Show();     // bis hierhin schon mal anzeigen

                SvxAdjust eAdjust = aAttrTab.pAdjust
                    ? ((const SvxAdjustItem&)aAttrTab.pAdjust->GetItem()).
                                             GetAdjust()
                    : SVX_ADJUST_END;
                BuildTable( eAdjust );
            }
            else
                bInsertUnknown = bKeepUnknown;
        }
        break;

    // Listen
    case HTML_DIRLIST_ON:
    case HTML_MENULIST_ON:
    case HTML_ORDERLIST_ON:
    case HTML_UNORDERLIST_ON:
        if( nOpenParaToken )
            EndPara();
        NewNumBulList( nToken );
        break;

    case HTML_DIRLIST_OFF:
    case HTML_MENULIST_OFF:
    case HTML_ORDERLIST_OFF:
    case HTML_UNORDERLIST_OFF:
        if( nOpenParaToken )
            EndPara();
        EndNumBulListItem( 0, TRUE, GetNumInfo().GetDepth()==1 );
        EndNumBulList( nToken );
        break;

    case HTML_LI_ON:
    case HTML_LISTHEADER_ON:
        if( nOpenParaToken &&
            (pPam->GetPoint()->nContent.GetIndex()
            || HTML_PARABREAK_ON==nOpenParaToken) )
        {
            // nure bei <P><LI> den Absatz beenden, aber nicht bei <DD><LI>
            EndPara();
        }

        EndNumBulListItem( 0, FALSE );// <LI>/<LH> beenden und keine Vorl. setzen
        NewNumBulListItem( nToken );
        break;

    case HTML_LI_OFF:
    case HTML_LISTHEADER_OFF:
        EndNumBulListItem( nToken, FALSE );
        break;

    // Attribute :
    case HTML_ITALIC_ON:
        {
            NewStdAttr( HTML_ITALIC_ON, &aAttrTab.pItalic,
                        SvxPostureItem(ITALIC_NORMAL) );
        }
        break;

    case HTML_BOLD_ON:
        {
            NewStdAttr( HTML_BOLD_ON, &aAttrTab.pBold,
                        SvxWeightItem(WEIGHT_BOLD) );
        }
        break;


    case HTML_STRIKE_ON:
    case HTML_STRIKETHROUGH_ON:
        {
            NewStdAttr( HTML_STRIKE_ON, &aAttrTab.pStrike,
                        SvxCrossedOutItem(STRIKEOUT_SINGLE) );
        }
        break;

    case HTML_UNDERLINE_ON:
        {
            NewStdAttr( HTML_UNDERLINE_ON, &aAttrTab.pUnderline,
                        SvxUnderlineItem(UNDERLINE_SINGLE) );
        }
        break;

    case HTML_SUPERSCRIPT_ON:
        {
            NewStdAttr( HTML_SUPERSCRIPT_ON, &aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUPER,HTML_ESC_PROP) );
        }
        break;

    case HTML_SUBSCRIPT_ON:
        {
            NewStdAttr( HTML_SUBSCRIPT_ON, &aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUB,HTML_ESC_PROP) );
        }
        break;

    case HTML_BLINK_ON:
        {
            NewStdAttr( HTML_BLINK_ON, &aAttrTab.pBlink,
                        SvxBlinkItem( TRUE ) );
        }
        break;

    case HTML_SPAN_ON:
        NewStdAttr( HTML_SPAN_ON );
        break;


    case HTML_ITALIC_OFF:
    case HTML_BOLD_OFF:
    case HTML_STRIKE_OFF:
    case HTML_UNDERLINE_OFF:
    case HTML_SUPERSCRIPT_OFF:
    case HTML_SUBSCRIPT_OFF:
    case HTML_BLINK_OFF:
    case HTML_SPAN_OFF:
        EndTag( nToken );
        break;

    case HTML_STRIKETHROUGH_OFF:
        EndTag( HTML_STRIKE_OFF );
        break;

    case HTML_BASEFONT_ON:
        NewBasefontAttr();
        break;
    case HTML_BASEFONT_OFF:
        EndBasefontAttr();
        break;
    case HTML_FONT_ON:
    case HTML_BIGPRINT_ON:
    case HTML_SMALLPRINT_ON:
        NewFontAttr( nToken );
        break;
    case HTML_FONT_OFF:
    case HTML_BIGPRINT_OFF:
    case HTML_SMALLPRINT_OFF:
        EndFontAttr( nToken );
        break;

    case HTML_EMPHASIS_ON:
    case HTML_CITIATION_ON:
    case HTML_STRONG_ON:
    case HTML_CODE_ON:
    case HTML_SAMPLE_ON:
    case HTML_KEYBOARD_ON:
    case HTML_VARIABLE_ON:
    case HTML_DEFINSTANCE_ON:
    case HTML_SHORTQUOTE_ON:
    case HTML_LANGUAGE_ON:
    case HTML_AUTHOR_ON:
    case HTML_PERSON_ON:
    case HTML_ACRONYM_ON:
    case HTML_ABBREVIATION_ON:
    case HTML_INSERTEDTEXT_ON:
    case HTML_DELETEDTEXT_ON:

    case HTML_TELETYPE_ON:
        NewCharFmt( nToken );
        break;

    case HTML_SDFIELD_ON:
        NewField();
        bCallNextToken = bInField && pTable!=0;
        break;

    case HTML_EMPHASIS_OFF:
    case HTML_CITIATION_OFF:
    case HTML_STRONG_OFF:
    case HTML_CODE_OFF:
    case HTML_SAMPLE_OFF:
    case HTML_KEYBOARD_OFF:
    case HTML_VARIABLE_OFF:
    case HTML_DEFINSTANCE_OFF:
    case HTML_SHORTQUOTE_OFF:
    case HTML_LANGUAGE_OFF:
    case HTML_AUTHOR_OFF:
    case HTML_PERSON_OFF:
    case HTML_ACRONYM_OFF:
    case HTML_ABBREVIATION_OFF:
    case HTML_INSERTEDTEXT_OFF:
    case HTML_DELETEDTEXT_OFF:

    case HTML_TELETYPE_OFF:
        EndTag( nToken );
        break;

    case HTML_HEAD_OFF:
        if( aStyleSource.Len() )
        {
            pCSS1Parser->ParseStyleSheet( aStyleSource );
            aStyleSource.Erase();
        }
        break;

    case HTML_DOCTYPE:
    case HTML_BODY_OFF:
    case HTML_HTML_ON:
    case HTML_HTML_OFF:
    case HTML_HEAD_ON:
    case HTML_TITLE_OFF:
        break;      // nicht weiter auswerten, oder???

    case HTML_INPUT:
        InsertInput();
        break;

    case HTML_TEXTAREA_ON:
        NewTextArea();
        bCallNextToken = bTextArea && pTable!=0;
        break;

    case HTML_SELECT_ON:
        NewSelect();
        bCallNextToken = bSelect && pTable!=0;
        break;

    case HTML_ANCHOR_ON:
        NewAnchor();
        break;

    case HTML_ANCHOR_OFF:
        EndAnchor();
        break;

    case HTML_COMMENT:
        if( aToken.Len() > 5 )
        {
            // als Post-It einfuegen
            String aComment( aToken.Copy( 3, aToken.Len()-5 ) );
            aComment.EraseLeadingChars().EraseTrailingChars();
            InsertComment( aComment );
        }
        break;

    case HTML_MAP_ON:
        // Image Maps werden asynchron gelesen: Zunaechst wird nur eine
        // ImageMap angelegt. Die Bereiche kommen spaeter. Trozdem wird
        // die ImageMap schon in das IMap-Array eingetragen, denn sie
        // koennte ja schon verwendet werden.
        pImageMap = new ImageMap;
        if( ParseMapOptions( pImageMap) )
        {
            if( !pImageMaps )
                pImageMaps = new ImageMaps;
            pImageMaps->Insert( pImageMap, pImageMaps->Count() );
        }
        else
        {
            delete pImageMap;
            pImageMap = 0;
        }
        break;

    case HTML_MAP_OFF:
        // jetzt gibt es keine ImageMap mehr (IMap nicht Loeschen, denn
        // die stckt ja schon in dem Array!)
        pImageMap = 0;
        break;

    case HTML_AREA:
        if( pImageMap )
            ParseAreaOptions( pImageMap, SFX_EVENT_MOUSEOVER_OBJECT,
                                         SFX_EVENT_MOUSEOUT_OBJECT );
        break;

    case HTML_FRAMESET_ON:
        bInsertUnknown = bKeepUnknown;
        break;

    case HTML_NOFRAMES_ON:
        if( IsInHeader() )
            FinishHeader( TRUE );
        bInsertUnknown = bKeepUnknown;
        break;

    case HTML_UNKNOWNCONTROL_ON:
        // Im Header muss der Inhalt von unbekannten Token ignoriert werden,
        // es sei denn, das Token faengt mit einem '!' an.
        if( IsInHeader() && !IsReadPRE() && !aUnknownToken.Len() &&
            sSaveToken.Len() && '!' != sSaveToken.GetChar(0) &&
            '%' != sSaveToken.GetChar(0) )
            aUnknownToken = sSaveToken;
        // kein break

    default:
        bInsertUnknown = bKeepUnknown;
        break;
    }

    if( bGetIDOption )
        InsertIDOption();

    if( bInsertUnknown )
    {
        String aComment(
            String::CreateFromAscii(TOOLS_CONSTASCII_STRINGPARAM("HTML: <")) );
        if( (HTML_TOKEN_ONOFF & nToken) != 0 && (1 & nToken) != 0 )
            aComment += '/';
        aComment += sSaveToken;
        if( aToken.Len() )
        {
            UnescapeToken();
            (aComment += ' ') += aToken;
        }
        aComment += '>';
        InsertComment( aComment );
    }

    // wenn es noch vorlaefige Absatz-Attribute gibt, der Absatz aber
    // nicht leer ist, dann sind die Absatz-Attribute entgueltig.
    if( aParaAttrs.Count() && pPam->GetPoint()->nContent.GetIndex() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );
}

/*  */

BOOL SwHTMLParser::AppendTxtNode( SwHTMLAppendMode eMode, BOOL bUpdateNum )
{
    // Ein harter Zeilen-Umbruch am Ende muss immer entfernt werden.
    // Einen zweiten ersetzen wir durch einen Absatz-Abstand.
    xub_StrLen nLFStripped = StripTrailingLF();
    if( (AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode) && nLFStripped > 1 )
        eMode = AM_SPACE;

    // die harten Attribute an diesem Absatz werden nie mehr ungueltig
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    if( AM_SPACE==eMode || AM_NOSPACE==eMode )
    {
        SwTxtNode *pTxtNode =
            pDoc->GetNodes()[pPam->GetPoint()->nNode]->GetTxtNode();

        const SvxULSpaceItem& rULSpace =
            (const SvxULSpaceItem&)pTxtNode->SwCntntNode::GetAttr( RES_UL_SPACE );

        BOOL bChange = AM_NOSPACE==eMode ? rULSpace.GetLower() > 0
                                         : rULSpace.GetLower() == 0;

        if( bChange )
        {
            const SvxULSpaceItem& rCollULSpace =
                pTxtNode->GetAnyFmtColl().GetULSpace();

            BOOL bMayReset = AM_NOSPACE==eMode ? rCollULSpace.GetLower() == 0
                                               : rCollULSpace.GetLower() > 0;

            if( bMayReset &&
                rCollULSpace.GetUpper() == rULSpace.GetUpper() )
            {
                pTxtNode->ResetAttr( RES_UL_SPACE );
            }
            else
            {
                pTxtNode->SwCntntNode::SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(),
                         AM_NOSPACE==eMode ? 0 : HTML_PARSPACE ) );
            }
        }
    }
    bNoParSpace = AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode;

    SwPosition aOldPos( *pPam->GetPoint() );

    BOOL bRet = pDoc->AppendTxtNode( *pPam->GetPoint() );

    // Zeichen-Attribute aufspalten und ggf keine setzen, die ueber den
    // ganzen Absatz gesetzt sind
    const SwNodeIndex& rEndIdx = aOldPos.nNode;
    xub_StrLen nEndCnt = aOldPos.nContent.GetIndex();
    const SwPosition& rPos = *pPam->GetPoint();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( USHORT nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
         nCnt--; ++pTbl )
    {
        _HTMLAttr *pAttr = *pTbl;
        if( pAttr && pAttr->GetItem().Which() < RES_PARATR_BEGIN )
        {
            BOOL bWholePara = FALSE;

            while( pAttr )
            {
                _HTMLAttr *pNext = pAttr->GetNext();
                if( pAttr->GetSttParaIdx() < rEndIdx.GetIndex() ||
                    (!bWholePara &&
                     pAttr->GetSttPara() == rEndIdx &&
                     pAttr->GetSttCnt() != nEndCnt) )
                {
                    // den besehrigen Teil setzen
                    _HTMLAttr *pSetAttr =
                        pAttr->Clone( rEndIdx, nEndCnt );
                    pAttr->ClearPrev();

                    bWholePara =
                        pAttr->GetSttPara() == rEndIdx &&
                        pAttr->GetSttCnt() == 0;

                    // Wenn das Attribut den gesamten Absatz umspannt, werden
                    // alle auesseren Attribute nicht mehr beachtet. Deshalb
                    // darf es auch nicht in die Prev-Liste eines ausseren
                    // Attributs eingetragen werden, denn dieses wird ja
                    // erstmal nicht gesetzt. Das fuehrt zu verschiebenungen,
                    // wenn Felder ins Rennen kommen (siehe #51020#)
                    if( !pNext || bWholePara )
                    {
                        USHORT nTmp = pAttr->bInsAtStart ? 0
                                                         : aSetAttrTab.Count();
                        aSetAttrTab.Insert( pSetAttr, nTmp );
                    }
                    else
                        pNext->InsertPrev( pSetAttr );
                }

                pAttr->SetStart( rPos );
                pAttr = pNext;
            }
        }
    }

    if( bUpdateNum )
    {
        if( GetNumInfo().GetDepth() )
            SetNodeNum( GetNumInfo().GetLevel() | NO_NUMLEVEL );
        else
            pPam->GetNode()->GetTxtNode()->ResetAttr( RES_PARATR_NUMRULE );
    }

    // Attrubute im Absatz davor sollte man jetzt setzen (wegen JavaScript)
    SetAttr();

    if( !pTable && !--nParaCnt )
        Show();

    return bRet;
}

void SwHTMLParser::AddParSpace()
{
    if( !bNoParSpace )
        return;

    bNoParSpace = FALSE;

    ULONG nNdIdx = pPam->GetPoint()->nNode.GetIndex() - 1;

    SwTxtNode *pTxtNode = pDoc->GetNodes()[nNdIdx]->GetTxtNode();
    if( !pTxtNode )
        return;

    SvxULSpaceItem rULSpace =
        (const SvxULSpaceItem&)pTxtNode->SwCntntNode::GetAttr( RES_UL_SPACE );
    if( !rULSpace.GetLower() )
    {
        const SvxULSpaceItem& rCollULSpace =
            pTxtNode->GetAnyFmtColl().GetULSpace();
        if( rCollULSpace.GetLower() &&
            rCollULSpace.GetUpper() == rULSpace.GetUpper() )
        {
            pTxtNode->ResetAttr( RES_UL_SPACE );
        }
        else
        {
            pTxtNode->SwCntntNode::SetAttr(
                SvxULSpaceItem( rULSpace.GetUpper(), HTML_PARSPACE )  );
        }
    }
}


void SwHTMLParser::Show()
{
    // Hier wird
    // - ein EndAction gerufen, damit formatiert wird
    // - ein Reschedule gerufen,
    // - die eiegen View-Shell wieder gesetzt
    // - und Start-Action gerufen

    ASSERT( SVPAR_WORKING==eState, "Show nicht im Working-State - Das kann ins Auge gehen" );
    ViewShell *pOldVSh = CallEndAction();

    SetSaveBaseURL();
    GetpApp()->Reschedule();
    GetSaveAndSetOwnBaseURL();

    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->GetLinkCnt() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
    }

    // Die ViewShell nochmal holen, denn sie koennte im Reschedule
    // zerstoert wirden sein.
    ViewShell *pVSh = CallStartAction( pOldVSh );
    ASSERT( pOldVSh == pVSh, "Show:: Shell wurde ausgetauscht" );

    // ist der aktuelle Node nicht mehr sichtbar, dann benutzen wir
    // eine groessere Schrittweite
    if( pVSh )
        nParaCnt = pDoc->GetNodes()[pPam->GetPoint()->nNode]
                       ->IsVisible(pVSh) ? 5 : 50;
}

void SwHTMLParser::ShowStatline()
{
    // Hier wird
    // - ein Reschedule gerufen, damit gescrollt werden kann
    // - die eiegen View-Shell wieder gesetzt
    // - ein Start/End-Action gerufen, wenn gescrollt wurde.

    ASSERT( SVPAR_WORKING==eState, "ShowStatLine nicht im Working-State - Das kann ins Auge gehen" );

    // Laufbalkenanzeige
    SetSaveBaseURL();
    if( !GetMedium() || !GetMedium()->IsRemote() )
    {
        ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        GetSaveAndSetOwnBaseURL();
        CheckActionViewShell();
    }
    else
    {
        GetpApp()->Reschedule();
        GetSaveAndSetOwnBaseURL();

        if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
            || 1 == pDoc->GetLinkCnt() )
            // wurde der Import vom SFX abgebrochen?
            eState = SVPAR_ERROR;

        ViewShell *pVSh = CheckActionViewShell();
        if( pVSh && pVSh->HasInvalidRect() )
        {
            CallEndAction( FALSE, FALSE );
            CallStartAction( pVSh, FALSE );
        }
    }
}

ViewShell *SwHTMLParser::CallStartAction( ViewShell *pVSh, BOOL bChkPtr )
{
    ASSERT( !pActionViewShell, "CallStartAction: ViewShell schon gesetzt" );

    if( !pVSh || bChkPtr )
    {
        ViewShell *pOldVSh = pVSh;
        pDoc->GetEditShell( &pVSh );
        ASSERT( !pVSh || !pOldVSh || pOldVSh == pVSh,
                "CallStartAction: Wer hat die ViewShell ausgetauscht?" );
#ifdef DEBUG
        if( pOldVSh && !pVSh )
            pVSh = 0;
#endif
    }
    pActionViewShell = pVSh;

    if( pActionViewShell )
    {
        if( pActionViewShell->ISA( SwEditShell ) )
            ((SwEditShell*)pActionViewShell)->StartAction();
        else
            pActionViewShell->StartAction();
    }

    return pActionViewShell;
}

ViewShell *SwHTMLParser::CallEndAction( BOOL bChkAction, BOOL bChkPtr )
{
    if( bChkPtr )
    {
        ViewShell *pVSh = 0;
        pDoc->GetEditShell( &pVSh );
        ASSERT( !pVSh || pActionViewShell == pVSh,
                "CallEndAction: Wer hat die ViewShell ausgetauscht?" );
#ifdef DEBUG
        if( pActionViewShell && !pVSh )
            pVSh = 0;
#endif
        if( pVSh != pActionViewShell )
            pActionViewShell = 0;
    }

    if( !pActionViewShell || (bChkAction && !pActionViewShell->ActionPend()) )
        return pActionViewShell;

    if( bSetCrsr )
    {
        // an allen CrsrEditShells die Cursor auf den Doc-Anfang setzen
        ViewShell *pSh = pActionViewShell;
        do {
            if( pSh->IsA( TYPE( SwCrsrShell ) ) )
                ((SwCrsrShell*)pSh)->SttDoc();
            pSh = (ViewShell *)pSh->GetNext();
        } while( pSh != pActionViewShell );

        bSetCrsr = FALSE;
    }
    if( pActionViewShell->ISA( SwEditShell ) )
    {
        //Schon gescrollt?, dann dafuer sorgen, dass die View sich nicht bewegt!
        const FASTBOOL bOldLock = pActionViewShell->IsViewLocked();
        pActionViewShell->LockView( TRUE );
        const FASTBOOL bOldEndActionByVirDev = pActionViewShell->IsEndActionByVirDev();
        pActionViewShell->SetEndActionByVirDev( TRUE );;
        ((SwEditShell*)pActionViewShell)->EndAction();
        pActionViewShell->SetEndActionByVirDev( bOldEndActionByVirDev );
        pActionViewShell->LockView( bOldLock );

        // bChkJumpMark ist nur gesetzt, wenn das Object auch gefunden wurde
        if( bChkJumpMark )
        {
            const Point aVisSttPos( DOCUMENTBORDER, DOCUMENTBORDER );
            if( GetMedium() && aVisSttPos == pActionViewShell->VisArea().Pos() )
                ::JumpToSwMark( pActionViewShell,
                                GetMedium()->GetURLObject().GetMark() );
            bChkJumpMark = FALSE;
        }
    }
    else
        pActionViewShell->EndAction();

    // sollte der Parser der Letzte sein, der das Doc haelt, dann kann
    // man hier abbrechen und einen Fehler setzen.
    if( 1 == pDoc->GetLinkCnt() )
    {
        eState = SVPAR_ERROR;
    }

    ViewShell *pVSh = pActionViewShell;
    pActionViewShell = 0;

    return pVSh;
}

ViewShell *SwHTMLParser::CheckActionViewShell()
{
    ViewShell *pVSh = 0;
    pDoc->GetEditShell( &pVSh );
    ASSERT( !pVSh || pActionViewShell == pVSh,
            "CheckActionViewShell: Wer hat die ViewShell ausgetauscht?" );
#ifdef DEBUG
    if( pActionViewShell && !pVSh )
        pVSh = 0;
#endif
    if( pVSh != pActionViewShell )
        pActionViewShell = 0;

    return pActionViewShell;
}

/*  */

void SwHTMLParser::_SetAttr( BOOL bChkEnd, BOOL bBeforeTable,
                             _HTMLAttrs *pPostIts )
{
    SwPaM* pAttrPam = new SwPaM( *pPam->GetPoint() );
    const SwNodeIndex& rEndIdx = pPam->GetPoint()->nNode;
    xub_StrLen nEndCnt = pPam->GetPoint()->nContent.GetIndex();
    _HTMLAttr* pAttr;
    SwCntntNode* pCNd;

    _HTMLAttrs aFields( 5, 5 );

    for( USHORT n = aSetAttrTab.Count(); n; )
    {
        pAttr = aSetAttrTab[ --n ];
        USHORT nWhich = pAttr->pItem->Which();

        ULONG nEndParaIdx = pAttr->GetEndParaIdx();
        BOOL bSetAttr;
        if( bChkEnd )
        {
            // fix #42192#: Zechen-Attribute mit Ende moeglich frueh,
            // also noch im aktuellen Absatz setzen (wegen JavaScript
            // und diversen Chats). das darf man aber nicht fuer Attribute,
            // die ueber den ganzen Absatz aufgspannt werden sollen, weil
            // sie aus Absatzvorlgen stammen, die nicht gesetzt werden
            // koennen. Weil die Attribute mit SETATTR_DONTREPLACE
            // eingefuegt werden, sollte man sie auch anchtraeglich
            // noch setzen koennen.
            bSetAttr = ( nEndParaIdx < rEndIdx.GetIndex() &&
                         (RES_LR_SPACE != nWhich || !GetNumInfo().GetNumRule()) ) ||
                       ( !pAttr->IsLikePara() &&
                         nEndParaIdx == rEndIdx.GetIndex() &&
                         pAttr->GetEndCnt() < nEndCnt &&
                         RES_CHRATR_BEGIN <= nWhich &&
                         RES_TXTATR_WITHEND_END > nWhich ) ||
                       ( bBeforeTable &&
                         nEndParaIdx == rEndIdx.GetIndex() &&
                         !pAttr->GetEndCnt() );
        }
        else
        {
            // Attribiute im Content-Bereich duerfen nicht gesetzt
            // werden, wenn wir in einem Sonderbereich stehen, aber
            // umgekekehrt schon.
            ULONG nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bSetAttr = nEndParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nEndParaIdx <= nEndOfIcons;
        }

        if( bSetAttr )
        {
            // Das Attribute darf nicht in der liste der vorlaeufigen
            // Absatz-Attribute stehen, weil es sonst geloescht wurde.
            USHORT ii = aParaAttrs.Count();
            while( ii-- )
            {
                ASSERT( pAttr != aParaAttrs[ii],
                        "SetAttr: Attribut duerfte noch nicht gesetzt werden" );
                aParaAttrs.Remove( ii );
            }


            // dann also setzen
            aSetAttrTab.Remove( n, 1 );

            while( pAttr )
            {
                _HTMLAttr *pPrev = pAttr->GetPrev();
                if( !pAttr->bValid )
                {
                    // ungueltige Attribute koennen gloescht werden
                    delete pAttr;
                    pAttr = pPrev;
                    continue; //break;
                }


                pCNd = pDoc->GetNodes()[ pAttr->nSttPara ]->GetCntntNode();
                if( !pCNd )
                {
                    // durch die elende Loescherei von Nodes kann auch mal
                    // ein Index auf einen End-Node zeigen :-(
                    if( pAttr->GetSttPara() == pAttr->GetEndPara() &&
                        (nWhich < RES_TXTATR_NOEND_BEGIN ||
                         nWhich >= RES_TXTATR_NOEND_END) )
                    {
                        // wenn der End-Index auch auf den Node zeigt
                        // brauchen wir auch kein Attribut mehr zu setzen,
                        // es sei denn, es ist ein Text-Attribut.
                        delete pAttr;
                        pAttr = pPrev;
                        continue; //break;
                    }
                    pCNd = pDoc->GetNodes().GoNext( &(pAttr->nSttPara) );
                    if( pCNd )
                        pAttr->nSttCntnt = 0;
                    else
                    {
                        ASSERT( !this, "SetAttr: GoNext() failed!" );
                        delete pAttr;
                        pAttr = pPrev;
                        continue; // break;
                    }
                }
                pAttrPam->GetPoint()->nNode = pAttr->nSttPara;



                // durch das Loeschen von BRs kann der Start-Index
                // auch mal hinter das Ende des Textes zeigen
                if( pAttr->nSttCntnt > pCNd->Len() )
                    pAttr->nSttCntnt = pCNd->Len();
                pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nSttCntnt );

                pAttrPam->SetMark();
                if( pAttr->GetSttPara() != pAttr->GetEndPara() &&
                    (nWhich < RES_TXTATR_NOEND_BEGIN ||
                     nWhich >= RES_TXTATR_NOEND_END) )
                {
                    pCNd = pDoc->GetNodes()[ pAttr->nEndPara ]->GetCntntNode();
                    if( !pCNd )
                    {
                        pCNd = pDoc->GetNodes().GoPrevious( &(pAttr->nEndPara) );
                        if( pCNd )
                            pAttr->nEndCntnt = pCNd->Len();
                        else
                        {
                            ASSERT( !this, "SetAttr: GoPrevious() failed!" );
                            pAttrPam->DeleteMark();
                            delete pAttr;
                            pAttr = pPrev;
                            continue; // break;
                        }
                    }

                    pAttrPam->GetPoint()->nNode = pAttr->nEndPara;
                }
                else if( pAttr->IsLikePara() )
                {
                    pAttr->nEndCntnt = pCNd->Len();
                }

                // durch das Loeschen von BRs kann der End-Index
                // auch mal hinter das Ende des Textes zeigen
                if( pAttr->nEndCntnt > pCNd->Len() )
                    pAttr->nEndCntnt = pCNd->Len();

                pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nEndCntnt );
                if( bBeforeTable &&
                    pAttrPam->GetPoint()->nNode.GetIndex() ==
                        rEndIdx.GetIndex() )
                {
                    // wenn wir vor dem Einfuegen einer Tabelle stehen
                    // und das Attribut im aktuellen Node beendet wird,
                    // muessen wir es im Node davor beenden oder wegschmeissen,
                    // wenn es erst in dem Node beginnt
                    if( nWhich != RES_BREAK && nWhich != RES_PAGEDESC &&
                        (nWhich < RES_TXTATR_NOEND_BEGIN ||
                         nWhich >= RES_TXTATR_NOEND_END) )
                    {
                        if( pAttrPam->GetMark()->nNode.GetIndex() !=
                            rEndIdx.GetIndex() )
                        {
                            ASSERT( !pAttrPam->GetPoint()->nContent.GetIndex(),
                                    "Content-Position vor Tabelle nicht 0???" );
                            pAttrPam->Move( fnMoveBackward );
                        }
                        else
                        {
                            pAttrPam->DeleteMark();
                            delete pAttr;
                            pAttr = pPrev;
                            continue;
                        }
                    }
                }

                switch( nWhich )
                {
                case RES_FLTR_BOOKMARK:     // dann also ein Bookmark einfuegen
                    {
                        String aName( ((SfxStringItem*)pAttr->pItem)->GetValue() );
                        USHORT nBookPos = pDoc->FindBookmark( aName );
                        if( nBookPos != USHRT_MAX )
                        {
                            const SwBookmark *pBkMk =
                                pDoc->GetBookmarks()[nBookPos];
                            if( pBkMk->GetPos() != *pAttrPam->GetPoint() )
                                pDoc->MakeUniqueBookmarkName( aName );
                            else
                                break; // keine doppelte Bookmark an dieser Pos
                        }
                        pAttrPam->DeleteMark();
                        pDoc->MakeBookmark( *pAttrPam, KeyCode(),
                                            aName, aEmptyStr );

                        // ggfs. ein Bookmark anspringen
                        if( JUMPTO_MARK == eJumpTo &&
                            aName == sJmpMark )
                        {
                            bChkJumpMark = TRUE;
                            eJumpTo = JUMPTO_NONE;
                        }
                    }
                    break;
                case RES_TXTATR_FIELD:
                    {
                        USHORT nFldWhich =
                            pPostIts ? ((const SwFmtFld *)pAttr->pItem)
                                            ->GetFld()->GetTyp()->Which() : 0;
                        if( pPostIts && (RES_POSTITFLD == nFldWhich ||
                                         RES_SCRIPTFLD == nFldWhich) )
                        {
                            pPostIts->Insert( pAttr, 0 );
                        }
                        else
                        {
                            aFields.Insert( pAttr, aFields.Count() );
                        }
                    }
                    pAttrPam->DeleteMark();
                    pAttr = pPrev;
                    continue;

                case RES_LR_SPACE:
                    if( pAttrPam->GetPoint()->nNode.GetIndex() ==
                        pAttrPam->GetMark()->nNode.GetIndex() &&
                        pCNd )
                    {
                        // wegen Numerierungen dieses Attribut direkt
                        // am Node setzen
                        pCNd->SetAttr( *pAttr->pItem );
                        break;
                    }
                    ASSERT( !this,
                            "LRSpace ueber mehrere Absaetze gesetzt!" );
                    // kein break (hier sollen wir trotzdem nie hinkommen;
                default:

                    // ggfs. ein Bookmark anspringen
                    if( RES_TXTATR_INETFMT == nWhich &&
                        JUMPTO_MARK == eJumpTo &&
                        sJmpMark == ((SwFmtINetFmt*)pAttr->pItem)->GetName() )
                    {
                        bChkJumpMark = TRUE;
                        eJumpTo = JUMPTO_NONE;
                    }

                    pDoc->Insert( *pAttrPam, *pAttr->pItem, SETATTR_DONTREPLACE );
                }
                pAttrPam->DeleteMark();

                delete pAttr;
                pAttr = pPrev;
            }
        }
    }

    for( n = aMoveFlyFrms.Count(); n; )
    {
        SwFrmFmt *pFrmFmt = aMoveFlyFrms[ --n ];

        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        ASSERT( FLY_AT_CNTNT==rAnchor.GetAnchorId(),
                "Nur Auto-Rahmen brauchen eine Spezialbehandlung" );
        const SwPosition *pFlyPos = rAnchor.GetCntntAnchor();
        ULONG nFlyParaIdx = pFlyPos->nNode.GetIndex();
        BOOL bMoveFly;
        if( bChkEnd )
        {
            bMoveFly = nFlyParaIdx < rEndIdx.GetIndex() ||
                       ( nFlyParaIdx == rEndIdx.GetIndex() &&
                         aMoveFlyCnts[n] < nEndCnt );
        }
        else
        {
            ULONG nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bMoveFly = nFlyParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nFlyParaIdx <= nEndOfIcons;
        }
        if( bMoveFly )
        {
            pFrmFmt->DelFrms();
            *pAttrPam->GetPoint() = *pFlyPos;
            pAttrPam->GetPoint()->nContent.Assign( pAttrPam->GetCntntNode(),
                                                   aMoveFlyCnts[n] );
            SwFmtAnchor aAnchor( rAnchor );
            aAnchor.SetType( FLY_AUTO_CNTNT );
            aAnchor.SetAnchor( pAttrPam->GetPoint() );
            pFrmFmt->SetAttr( aAnchor );

            const SwFmtHoriOrient& rHoriOri = pFrmFmt->GetHoriOrient();
            if( HORI_LEFT == rHoriOri.GetHoriOrient() )
            {
                SwFmtHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetRelationOrient( REL_CHAR );
                pFrmFmt->SetAttr( aHoriOri );
            }
            const SwFmtVertOrient& rVertOri = pFrmFmt->GetVertOrient();
            if( VERT_TOP == rVertOri.GetVertOrient() )
            {
                SwFmtVertOrient aVertOri( rVertOri );
                aVertOri.SetRelationOrient( REL_CHAR );
                pFrmFmt->SetAttr( aVertOri );
            }

            pFrmFmt->MakeFrms();
            aMoveFlyFrms.Remove( n, 1 );
            aMoveFlyCnts.Remove( n, 1 );
        }
    }
    while( aFields.Count() )
    {
        pAttr = aFields[0];

        pCNd = pDoc->GetNodes()[ pAttr->nSttPara ]->GetCntntNode();
        pAttrPam->GetPoint()->nNode = pAttr->nSttPara;
        pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nSttCntnt );

        if( bBeforeTable &&
            pAttrPam->GetPoint()->nNode.GetIndex() == rEndIdx.GetIndex() )
        {
            ASSERT( !bBeforeTable, "Aha, der Fall tritt also doch ein" );
            ASSERT( !pAttrPam->GetPoint()->nContent.GetIndex(),
                    "Content-Position vor Tabelle nicht 0???" );
            // !!!
            pAttrPam->Move( fnMoveBackward );
        }

        pDoc->Insert( *pAttrPam, *pAttr->pItem );

        aFields.Remove( 0, 1 );
        delete pAttr;
    }

    delete pAttrPam;
}

void SwHTMLParser::NewAttr( _HTMLAttr **ppAttr, const SfxPoolItem& rItem )
{
    // Font-Hoehen und -Farben- sowie Escapement-Attribute duerfen nicht
    // zusammengefasst werden. Sie werden deshalb in einer Liste gespeichert,
    // in der das zuletzt aufgespannte Attribut vorne steht und der Count
    // immer 1 ist. Fuer alle anderen Attribute wird der Count einfach
    // hochgezaehlt.
    if( *ppAttr )
    {
        _HTMLAttr *pAttr = new _HTMLAttr( *pPam->GetPoint(), rItem,
                                            ppAttr );
        pAttr->InsertNext( *ppAttr );
        (*ppAttr) = pAttr;
    }
    else
        (*ppAttr) = new _HTMLAttr( *pPam->GetPoint(), rItem, ppAttr );
}

void SwHTMLParser::EndAttr( _HTMLAttr* pAttr, _HTMLAttr **ppDepAttr,
                            BOOL bChkEmpty )
{
    ASSERT( !ppDepAttr, "SwHTMLParser::EndAttr: ppDepAttr-Feature ungetestet?" );
    // Der Listenkopf ist im Attribut gespeichert
    _HTMLAttr **ppHead = pAttr->ppHead;

    ASSERT( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    // die aktuelle Psoition als Ende-Position merken
    const SwNodeIndex* pEndIdx = &pPam->GetPoint()->nNode;
    xub_StrLen nEndCnt = pPam->GetPoint()->nContent.GetIndex();

    // WIrd das zueltzt gestartete oder ein frueher gestartetes Attribut
    // beendet?
    _HTMLAttr *pLast = 0;
    if( ppHead && pAttr != *ppHead )
    {
        // Es wird nicht das zuletzt gestartete Attribut beendet

        // Dann suche wir das unmittelbar danach gestartete Attribut, das
        // ja ebenfalls noch nicht beendet wurde (sonst stuende es nicht
        // mehr in der Liste
        pLast = *ppHead;
        while( pLast && pLast->GetNext() != pAttr )
            pLast = pLast->GetNext();

        ASSERT( pLast, "Attribut nicht in eigener Liste gefunden!" );

        // das Attribut nicht an der PaM-Psoition beenden, sondern da,
        // wo das danch gestartete Attribut anfing???
        //pEndIdx = &pPrev->GetSttPara();
        //nEndCnt = pPrev->GetSttCnt();
    }

    BOOL bMoveBack = FALSE;
    USHORT nWhich = pAttr->pItem->Which();
    if( /*!pLast &&*/ !nEndCnt && RES_PARATR_BEGIN <= nWhich &&
        *pEndIdx != pAttr->GetSttPara() )
    {
        // dann eine Cntntnt Position zurueck!
        bMoveBack = pPam->Move( fnMoveBackward );
        nEndCnt = pPam->GetPoint()->nContent.GetIndex();
    }

    // nun das Attrubut beenden
    _HTMLAttr *pNext = pAttr->GetNext();

    // ein Bereich ??
    if( !bChkEmpty || (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
        RES_PAGEDESC == nWhich || RES_BREAK == nWhich ||
        *pEndIdx != pAttr->GetSttPara() ||
        nEndCnt != pAttr->GetSttCnt() )
    {
        pAttr->nEndPara = *pEndIdx;
        pAttr->nEndCntnt = nEndCnt;
        pAttr->bInsAtStart = RES_TXTATR_INETFMT != nWhich &&
                             RES_TXTATR_CHARFMT != nWhich;

        if( !pNext )
        {
            // keine offenen Attribute dieses Typs mehr da,
            // dann koennen alle gesetzt werden, es sei denn
            // sie haengen noch von einem anderen Attribut ab,
            // dann werden sie dort angehaengt
            if( ppDepAttr && *ppDepAttr )
                (*ppDepAttr)->InsertPrev( pAttr );
            else
            {
                USHORT nTmp = pAttr->bInsAtStart ? 0 : aSetAttrTab.Count();
                aSetAttrTab.Insert( pAttr, nTmp );
            }
        }
        else
        {
            // es gibt noch andere offene Attribute des Typs,
            // daher muss das Setzen zurueckgestellt werden.
            // das aktuelle Attribut wird deshalb hinten an die
            // Previous-Liste des Nachfolgers angehaengt
            pNext->InsertPrev( pAttr );
        }
    }
    else
    {
        // dann nicht einfuegen, sondern Loeschen. Durch das "tuerken" von
        // Vorlagen durch harte Attributierung koennen sich auch mal andere
        // leere Attribute in der Prev-Liste befinden, die dann trotzdem
        // gesetzt werden muessen
        _HTMLAttr *pPrev = pAttr->GetPrev();
        delete pAttr;

        if( pPrev )
        {
            // Die Previous-Attribute muessen trotzdem gesetzt werden.
            if( pNext )
                pNext->InsertPrev( pPrev );
            else
            {
                USHORT nTmp = pPrev->bInsAtStart ? 0 : aSetAttrTab.Count();
                aSetAttrTab.Insert( pPrev, nTmp );
            }
        }

    }

    // wenn das erste Attribut der Liste gesetzt wurde muss noch der
    // Listenkopf korrigiert werden.
    if( pLast )
        pLast->pNext = pNext;
    else if( ppHead )
        *ppHead = pNext;

    if( bMoveBack )
        pPam->Move( fnMoveForward );
}

void SwHTMLParser::DeleteAttr( _HTMLAttr* pAttr )
{
    // Hier darf es keine vorlauefigen Absatz-Attribute geben, den die
    // koennten jetzt gesetzt werden und dann sind die Zeiger ungueltig!!!
    ASSERT( !aParaAttrs.Count(),
            "Hoechste Gefahr: Es gibt noch nicht-endgueltige Absatz-Attribute" );
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    // Der Listenkopf ist im Attribut gespeichert
    _HTMLAttr **ppHead = pAttr->ppHead;

    ASSERT( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    // Wird das zueltzt gestartete oder ein frueher gestartetes Attribut
    // entfernt?
    _HTMLAttr *pLast = 0;
    if( ppHead && pAttr != *ppHead )
    {
        // Es wird nicht das zuletzt gestartete Attribut beendet

        // Dann suche wir das unmittelbar danach gestartete Attribut, das
        // ja ebenfalls noch nicht beendet wurde (sonst stuende es nicht
        // mehr in der Liste
        pLast = *ppHead;
        while( pLast && pLast->GetNext() != pAttr )
            pLast = pLast->GetNext();

        ASSERT( pLast, "Attribut nicht in eigener Liste gefunden!" );
    }

    // nun das Attrubut entfernen
    _HTMLAttr *pNext = pAttr->GetNext();
    _HTMLAttr *pPrev = pAttr->GetPrev();
    delete pAttr;

    if( pPrev )
    {
        // Die Previous-Attribute muessen trotzdem gesetzt werden.
        if( pNext )
            pNext->InsertPrev( pPrev );
        else
        {
            USHORT nTmp = pPrev->bInsAtStart ? 0 : aSetAttrTab.Count();
            aSetAttrTab.Insert( pPrev, nTmp );
        }
    }

    // wenn das erste Attribut der Liste entfernt wurde muss noch der
    // Listenkopf korrigiert werden.
    if( pLast )
        pLast->pNext = pNext;
    else if( ppHead )
        *ppHead = pNext;
}

void SwHTMLParser::SaveAttrTab( _HTMLAttrTable& rNewAttrTab )
{
    // Hier darf es keine vorlauefigen Absatz-Attribute geben, den die
    // koennten jetzt gesetzt werden und dann sind die Zeiger ungueltig!!!
    ASSERT( !aParaAttrs.Count(),
            "Hoechste Gefahr: Es gibt noch nicht-endgueltige Absatz-Attribute" );
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;

    for( USHORT nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
         nCnt--; (++pTbl, ++pSaveTbl) )
    {
        *pSaveTbl = *pTbl;

        _HTMLAttr *pAttr = *pSaveTbl;
        while( pAttr )
        {
            pAttr->SetHead( pSaveTbl );
            pAttr = pAttr->GetNext();
        }

        *pTbl = 0;
    }
}

void SwHTMLParser::SplitAttrTab( _HTMLAttrTable& rNewAttrTab,
                                 BOOL bMoveEndBack )
{
    // Hier darf es keine vorlauefigen Absatz-Attribute geben, den die
    // koennten jetzt gesetzt werden und dann sind die Zeiger ungueltig!!!
    ASSERT( !aParaAttrs.Count(),
            "Hoechste Gefahr: Es gibt noch nicht-endgueltige Absatz-Attribute" );
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    const SwNodeIndex& nSttIdx = pPam->GetPoint()->nNode;
    SwNodeIndex nEndIdx( nSttIdx );

    // alle noch offenen Attribute beenden und hinter der Tabelle
    // neu aufspannen
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;
    BOOL bSetAttr = TRUE;
    xub_StrLen nSttCnt = pPam->GetPoint()->nContent.GetIndex();
    xub_StrLen nEndCnt = nSttCnt;

    if( bMoveEndBack )
    {
        ULONG nOldEnd = nEndIdx.GetIndex();
        ULONG nTmpIdx;
        if( ( nTmpIdx = pDoc->GetNodes().GetEndOfExtras().GetIndex()) >= nOldEnd ||
            ( nTmpIdx = pDoc->GetNodes().GetEndOfAutotext().GetIndex()) >= nOldEnd )
        {
            nTmpIdx = pDoc->GetNodes().GetEndOfInserts().GetIndex();
        }
        SwCntntNode* pCNd = pDoc->GetNodes().GoPrevious(&nEndIdx);

        // keine Attribute setzen, wenn der PaM aus dem Content-Bereich
        // herausgeschoben wurde.
        bSetAttr = pCNd && nTmpIdx < nEndIdx.GetIndex();

        nEndCnt = (bSetAttr ? pCNd->Len() : 0);
    }
    for( USHORT nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
         nCnt--; (++pTbl, ++pSaveTbl) )
    {
        _HTMLAttr *pAttr = *pTbl;
        *pSaveTbl = 0;
        while( pAttr )
        {
            _HTMLAttr *pNext = pAttr->GetNext();
            _HTMLAttr *pPrev = pAttr->GetPrev();

            if( bSetAttr &&
                ( pAttr->GetSttParaIdx() < nEndIdx.GetIndex() ||
                  (pAttr->GetSttPara() == nEndIdx &&
                   pAttr->GetSttCnt() != nEndCnt) ) )
            {
                // das Attribut muss vor der Liste gesetzt werden. Da wir
                // das Original noch brauchen, weil Zeiger auf das Attribut
                // noch in den Kontexten existieren, muessen wir es clonen.
                // Die Next-Liste geht dabei verloren, aber die
                // Previous-Liste bleibt erhalten
                _HTMLAttr *pSetAttr = pAttr->Clone( nEndIdx, nEndCnt );

                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    USHORT nTmp = pSetAttr->bInsAtStart ? 0
                                                        : aSetAttrTab.Count();
                    aSetAttrTab.Insert( pSetAttr, nTmp );
                }
            }
            else if( pPrev )
            {
                // Wenn das Attribut nicht gesetzt vor der Tabelle
                // gesetzt werden muss, muessen der Previous-Attribute
                // trotzdem gesetzt werden.
                if( pNext )
                    pNext->InsertPrev( pPrev );
                else
                {
                    USHORT nTmp = pPrev->bInsAtStart ? 0 : aSetAttrTab.Count();
                    aSetAttrTab.Insert( pPrev, nTmp );
                }
            }

            // den Start des Attributs neu setzen und die Verkettungen
            // aufbrechen
            pAttr->Reset( nSttIdx, nSttCnt, pSaveTbl );

            if( *pSaveTbl )
            {
                _HTMLAttr *pSAttr = *pSaveTbl;
                while( pSAttr->GetNext() )
                    pSAttr = pSAttr->GetNext();
                pSAttr->InsertNext( pAttr );
            }
            else
                *pSaveTbl = pAttr;

            pAttr = pNext;
        }

        *pTbl = 0;
    }
}

void SwHTMLParser::RestoreAttrTab( const _HTMLAttrTable& rNewAttrTab,
                                   BOOL bSetNewStart )
{
    // Hier darf es keine vorlauefigen Absatz-Attribute geben, den die
    // koennten jetzt gesetzt werden und dann sind die Zeiger ungueltig!!!
    ASSERT( !aParaAttrs.Count(),
            "Hoechste Gefahr: Es gibt noch nicht-endgueltige Absatz-Attribute" );
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;

    for( USHORT nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
        nCnt--; (++pTbl, ++pSaveTbl) )
    {
        ASSERT( !*pTbl, "Die Attribut-Tabelle ist nicht leer!" );

        const SwPosition *pPos = pPam->GetPoint();
        const SwNodeIndex& rSttPara = pPos->nNode;
        xub_StrLen nSttCnt = pPos->nContent.GetIndex();

        *pTbl = *pSaveTbl;

        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            ASSERT( !pAttr->GetPrev() || !pAttr->GetPrev()->ppHead,
                    "Previous-Attribut hat noch einen Header" );
            pAttr->SetHead( pTbl );
            if( bSetNewStart )
            {
                pAttr->nSttPara = rSttPara;
                pAttr->nEndPara = rSttPara;
                pAttr->nSttCntnt = nSttCnt;
                pAttr->nEndCntnt = nSttCnt;
            }
            pAttr = pAttr->GetNext();
        }

        *pSaveTbl = 0;
    }
}

void SwHTMLParser::InsertAttr( const SfxPoolItem& rItem, BOOL bLikePara,
                               BOOL bInsAtStart )
{
    _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(),
                                     rItem );
    if( bLikePara )
        pTmp->SetLikePara();
    USHORT nTmp = bInsAtStart ? 0 : aSetAttrTab.Count();
    aSetAttrTab.Insert( pTmp, nTmp );
}

void SwHTMLParser::InsertAttrs( _HTMLAttrs& rAttrs )
{
    while( rAttrs.Count() )
    {
        _HTMLAttr *pAttr = rAttrs[0];
        InsertAttr( pAttr->GetItem() );
        rAttrs.Remove( 0, 1 );
        delete pAttr;
    }
}

/*  */

void SwHTMLParser::NewStdAttr( int nToken )
{
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            if( HTML_SPAN_ON != nToken || !aClass.Len() ||
                !CreateContainer( aClass, aItemSet, aPropInfo, pCntxt ) )
                DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::NewStdAttr( int nToken, _HTMLAttr **ppAttr,
                               const SfxPoolItem & rItem )
{
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        aItemSet.Put( rItem );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
    }
    else
    {
        InsertAttr( ppAttr ,rItem, pCntxt );
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::EndTag( int nToken )
{
    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( nToken & ~1 );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }
}


void SwHTMLParser::NewBasefontAttr()
{
    String aId, aStyle, aClass;
    USHORT nSize = 3;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_SIZE:
            nSize = (USHORT)pOption->GetNumber();
            break;
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        }
    }

    if( nSize < 1 )
        nSize = 1;

    if( nSize > 7 )
        nSize = 7;

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_BASEFONT_ON );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        aItemSet.Put( SvxFontHeightItem( aFontHeights[nSize-1] ) );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
    }
    else
    {
        InsertAttr( &aAttrTab.pFontHeight, SvxFontHeightItem( aFontHeights[nSize-1] ), pCntxt );
    }

    // den Kontext merken
    PushContext( pCntxt );

    // die Font-Size merken
    aBaseFontStack.Insert( nSize, aBaseFontStack.Count() );
}

void SwHTMLParser::EndBasefontAttr()
{
    EndTag( HTML_BASEFONT_ON );

    // Stack-Unterlauf in Tabellen vermeiden
    if( aBaseFontStack.Count() > nBaseFontStMin )
        aBaseFontStack.Remove( aBaseFontStack.Count()-1, 1 );
}

void SwHTMLParser::NewFontAttr( int nToken )
{
    USHORT nBaseSize =
        ( aBaseFontStack.Count() > nBaseFontStMin
            ? (aBaseFontStack[aBaseFontStack.Count()-1] & FONTSIZE_MASK)
            : 3 );
    USHORT nFontSize =
        ( aFontStack.Count() > nFontStMin
            ? (aFontStack[aFontStack.Count()-1] & FONTSIZE_MASK)
            : nBaseSize );

    String aFace, aId, aStyle, aClass;
    Color aColor;
    ULONG nFontHeight = 0;  // tatsaechlich einzustellende Font-Hoehe
    USHORT nSize = 0;       // Fontgroesse in Netscape-Notation (1-7)
    BOOL bColor = FALSE;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_SIZE:
            if( HTML_FONT_ON==nToken && pOption->GetString().Len() )
            {
                INT32 nSSize;
                if( '+' == pOption->GetString().GetChar(0) ||
                    '-' == pOption->GetString().GetChar(0) )
                    nSSize = nBaseSize + pOption->GetSNumber();
                else
                    nSSize = (INT32)pOption->GetNumber();

                if( nSSize < 1 )
                    nSSize = 1;
                else if( nSSize > 7 )
                    nSSize = 7;

                nSize = (USHORT)nSSize;
                nFontHeight = aFontHeights[nSize-1];
            }
            break;
        case HTML_O_COLOR:
            if( HTML_FONT_ON==nToken )
            {
                pOption->GetColor( aColor );
                bColor = TRUE;
            }
            break;
        case HTML_O_FACE:
            if( HTML_FONT_ON==nToken )
                aFace = pOption->GetString();
            break;
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        }
    }

    if( HTML_FONT_ON != nToken )
    {
        // HTML_BIGPRINT_ON oder HTML_SMALLPRINT_ON

        // in Ueberschriften bestimmt die aktuelle Ueberschrift
        // die Font-Hoehe und nicht BASEFONT
        USHORT nPoolId = GetCurrFmtColl()->GetPoolFmtId();
        if( (nPoolId>=RES_POOLCOLL_HEADLINE1 &&
             nPoolId<=RES_POOLCOLL_HEADLINE6) )
        {
            // wenn die Schriftgroesse in der Ueberschrift noch
            // nicht veraendert ist, die aus der Vorlage nehmen
            if( nFontStHeadStart==aFontStack.Count() )
                nFontSize = 6 - (nPoolId - RES_POOLCOLL_HEADLINE1);
        }
        else
            nPoolId = 0;

        if( HTML_BIGPRINT_ON == nToken )
            nSize = ( nFontSize<7 ? nFontSize+1 : 7 );
        else
            nSize = ( nFontSize>1 ? nFontSize-1 : 1 );

        // in Ueberschriften wird die neue Fonthoehe wenn moeglich aus
        // den Vorlagen geholt.
        if( nPoolId && nSize>=1 && nSize <=6 )
            nFontHeight =
                pCSS1Parser->GetTxtCollFromPool(
                    RES_POOLCOLL_HEADLINE1+6-nSize )->GetSize().GetHeight();
        else
            nFontHeight = aFontHeights[nSize-1];
    }

    ASSERT( !nSize == !nFontHeight, "HTML-Font-Size != Font-Height" );

    String aFontName, aStyleName;
    FontFamily eFamily = FAMILY_DONTKNOW;   // Family und Pitch,
    FontPitch ePitch = PITCH_DONTKNOW;      // falls nicht gefunden
    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();

    if( aFace.Len() && !pCSS1Parser->IsIgnoreFontFamily() )
    {
        const FontList *pFList = 0;
        SwDocShell *pDocSh = pDoc->GetDocShell();
        if( pDocSh )
        {
            const SvxFontListItem *pFListItem =
               (const SvxFontListItem *)pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST);
            if( pFListItem )
                pFList = pFListItem->GetFontList();
        }

        BOOL bFound = FALSE;
        xub_StrLen nStrPos = 0;
        while( nStrPos!=STRING_NOTFOUND )
        {
            String aFName = aFace.GetToken( 0, ',', nStrPos );
            aFName.EraseTrailingChars().EraseLeadingChars();
            if( aFName.Len() )
            {
                if( !bFound && pFList )
                {
                    sal_Handle hFont = pFList->GetFirstFontInfo( aFName );
                    if( 0 != hFont )
                    {
                        const FontInfo& rFInfo = pFList->GetFontInfo( hFont );
                        if( RTL_TEXTENCODING_DONTKNOW != rFInfo.GetCharSet() )
                        {
                            bFound = TRUE;
                            if( RTL_TEXTENCODING_SYMBOL == rFInfo.GetCharSet() )
                                eEnc = RTL_TEXTENCODING_SYMBOL;
                        }
                    }
                }
                if( aFontName.Len() )
                    aFontName += ';';
                aFontName += aFName;
            }
        }
    }


    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( nFontHeight )
            aItemSet.Put( SvxFontHeightItem( nFontHeight ) );
        if( bColor )
            aItemSet.Put( SvxColorItem(aColor) );
        if( aFontName.Len() )
            aItemSet.Put( SvxFontItem( eFamily, aFontName, aStyleName,
                                       ePitch, eEnc ) );


        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
    }
    else
    {
        if( nFontHeight )
            InsertAttr( &aAttrTab.pFontHeight,
                        SvxFontHeightItem( nFontHeight ), pCntxt );
        if( bColor )
            InsertAttr( &aAttrTab.pFontColor, SvxColorItem(aColor), pCntxt );
        if( aFontName.Len() )
            InsertAttr( &aAttrTab.pFont,
                        SvxFontItem( eFamily, aFontName, aStyleName, ePitch,
                                     eEnc ), pCntxt );
    }

    // den Kontext merken
    PushContext( pCntxt );

    aFontStack.Insert( nSize, aFontStack.Count() );
}

void SwHTMLParser::EndFontAttr( int nToken )
{
    EndTag( nToken );

    // Stack-Unterlauf in Tabellen vermeiden
    if( aFontStack.Count() > nFontStMin )
        aFontStack.Remove( aFontStack.Count()-1, 1 );
}

/*  */

void SwHTMLParser::NewPara()
{
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    eParaAdjust = SVX_ADJUST_END;
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_ALIGN:
                eParaAdjust = (SvxAdjust)pOption->GetEnum( aHTMLPAlignTable, eParaAdjust );
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt =
        aClass.Len() ? new _HTMLAttrContext( HTML_PARABREAK_ON,
                                             RES_POOLCOLL_TEXT, aClass )
                     : new _HTMLAttrContext( HTML_PARABREAK_ON );

    // Styles parsen (Class nicht beruecksichtigen. Das geht nur, solange
    // keine der CSS1-Properties der Klasse hart formatiert werden muss!!!)
    if( HasStyleOptions( aStyle, aId, aEmptyStr ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo ) )
        {
            ASSERT( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust), pCntxt );

    // und auf den Stack packen
    PushContext( pCntxt );

    // die aktuelle Vorlage oder deren Attribute setzen
    SetTxtCollAttrs( aClass.Len() ? pCntxt : 0 );

    // Laufbalkenanzeige
    ShowStatline();

    ASSERT( !nOpenParaToken, "Jetzt geht ein offenes Absatz-Element verloren" );
    nOpenParaToken = HTML_PARABREAK_ON;
}

void SwHTMLParser::EndPara( BOOL bReal )
{
    if( HTML_LI_ON==nOpenParaToken && pTable )
    {
        const SwNumRule *pNumRule = pPam->GetNode()->GetTxtNode()->GetNumRule();
        ASSERT( pNumRule, "Wo ist die Numrule geblieben" );
        if( pNumRule )
            pDoc->UpdateNumRule( pNumRule->GetName(),
                                    pPam->GetPoint()->nNode.GetIndex() );
    }

    // leere Absaetze werden von Netscape uebersprungen, von uns jetzt auch
    if( bReal )
    {
        if( pPam->GetPoint()->nContent.GetIndex() )
            AppendTxtNode( AM_SPACE );
        else
            AddParSpace();
    }

    // wenn ein DD oder DT offen war, handelt es sich um eine
    // implizite Def-Liste, die jetzt beendet werden muss
    if( (nOpenParaToken==HTML_DT_ON || nOpenParaToken==HTML_DD_ON) &&
        nDefListDeep)
    {
        nDefListDeep--;
    }

    // den Kontext vom Stack holen. Er kann auch von einer implizit
    // geoeffneten Definitionsliste kommen
    _HTMLAttrContext *pCntxt =
        PopContext( nOpenParaToken ? (nOpenParaToken & ~1)
                                   : HTML_PARABREAK_ON );

    // Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage neu setzen
    if( bReal )
        SetTxtCollAttrs();

    nOpenParaToken = 0;
}


void SwHTMLParser::NewHeading( int nToken )
{
    eParaAdjust = SVX_ADJUST_END;

    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_ALIGN:
                eParaAdjust = (SvxAdjust)pOption->GetEnum( aHTMLPAlignTable, eParaAdjust );
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    // die passende Vorlage suchen
    USHORT nTxtColl;
    switch( nToken )
    {
    case HTML_HEAD1_ON:         nTxtColl = RES_POOLCOLL_HEADLINE1;  break;
    case HTML_HEAD2_ON:         nTxtColl = RES_POOLCOLL_HEADLINE2;  break;
    case HTML_HEAD3_ON:         nTxtColl = RES_POOLCOLL_HEADLINE3;  break;
    case HTML_HEAD4_ON:         nTxtColl = RES_POOLCOLL_HEADLINE4;  break;
    case HTML_HEAD5_ON:         nTxtColl = RES_POOLCOLL_HEADLINE5;  break;
    case HTML_HEAD6_ON:         nTxtColl = RES_POOLCOLL_HEADLINE6;  break;
    default:                    nTxtColl = RES_POOLCOLL_STANDARD;   break;
    }

    // den Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nTxtColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo ) )
        {
            ASSERT( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust), pCntxt );

    // udn auf den Stack packen
    PushContext( pCntxt );

    // und die Vorlage oder deren Attribute setzen
    SetTxtCollAttrs( pCntxt );

    nFontStHeadStart = aFontStack.Count();

    // Laufbalkenanzeige
    ShowStatline();
}

void SwHTMLParser::EndHeading()
{
    // einen neuen Absatz aufmachen
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    // Kontext zu dem Token suchen und vom Stack holen
    _HTMLAttrContext *pCntxt = 0;
    USHORT nPos = aContexts.Count();
    while( !pCntxt && nPos>nContextStMin )
    {
        switch( aContexts[--nPos]->GetToken() )
        {
        case HTML_HEAD1_ON:
        case HTML_HEAD2_ON:
        case HTML_HEAD3_ON:
        case HTML_HEAD4_ON:
        case HTML_HEAD5_ON:
        case HTML_HEAD6_ON:
            pCntxt = aContexts[nPos];
            aContexts.Remove( nPos, 1 );
            break;
        }
    }

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // die bisherige Vorlage neu setzen
    SetTxtCollAttrs();

    nFontStHeadStart = nFontStMin;
}

/*  */

void SwHTMLParser::NewTxtFmtColl( int nToken, USHORT nColl )
{
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    SwHTMLAppendMode eMode = AM_NORMAL;
    switch( nToken )
    {
    case HTML_LISTING_ON:
    case HTML_XMP_ON:
        // Diese beiden Tags werden jetzt auf die PRE-Vorlage gemappt.
        // Fuer dem Fall, dass ein CLASS angegeben ist, loeschen wir
        // es damit wir nicht die CLASS der PRE-Vorlage bekommen.
        aClass = aEmptyStr;
    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE30_ON:
    case HTML_PREFORMTXT_ON:
        eMode = AM_SPACE;
        break;
    case HTML_ADDRESS_ON:
        eMode = AM_NOSPACE; // ADDRESS kann auf einen <P> ohne </P> folgen
        break;
    case HTML_DT_ON:
    case HTML_DD_ON:
        eMode = AM_SOFTNOSPACE;
        break;
    default:
        ASSERT( !this, "unbekannte Vorlage" );
        break;
    }
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // ... und in einem Kontext merken
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo ) )
        {
            ASSERT( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTxtCollAttrs( pCntxt );

    // Laufbalkenanzeige aktualisieren
    ShowStatline();
}

void SwHTMLParser::EndTxtFmtColl( int nToken )
{
    SwHTMLAppendMode eMode = AM_NORMAL;
    switch( nToken & ~1 )
    {
    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE30_ON:
    case HTML_PREFORMTXT_ON:
    case HTML_LISTING_ON:
    case HTML_XMP_ON:
        eMode = AM_SPACE;
        break;
    case HTML_ADDRESS_ON:
    case HTML_DT_ON:
    case HTML_DD_ON:
        eMode = AM_SOFTNOSPACE;
        break;
    default:
        ASSERT( !this, "unbekannte Vorlage" );
        break;
    }
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pCntxt = PopContext( nToken & ~1 );

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage setzen
    SetTxtCollAttrs();
}

/*  */

void SwHTMLParser::NewDefList()
{
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    BOOL bSpace = (GetNumInfo().GetDepth() + nDefListDeep) == 0;
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    // ein Level mehr
    nDefListDeep++;


    BOOL bInDD = FALSE, bNotInDD = FALSE;
    USHORT nPos = aContexts.Count();
    while( !bInDD && !bNotInDD && nPos>nContextStMin )
    {
        USHORT nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDD = TRUE;
            break;
        case HTML_DD_ON:
            bInDD = TRUE;
            break;
        }
    }


    // ... und in einem Kontext merken
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_DEFLIST_ON );

    // darin auch die Raender merken
    USHORT nLeft=0, nRight=0;
    short nIndent=0;
    GetMarginsFromContext( nLeft, nRight, nIndent );

    // Die Einrueckung, die sich schon aus einem DL-ergibt, entspricht der
    // eines DT auf dem aktuellen Level, und die entspricht der eines
    // DD auf dem Level davor. Fue einen Level >=2 muss also ein DD-Abstand
    // hinzugefuegt werden
    if( !bInDD && nDefListDeep > 1 )
    {

        // und den der DT-Vorlage des aktuellen Levels
        SvxLRSpaceItem rLRSpace =
            pCSS1Parser->GetTxtFmtColl( RES_POOLCOLL_HTML_DD, aEmptyStr )
                       ->GetLRSpace();
        nLeft += rLRSpace.GetTxtLeft();
    }

    pCntxt->SetMargins( nLeft, nRight, nIndent );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die Attribute der neuen Vorlage setzen
    if( nDefListDeep > 1 )
        SetTxtCollAttrs( pCntxt );
}

void SwHTMLParser::EndDefList()
{
    BOOL bSpace = (GetNumInfo().GetDepth() + nDefListDeep) == 1;
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    // ein Level weniger
    if( nDefListDeep > 0 )
        nDefListDeep--;

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pCntxt = PopContext( HTML_DEFLIST_ON );

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und Vorlage setzen
    SetTxtCollAttrs();
}

void SwHTMLParser::NewDefListItem( int nToken )
{
    // festellen, ob das DD/DT in einer DL vorkommt
    BOOL bInDefList = FALSE, bNotInDefList = FALSE;
    USHORT nPos = aContexts.Count();
    while( !bInDefList && !bNotInDefList && nPos>nContextStMin )
    {
        USHORT nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
            bInDefList = TRUE;
            break;
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDefList = TRUE;
            break;
        }
    }

    // wenn nicht, implizit eine neue DL aufmachen
    if( !bInDefList )
    {
        nDefListDeep++;
        ASSERT( !nOpenParaToken,
                "Jetzt geht ein offenes Absatz-Element verloren" );
        nOpenParaToken = nToken;
    }

    NewTxtFmtColl( nToken, nToken==HTML_DD_ON ? RES_POOLCOLL_HTML_DD
                                              : RES_POOLCOLL_HTML_DT );
}

void SwHTMLParser::EndDefListItem( int nToken, BOOL bSetColl,
                                   BOOL bLastPara )
{
    // einen neuen Absatz aufmachen
    if( !nToken && pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SOFTNOSPACE );

    // Kontext zu dem Token suchen und vom Stack holen
    nToken &= ~1;
    _HTMLAttrContext *pCntxt = 0;
    USHORT nPos = aContexts.Count();
    while( !pCntxt && nPos>nContextStMin )
    {
        USHORT nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DD_ON:
        case HTML_DT_ON:
            if( !nToken || nToken == nCntxtToken  )
            {
                pCntxt = aContexts[nPos];
                aContexts.Remove( nPos, 1 );
            }
            break;
        case HTML_DEFLIST_ON:
            // keine DD/DT ausserhalb der aktuelen DefListe betrachten
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            // und auch nicht ausserhalb einer anderen Liste
            nPos = nContextStMin;
            break;
        }
    }

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage setzen
    if( bSetColl )
        SetTxtCollAttrs();
}

/*  */

BOOL SwHTMLParser::HasCurrentParaFlys( BOOL bNoSurroundOnly,
                                       BOOL bSurroundOnly ) const
{
    // bNoSurroundOnly:     Der Absatz enthaelt mindestens einen Rahmen
    //                      ohne Umlauf
    // bSurroundOnly:       Der Absatz enthaelt mindestens einen Rahmen
    //                      mit Umlauf aber keinen ohne Umlauf
    // sonst:               Der Absatz enthaelt irgendeinen Rahmen
    SwNodeIndex& rNodeIdx = pPam->GetPoint()->nNode;

    SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;
    const SwPosition* pAPos;
    const SwSpzFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

    USHORT i;
    BOOL bFound = FALSE;
    for( i=0; i<rFrmFmtTbl.Count(); i++ )
    {
        pFmt = rFrmFmtTbl[i];
        pAnchor = &pFmt->GetAnchor();
        // Ein Rahmen wurde gefunden, wenn
        // - er absatzgebunden ist, und
        // - im aktuellen Absatz verankert ist, und
        //   - jeder absatzgebunene Rahmen zaehlt, oder
        //   - (nur Rahmen oder umlauf zaehlen und ) der Rahmen keinen
        //     Umlauf besitzt

        if( 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
            (FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
             FLY_AUTO_CNTNT == pAnchor->GetAnchorId()) &&
            pAPos->nNode == rNodeIdx )
        {
            if( !(bNoSurroundOnly || bSurroundOnly) )
            {
                bFound = TRUE;
                break;
            }
            else
            {
                // fix #42282#: Wenn Rahmen mit Umlauf gesucht sind,
                // auch keine mit Durchlauf beachten. Dabei handelt es
                // sich (noch) um HIDDEN-Controls, und denen weicht man
                // besser auch nicht aus.
                SwSurround eSurround = pFmt->GetSurround().GetSurround();
                if( bNoSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                if( bSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = FALSE;
                        break;
                    }
                    else if( SURROUND_THROUGHT!=eSurround )
                    {
                        bFound = TRUE;
                        // weitersuchen: Es koennten ja noch welche ohne
                        // Umlauf kommen ...
                    }
                }
            }
        }
    }

    return bFound;
}

/*  */

// die speziellen Methoden zum Einfuegen von Objecten

const SwFmtColl *SwHTMLParser::GetCurrFmtColl() const
{
    const SwCntntNode* pCNd = pPam->GetCntntNode();
    return &pCNd->GetAnyFmtColl();
}


void SwHTMLParser::SetTxtCollAttrs( _HTMLAttrContext *pContext )
{
    SwTxtFmtColl *pCollToSet = 0;   // die zu setzende Vorlage
    SfxItemSet *pItemSet = 0;       // der Set fuer harte Attrs
    USHORT nTopColl = pContext ? pContext->GetTxtFmtColl() : 0;
    const String& rTopClass = pContext ? pContext->GetClass() : (const String&) aEmptyStr;
    USHORT nDfltColl = RES_POOLCOLL_TEXT;

    BOOL bInPRE=FALSE;                          // etwas Kontext Info

    USHORT nLeftMargin = 0, nRightMargin = 0;   // die Einzuege und
    short nFirstLineIndent = 0;                 // Abstaende

    for( USHORT i=nContextStAttrMin; i<aContexts.Count(); i++ )
    {
        const _HTMLAttrContext *pCntxt = aContexts[i];

        USHORT nColl = pCntxt->GetTxtFmtColl();
        if( nColl )
        {
            // Es gibt eine Vorlage, die zu setzen ist. Dann
            // muss zunaechst einmal entschieden werden,
            // ob die Vorlage auch gesetzt werden kann
            BOOL bSetThis = TRUE;
            switch( nColl )
            {
            case USHORT(RES_POOLCOLL_HTML_PRE):
                bInPRE = TRUE;
                break;
            case USHORT(RES_POOLCOLL_TEXT):
                // <TD><P CLASS=xxx> muss TD.xxx werden
                if( nDfltColl==RES_POOLCOLL_TABLE ||
                    nDfltColl==RES_POOLCOLL_TABLE_HDLN )
                    nColl = nDfltColl;
                break;
            case USHORT(RES_POOLCOLL_HTML_HR):
                // <HR> auch in <PRE> als Vorlage setzen, sonst kann man sie
                // nicht mehr exportieren
                break;
            default:
                if( bInPRE )
                    bSetThis = FALSE;
                break;
            }

            SwTxtFmtColl *pNewColl =
                pCSS1Parser->GetTxtFmtColl( nColl, pCntxt->GetClass() );

            if( bSetThis )
            {
                // wenn jetzt eine andere Vorlage gesetzt werden soll als
                // bisher, muss die bishere Vorlage durch harte Attributierung
                // ersetzt werden

                if( pCollToSet )
                {
                    // die Attribute, die die bisherige Vorlage setzt
                    // hart einfuegen
                    if( !pItemSet )
                        pItemSet = new SfxItemSet( pCollToSet->GetAttrSet() );
                    else
                    {
                        const SfxItemSet& rCollSet = pCollToSet->GetAttrSet();
                        SfxItemSet aItemSet( *rCollSet.GetPool(),
                                             rCollSet.GetRanges() );
                        aItemSet.Set( rCollSet );
                        pItemSet->Put( aItemSet );
                    }
                    // aber die Attribute, die aktuelle Vorlage setzt
                    // entfernen, weil sie sonst spaeter ueberschrieben
                    // werden
                    pItemSet->Differentiate( pNewColl->GetAttrSet() );
                }

                pCollToSet = pNewColl;
            }
            else
            {
                // hart Attributieren
                if( !pItemSet )
                    pItemSet = new SfxItemSet( pNewColl->GetAttrSet() );
                else
                {
                    const SfxItemSet& rCollSet = pNewColl->GetAttrSet();
                    SfxItemSet aItemSet( *rCollSet.GetPool(),
                                         rCollSet.GetRanges() );
                    aItemSet.Set( rCollSet );
                    pItemSet->Put( aItemSet );
                }
            }
        }
        else
        {
            // vielliecht gibt es ja eine Default-Vorlage?
            nColl = pCntxt->GetDfltTxtFmtColl();
            if( nColl )
                nDfltColl = nColl;
        }

        // ggf. neue Absatz-Einzuege holen
        if( pCntxt->IsLRSpaceChanged() )
        {
            USHORT nLeft=0, nRight=0;

            pCntxt->GetMargins( nLeft, nRight, nFirstLineIndent );
            nLeftMargin = nLeft;
            nRightMargin = nRight;
        }
    }

    // wenn im aktuellen Kontext eine neue Vorlage gesetzt werden soll,
    // muessen deren Absatz-Abstaende noch in den Kontext eingetragen werden
    if( pContext && nTopColl )
    {
        // <TD><P CLASS=xxx> muss TD.xxx werden
        if( nTopColl==RES_POOLCOLL_TEXT &&
            (nDfltColl==RES_POOLCOLL_TABLE ||
             nDfltColl==RES_POOLCOLL_TABLE_HDLN) )
            nTopColl = nDfltColl;

        const SwTxtFmtColl *pTopColl =
            pCSS1Parser->GetTxtFmtColl( nTopColl, rTopClass );
        const SfxItemSet& rItemSet = pTopColl->GetAttrSet();
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,TRUE, &pItem) )
        {
            const SvxLRSpaceItem *pLRItem =
                (const SvxLRSpaceItem *)pItem;

            USHORT nLeft = pLRItem->GetTxtLeft();
            USHORT nRight = pLRItem->GetRight();
            nFirstLineIndent = pLRItem->GetTxtFirstLineOfst();

            // In Definitions-Listen enthalten die Abstaende auch die der
            // vorhergehenden Level
            if( RES_POOLCOLL_HTML_DD == nTopColl )
            {
                const SvxLRSpaceItem& rDTLRSpace = pCSS1Parser
                    ->GetTxtFmtColl( RES_POOLCOLL_HTML_DT, aEmptyStr )
                    ->GetLRSpace();
                nLeft -= rDTLRSpace.GetTxtLeft();
                nRight -= rDTLRSpace.GetRight();
            }
            else if( RES_POOLCOLL_HTML_DT == nTopColl )
            {
                nLeft = 0;
                nRight = 0;
            }

            // die Absatz-Abstaende addieren sich
            nLeftMargin += nLeft;
            nRightMargin += nRight;

            pContext->SetMargins( nLeftMargin, nRightMargin,
                                  nFirstLineIndent );
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,TRUE, &pItem) )
        {
            const SvxULSpaceItem *pULItem =
                (const SvxULSpaceItem *)pItem;
            pContext->SetULSpace( pULItem->GetUpper(), pULItem->GetLower() );
        }
    }

    // wenn gar keine Vorlage im Kontext gesetzt ist, Textkoerper nehmen
    if( !pCollToSet )
    {
        pCollToSet = pCSS1Parser->GetTxtCollFromPool( nDfltColl );
        const SvxLRSpaceItem& rLRItem = pCollToSet->GetLRSpace();
        if( !nLeftMargin )
            nLeftMargin = rLRItem.GetTxtLeft();
        if( !nRightMargin )
            nRightMargin = rLRItem.GetRight();
        if( !nFirstLineIndent )
            nFirstLineIndent = rLRItem.GetTxtFirstLineOfst();
    }

    // bisherige harte Attributierung des Absatzes entfernen
    if( aParaAttrs.Count() )
    {
        for( i=0; i<aParaAttrs.Count(); i++ )
            aParaAttrs[i]->Invalidate();

        aParaAttrs.Remove( 0, aParaAttrs.Count() );
    }

    // Die Vorlage setzen
    pDoc->SetTxtFmtColl( *pPam, pCollToSet );

    // ggf. noch den Absatz-Einzug korrigieren
    const SvxLRSpaceItem& rLRItem = pCollToSet->GetLRSpace();
    BOOL bSetLRSpace;

#ifndef NUM_RELSPACE
    SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();
    ASSERT( pTxtNode, "TxtFmtColl an Nicht-Text-Node setzen???" );
    const SwNodeNum *pNodeNum = pTxtNode->GetNum();
    if( GetNumInfo().GetDepth() > 0 || pNodeNum )
    {
        // Die Werte fuer den linken Rand und den Einzug koennte man auch
        // aus der NumRule holen. Berechnen ist aber einfacher.

        ASSERT( (GetNumInfo().GetDepth() > 0) ==
                    (GetNumInfo().GetNumRule() != 0),
                "Keine NumRule in Aufzaehlung oder umgekehrt" );

        ASSERT( pNodeNum, "Kein SwNodeNum am Absatz in Aufzaehlung" );

        BOOL bNumbered = FALSE;
        if( pNodeNum && !(NO_NUMLEVEL & pNodeNum->GetLevel()) )
        {
            // der Erstzeilen-Einzug des List-Items muss noch zu dem
            // sowieso schon bestehenden addiert werden, weil er im
            // LI-Kontext nicht gesetzt wurde. Das ist leider nicht moeglich,
            // weil auch nicht-numerierte Absaetze in diesem Kontext
            // vorkommen koennen.
            nFirstLineIndent += HTML_NUMBUL_INDENT;
            bNumbered = TRUE;
        }

        // In LI-Absaetzen ausserhalb von OL/UL ist nNumBulListDeep 0, aber
        // es muss trotzdem um die Breite des Bullet-Zeichens eingerueckt
        // werden.
        USHORT nLeftMarginNumBul = (GetNumInfo().GetDepth() * HTML_NUMBUL_MARGINLEFT);
        if( !nLeftMarginNumBul )
            nLeftMarginNumBul = (USHORT)(-HTML_NUMBUL_INDENT);

        bSetLRSpace = nLeftMargin != nLeftMarginNumBul ||
                      (bNumbered && nFirstLineIndent != HTML_NUMBUL_INDENT) ||
                      (!bNumbered && nFirstLineIndent) ||
                      nRightMargin != rLRItem.GetRight();

        pTxtNode->SetNumLSpace( !bSetLRSpace );
    }
    else
    {
#endif
        bSetLRSpace = nLeftMargin != rLRItem.GetTxtLeft() ||
                      nFirstLineIndent != rLRItem.GetTxtFirstLineOfst() ||
                      nRightMargin != rLRItem.GetRight();
#ifndef NUM_RELSPACE
    }
#endif

    if( bSetLRSpace )
    {
        SvxLRSpaceItem aLRItem( rLRItem );
        aLRItem.SetTxtLeft( nLeftMargin );
        aLRItem.SetRight( nRightMargin );
        aLRItem.SetTxtFirstLineOfst( nFirstLineIndent );
        if( pItemSet )
            pItemSet->Put( aLRItem );
        else
        {
            NewAttr( &aAttrTab.pLRSpace, aLRItem );
            aAttrTab.pLRSpace->SetLikePara();
            aParaAttrs.Insert( aAttrTab.pLRSpace, aParaAttrs.Count() );
            EndAttr( aAttrTab.pLRSpace, 0, FALSE );
        }
    }

    // und nun noch die Attribute setzen
    if( pItemSet )
    {
        InsertParaAttrs( *pItemSet );
        delete pItemSet;
    }
}

/*  */

void SwHTMLParser::NewCharFmt( int nToken )
{
    String aId, aStyle, aClass;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

    // die Vorlage setzen und im Kontext merken
    SwCharFmt* pCFmt = pCSS1Parser->GetChrFmt( nToken, aClass );
    ASSERT( pCFmt, "keine Zeichenvorlage zu Token gefunden" );


    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo ) )
        {
            ASSERT( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, TRUE );
        }
    }

    // Zeichen-Vorlagen werden in einem eigenen Stack gehalten und
    // koennen nie durch Styles eingefuegt werden. Das Attribut ist deshalb
    // auch gar nicht im CSS1-Which-Range enthalten
    if( pCFmt )
        InsertAttr( &aAttrTab.pCharFmts, SwFmtCharFmt( pCFmt ), pCntxt );

    // den Kontext merken
    PushContext( pCntxt );
}


/*  */

void SwHTMLParser::InsertSpacer()
{
    // und es ggf. durch die Optionen veraendern
    String aId;
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;
    Size aSize( 0, 0);
    long nSize = 0;
    BOOL bPrcWidth = FALSE;
    BOOL bPrcHeight = FALSE;
    USHORT nType = HTML_SPTYPE_HORI;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_TYPE:
            pOption->GetEnum( nType, aHTMLSpacerTypeTable );
            break;
        case HTML_O_ALIGN:
            eVertOri =
                (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable,
                                                eVertOri );
            eHoriOri =
                (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable,
                                                eHoriOri );
            break;
        case HTML_O_WIDTH:
            // erstmal nur als Pixelwerte merken!
            bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)pOption->GetNumber();
            break;
        case HTML_O_HEIGHT:
            // erstmal nur als Pixelwerte merken!
            bPrcHeight = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)pOption->GetNumber();
            break;
        case HTML_O_SIZE:
            // erstmal nur als Pixelwerte merken!
            nSize = pOption->GetNumber();
            break;
        }
    }

    switch( nType )
    {
    case HTML_SPTYPE_BLOCK:
        {
            // einen leeren Textrahmen anlegen

            // den Itemset holen
            SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
            if( !IsNewDoc() )
                Reader::ResetFrmFmtAttrs( aFrmSet );

            // den Anker und die Ausrichtung setzen
            SetAnchorAndAdjustment( eVertOri, eHoriOri, aFrmSet );

            // und noch die Groesse des Rahmens
            Size aDfltSz( MINFLY, MINFLY );
            Size aSpace( 0, 0 );
            SfxItemSet aDummyItemSet( pDoc->GetAttrPool(),
                                 pCSS1Parser->GetWhichMap() );
            SvxCSS1PropertyInfo aDummyPropInfo;

            SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight,
                        aDummyItemSet, aDummyPropInfo, aFrmSet );
            SetSpace( aSpace, aDummyItemSet, aDummyPropInfo, aFrmSet );

            // den Inhalt schuetzen
            SvxProtectItem aProtectItem;
            aProtectItem.SetCntntProtect( TRUE );
            aFrmSet.Put( aProtectItem );

            // der Rahmen anlegen
            RndStdIds eAnchorId =
                ((const SwFmtAnchor &)aFrmSet.Get(RES_ANCHOR)).GetAnchorId();
            SwFrmFmt *pFlyFmt = pDoc->MakeFlySection( eAnchorId,
                                            pPam->GetPoint(), &aFrmSet );
            // Ggf Frames anlegen und auto-geb. Rahmen registrieren
            RegisterFlyFrm( pFlyFmt );
        }
        break;
    case HTML_SPTYPE_VERT:
        if( nSize > 0 )
        {
            if( nSize && Application::GetDefaultDevice() )
            {
                nSize = Application::GetDefaultDevice()
                            ->PixelToLogic( Size(0,nSize),
                                            MapMode(MAP_TWIP) ).Height();
            }

            // einen Absatz-Abstand setzen
            SwTxtNode *pTxtNode = 0;
            if( !pPam->GetPoint()->nContent.GetIndex() )
            {
                // den unteren Absatz-Abstand des vorherigen Nodes aendern,
                // wenn moeglich

                SetAttr();  // noch offene Absatz-Attribute setzen

                pTxtNode = pDoc->GetNodes()[pPam->GetPoint()->nNode.GetIndex()-1]
                               ->GetTxtNode();

                // Wenn der Abstz davor kein Txtenode ist, dann wird jetzt
                // ein leere Absatz angelegt, der eh schon eine Zeilenhoehe
                // Abstand erzeugt.
                if( !pTxtNode )
                    nSize = nSize>HTML_PARSPACE ? nSize-HTML_PARSPACE : 0;
            }

            if( pTxtNode )
            {
                SvxULSpaceItem aULSpace( (const SvxULSpaceItem&)pTxtNode
                    ->SwCntntNode::GetAttr( RES_UL_SPACE ) );
                aULSpace.SetLower( aULSpace.GetLower() + (USHORT)nSize );
                pTxtNode->SwCntntNode::SetAttr( aULSpace );
            }
            else
            {
                NewAttr( &aAttrTab.pULSpace, SvxULSpaceItem( 0, (USHORT)nSize ) );
                EndAttr( aAttrTab.pULSpace, 0, FALSE );

                AppendTxtNode();    // nicht am Abstand drehen!
            }
        }
        break;
    case HTML_SPTYPE_HORI:
        if( nSize > 0 )
        {
            // wenn der Absatz noch leer ist, einen Erstzeilen-Einzug
            // setzen, sondern Sperrschrift ueber einem Space aufspannen

            if( nSize && Application::GetDefaultDevice() )
            {
                nSize = Application::GetDefaultDevice()
                            ->PixelToLogic( Size(nSize,0),
                                            MapMode(MAP_TWIP) ).Width();
            }

            if( !pPam->GetPoint()->nContent.GetIndex() )
            {
                USHORT nLeft=0, nRight=0;
                short nIndent = 0;

                GetMarginsFromContextWithNumBul( nLeft, nRight, nIndent );
                nIndent += (short)nSize;

                SvxLRSpaceItem aLRItem;
                aLRItem.SetTxtLeft( nLeft );
                aLRItem.SetRight( nRight );
                aLRItem.SetTxtFirstLineOfst( nIndent );

                NewAttr( &aAttrTab.pLRSpace, aLRItem );
                EndAttr( aAttrTab.pLRSpace, 0, FALSE );
            }
            else
            {
                NewAttr( &aAttrTab.pKerning, SvxKerningItem( (short)nSize ) );
                String aTmp( ' ' );
                pDoc->Insert( *pPam, aTmp /*, CHARSET_ANSI*/ );
                EndAttr( aAttrTab.pKerning );
            }
        }
    }
}

USHORT SwHTMLParser::ToTwips( USHORT nPixel ) const
{
    if( nPixel && Application::GetDefaultDevice() )
    {
        long nTwips = Application::GetDefaultDevice()->PixelToLogic(
                    Size( nPixel, nPixel ), MapMode( MAP_TWIP ) ).Width();
        return nTwips <= USHRT_MAX ? (USHORT)nTwips : USHRT_MAX;
    }
    else
        return nPixel;
}

const SwTwips SwHTMLParser::GetCurrentBrowseWidth()
{
    SwTwips nWidth = SwHTMLTableLayout::GetBrowseWidth( *pDoc );
    if( nWidth )
        return nWidth;

    if( !aHTMLPageSize.Width() )
    {
        const SwFrmFmt& rPgFmt = pCSS1Parser->GetMasterPageDesc()->GetMaster();

        const SwFmtFrmSize& rSz   = rPgFmt.GetFrmSize();
        const SvxLRSpaceItem& rLR = rPgFmt.GetLRSpace();
        const SvxULSpaceItem& rUL = rPgFmt.GetULSpace();
        const SwFmtCol& rCol = rPgFmt.GetCol();

        aHTMLPageSize.Width() = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();
        aHTMLPageSize.Height() = rSz.GetHeight() - rUL.GetUpper() - rUL.GetLower();

        if( 1 < rCol.GetNumCols() )
            aHTMLPageSize.Width() /= rCol.GetNumCols();
    }

    return aHTMLPageSize.Width();
}


/*  */

void SwHTMLParser::InsertIDOption()
{
    String aId;
    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        if( HTML_O_ID==pOption->GetToken() )
        {
            aId = pOption->GetString();
            break;
        }
    }

    if( aId.Len() )
        InsertBookmark( aId );
}


/*  */


void SwHTMLParser::InsertLineBreak()
{
    // <BR CLEAR=xxx> wird wie folgt behandelt:
    // 1.) Es werden nur nur absatzgebundene Rahmen betrachtet, die
    //     im aktuellen Absatz verankert sind.
    // 2.) Fuer linksbuendig ausgerichtete Rahmen wird bei CLEAR=LEFT
    //     oder ALL und auf rechtsbuendige ausgerichtete Rahmen bei
    //     CLEAR=RIGHT oder ALL der Durchlauf wie folgt geaendert:
    // 3.) Wenn der Absatz keinen Text enthaelt, bekommt der Rahmen keinen
    //     Umlauf
    // 4.) sonst erhaelt ein links ausgerichteter Rahmen eine rechten
    //     "nur Anker" Umlauf und recht rechst ausg. Rahmen einen linken
    //     "nur Anker" Umlauf.
    // 5.) wenn in einem nicht-leeren Absatz der Umlauf eines Rahmens
    //     geaendert wird, wird ein neuer Absatz aufgemacht
    // 6.) Wenn von keinem Rahmen der Umlauf geaendert wird, wird ein
    //     harter Zeilenumbruch eingefuegt

    String aId, aStyle, aClass;             // die ID der Bookmark
    BOOL bClearLeft = FALSE, bClearRight = FALSE;
    BOOL bCleared = FALSE;  // wurde ein CLEAR ausgefuehrt?

    // dann holen wir mal die Optionen
    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_CLEAR:
                {
                    const String &aClear = pOption->GetString();
                    if( aClear.EqualsIgnoreCaseAscii( sHTML_AL_all ) )
                    {
                        bClearLeft = TRUE;
                        bClearRight = TRUE;
                    }
                    else if( aClear.EqualsIgnoreCaseAscii( sHTML_AL_left ) )
                        bClearLeft = TRUE;
                    else if( aClear.EqualsIgnoreCaseAscii( sHTML_AL_right ) )
                        bClearRight = TRUE;
                }
                break;
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // CLEAR wird nur fuer den aktuellen Absaetz unterstuetzt
    if( bClearLeft || bClearRight )
    {
        SwNodeIndex& rNodeIdx = pPam->GetPoint()->nNode;
        SwTxtNode* pTxtNd = rNodeIdx.GetNode().GetTxtNode();
        if( pTxtNd )
        {
            SwFrmFmt* pFmt;
            const SwFmtAnchor* pAnchor;
            const SwPosition* pAPos;
            const SwSpzFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

            for( USHORT i=0; i<rFrmFmtTbl.Count(); i++ )
            {
                pFmt = rFrmFmtTbl[i];
                pAnchor = &pFmt->GetAnchor();
                if( 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
                    (FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
                     FLY_AUTO_CNTNT == pAnchor->GetAnchorId()) &&
                    pAPos->nNode == rNodeIdx &&
                    pFmt->GetSurround().GetSurround() != SURROUND_NONE )
                {
                    SwHoriOrient eHori = RES_DRAWFRMFMT == pFmt->Which()
                        ? HORI_LEFT
                        : pFmt->GetHoriOrient().GetHoriOrient();

                    SwSurround eSurround = SURROUND_PARALLEL;
                    if( pPam->GetPoint()->nContent.GetIndex() )
                    {
                        if( bClearLeft && HORI_LEFT==eHori )
                            eSurround = SURROUND_RIGHT;
                        else if( bClearRight && HORI_RIGHT==eHori )
                            eSurround = SURROUND_LEFT;
                    }
                    else if( (bClearLeft && HORI_LEFT==eHori) ||
                             (bClearRight && HORI_RIGHT==eHori) )
                    {
                        eSurround = SURROUND_NONE;
                    }

                    if( SURROUND_PARALLEL != eSurround )
                    {
                        SwFmtSurround aSurround( eSurround );
                        if( SURROUND_NONE != eSurround )
                            aSurround.SetAnchorOnly( TRUE );
                        pFmt->SetAttr( aSurround );
                        bCleared = TRUE;
                    }
                } // Anker ist nicht im Node
            } // Schleife ueber Fly-Frames
        } // kein Text-Node
    } // kein CLEAR

    // Styles parsen
    SvxFmtBreakItem aBreakItem;
    BOOL bBreakItem = FALSE;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            if( pCSS1Parser->SetFmtBreak( aItemSet, aPropInfo ) )
            {
                aBreakItem = (const SvxFmtBreakItem &)aItemSet.Get( RES_BREAK );
                bBreakItem = TRUE;
            }
            if( aPropInfo.aId.Len() )
                InsertBookmark( aPropInfo.aId );
        }
    }

    if( bBreakItem && SVX_BREAK_PAGE_AFTER==aBreakItem.GetBreak() )
    {
        NewAttr( &aAttrTab.pBreak, aBreakItem );
        EndAttr( aAttrTab.pBreak, 0, FALSE );
    }

    if( !bCleared && !bBreakItem )
    {
        // wenn kein CLEAR ausgefuehrt werden sollte oder konnte, wird
        // ein Zeilenumbruch eingefgt
        String sTmp( (sal_Unicode)0x0a );   // make the Mac happy :-)
        pDoc->Insert( *pPam, sTmp );
    }
    else if( pPam->GetPoint()->nContent.GetIndex() )
    {
        // wenn ein Claer in einem nicht-leeren Absatz ausgefuehrt wurde,
        // muss anschliessen ein neuer Absatz aufgemacht werden
        // MIB 21.02.97: Eigentlich muesste man hier den unteren Absatz-
        // Absatnd auf 0 drehen. Das geht aber bei sowas wie <BR ..><P>
        // schief (>Netacpe). Deshalb lassen wir das erstmal.
        AppendTxtNode( AM_NOSPACE );
    }
    if( bBreakItem && SVX_BREAK_PAGE_BEFORE==aBreakItem.GetBreak() )
    {
        NewAttr( &aAttrTab.pBreak, aBreakItem );
        EndAttr( aAttrTab.pBreak, 0, FALSE );
    }
}

void SwHTMLParser::InsertHorzRule()
{
    USHORT nSize = 0;
    USHORT nWidth = 0;

    SvxAdjust eAdjust = SVX_ADJUST_END;

    BOOL bPrcWidth = FALSE;
    BOOL bNoShade = FALSE;
    BOOL bColor = FALSE;

    Color aColor;
    String aId;

    // dann holen wir mal die Optionen
    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_SIZE:
            nSize = (USHORT)pOption->GetNumber();
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (pOption->GetString().Search('%') != STRING_NOTFOUND);
            nWidth = (USHORT)pOption->GetNumber();
            if( bPrcWidth && nWidth>=100 )
            {
                // 100%-Linien sind der default-Fall (keine Attrs neotig)
                nWidth = 0;
                bPrcWidth = FALSE;
            }
            break;
        case HTML_O_ALIGN:
            eAdjust =
                (SvxAdjust)pOption->GetEnum( aHTMLPAlignTable, eAdjust );
            break;
        case HTML_O_NOSHADE:
            bNoShade = TRUE;
            break;
        case HTML_O_COLOR:
            pOption->GetColor( aColor );
            bColor = TRUE;
            break;
        }
    }

    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_NOSPACE );
    if( nOpenParaToken )
        EndPara();
    AppendTxtNode();
    pPam->Move( fnMoveBackward );

    // ... und in einem Kontext merken
    _HTMLAttrContext *pCntxt =
        new _HTMLAttrContext( HTML_HORZRULE, RES_POOLCOLL_HTML_HR, aEmptyStr );

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTxtCollAttrs( pCntxt );

    // die harten Attribute an diesem Absatz werden nie mehr ungueltig
    if( aParaAttrs.Count() )
        aParaAttrs.Remove( 0, aParaAttrs.Count() );

    if( nSize>0 || bColor || bNoShade )
    {
        // Farbe und/oder Breite der Linie setzen
        if( !bColor )
            aColor.SetColor( COL_GRAY );

        SvxBorderLine aBorderLine( &aColor );
        if( nSize )
        {
            long nPWidth = 0;
            long nPHeight = (long)nSize;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            SvxCSS1Parser::SetBorderWidth( aBorderLine, (USHORT)nPHeight,
                                           !bNoShade );
        }
        else if( bNoShade )
        {
            aBorderLine.SetOutWidth( DEF_LINE_WIDTH_2 );
        }
        else
        {
            aBorderLine.SetOutWidth( DEF_DOUBLE_LINE0_OUT );
            aBorderLine.SetInWidth( DEF_DOUBLE_LINE0_IN );
            aBorderLine.SetDistance( DEF_DOUBLE_LINE0_DIST );
        }

        SvxBoxItem aBoxItem;
        aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
        _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(), aBoxItem );
        aSetAttrTab.Insert( pTmp, aSetAttrTab.Count() );
    }
    if( nWidth )
    {
        // Wenn wir in keiner Tabelle sind, wird die Breitenangabe durch
        // Absatz-Einzuege "getuerkt". In einer Tabelle macht das wenig
        // Sinn. Um zu Vermeiden, dass die Linie bei der Breitenberechnung
        // beruecksichtigt wird, bekommt sie aber trotzdem entsprechendes
        // LRSpace-Item verpasst.
#ifdef FIX41370
        const SwFmtColl *pColl = GetCurrFmtColl();
        SvxLRSpaceItem aLRItem( pColl->GetLRSpace() );
#endif
        if( !pTable )
        {
            // Laenge und Ausrichtung der Linie ueber Absatz-Einzuege "tuerken"
            long nBrowseWidth = GetCurrentBrowseWidth();
            nWidth = bPrcWidth ? (USHORT)((nWidth*nBrowseWidth) / 100)
                               : ToTwips( (USHORT)nBrowseWidth );
            if( nWidth < MINLAY )
                nWidth = MINLAY;

            if( (long)nWidth < nBrowseWidth )
            {
#ifndef FIX41370
                const SwFmtColl *pColl = GetCurrFmtColl();
                SvxLRSpaceItem aLRItem( pColl->GetLRSpace() );
#endif
                long nDist = nBrowseWidth - nWidth;

                switch( eAdjust )
                {
                case SVX_ADJUST_RIGHT:
                    aLRItem.SetTxtLeft( (USHORT)nDist );
                    break;
                case SVX_ADJUST_LEFT:
                    aLRItem.SetRight( (USHORT)nDist );
                    break;
                case SVX_ADJUST_CENTER:
                default:
                    nDist /= 2;
                    aLRItem.SetTxtLeft( (USHORT)nDist );
                    aLRItem.SetRight( (USHORT)nDist );
                    break;
                }

#ifndef FIX41370
                _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(), aLRItem );
                aSetAttrTab.Insert( pTmp, aSetAttrTab.Count() );
#endif
            }
        }

#ifdef FIX41370
        _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(), aLRItem );
        aSetAttrTab.Insert( pTmp, aSetAttrTab.Count() );
#endif
    }

    // Bookmarks koennen nicht in Hyperlinks eingefueht werden
    if( aId.Len() )
        InsertBookmark( aId );

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pPoppedContext = PopContext( HTML_HORZRULE );
    ASSERT( pPoppedContext==pCntxt, "wo kommt denn da ein HR-Kontext her?" );
    delete pPoppedContext;

    pPam->Move( fnMoveForward );

    // und im Absatz danach die dort aktuelle Vorlage setzen
    SetTxtCollAttrs();
}

void SwHTMLParser::ParseMoreMetaOptions()
{
    String aName, aContent;
    BOOL bHTTPEquiv = FALSE, bChanged = FALSE;

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[ --i ];
        switch( pOption->GetToken() )
        {
        case HTML_O_NAME:
            aName = pOption->GetString();
            bHTTPEquiv = FALSE;
            break;
        case HTML_O_HTTPEQUIV:
            aName = pOption->GetString();
            bHTTPEquiv = TRUE;
            break;
        case HTML_O_CONTENT:
            aContent = pOption->GetString();
            break;
        }
    }

    // Hier wird es etwas tricky: Wir wissen genau, daá die Dok-Info
    // nicht geaendert wurde. Deshalb genuegt es, auf Generator und
    // auf refresh abzufragen, um noch nicht verarbeitete Token zu finden,
    // denn das sind die einzigen, die die Dok-Info nicht modifizieren.
    if( aName.EqualsIgnoreCaseAscii( sHTML_META_generator ) ||
        aName.EqualsIgnoreCaseAscii( sHTML_META_refresh ) ||
        aName.EqualsIgnoreCaseAscii( sHTML_META_content_type ) ||
        aName.EqualsIgnoreCaseAscii( sHTML_META_content_script_type ) )
        return;

    aContent.EraseAllChars( _CR );
    aContent.EraseAllChars( _LF );

    if( aName.EqualsIgnoreCaseAscii( sHTML_META_sdendnote ) )
    {
        FillEndNoteInfo( aContent );
        return;
    }

    if( aName.EqualsIgnoreCaseAscii( sHTML_META_sdfootnote ) )
    {
        FillFootNoteInfo( aContent );
        return;
    }

    String sText(
            String::CreateFromAscii(TOOLS_CONSTASCII_STRINGPARAM("HTML: <")) );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM(sHTML_meta) );
    sText.Append( ' ' );
    if( bHTTPEquiv  )
        sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM(sHTML_O_httpequiv) );
    else
        sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM(sHTML_O_name) );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM("=\"") );
    sText.Append( aName );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM("\" ") );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM(sHTML_O_content) );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM("=\"") );
    sText.Append( aContent );
    sText.AppendAscii( TOOLS_CONSTASCII_STRINGPARAM("\">") );

    SwPostItField aPostItFld(
        (SwPostItFieldType*)pDoc->GetSysFldType( RES_POSTITFLD ),
        aEmptyStr, sText, Date() );
    SwFmtFld aFmtFld( aPostItFld );
    InsertAttr( aFmtFld );
}

/*  */

#ifdef USED
void __EXPORT SwHTMLParser::SaveState( int nToken )
{
    HTMLParser::SaveState( nToken );
}

void __EXPORT SwHTMLParser::RestoreState()
{
    HTMLParser::RestoreState();
}
#endif

/*  */

_HTMLAttr::_HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
                      _HTMLAttr **ppHd )
    : nSttPara( rPos.nNode ), nEndPara( rPos.nNode ),
    nSttCntnt( rPos.nContent.GetIndex() ),
    nEndCntnt(rPos.nContent.GetIndex() ),
    nCount( 1 ), pNext( 0 ), pPrev( 0 ), ppHead( ppHd ),
    bInsAtStart( TRUE ), bLikePara( FALSE ), bValid( TRUE )
{
    pItem = rItem.Clone();
}

_HTMLAttr::_HTMLAttr( const _HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
                      USHORT nEndCnt, _HTMLAttr **ppHd ) :
    nSttPara( rAttr.nSttPara ), nEndPara( rEndPara ),
    nSttCntnt( rAttr.nSttCntnt ), nEndCntnt( nEndCnt ),
    nCount( rAttr.nCount ), pNext( 0 ), pPrev( 0 ), ppHead( ppHd ),
    bInsAtStart( rAttr.bInsAtStart ), bLikePara( rAttr.bLikePara ),
    bValid( rAttr.bValid )
{
    pItem = rAttr.pItem->Clone();
}

_HTMLAttr::~_HTMLAttr()
{
    delete pItem;
}

_HTMLAttr *_HTMLAttr::Clone( const SwNodeIndex& rEndPara, USHORT nEndCnt ) const
{
    // das Attribut mit der alten Start-Position neu anlegen
    _HTMLAttr *pNew = new _HTMLAttr( *this, rEndPara, nEndCnt, ppHead );

    // die Previous-Liste muss uebernommen werden, die Next-Liste nicht!
    pNew->pPrev = pPrev;

    return pNew;
}

void _HTMLAttr::Reset( const SwNodeIndex& rSttPara, USHORT nSttCnt,
                       _HTMLAttr **ppHd )
{
    // den Anfang (und das Ende) neu setzen
    nSttPara = rSttPara;
    nSttCntnt = nSttCnt;
    nEndPara = rSttPara;
    nEndCntnt = nSttCnt;

    // den Head korrigieren und die Verkettungen aufheben
    pNext = 0;
    pPrev = 0;
    ppHead = ppHd;
}

void _HTMLAttr::InsertPrev( _HTMLAttr *pPrv )
{
    ASSERT( !pPrv->pNext || pPrv->pNext == this,
            "_HTMLAttr::InsertPrev: pNext falsch" );
    pPrv->pNext = 0;

    ASSERT( 0 == pPrv->ppHead || ppHead == pPrv->ppHead,
            "_HTMLAttr::InsertPrev: ppHead falsch" );
    pPrv->ppHead = 0;

    _HTMLAttr *pAttr = this;
    while( pAttr->GetPrev() )
        pAttr = pAttr->GetPrev();

    pAttr->pPrev = pPrv;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/31 09:07:20  mib
      #79777#: Oboslete assert removed

      Revision 1.1.1.1  2000/09/18 17:14:56  hr
      initial import

      Revision 1.494  2000/09/18 16:04:47  willem.vandorp
      OpenOffice header added.

      Revision 1.493  2000/09/05 14:01:29  mib
      #78294#: removed support for frameset documents

      Revision 1.492  2000/08/04 10:55:23  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.491  2000/06/26 09:52:42  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.490  2000/06/13 09:37:51  os
      using UCB

      Revision 1.489  2000/04/10 12:20:58  mib
      unicode

      Revision 1.488  2000/03/17 15:19:53  mib
      #74249#: Call EndAction before collaing DocumentDetected

      Revision 1.487  2000/03/13 15:07:03  jp
      remove JavaScript

      Revision 1.486  2000/03/03 15:21:01  os
      StarView remainders removed

      Revision 1.485  2000/03/03 12:44:32  mib
      Removed JavaScript

      Revision 1.484  2000/02/11 15:41:10  hr
      #70473# changes for unicode

      Revision 1.483  2000/01/20 11:48:38  jp
      Bug #72119#: HTML-Template is moved into an other directory

      Revision 1.482  1999/10/21 17:50:12  jp
      have to change - SearchFile with SfxIniManager, dont use SwFinder for this

      Revision 1.481  1999/09/21 09:50:36  mib
      multiple text encodings

      Revision 1.480  1999/09/17 12:14:52  mib
      support of multiple and non system text encodings

      Revision 1.479  1999/07/21 14:11:18  JP
      Bug #67779#: set any MsgBoxType at the StringErrorInfo


      Rev 1.478   21 Jul 1999 16:11:18   JP
   Bug #67779#: set any MsgBoxType at the StringErrorInfo

      Rev 1.477   11 Jun 1999 10:27:36   MIB
   #66744#: EnableModified flag

      Rev 1.476   10 Jun 1999 10:34:32   JP
   have to change: no AppWin from SfxApp

      Rev 1.475   27 Apr 1999 16:42:26   JP
   Bug #65314#: neu: IsJavaScriptEnabled

      Rev 1.474   15 Apr 1999 13:48:30   MIB
   #41833#: Styles fuer A-Tag

      Rev 1.473   13 Apr 1999 10:58:56   MIB
   #64638#: ASP-tags wie unbekannte Tags behandeln

      Rev 1.472   09 Apr 1999 17:52:00   MA
   #64467# EndAction und VirDev

      Rev 1.471   08 Apr 1999 17:33:44   MIB
   #64522#: Styles fuer TD/TH auswerten

      Rev 1.470   08 Apr 1999 12:45:54   MA
   #64467# F?r die EndAction immer alles per VirDev painten

      Rev 1.469   07 Apr 1999 13:50:46   MIB
   #64327#: GetBrowseWidth fuer Tabellen in Rahmen richtig

      Rev 1.468   29 Mar 1999 16:06:08   MIB
   #64051#: Undo auch nach JavaScript anschalten

      Rev 1.467   17 Mar 1999 16:41:52   MIB
   #63049#: Numerierungen mit relativen Abstaenden

      Rev 1.466   15 Mar 1999 13:14:30   JP
   Bug #63339#: Einfuegen Doc - Absaetze richtig zusammenfassen

      Rev 1.465   11 Mar 1999 23:55:22   JP
   Task #63171#: Optionen fuer Feld-/LinkUpdate Doc oder Modul lokal

      Rev 1.464   10 Mar 1999 15:43:22   MIB
   #62682#: Beim Setzen der Control-Groesse wenn noetig auf die ViewShell warten

      Rev 1.463   25 Feb 1999 17:27:10   MIB
   #61691#: falscher Assert

      Rev 1.462   25 Feb 1999 16:01:58   MIB
   #61949#: globale Shell entsorgti, #62261#: <BR CLEAR> auch fuer Controls

      Rev 1.461   23 Feb 1999 08:56:34   MA
   Syntaxfehler fuer Update

      Rev 1.460   22 Feb 1999 08:40:30   MA
   1949globale Shell entsorgt, Shells am RootFrm

      Rev 1.459   11 Jan 1999 10:45:30   MIB
   #60137#: UCS2-/UTF8-Unterstuetzung jetzt auch fuer F-Sets richtig

      Rev 1.458   18 Dec 1998 09:23:34   MIB
   #60137#: UTF-8-/UCS-2-Unterstuetzung fuer HTML

      Rev 1.457   04 Dec 1998 14:23:58   MIB
   #54626#: <LH> ausserhalb von <OL>/<UL> verbessert, entspricht #55072#

      Rev 1.456   02 Dec 1998 10:55:12   MIB
   #59886#: Kein Assert bei Attributen in Previous-Listen mehr

      Rev 1.455   23 Nov 1998 13:12:46   OM
   #58216# Enum fuer Aktualisieren von Verknuepfungen

      Rev 1.454   20 Nov 1998 11:51:24   MIB
   #59064#: Unbekannte Token/<NOSCRIPT> und <NOEMEBED> beibehalten

      Rev 1.453   18 Nov 1998 10:24:30   MIB
   #59579#: Inhalt von <TITLE> und <NOSCRIPT> nicht zur Filter-Detection heranziehen

      Rev 1.452   13 Nov 1998 14:38:26   MIB
   Nachtrag zu #58450#: Fuss-/Endnoten: Assert bei Verwendung der Vorlegen weg

      Rev 1.451   11 Nov 1998 17:42:02   MIB
   #59059#: Beim Abbrechen Pendung-Stack noch aufraeumen

      Rev 1.450   02 Nov 1998 17:14:26   MIB
   #58480#: Fuss-/Endnoten

      Rev 1.449   23 Oct 1998 09:47:16   MIB
   #58172#: Nach JS-Aufruf falls vorhanden zuletzt aktive ViewShell wieder setzten

      Rev 1.448   13 Oct 1998 17:28:34   MIB
   #57901#: Fixed-Font-Namen vom System holen, alten Namen entfernt

      Rev 1.447   07 Oct 1998 11:07:26   MIB
   #57559#: Erstes Token immer im Continue behandeln, 55072 erstmal raus

      Rev 1.446   01 Sep 1998 12:06:50   MIB
   #55072#: <LI> ohne <OL>/<UL> verbessert

      Rev 1.445   01 Sep 1998 09:52:02   MIB
   #55248#: Header beenden, wenn Dokuemnttyp erkannt wurde

      Rev 1.444   31 Aug 1998 18:43:54   MIB
   #55134#: DocumentDetected zum Anlegen der View aufrufen, bevor Controls eingef. werden

      Rev 1.443   26 Aug 1998 13:29:00   MIB
   #54970#: bCallNextToken nicht setzen, wenn keine Listbox, Applet, etc. einefuegt wurde

      Rev 1.442   26 Aug 1998 09:30:42   MIB
   #55144#: Umlauf bei Tabellen in Rahmen richtig setzen und beachten

      Rev 1.441   25 Aug 1998 11:09:42   MIB
   #50455#: Applet erst beim </APPLET> anlegen, weil erst dann alle Parameter da sind

      Rev 1.440   28 Jul 1998 12:04:58   MIB
   #54025#: Auf ObjectDying der ViewShell achten, #52559#: JavaScript-Zeilennummern

      Rev 1.439   23 Jul 1998 11:14:12   JP
   Task #52654#: Einfuegen Doc nicht mit einer CrsrShell sondern mit einen PaM

      Rev 1.438   13 Jul 1998 09:20:28   MIB
   #50966#: Fonts optional ignorieren

      Rev 1.437   08 Jul 1998 17:19:46   MIB
   StarScript

      Rev 1.436   02 Jul 1998 15:50:10   JP
   Modules Path ueber den PathFinder herausfinden

      Rev 1.435   26 Jun 1998 12:21:48   OM
   #51184# SubType an ExpressionFields richtig setzen und abfragen

      Rev 1.434   15 Jun 1998 15:39:02   JP
   Bug #51132#: _SetAttr - nicht pAttr sonder pAttr->pItem ist das Attribut

      Rev 1.433   15 Jun 1998 13:13:28   MIB
   fix #51020#: Im AppendTxtNode Attrs die den ganzen Absatz sofort setzen

      Rev 1.432   08 Jun 1998 12:08:42   MIB
   fix #50795#: <BODY>-Tag ist sicheres Indiz fuer Text-Dokument

      Rev 1.431   05 Jun 1998 14:04:32   JP
   Bug #42487#: Sprung zum Mark schon waehrend des Ladens ausfuehren

      Rev 1.430   20 May 1998 10:41:04   MIB
   fix #50341#: Script-Feld fuer nicht beendetes Script einfuegen

      Rev 1.429   15 May 1998 11:31:02   MIB
   UNO-Controls statt VC-Controls (noch geht nicht viel)

      Rev 1.428   12 May 1998 15:50:44   JP
   rund um Flys/DrawObjs im Doc/FESh umgestellt/optimiert

      Rev 1.427   29 Apr 1998 17:37:52   MIB
   fix: HTML-Seitenvorlage in SW-Doks wieder setzen

      Rev 1.426   21 Apr 1998 13:47:26   MIB
   fix: Keine Bookmark fuer abs-pos Objekte mit ID einfuegen

      Rev 1.425   16 Apr 1998 11:36:42   MIB
   Keine HTML-Seiten-Vorlage mehr setzen, Warnings

      Rev 1.424   15 Apr 1998 14:54:36   MIB
   Zwei-seitige Printing-Extensions

      Rev 1.423   03 Apr 1998 12:22:16   MIB
   Export des Rahmen-Namens als ID

      Rev 1.422   02 Apr 1998 19:42:40   MIB
   Positionierung von spaltigen Rahmen

      Rev 1.421   27 Mar 1998 17:28:54   MIB
   direkte absolute Positionierung und Groessen-Export von Controls und Marquee

      Rev 1.420   27 Mar 1998 09:58:32   MIB
   direkte Positionierung von Grafiken etc.

      Rev 1.419   25 Mar 1998 12:13:26   MIB
   abs.-pos. Container angefangen, Statistik-Feld

      Rev 1.418   20 Mar 1998 10:46:12   MIB
   Font-Listen in Font-Attributen

      Rev 1.417   13 Mar 1998 09:29:36   MIB
   Rahmen-Anker-Verschiebung entfernt

      Rev 1.416   28 Feb 1998 07:28:58   MH
   chg: [OW|MTF] -> UNX

      Rev 1.415   27 Feb 1998 16:22:42   JP
   ObjectDying-MessageItem umbenannt

      Rev 1.414   27 Feb 1998 14:05:42   MIB
   Auto-gebundene Rahmen

      Rev 1.413   24 Feb 1998 11:31:22   MIB
   Jetzt auch HTML_LISTING und HTML_XMP vernichtet (wegen #42029#)

      Rev 1.412   20 Feb 1998 19:02:22   MA
   header

      Rev 1.411   20 Feb 1998 13:27:14   MA
   headerfiles gewandert

      Rev 1.410   17 Feb 1998 10:51:04   MIB
   HTML-H6 -> HEADLINE6

      Rev 1.409   16 Feb 1998 12:28:42   MIB
   DokInfo-Feld jetzt auch fixed

      Rev 1.408   10 Feb 1998 09:51:20   MIB
   fix: Fuer Absatz-Abstand am Start-/Ende von Listen auch OL/UL/DL beachten

      Rev 1.407   29 Jan 1998 21:34:22   JP
   GetEndOfIcons ersetzt durch GetEndOfExtras, das auf GetEndOfRedlines mappt

      Rev 1.406   23 Jan 1998 16:10:04   MA
   includes

      Rev 1.405   22 Jan 1998 19:58:26   JP
   CTOR des SwPaM umgestellt

      Rev 1.404   19 Jan 1998 16:24:02   MIB
   Numerierungs-Umbau

      Rev 1.403   17 Dec 1997 15:48:26   ER
   cast fuer IRIX

      Rev 1.402   16 Dec 1997 18:09:04   JP
   GetSearchDelim gegen SFX_SEARCH_DELIMITER ausgetauscht

      Rev 1.401   03 Dec 1997 12:42:50   JP
   Extensions auf dem MAC zulassen

      Rev 1.400   26 Nov 1997 19:09:52   MA
   includes

      Rev 1.399   25 Nov 1997 11:53:20   TJ
   include svhtml.hxx

      Rev 1.398   17 Nov 1997 10:17:28   JP
   Umstellung Numerierung

      Rev 1.397   30 Oct 1997 18:19:26   JP
   DocumentVorlagen-Verwaltung in die Basisklasse verschoben

      Rev 1.396   20 Oct 1997 12:21:42   MIB
   fix: Kleine CSS1-Attribute aus Class hart setzen, wenn Vorlage benutzt wird.

      Rev 1.395   20 Oct 1997 09:52:58   MIB
   Nur fuer absolute positioning benoetigten Code groesstenteils auskommentiert

      Rev 1.394   17 Oct 1997 13:21:30   MIB
   page-break-xxx auch uber STYLE/CLASS und ID-Optionen

      Rev 1.393   14 Oct 1997 14:36:42   JP
   pNext vom Ring wurde privat; zugriff ueber GetNext()

      Rev 1.392   14 Oct 1997 14:30:20   MIB
   fix #44228#: ViewShell-Zerstoerung/Rekonstruktion ueber Clients

      Rev 1.391   09 Oct 1997 14:40:46   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.390   06 Oct 1997 15:41:24   OM
   Feldumstellung

      Rev 1.389   06 Oct 1997 15:27:24   MIB
   Kein NewApplet aufrufen, wenn SOLAR_JAVA nicht gesetzt

      Rev 1.388   19 Sep 1997 12:20:24   MIB
   fix: Seiten-Umbruch auch vor Tabellen temporaer setzen

      Rev 1.387   19 Sep 1997 11:27:38   MIB
   fix #41370#: HRs in Tabellen-Zellen (nicht freigeschaltet)

      Rev 1.386   18 Sep 1997 16:38:12   JP
   Continue: beim Einfuegen von nur einem Node nicht falsch zusammenfassen

      Rev 1.385   18 Sep 1997 09:18:34   JP
   Teilfix fuer Bug 42487

      Rev 1.384   16 Sep 1997 17:23:52   MIB
   abs. Positioning fuer Absatz-Tags

      Rev 1.383   16 Sep 1997 14:55:36   MIB
   ITEMID_BOXINFOITEM (voreubergendend) definieren

      Rev 1.382   16 Sep 1997 11:00:44   MIB
   Kopf-/Fusszeilen ohne Moven von Nodes, autom. Beenden von Bereichen/Rahmen

      Rev 1.381   12 Sep 1997 11:51:20   MIB
   fix #41136#: &(xxx);-Makros

      Rev 1.380   10 Sep 1997 11:00:24   MIB
   HTMLReader::IsFormat() wieder raus

      Rev 1.379   09 Sep 1997 14:09:48   MIB
   Ueberall Browse-View-Breite statt Seitenbreite verwenden

      Rev 1.378   08 Sep 1997 17:41:40   MIB
   Verankerung von AUTO_CNTNT-Rahmen nicht mehr verschieben

      Rev 1.377   04 Sep 1997 12:36:54   JP
   Umstellungen fuer FilterDetection im SwModule und SwDLL

      Rev 1.376   02 Sep 1997 20:45:32   OS
   Header

      Rev 1.375   02 Sep 1997 11:12:08   MIB
   fix #42192#: Attribute schnellstmoeglich setzen

      Rev 1.374   01 Sep 1997 11:54:46   MIB
   fix #42679#: Im Titel nur auf </TITLE> achten, keine Schleife mehr.

      Rev 1.373   29 Aug 1997 16:50:20   OS
   DLL-Umstellung

      Rev 1.372   15 Aug 1997 12:47:46   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.371   14 Aug 1997 13:38:10   MA
   #42534# Error in NextToken abfragen und State setzen

      Rev 1.370   12 Aug 1997 13:44:22   OS
   Header-Umstellung

      Rev 1.369   11 Aug 1997 14:06:24   OM
   Headerfile-Umstellung

      Rev 1.368   07 Aug 1997 13:15:18   MIB
   fix #42192#: Zeichen-Attribute moeglicht frueh setzen. Noch nicht freigeschaltet

      Rev 1.367   04 Aug 1997 13:53:42   MIB
   aboslute psoitioning (fuer fast alle Zeichen-Attribute/-Vorlagen)

      Rev 1.366   31 Jul 1997 10:44:44   MIB
   DIV-Stack weg

      Rev 1.365   25 Jul 1997 13:00:38   MIB
   fix #40408#: <NOEMBED>-Tag

      Rev 1.364   18 Jul 1997 10:54:32   MIB
   fix #40951#: In Reschedule/DocDetected absichtlich geaenderte URLs beachten

      Rev 1.363   16 Jul 1997 18:38:40   MIB
   non-pro: Asserts zur Call-Stack-Kontrolle, kein Assert f. frueher gesetze Attrs

      Rev 1.362   10 Jul 1997 16:44:46   MIB
   fix #41379#: Keine Absatz-Attribute fuer leere Absaetze (auch hinter Tabellen)

      Rev 1.361   10 Jul 1997 12:02:50   MIB
   fix #41547#: Keine Tabellen in Tabellen einfuegen

*************************************************************************/

