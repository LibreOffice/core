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


#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/sfx.hrc>
#include <svx/svxids.hrc>
#if OSL_DEBUG_LEVEL > 0
#include <stdlib.h>
#endif
#include <hintids.hxx>

#include <svl/stritem.hxx>
#include <svtools/imap.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svtools/ctrltool.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>

#include <svtools/htmlcfg.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/flstitem.hxx>


#include <frmatr.hxx>
#include <charatr.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <txtfld.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <fmtfollowtextflow.hxx>
#include <docary.hxx>
#include <docstat.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <expfld.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>        // fuer SwBookmark ...
#include <docsh.hxx>
#include <editsh.hxx>       // fuer Start/EndAction
#include <docufld.hxx>
#include <swcss1.hxx>
#include <htmlvsh.hxx>
#include <fltini.hxx>
#include <htmltbl.hxx>
#include <htmlnum.hxx>
#include <swhtml.hxx>
#include <linkenum.hxx>
#include <breakit.hxx>
#include <SwAppletImpl.hxx>

#include <sfx2/viewfrm.hxx>

#include <statstr.hrc>          // ResId fuer Statusleiste
#include <swerror.h>

#define FONTSIZE_MASK           7

#define HTML_ESC_PROP 80
#define HTML_ESC_SUPER DFLT_ESC_SUPER
#define HTML_ESC_SUB DFLT_ESC_SUB

#define HTML_SPTYPE_BLOCK 1
#define HTML_SPTYPE_HORI 2
#define HTML_SPTYPE_VERT 3

using editeng::SvxBorderLine;
using namespace ::com::sun::star;

// <P ALIGN=xxx>, <Hn ALIGN=xxx>, <TD ALIGN=xxx> usw.
HTMLOptionEnum aHTMLPAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,  SVX_ADJUST_LEFT     },
    { OOO_STRING_SVTOOLS_HTML_AL_center,    SVX_ADJUST_CENTER   },
    { OOO_STRING_SVTOOLS_HTML_AL_middle,    SVX_ADJUST_CENTER   }, // Netscape
    { OOO_STRING_SVTOOLS_HTML_AL_right, SVX_ADJUST_RIGHT    },
    { OOO_STRING_SVTOOLS_HTML_AL_justify,   SVX_ADJUST_BLOCK    },
    { OOO_STRING_SVTOOLS_HTML_AL_char,  SVX_ADJUST_LEFT     },
    { 0,                0                   }
};

// <SPACER TYPE=...>
static HTMLOptionEnum aHTMLSpacerTypeTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_block,     HTML_SPTYPE_BLOCK       },
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_horizontal,    HTML_SPTYPE_HORI        },
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_vertical,  HTML_SPTYPE_VERT        },
    { 0,                    0                       }
};

HTMLReader::HTMLReader()
{
    bTmplBrowseMode = sal_True;
}

String HTMLReader::GetTemplateName() const
{
    OUString sTemplate("internal/html");

    OUString sTemplateWithoutExt( sTemplate );
    // first search for OpenDocument Writer/Web template
    sTemplate += ".oth";

    SvtPathOptions aPathOpt;
    // OpenDocument Writer/Web template (extension .oth)
    bool bSet = aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );

    if( !bSet )
    {
        // no OpenDocument Writer/Web template found.
        // search for OpenOffice.org Writer/Web template
        sTemplate = sTemplateWithoutExt;
        sTemplate += ".stw";
        bSet = aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE );
    }

    if( !bSet )
    {
        sTemplate = "";
        OSL_ENSURE( !this,
            "The default HTML template cannot be found in the defined template directories!");
    }

    return sTemplate;
}

int HTMLReader::SetStrmStgPtr()
{
    OSL_ENSURE( pMedium, "Wo ist das Medium??" );

    if( pMedium->IsRemote() || !pMedium->IsStorage() )
    {
        pStrm = pMedium->GetInStream();
        return sal_True;
    }
    return sal_False;

}

    // Aufruf fuer die allg. Reader-Schnittstelle
sal_uLong HTMLReader::Read( SwDoc &rDoc, const String& rBaseURL, SwPaM &rPam, const String & rName )
{
    if( !pStrm )
    {
        OSL_ENSURE( pStrm, "HTML-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    if( !bInsertMode )
    {
        Reader::ResetFrmFmts( rDoc );

        // Die HTML-Seitenvorlage setzen, wenn des kein HTML-Dokument ist,
        // sonst ist sie schon gesetzt.
        if( !rDoc.get(IDocumentSettingAccess::HTML_MODE) )
        {
            rDoc.InsertPoolItem( rPam, SwFmtPageDesc(
                rDoc.GetPageDescFromPool( RES_POOLPAGE_HTML, false )), 0 );
        }
    }

    // damit keiner das Doc klaut!
    rDoc.acquire();
    sal_uLong nRet = 0;
    SvParserRef xParser = new SwHTMLParser( &rDoc, rPam, *pStrm,
                                            rName, rBaseURL, !bInsertMode, pMedium,
                                            IsReadUTF8(),
                                            bIgnoreHTMLComments );

    SvParserState eState = xParser->CallParser();

    if( SVPAR_PENDING == eState )
        pStrm->ResetError();
    else if( SVPAR_ACCEPTED != eState )
    {
        String sErr( OUString::number((sal_Int32)xParser->GetLineNr()));
        sErr += ',';
        sErr += OUString::number((sal_Int32)xParser->GetLinePos());

        // den Stream als Fehlernummer Transporter benutzen
        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }


    return nRet;
}



SwHTMLParser::SwHTMLParser( SwDoc* pD, SwPaM& rCrsr, SvStream& rIn,
                            const OUString& rPath,
                            const OUString& rBaseURL,
                            int bReadNewDoc,
                            SfxMedium* pMed, sal_Bool bReadUTF8,
                            sal_Bool bNoHTMLComments )
    : SfxHTMLParser( rIn, static_cast< sal_Bool >(bReadNewDoc), pMed ),
    SwClient( 0 ),
    aPathToFile( rPath ),
    sBaseURL( rBaseURL ),
    pAppletImpl( 0 ),
    pCSS1Parser( 0 ),
    pNumRuleInfo( new SwHTMLNumRuleInfo ),
    pPendStack( 0 ),
    pDoc( pD ),
    pActionViewShell( 0 ),
    pSttNdIdx( 0 ),
    pTable(0),
    pFormImpl( 0 ),
    pMarquee( 0 ),
    pField( 0 ),
    pImageMap( 0 ),
    pImageMaps( 0 ),
    pFootEndNoteImpl( 0 ),
    nScriptStartLineNr( 0 ),
    nBaseFontStMin( 0 ),
    nFontStMin( 0 ),
    nDefListDeep( 0 ),
    nFontStHeadStart( 0 ),
    nSBModuleCnt( 0 ),
    nMissingImgMaps( 0 ),
    nParaCnt( 5 ),
    // #i83625#
    nContextStMin( 0 ),
    nContextStAttrMin( 0 ),
    nOpenParaToken( 0 ),
    eJumpTo( JUMPTO_NONE ),
#ifdef DBG_UTIL
    m_nContinue( 0 ),
#endif
    eParaAdjust( SVX_ADJUST_END ),
    bDocInitalized( sal_False ),
    bSetModEnabled( sal_False ),
    bInFloatingFrame( sal_False ),
    bInField( sal_False ),
    bCallNextToken( sal_False ),
    bIgnoreRawData( sal_False ),
    bNoParSpace( sal_False ),
    bInNoEmbed( sal_False ),
    bInTitle( sal_False ),
    bUpdateDocStat( sal_False ),
    bFixSelectWidth( sal_False ),
    bFixSelectHeight( sal_False ),
    bTextArea( sal_False ),
    bSelect( sal_False ),
    bInFootEndNoteAnchor( sal_False ),
    bInFootEndNoteSymbol( sal_False ),
    bIgnoreHTMLComments( bNoHTMLComments ),
    bRemoveHidden( sal_False ),
    pTempViewFrame(0)
{
    nEventId = 0;
    bUpperSpace = bViewCreated = bChkJumpMark =
    bSetCrsr = sal_False;

    eScriptLang = HTML_SL_UNKNOWN;
    bAnyStarBasic = sal_True;

    rCrsr.DeleteMark();
    pPam = &rCrsr; // re-use existing cursor: avoids spurious ~SwIndexReg assert
    memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));

    // Die Font-Groessen 1-7 aus der INI-Datei lesen
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    aFontHeights[0] = rHtmlOptions.GetFontSize( 0 ) * 20;
    aFontHeights[1] = rHtmlOptions.GetFontSize( 1 ) * 20;
    aFontHeights[2] = rHtmlOptions.GetFontSize( 2 ) * 20;
    aFontHeights[3] = rHtmlOptions.GetFontSize( 3 ) * 20;
    aFontHeights[4] = rHtmlOptions.GetFontSize( 4 ) * 20;
    aFontHeights[5] = rHtmlOptions.GetFontSize( 5 ) * 20;
    aFontHeights[6] = rHtmlOptions.GetFontSize( 6 ) * 20;

    bKeepUnknown = rHtmlOptions.IsImportUnknown();

    if(bReadNewDoc)
    {
        //CJK has different defaults, so a different object should be used for this
        //RES_CHARTR_CJK_FONTSIZE is a valid value
        SvxFontHeightItem aFontHeight(aFontHeights[2], 100, RES_CHRATR_FONTSIZE);
        pDoc->SetDefault( aFontHeight );
        SvxFontHeightItem aFontHeightCJK(aFontHeights[2], 100, RES_CHRATR_CJK_FONTSIZE);
        pDoc->SetDefault( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL(aFontHeights[2], 100, RES_CHRATR_CTL_FONTSIZE);
        pDoc->SetDefault( aFontHeightCTL );

        // #i18732# - adjust default of option 'FollowTextFlow'
        // TODO: not sure what the appropriate default for HTML should be?
        pDoc->SetDefault( SwFmtFollowTextFlow(true) );
    }

    // Waehrend des Imports in den HTML-Modus schalten, damit die
    // richrigen Vorlagen angelegt werden
    bOldIsHTMLMode = pDoc->get(IDocumentSettingAccess::HTML_MODE);
    pDoc->set(IDocumentSettingAccess::HTML_MODE, true);

    pCSS1Parser = new SwCSS1Parser( pDoc, aFontHeights, sBaseURL, IsNewDoc() );
    pCSS1Parser->SetIgnoreFontFamily( rHtmlOptions.IsIgnoreFontFamily() );

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
    pCSS1Parser->SetDfltEncoding( osl_getThreadTextEncoding() );

    // Timer nur bei ganz normalen Dokumenten aufsetzen!
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if( pDocSh )
    {
        bViewCreated = sal_True;            // nicht, synchron laden

        // es ist ein Sprungziel vorgegeben.

        if( pMed )
        {
            sJmpMark = pMed->GetURLObject().GetMark();
            if( !sJmpMark.isEmpty() )
            {
                eJumpTo = JUMPTO_MARK;
                sal_Int32 nLastPos = sJmpMark.lastIndexOf( cMarkSeparator );
                sal_Int32 nPos =  nLastPos != -1 ? nLastPos : 0;

                String sCmp;
                if (nPos)
                {
                    sCmp = comphelper::string::remove(
                        sJmpMark.copy(nPos + 1), ' ');
                }

                if( sCmp.Len() )
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
                        nPos = -1;
                }
                else
                    nPos = -1;

                if( nPos != -1 )
                    sJmpMark = sJmpMark.copy( 0, nPos );
                if( sJmpMark.isEmpty() )
                    eJumpTo = JUMPTO_NONE;
            }
        }
    }
}

SwHTMLParser::~SwHTMLParser()
{
#ifdef DBG_UTIL
    OSL_ENSURE( !m_nContinue, "DTOR im Continue!" );
#endif
    sal_Bool bAsync = pDoc->IsInLoadAsynchron();
    pDoc->SetInLoadAsynchron( sal_False );
    pDoc->set(IDocumentSettingAccess::HTML_MODE, bOldIsHTMLMode);

    if( pDoc->GetDocShell() && nEventId )
        Application::RemoveUserEvent( nEventId );

    // das DocumentDetected kann ggfs. die DocShells loeschen, darum nochmals
    // abfragen
    if( pDoc->GetDocShell() )
    {
        // Gelinkte Bereiche updaten
        sal_uInt16 nLinkMode = pDoc->getLinkUpdateMode( true );
        if( nLinkMode != NEVER && bAsync &&
            SFX_CREATE_MODE_INTERNAL!=pDoc->GetDocShell()->GetCreateMode() )
            pDoc->GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL,
                                                   true, false );

        if ( pDoc->GetDocShell()->IsLoading() )
        {
            // #i59688#
            pDoc->GetDocShell()->LoadingFinished();
        }
    }

    delete pSttNdIdx;

    if( !aSetAttrTab.empty() )
    {
        OSL_ENSURE( aSetAttrTab.empty(),"Es stehen noch Attribute auf dem Stack" );
        for ( _HTMLAttrs::const_iterator it = aSetAttrTab.begin();
              it != aSetAttrTab.end(); ++it )
            delete *it;
        aSetAttrTab.clear();
    }

    delete pCSS1Parser;
    delete pNumRuleInfo;
    DeleteFormImpl();
    DeleteFootEndNoteImpl();

    OSL_ENSURE( !pTable, "Es existiert noch eine offene Tabelle" );
    delete pImageMaps;

    OSL_ENSURE( !pPendStack,
            "SwHTMLParser::~SwHTMLParser: Hier sollte es keinen Pending-Stack mehr geben" );
    while( pPendStack )
    {
        SwPendingStack* pTmp = pPendStack;
        pPendStack = pPendStack->pNext;
        delete pTmp->pData;
        delete pTmp;
    }

    if( !pDoc->release() )
    {
        // keiner will mehr das Doc haben, also weg damit
        delete pDoc;
        pDoc = NULL;
    }

    if ( pTempViewFrame )
    {
        pTempViewFrame->DoClose();

        // the temporary view frame is hidden, so the hidden flag might need to be removed
        if ( bRemoveHidden && pDoc && pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() )
            pDoc->GetDocShell()->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
    }
}

IMPL_LINK( SwHTMLParser, AsyncCallback, void*, /*pVoid*/ )
{
    nEventId=0;

    // #i47907# - If the document has already been destructed,
    // the parser should be aware of this:
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
    }

    GetAsynchCallLink().Call(0);
    return 0;
}

SvParserState SwHTMLParser::CallParser()
{
    // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
    pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
    if( !IsNewDoc() )       // in ein Dokument einfuegen ?
    {
        const SwPosition* pPos = pPam->GetPoint();

        pDoc->SplitNode( *pPos, false );

        *pSttNdIdx = pPos->nNode.GetIndex()-1;
        pDoc->SplitNode( *pPos, false );

        SwPaM aInsertionRangePam( *pPos );

        pPam->Move( fnMoveBackward );

        // split any redline over the insertion point
        aInsertionRangePam.SetMark();
        *aInsertionRangePam.GetPoint() = *pPam->GetPoint();
        aInsertionRangePam.Move( fnMoveBackward );
        pDoc->SplitRedline( aInsertionRangePam );

        pDoc->SetTxtFmtColl( *pPam,
                pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    }

    if( GetMedium() )
    {
        if( !bViewCreated )
        {
            nEventId = Application::PostUserEvent( LINK( this, SwHTMLParser, AsyncCallback ), 0 );
        }
        else
        {
            bViewCreated = sal_True;
            nEventId = 0;
        }
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

    pDoc->GetPageDesc( 0 ).Add( this );

    SvParserState eRet = HTMLParser::CallParser();
    return eRet;
}

void SwHTMLParser::Continue( int nToken )
{
#ifdef DBG_UTIL
    OSL_ENSURE(!m_nContinue, "Continue im Continue - not supposed to happen");
    m_nContinue++;
#endif

    // Wenn der Import (vom SFX) abgebrochen wurde, wird ein Fehler
    // gesetzt aber trotzdem noch weiter gemacht, damit vernuenftig
    // aufgeraeumt wird.
    OSL_ENSURE( SVPAR_ERROR!=eState,
            "SwHTMLParser::Continue: bereits ein Fehler gesetzt" );
    if( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        eState = SVPAR_ERROR;

    // Die ViewShell vom Dokument holen, merken und als aktuelle setzen.
    ViewShell *pInitVSh = CallStartAction();

    if( SVPAR_ERROR != eState && GetMedium() && !bViewCreated )
    {
        // Beim ersten Aufruf erstmal returnen, Doc anzeigen
        // und auf Timer Callback warten.
        // An dieser Stelle wurde im CallParser gerade mal ein Zeichen
        // gelesen und ein SaveState(0) gerufen.
        eState = SVPAR_PENDING;
        bViewCreated = sal_True;
        pDoc->SetInLoadAsynchron( sal_True );

#ifdef DBG_UTIL
        m_nContinue--;
#endif

        return;
    }

    bSetModEnabled = sal_False;
    if( pDoc->GetDocShell() &&
        0 != (bSetModEnabled = pDoc->GetDocShell()->IsEnableSetModified()) )
    {
        pDoc->GetDocShell()->EnableSetModified( sal_False );
    }

    // waehrend des einlesens kein OLE-Modified rufen
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    sal_Bool bModified = pDoc->IsModified();
    bool const bWasUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);

    // Wenn der Import abgebrochen wird, kein Continue mehr rufen.
    // Falls ein Pending-Stack existiert aber durch einen Aufruf
    // von NextToken dafuer sorgen, dass der Pending-Stack noch
    // beendet wird.
    if( SVPAR_ERROR == eState )
    {
        OSL_ENSURE( !pPendStack || pPendStack->nToken,
                "SwHTMLParser::Continue: Pending-Stack ohne Token" );
        if( pPendStack && pPendStack->nToken )
            NextToken( pPendStack->nToken );
        OSL_ENSURE( !pPendStack,
                "SwHTMLParser::Continue: Es gibt wieder einen Pend-Stack" );
    }
    else
    {
        HTMLParser::Continue( pPendStack ? pPendStack->nToken : nToken );
    }

    // Laufbalken wieder abschalten
    EndProgress( pDoc->GetDocShell() );

    sal_Bool bLFStripped = sal_False;
    if( SVPAR_PENDING != GetStatus() )
    {
        // noch die letzten Attribute setzen
        {
            if( !aScriptSource.isEmpty() )
            {
                SwScriptFieldType *pType =
                    (SwScriptFieldType*)pDoc->GetSysFldType( RES_SCRIPTFLD );

                SwScriptField aFld( pType, aScriptType, aScriptSource,
                                    sal_False );
                InsertAttr( SwFmtFld( aFld ) );
            }

            if( pAppletImpl )
            {
                if( pAppletImpl->GetApplet().is() )
                    EndApplet();
                else
                    EndObject();
            }

            // ggf. ein noch vorhandes LF hinter dem letzen Absatz entfernen
            if( IsNewDoc() )
                bLFStripped = StripTrailingLF() > 0;

            // noch offene Nummerierungen beenden.
            while( GetNumInfo().GetNumRule() )
                EndNumBulList();

            OSL_ENSURE( !nContextStMin, "Es gibt geschuetzte Kontexte" );
            nContextStMin = 0;
            while( aContexts.size() )
            {
                _HTMLAttrContext *pCntxt = PopContext();
                if( pCntxt )
                {
                    EndContext( pCntxt );
                    delete pCntxt;
                }
            }

            if( !aParaAttrs.empty() )
                aParaAttrs.clear();

            SetAttr( sal_False );

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
                xub_StrLen nStt = pTxtNode->GetTxt().getLength();
                // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
                if( pPam->GetPoint()->nNode == aNxtIdx )
                {
                    pPam->GetPoint()->nNode = *pSttNdIdx;
                    pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
                }

#if OSL_DEBUG_LEVEL > 0
// !!! sollte nicht moeglich sein, oder ??
OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( sal_True ).nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( sal_False ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( sal_True ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( sal_True ).nContent.GetIndex();
    pPam->GetBound( sal_True ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().getLength() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( sal_False ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( sal_False ).nContent.GetIndex();
    pPam->GetBound( sal_False ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().getLength() + nCntPos );
}
#endif
                // Zeichen Attribute beibehalten!
                SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
                if (pTxtNode->GetTxt().getLength())
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
            sal_uLong nNodeIdx = pPos->nNode.GetIndex();

            sal_Bool bHasFlysOrMarks =
                HasCurrentParaFlys() || HasCurrentParaBookmarks( sal_True );

            if( IsNewDoc() )
            {
                const SwNode *pPrev = pDoc->GetNodes()[nNodeIdx -1];
                if( !pPam->GetPoint()->nContent.GetIndex() &&
                    ( pPrev->IsCntntNode() ||
                      (pPrev->IsEndNode() &&
                      pPrev->StartOfSectionNode()->IsSectionNode()) ) )
                {
                    SwCntntNode* pCNd = pPam->GetCntntNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() && !bHasFlysOrMarks )
                    {
                        ViewShell *pVSh = CheckActionViewShell();
                        SwCrsrShell *pCrsrSh = pVSh && pVSh->ISA(SwCrsrShell)
                                        ? static_cast < SwCrsrShell * >( pVSh )
                                        : 0;
                        if( pCrsrSh &&
                            pCrsrSh->GetCrsr()->GetPoint()
                                   ->nNode.GetIndex() == nNodeIdx )
                        {
                            pCrsrSh->MovePara(fnParaPrev, fnParaEnd );
                            pCrsrSh->SetMark();
                            pCrsrSh->ClearMark();
                        }
                        pPam->GetBound(sal_True).nContent.Assign( 0, 0 );
                        pPam->GetBound(sal_False).nContent.Assign( 0, 0 );
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
                else if (pAktNd->GetTxt().isEmpty())
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
                pTxtNode->ResetAllAttr();

                if( pPrev->HasSwAttrSet() )
                    pTxtNode->SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(sal_True).nNode.GetNode() == pPrev )
                    pPam->GetBound(sal_True).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(sal_False).nNode.GetNode() == pPrev )
                    pPam->GetBound(sal_False).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }

        // adjust AutoLoad in DocumentProperties
        if( IsNewDoc() )
        {
            SwDocShell *pDocShell(pDoc->GetDocShell());
            OSL_ENSURE(pDocShell, "no SwDocShell");
            if (pDocShell) {
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    pDocShell->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps(
                    xDPS->getDocumentProperties());
                OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
                if ( xDocProps.is() && (xDocProps->getAutoloadSecs() > 0) &&
                     (xDocProps->getAutoloadURL().isEmpty()) )
                {
                    xDocProps->setAutoloadURL(aPathToFile);
                }
            }
        }

        if( bUpdateDocStat )
        {
            pDoc->UpdateDocStat();
        }
    }

    if( SVPAR_PENDING != GetStatus() )
        delete pSttNdIdx, pSttNdIdx = 0;

    // sollte der Parser der Letzte sein, der das Doc haelt, dann braucht
    // man hier auch nichts mehr tun, Doc wird gleich zerstoert!
    if( 1 < pDoc->getReferenceCount() )
    {
        if( bWasUndo )
        {
            pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
            pDoc->GetIDocumentUndoRedo().DoUndo(true);
        }
        else if( !pInitVSh )
        {
            // Wenn zu Beginn des Continue keine Shell vorhanden war,
            // kann trotzdem mitlerweile eine angelegt worden sein.
            // In dieses Fall stimmt das bWasUndo-Flag nicht und
            // wir muessen das Undo noch anschalten.
            ViewShell *pTmpVSh = CheckActionViewShell();
            if( pTmpVSh )
            {
                pDoc->GetIDocumentUndoRedo().DoUndo(true);
            }
        }

        pDoc->SetOle2Link( aOLELink );
        if( !bModified )
            pDoc->ResetModified();
        if( bSetModEnabled && pDoc->GetDocShell() )
        {
            pDoc->GetDocShell()->EnableSetModified( sal_True );
            bSetModEnabled = sal_False; // this is unnecessary here
        }
    }


    // Wenn die Dokuemnt-ViewShell noch existiert und eine Action
    // offen ist (muss bei Abbruch nicht sein), die Action beenden,
    // uns von der Shell abmelden und schliesslich die alte Shell
    // wieder rekonstruieren.
    CallEndAction( sal_True );

#ifdef DBG_UTIL
    m_nContinue--;
#endif
}

void SwHTMLParser::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_OBJECTDYING:
        if( ((SwPtrMsgPoolItem *)pOld)->pObject == GetRegisteredIn() )
        {
            // dann uns selbst beenden
            GetRegisteredInNonConst()->Remove( this );
            ReleaseRef();                   // ansonsten sind wir fertig!
        }
        break;
    }
}

void SwHTMLParser::DocumentDetected()
{
    OSL_ENSURE( !bDocInitalized, "DocumentDetected mehrfach aufgerufen" );
    bDocInitalized = sal_True;
    if( IsNewDoc() )
    {
        if( IsInHeader() )
            FinishHeader( sal_True );

        CallEndAction( sal_True, sal_True );

        pDoc->GetIDocumentUndoRedo().DoUndo(false);
        // Durch das DocumentDetected wurde im allgemeinen eine
        // ViewShell angelegt. Es kann aber auch sein, dass sie
        // erst spaeter angelegt wird, naemlich dann, wenn die UI
        // gecaptured ist.
        CallStartAction();
    }
}

// is called for every token that is recognised in CallParser
void SwHTMLParser::NextToken( int nToken )
{
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        // Was the import cancelled by SFX? If a pending stack
        // exists, clean it.
        eState = SVPAR_ERROR;
        OSL_ENSURE( !pPendStack || pPendStack->nToken,
                "SwHTMLParser::NextToken: Pending-Stack without token" );
        if( 1 == pDoc->getReferenceCount() || !pPendStack )
            return ;
    }

#if OSL_DEBUG_LEVEL > 0
    if( pPendStack )
    {
        switch( nToken )
        {
            // tables are read by recursive method calls
        case HTML_TABLE_ON:
            // For CSS declarations we might have to wait
            // for a file download to finish
        case HTML_LINK:
            // For controls we might have to set the size.
        case HTML_INPUT:
        case HTML_TEXTAREA_ON:
        case HTML_SELECT_ON:
        case HTML_SELECT_OFF:
            break;
        default:
            OSL_ENSURE( !pPendStack, "Unbekanntes Token fuer Pending-Stack" );
            break;
        }
    }
#endif

    // The following special cases have to be treated before the
    // filter detection, because Netscape doesn't reference the content
    // of the title for filter detection either.
    if( !pPendStack )
    {
        if( bInTitle )
        {
            switch( nToken )
            {
            case HTML_TITLE_OFF:
                if( IsNewDoc() && !sTitle.isEmpty() )
                {
                    if( pDoc->GetDocShell() ) {
                        uno::Reference<document::XDocumentPropertiesSupplier>
                            xDPS(pDoc->GetDocShell()->GetModel(),
                            uno::UNO_QUERY_THROW);
                        uno::Reference<document::XDocumentProperties> xDocProps(
                            xDPS->getDocumentProperties());
                        OSL_ENSURE(xDocProps.is(), "no DocumentProperties");
                        if (xDocProps.is()) {
                            xDocProps->setTitle(sTitle);
                        }

                        pDoc->GetDocShell()->SetTitle( sTitle );
                    }
                }
                bInTitle = sal_False;
                sTitle = "";
                break;

            case HTML_NONBREAKSPACE:
                sTitle += " ";
                break;

            case HTML_SOFTHYPH:
                sTitle += "-";
                break;

            case HTML_TEXTTOKEN:
                sTitle += aToken;
                break;

            default:
                sTitle += "<";
                if( (HTML_TOKEN_ONOFF & nToken) && (1 & nToken) )
                    sTitle += "/";
                sTitle += sSaveToken;
                if( !aToken.isEmpty() )
                {
                    sTitle += " ";
                    sTitle += aToken;
                }
                sTitle += ">";
                break;
            }

            return;
        }
    }

    // Find out what type of document it is if we don't know already.
    // For Controls this has to be finished before the control is inserted
    // because for inserting a View is needed.
    if( !bDocInitalized )
        DocumentDetected();

    sal_Bool bGetIDOption = sal_False, bInsertUnknown = sal_False;
    sal_Bool bUpperSpaceSave = bUpperSpace;
    bUpperSpace = sal_False;

    // The following special cases may or have to be treated after the
    // filter detection
    if( !pPendStack )
    {
        if( bInFloatingFrame )
        {
            // <SCRIPT> is ignored here (from us), because it is ignored in
            // Applets as well
            if( HTML_IFRAME_OFF == nToken )
            {
                bCallNextToken = sal_False;
                EndFloatingFrame();
            }

            return;
        }
        else if( bInNoEmbed )
        {
            switch( nToken )
            {
            case HTML_NOEMBED_OFF:
                aContents = convertLineEnd(aContents, GetSystemLineEnd());
                InsertComment( aContents, OOO_STRING_SVTOOLS_HTML_noembed );
                aContents = "";
                bCallNextToken = sal_False;
                bInNoEmbed = sal_False;
                break;

            case HTML_RAWDATA:
                InsertCommentText( OOO_STRING_SVTOOLS_HTML_noembed );
                break;

            default:
                OSL_ENSURE( !this, "SwHTMLParser::NextToken: invalid tag" );
                break;
            }

            return;
        }
        else if( pAppletImpl )
        {
            // in an applet only <PARAM> tags and the </APPLET> tag
            // are of interest for us (for the moment)
            // <SCRIPT> is ignored here (from Netscape)!

            switch( nToken )
            {
            case HTML_APPLET_OFF:
                bCallNextToken = sal_False;
                EndApplet();
                break;
            case HTML_OBJECT_OFF:
                bCallNextToken = sal_False;
                EndObject();
                break;

            case HTML_PARAM:
                InsertParam();
                break;
            }

            return;
        }
        else if( bTextArea )
        {
            // in a TextArea everything up to </TEXTAREA> is inserted as text.
            // <SCRIPT> is ignored here (from Netscape)!

            switch( nToken )
            {
            case HTML_TEXTAREA_OFF:
                bCallNextToken = sal_False;
                EndTextArea();
                break;

            default:
                InsertTextAreaText( static_cast< sal_uInt16 >(nToken) );
                break;
            }

            return;
        }
        else if( bSelect )
        {
            // HAS to be treated after bNoScript!
            switch( nToken )
            {
            case HTML_SELECT_OFF:
                bCallNextToken = sal_False;
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
                // treat in normal switch
                break;

            default:
                // ignore
                return;
            }
        }
        else if( pMarquee )
        {
            // in a TextArea everything up to </TEXTAREA> is inserted as text.
            // The <SCRIPT> tags are ignored from MS-IE, we ignore the whole
            // script.
            switch( nToken )
            {
            case HTML_MARQUEE_OFF:
                bCallNextToken = sal_False;
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
                bCallNextToken = sal_False;
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
                bCallNextToken = sal_False;
                break;

            case HTML_TEXTTOKEN:
                InsertFootEndNoteText();
                break;
            }
            return;
        }
        else if( !aUnknownToken.isEmpty() )
        {
            // Paste content of unknown tags.
            // (but surely if we are not in the header section) fdo#36080 fdo#34666
            if (!aToken.isEmpty() && !IsInHeader() )
            {
                if( !bDocInitalized )
                    DocumentDetected();
                pDoc->InsertString( *pPam, aToken );

                // if there are temporary paragraph attributes and the
                // paragraph isn't empty then the paragraph attributes
                // are final.
                if( !aParaAttrs.empty() )
                    aParaAttrs.clear();

                SetAttr();
            }

            // Unknown token in the header are only closed by a matching
            // end-token, </HEAD> or <BODY>. Text inside is ignored.
            switch( nToken )
            {
            case HTML_UNKNOWNCONTROL_OFF:
                if( aUnknownToken.startsWith(sSaveToken) )
                    return;
            case HTML_FRAMESET_ON:
            case HTML_HEAD_OFF:
            case HTML_BODY_ON:
            case HTML_IMAGE:        // Don't know why Netscape acts this way.
                aUnknownToken = "";
                break;
            case HTML_TEXTTOKEN:
                return;
            default:
                aUnknownToken = "";
                break;
            }
        }
    }

    switch( nToken )
    {
    case HTML_BODY_ON:
        if( !aStyleSource.isEmpty() )
        {
            pCSS1Parser->ParseStyleSheet( aStyleSource );
            aStyleSource = "";
        }
        if( IsNewDoc() )
        {
            InsertBodyOptions();
            // If there is a template for the first or the right page,
            // it is set here.
            const SwPageDesc *pPageDesc = 0;
            if( pCSS1Parser->IsSetFirstPageDesc() )
                pPageDesc = pCSS1Parser->GetFirstPageDesc();
            else if( pCSS1Parser->IsSetRightPageDesc() )
                pPageDesc = pCSS1Parser->GetRightPageDesc();

            if( pPageDesc )
            {
                pDoc->InsertPoolItem( *pPam, SwFmtPageDesc( pPageDesc ), 0 );
            }
        }
        break;

    case HTML_LINK:
        InsertLink();
        break;

    case HTML_BASE:
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_HREF:
                    sBaseURL = rOption.GetString();
                    break;
                case HTML_O_TARGET:
                    if( IsNewDoc() )
                    {
                        SwDocShell *pDocShell(pDoc->GetDocShell());
                        OSL_ENSURE(pDocShell, "no SwDocShell");
                        if (pDocShell) {
                            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
                            uno::Reference<document::XDocumentProperties>
                                xDocProps(xDPS->getDocumentProperties());
                            OSL_ENSURE(xDocProps.is(),"no DocumentProperties");
                            if (xDocProps.is()) {
                                xDocProps->setDefaultTarget(
                                    rOption.GetString());
                            }
                        }
                    }
                    break;
                }
            }
        }
        break;

    case HTML_META:
        {
            SvKeyValueIterator *pHTTPHeader = 0;
            if( IsNewDoc() )
            {
                SwDocShell *pDocSh = pDoc->GetDocShell();
                if( pDocSh )
                    pHTTPHeader = pDocSh->GetHeaderAttributes();
            }
            SwDocShell *pDocShell(pDoc->GetDocShell());
            OSL_ENSURE(pDocShell, "no SwDocShell");
            if (pDocShell)
            {
                uno::Reference<document::XDocumentProperties> xDocProps;
                if (IsNewDoc())
                {
                    const uno::Reference<document::XDocumentPropertiesSupplier>
                        xDPS( pDocShell->GetModel(), uno::UNO_QUERY_THROW );
                    xDocProps = xDPS->getDocumentProperties();
                    OSL_ENSURE(xDocProps.is(), "DocumentProperties is null");
                }
                ParseMetaOptions( xDocProps, pHTTPHeader );
            }
        }
        break;

    case HTML_TITLE_ON:
        bInTitle = sal_True;
        break;

    case HTML_SCRIPT_ON:
        NewScript();
        break;

    case HTML_SCRIPT_OFF:
        EndScript();
        break;

    case HTML_NOSCRIPT_ON:
    case HTML_NOSCRIPT_OFF:
        bInsertUnknown = sal_True;
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
                if( !aStyleSource.isEmpty() )
                    aStyleSource += "\n";
                aStyleSource += aToken;
            }
        }
        break;

    case HTML_OBJECT_ON:
#ifdef SOLAR_JAVA
        NewObject();
        bCallNextToken = pAppletImpl!=0 && pTable!=0;
#endif
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
            bGetIDOption = sal_True;
            // <BR>s in <PRE> resemble true LFs, hence no break

    case HTML_NEWPARA:
        // CR in PRE/LISTING/XMP
        {
            if( HTML_NEWPARA==nToken ||
                pPam->GetPoint()->nContent.GetIndex() )
            {
                AppendTxtNode(); // there is no LF at this place
                                 // therefore it will cause no problems
                SetTxtCollAttrs();
            }
            // progress bar
            if( !GetMedium() || !GetMedium()->IsRemote() )
                ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        }
        break;

    case HTML_NONBREAKSPACE:
        pDoc->InsertString( *pPam, OUString(CHAR_HARDBLANK) );
        break;

    case HTML_SOFTHYPH:
        pDoc->InsertString( *pPam, OUString(CHAR_SOFTHYPHEN) );
        break;

    case HTML_LINEFEEDCHAR:
        if( pPam->GetPoint()->nContent.GetIndex() )
            AppendTxtNode();
        if( !pTable && !pDoc->IsInHeaderFooter( pPam->GetPoint()->nNode ) )
        {
            NewAttr( &aAttrTab.pBreak, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK) );
            EndAttr( aAttrTab.pBreak, 0, sal_False );
        }
        break;

    case HTML_TEXTTOKEN:
        // insert string without spanning attributes at the end.
        if( !aToken.isEmpty() && ' '==aToken[0] && !IsReadPRE() )
        {
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
            if( nPos )
            {
                const String& rText =
                    pPam->GetPoint()->nNode.GetNode().GetTxtNode()->GetTxt();
                sal_Unicode cLast = rText.GetChar(--nPos);
                if( ' ' == cLast || '\x0a' == cLast)
                    aToken = aToken.copy(1);
            }
            else
                aToken = aToken.copy(1);

            if( aToken.isEmpty() )
            {
                bUpperSpace = bUpperSpaceSave;
                break;
            }
        }

        if( !aToken.isEmpty() )
        {
            if( !bDocInitalized )
                DocumentDetected();
            pDoc->InsertString( *pPam, aToken );

            // if there are temporary paragraph attributes and the
            // paragraph isn't empty then the paragraph attributes
            // are final.
            if( !aParaAttrs.empty() )
                aParaAttrs.clear();

            SetAttr();
        }
        break;

    case HTML_HORZRULE:
        InsertHorzRule();
        break;

    case HTML_IMAGE:
        InsertImage();
        // if only the parser references the doc, we can break and set
        // an error code
        if( 1 == pDoc->getReferenceCount() )
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
        bInNoEmbed = sal_True;
        bCallNextToken = pTable!=0;
        ReadRawData( OOO_STRING_SVTOOLS_HTML_noembed );
        break;

    case HTML_DEFLIST_ON:
        if( nOpenParaToken )
            EndPara();
        NewDefList();
        break;
    case HTML_DEFLIST_OFF:
        if( nOpenParaToken )
            EndPara();
        EndDefListItem( 0, sal_False, 1==nDefListDeep );
        EndDefList();
        break;

    case HTML_DD_ON:
    case HTML_DT_ON:
        if( nOpenParaToken )
            EndPara();
        EndDefListItem( 0, sal_False );// close <DD>/<DT> and set no template
        NewDefListItem( nToken );
        break;

    case HTML_DD_OFF:
    case HTML_DT_OFF:
        // c.f. HTML_LI_OFF
        // Actually we should close a DD/DT now.
        // But neither Netscape nor Microsoft do this and so don't we.
        EndDefListItem( nToken, sal_False );
        break;

    // divisions
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

    // templates
    case HTML_PARABREAK_ON:
        if( nOpenParaToken )
            EndPara( sal_True );
        NewPara();
        break;

    case HTML_PARABREAK_OFF:
        EndPara( sal_True );
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
        bNoParSpace = sal_True; // the last PRE-paragraph gets a spacing
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
            OSL_ENSURE( !pTable, "table in table not allowed here" );
            if( !pTable && (IsNewDoc() || !pPam->GetNode()->FindTableNode()) &&
                (pPam->GetPoint()->nNode.GetIndex() >
                            pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                !pPam->GetNode()->FindFootnoteStartNode() ) )
            {
                if ( nParaCnt < 5 )
                    Show();     // show what we have up to here

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

    // lists
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
        EndNumBulListItem( 0, sal_True, GetNumInfo().GetDepth()==1 );
        EndNumBulList( nToken );
        break;

    case HTML_LI_ON:
    case HTML_LISTHEADER_ON:
        if( nOpenParaToken &&
            (pPam->GetPoint()->nContent.GetIndex()
            || HTML_PARABREAK_ON==nOpenParaToken) )
        {
            // only finish paragraph for <P><LI>, not for <DD><LI>
            EndPara();
        }

        EndNumBulListItem( 0, sal_False );// close <LI>/<LH> and don't set a template
        NewNumBulListItem( nToken );
        break;

    case HTML_LI_OFF:
    case HTML_LISTHEADER_OFF:
        EndNumBulListItem( nToken, sal_False );
        break;

    // Attribute :
    case HTML_ITALIC_ON:
        {
            SvxPostureItem aPosture( ITALIC_NORMAL, RES_CHRATR_POSTURE );
            SvxPostureItem aPostureCJK( ITALIC_NORMAL, RES_CHRATR_CJK_POSTURE );
            SvxPostureItem aPostureCTL( ITALIC_NORMAL, RES_CHRATR_CTL_POSTURE );
            NewStdAttr( HTML_ITALIC_ON,
                           &aAttrTab.pItalic, aPosture,
                           &aAttrTab.pItalicCJK, &aPostureCJK,
                           &aAttrTab.pItalicCTL, &aPostureCTL );
        }
        break;

    case HTML_BOLD_ON:
        {
            SvxWeightItem aWeight( WEIGHT_BOLD, RES_CHRATR_WEIGHT );
            SvxWeightItem aWeightCJK( WEIGHT_BOLD, RES_CHRATR_CJK_WEIGHT );
            SvxWeightItem aWeightCTL( WEIGHT_BOLD, RES_CHRATR_CTL_WEIGHT );
            NewStdAttr( HTML_BOLD_ON,
                        &aAttrTab.pBold, aWeight,
                        &aAttrTab.pBoldCJK, &aWeightCJK,
                        &aAttrTab.pBoldCTL, &aWeightCTL );
        }
        break;


    case HTML_STRIKE_ON:
    case HTML_STRIKETHROUGH_ON:
        {
            NewStdAttr( HTML_STRIKE_ON, &aAttrTab.pStrike,
                        SvxCrossedOutItem(STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT) );
        }
        break;

    case HTML_UNDERLINE_ON:
        {
            NewStdAttr( HTML_UNDERLINE_ON, &aAttrTab.pUnderline,
                        SvxUnderlineItem(UNDERLINE_SINGLE, RES_CHRATR_UNDERLINE) );
        }
        break;

    case HTML_SUPERSCRIPT_ON:
        {
            NewStdAttr( HTML_SUPERSCRIPT_ON, &aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUPER,HTML_ESC_PROP, RES_CHRATR_ESCAPEMENT) );
        }
        break;

    case HTML_SUBSCRIPT_ON:
        {
            NewStdAttr( HTML_SUBSCRIPT_ON, &aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUB,HTML_ESC_PROP, RES_CHRATR_ESCAPEMENT) );
        }
        break;

    case HTML_BLINK_ON:
        {
            NewStdAttr( HTML_BLINK_ON, &aAttrTab.pBlink,
                        SvxBlinkItem( sal_True, RES_CHRATR_BLINK ) );
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
        if( !aStyleSource.isEmpty() )
        {
            pCSS1Parser->ParseStyleSheet( aStyleSource );
            aStyleSource = "";
        }
        break;

    case HTML_DOCTYPE:
    case HTML_BODY_OFF:
    case HTML_HTML_OFF:
    case HTML_HEAD_ON:
    case HTML_TITLE_OFF:
        break;      // don't evaluate further???
    case HTML_HTML_ON:
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                if( HTML_O_DIR == rOption.GetToken() )
                {
                    const String& rDir = rOption.GetString();
                    SfxItemSet aItemSet( pDoc->GetAttrPool(),
                                         pCSS1Parser->GetWhichMap() );
                    SvxCSS1PropertyInfo aPropInfo;
                    String aDummy;
                    ParseStyleOptions( aDummy, aDummy, aDummy, aItemSet,
                                       aPropInfo, 0, &rDir );

                    pCSS1Parser->SetPageDescAttrs( 0, &aItemSet );
                    break;
                }
            }
        }
        break;

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
        if( ( aToken.getLength() > 5 ) && ( ! bIgnoreHTMLComments ) )
        {
            // insert as Post-It
            // If there are no space characters right behind
            // the <!-- and on front of the -->, leave the comment untouched.
            if( ' ' == aToken[ 3 ] &&
                ' ' == aToken[ aToken.getLength()-3 ] )
            {
                String aComment( aToken.copy( 3, aToken.getLength()-5 ) );
                InsertComment(comphelper::string::strip(aComment, ' '));
            }
            else
            {
                OUStringBuffer aComment;
                aComment.append('<').append(aToken).append('>');
                InsertComment( aComment.makeStringAndClear() );
            }
        }
        break;

    case HTML_MAP_ON:
        // Image Maps are read asynchronously: At first only an image map is created
        // Areas are processed later. Nevertheless the
        // ImageMap is inserted into the IMap-Array, because it might be used
        // already.
        pImageMap = new ImageMap;
        if( ParseMapOptions( pImageMap) )
        {
            if( !pImageMaps )
                pImageMaps = new ImageMaps;
            pImageMaps->push_back( pImageMap );
        }
        else
        {
            delete pImageMap;
            pImageMap = 0;
        }
        break;

    case HTML_MAP_OFF:
        // there is no ImageMap anymore (don't delete IMap, because it's
        // already contained in the array!)
        pImageMap = 0;
        break;

    case HTML_AREA:
        if( pImageMap )
            ParseAreaOptions( pImageMap, sBaseURL, SFX_EVENT_MOUSEOVER_OBJECT,
                                         SFX_EVENT_MOUSEOUT_OBJECT );
        break;

    case HTML_FRAMESET_ON:
        bInsertUnknown = bKeepUnknown;
        break;

    case HTML_NOFRAMES_ON:
        if( IsInHeader() )
            FinishHeader( sal_True );
        bInsertUnknown = bKeepUnknown;
        break;

    case HTML_UNKNOWNCONTROL_ON:
        // Ignore content of unknown token in the header, if the token
        // does not start with a '!'.
        // (but judging from the code, also if does not start with a '%')
        // (and also if we're not somewhere we consider PRE)
        if( IsInHeader() && !IsReadPRE() && aUnknownToken.isEmpty() &&
            sSaveToken.Len() && '!' != sSaveToken.GetChar(0) &&
            '%' != sSaveToken.GetChar(0) )
            aUnknownToken = sSaveToken;
        // no break

    default:
        bInsertUnknown = bKeepUnknown;
        break;
    }

    if( bGetIDOption )
        InsertIDOption();

    if( bInsertUnknown )
    {
        String aComment(OUString("HTML: <"));
        if( (HTML_TOKEN_ONOFF & nToken) != 0 && (1 & nToken) != 0 )
            aComment += '/';
        aComment += sSaveToken;
        if( !aToken.isEmpty() )
        {
            UnescapeToken();
            (aComment += ' ') += aToken;
        }
        aComment += '>';
        InsertComment( aComment );
    }

    // if there are temporary paragraph attributes and the
    // paragraph isn't empty then the paragraph attributes are final.
    if( !aParaAttrs.empty() && pPam->GetPoint()->nContent.GetIndex() )
        aParaAttrs.clear();
}


extern bool swhtml_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 );

static void lcl_swhtml_getItemInfo( const _HTMLAttr& rAttr,
                                 sal_Bool& rScriptDependent, sal_Bool& rFont,
                                 sal_uInt16& rScriptType )
{
    sal_uInt16 nWhich = rAttr.GetItem().Which();
    switch( nWhich )
    {
    case RES_CHRATR_FONT:
        rFont = sal_True;
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        rScriptType = i18n::ScriptType::LATIN;
        rScriptDependent = sal_True;
        break;
    case RES_CHRATR_CJK_FONT:
        rFont = sal_True;
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        rScriptType = i18n::ScriptType::ASIAN;
        rScriptDependent = sal_True;
        break;
    case RES_CHRATR_CTL_FONT:
        rFont = sal_True;
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        rScriptType = i18n::ScriptType::COMPLEX;
        rScriptDependent = sal_True;
        break;
    default:
        rScriptDependent = sal_False;
        rFont = sal_False;
        break;
    }
}

sal_Bool SwHTMLParser::AppendTxtNode( SwHTMLAppendMode eMode, sal_Bool bUpdateNum )
{
    // Ein harter Zeilen-Umbruch am Ende muss immer entfernt werden.
    // Einen zweiten ersetzen wir durch einen Absatz-Abstand.
    xub_StrLen nLFStripped = StripTrailingLF();
    if( (AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode) && nLFStripped > 1 )
        eMode = AM_SPACE;

    // die harten Attribute an diesem Absatz werden nie mehr ungueltig
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    if( AM_SPACE==eMode || AM_NOSPACE==eMode )
    {
        SwTxtNode *pTxtNode =
            pPam->GetPoint()->nNode.GetNode().GetTxtNode();

        const SvxULSpaceItem& rULSpace =
            (const SvxULSpaceItem&)pTxtNode->SwCntntNode::GetAttr( RES_UL_SPACE );

        sal_Bool bChange = AM_NOSPACE==eMode ? rULSpace.GetLower() > 0
                                         : rULSpace.GetLower() == 0;

        if( bChange )
        {
            const SvxULSpaceItem& rCollULSpace =
                pTxtNode->GetAnyFmtColl().GetULSpace();

            sal_Bool bMayReset = AM_NOSPACE==eMode ? rCollULSpace.GetLower() == 0
                                               : rCollULSpace.GetLower() > 0;

            if( bMayReset &&
                rCollULSpace.GetUpper() == rULSpace.GetUpper() )
            {
                pTxtNode->ResetAttr( RES_UL_SPACE );
            }
            else
            {
                pTxtNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(),
                         AM_NOSPACE==eMode ? 0 : HTML_PARSPACE, RES_UL_SPACE ) );
            }
        }
    }
    bNoParSpace = AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode;

    SwPosition aOldPos( *pPam->GetPoint() );

    sal_Bool bRet = pDoc->AppendTxtNode( *pPam->GetPoint() );

    // Zeichen-Attribute aufspalten und ggf keine setzen, die ueber den
    // ganzen Absatz gesetzt sind
    const SwNodeIndex& rEndIdx = aOldPos.nNode;
    xub_StrLen nEndCnt = aOldPos.nContent.GetIndex();
    const SwPosition& rPos = *pPam->GetPoint();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
         nCnt--; ++pTbl )
    {
        _HTMLAttr *pAttr = *pTbl;
        if( pAttr && pAttr->GetItem().Which() < RES_PARATR_BEGIN )
        {
            sal_Bool bWholePara = sal_False;

            while( pAttr )
            {
                _HTMLAttr *pNext = pAttr->GetNext();
                if( pAttr->GetSttParaIdx() < rEndIdx.GetIndex() ||
                    (!bWholePara &&
                     pAttr->GetSttPara() == rEndIdx &&
                     pAttr->GetSttCnt() != nEndCnt) )
                {
                    bWholePara =
                        pAttr->GetSttPara() == rEndIdx &&
                        pAttr->GetSttCnt() == 0;

                    xub_StrLen nStt = pAttr->nSttCntnt;
                    sal_Bool bScript = sal_False, bFont = sal_False;
                    sal_uInt16 nScriptItem;
                    sal_Bool bInsert = sal_True;
                       lcl_swhtml_getItemInfo( *pAttr, bScript, bFont,
                                            nScriptItem );
                        // den besehrigen Teil setzen
                    if( bInsert && bScript )
                    {
                        const SwTxtNode *pTxtNd =
                            pAttr->GetSttPara().GetNode().GetTxtNode();
                        OSL_ENSURE( pTxtNd, "No text node" );
                        if( pTxtNd )
                        {
                            const String& rText = pTxtNd->GetTxt();
                            sal_uInt16 nScriptTxt =
                                g_pBreakIt->GetBreakIter()->getScriptType(
                                            rText, pAttr->GetSttCnt() );
                            xub_StrLen nScriptEnd = (xub_StrLen)g_pBreakIt->GetBreakIter()
                                    ->endOfScript( rText, nStt, nScriptTxt );
                            while( nScriptEnd < nEndCnt )
                            {
                                if( nScriptItem == nScriptTxt )
                                {
                                    _HTMLAttr *pSetAttr =
                                        pAttr->Clone( rEndIdx, nScriptEnd );
                                    pSetAttr->nSttCntnt = nStt;
                                    pSetAttr->ClearPrev();
                                    if( !pNext || bWholePara )
                                    {
                                        if (pSetAttr->bInsAtStart)
                                            aSetAttrTab.push_front( pSetAttr );
                                        else
                                            aSetAttrTab.push_back( pSetAttr );
                                    }
                                    else
                                        pNext->InsertPrev( pSetAttr );
                                }
                                nStt = nScriptEnd;
                                nScriptTxt = g_pBreakIt->GetBreakIter()->getScriptType(
                                                rText, nStt );
                                nScriptEnd = (xub_StrLen)g_pBreakIt->GetBreakIter()
                                    ->endOfScript( rText, nStt, nScriptTxt );
                            }
                            bInsert = nScriptItem == nScriptTxt;
                        }
                    }
                    if( bInsert )
                    {
                        _HTMLAttr *pSetAttr =
                            pAttr->Clone( rEndIdx, nEndCnt );
                        pSetAttr->nSttCntnt = nStt;

                        // Wenn das Attribut den gesamten Absatz umspannt, werden
                        // alle auesseren Attribute nicht mehr beachtet. Deshalb
                        // darf es auch nicht in die Prev-Liste eines ausseren
                        // Attributs eingetragen werden, denn dieses wird ja
                        // erstmal nicht gesetzt. Das fuehrt zu verschiebenungen,
                        // wenn Felder ins Rennen kommen
                        if( !pNext || bWholePara )
                        {
                            if (pSetAttr->bInsAtStart)
                                aSetAttrTab.push_front( pSetAttr );
                            else
                                aSetAttrTab.push_back( pSetAttr );
                        }
                        else
                            pNext->InsertPrev( pSetAttr );
                    }
                    else
                    {
                        _HTMLAttr *pPrev = pAttr->GetPrev();
                        if( pPrev )
                        {
                            // Die Previous-Attribute muessen trotzdem gesetzt werden.
                            if( !pNext || bWholePara )
                            {
                                if (pPrev->bInsAtStart)
                                    aSetAttrTab.push_front( pPrev );
                                else
                                    aSetAttrTab.push_back( pPrev );
                            }
                            else
                                pNext->InsertPrev( pPrev );
                        }
                    }
                    pAttr->ClearPrev();
                }

                pAttr->SetStart( rPos );
                pAttr = pNext;
            }
        }
    }

    if( bUpdateNum )
    {
        if( GetNumInfo().GetDepth() )
        {
            sal_uInt8 nLvl = GetNumInfo().GetLevel();
            SetNodeNum( nLvl, false );
        }
        else
            pPam->GetNode()->GetTxtNode()->ResetAttr( RES_PARATR_NUMRULE );
    }

    // Attrubute im Absatz davor sollte man jetzt setzen (wegen JavaScript)
    SetAttr();

    // Now it is time to get rid of all script dependent hints that are
    // equal to the settings in the style
    SwTxtNode *pTxtNd = rEndIdx.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "There is the txt node" );
    sal_uInt16 nCntAttr = (pTxtNd  && pTxtNd->GetpSwpHints())
                            ? pTxtNd->GetSwpHints().Count() : 0;
    if( nCntAttr )
    {
        // These are the end position of all script depenent hints.
        // If we find a hint that starts before the current end position,
        // we have to set it. If we find a hint that start behind or at
        // that position, we have to take the hint's value into account.
        // If it is equal to the style, or in fact the paragarph's value
        // for that hint, the hint is removed. Otherwise it's end position
        // is remembered.
        xub_StrLen aEndPos[15] =
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        SwpHints& rHints = pTxtNd->GetSwpHints();
        for( sal_uInt16 i=0; i < nCntAttr; i++ )
        {
            SwTxtAttr *pHt = rHints.GetTextHint( i );
            sal_uInt16 nWhich = pHt->Which();
            sal_Int16 nIdx = -1;
            //In 'hintids.hxx', the following five attributes don't follow
            //each other in the Latin attributes as they do among CJK and
            //CTL attributes, so the old code just made a mess, IMHO.
            //E.g. 29-22=7, which should be LANGUAGE, but it's FONT.
            //Moreover, it should occur between 0 and 4.
            //Since it would be too risky to change the attribute codes,
            //I repaired the following source code the 'brute force' way.

            //Old code:
            /*
            if( RES_CHRATR_CJK_FONT <= nWhich &&
                    nWhich <= RES_CHRATR_CTL_WEIGHT )
            {
                nIdx = static_cast< sal_uInt16 >(nWhich - RES_CHRATR_CJK_FONT + 5);
            }
            else switch...
            */

            if( RES_CHRATR_CJK_FONT == nWhich || RES_CHRATR_CTL_FONT == nWhich )
            {
                nIdx = static_cast< sal_uInt16 >(0);
            }
            else if( RES_CHRATR_CJK_FONTSIZE == nWhich || RES_CHRATR_CTL_FONTSIZE == nWhich )
            {
                nIdx = static_cast< sal_uInt16 >(1);
            }
            else if( RES_CHRATR_CJK_LANGUAGE == nWhich || RES_CHRATR_CTL_LANGUAGE == nWhich )
            {
                nIdx = static_cast< sal_uInt16 >(2);
            }
            else if( RES_CHRATR_CJK_POSTURE == nWhich || RES_CHRATR_CTL_POSTURE == nWhich )
            {
                nIdx = static_cast< sal_uInt16 >(3);
            }
            else if( RES_CHRATR_CJK_WEIGHT == nWhich || RES_CHRATR_CTL_WEIGHT == nWhich )
            {
                nIdx = static_cast< sal_uInt16 >(4);
            }
            else switch( nWhich )
            {
            case RES_CHRATR_FONT:       nIdx = 0;   break;
            case RES_CHRATR_FONTSIZE:   nIdx = 1;   break;
            case RES_CHRATR_LANGUAGE:   nIdx = 2;   break;
            case RES_CHRATR_POSTURE:    nIdx = 3;   break;
            case RES_CHRATR_WEIGHT:     nIdx = 4;   break;
            }
            if( nIdx != -1 )
            {
                xub_StrLen nStt = *pHt->GetStart();
                if( nStt >= aEndPos[nIdx] )
                {
                    sal_Bool bFont = (nIdx % 5) == 0;
                    const SfxPoolItem& rItem =
                        ((const SwCntntNode *)pTxtNd)->GetAttr( nWhich );
                    if( bFont ? swhtml_css1atr_equalFontItems(rItem,pHt->GetAttr())
                              : rItem == pHt->GetAttr() )
                    {
                        // The hint is the same as set in the paragraph and
                        // therefore, it can be deleted
                        // CAUTION!!! This WILL delete the hint and it MAY
                        // also delete the SwpHints!!! To avoid any trouble
                        // we leave the loop immediately if this is the last
                        // hint.
                        pTxtNd->DeleteAttribute( pHt );
                        if( 1 == nCntAttr )
                            break;
                        i--;
                        nCntAttr--;
                    }
                    else
                    {
                        // The hint is different. Therefore all hints within that
                        // hint have to be ignored.
                        aEndPos[nIdx] = pHt->GetEnd() ? *pHt->GetEnd() : nStt;
                    }
                }
                else
                {
                    // The hint starts before another one ends.
                    // The hint in this case is not deleted
                    OSL_ENSURE( pHt->GetEnd() && *pHt->GetEnd() <= aEndPos[nIdx],
                            "hints aren't nested properly!" );
                }
            }
        }
    }


    if( !pTable && !--nParaCnt )
        Show();

    return bRet;
}

void SwHTMLParser::AddParSpace()
{
    //If it already has ParSpace, return
    if( !bNoParSpace )
        return;

    bNoParSpace = sal_False;

    sal_uLong nNdIdx = pPam->GetPoint()->nNode.GetIndex() - 1;

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
            //What I do here, is that I examine the attributes, and if
            //I find out, that it's CJK/CTL, then I set the paragraph space
            //to the value set in HTML_CJK_PARSPACE/HTML_CTL_PARSPACE.

            sal_Bool bIsCJK = false;
            sal_Bool bIsCTL = false;
            SwpHints& rHints = pTxtNode->GetSwpHints();
            sal_uInt16 nWhich;
            SwTxtAttr *pHt;

            sal_uInt16 nCntAttr = (pTxtNode  && pTxtNode->GetpSwpHints())
                            ? pTxtNode->GetSwpHints().Count() : 0;

            for(sal_uInt16 i = 0; (i < nCntAttr) && !bIsCJK; ++i)
            {
               pHt = rHints.GetTextHint(i);
               nWhich = pHt->Which();
               if( RES_CHRATR_CJK_FONT == nWhich ||
                   RES_CHRATR_CJK_FONTSIZE == nWhich ||
                   RES_CHRATR_CJK_LANGUAGE == nWhich ||
                   RES_CHRATR_CJK_POSTURE == nWhich ||
                   RES_CHRATR_CJK_WEIGHT == nWhich )
                       bIsCJK = true;
               if( RES_CHRATR_CTL_FONT == nWhich ||
                   RES_CHRATR_CTL_FONTSIZE == nWhich ||
                   RES_CHRATR_CTL_LANGUAGE == nWhich ||
                   RES_CHRATR_CTL_POSTURE == nWhich ||
                   RES_CHRATR_CTL_WEIGHT == nWhich )
                       bIsCJK = false;
            }

            if( bIsCTL )
            {
                pTxtNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(), HTML_CTL_PARSPACE, RES_UL_SPACE )  );
            }
            else if( bIsCJK )
            {
                pTxtNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(), HTML_CJK_PARSPACE, RES_UL_SPACE )  );
            } else {
                pTxtNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(), HTML_PARSPACE, RES_UL_SPACE )  );
            }
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

    OSL_ENSURE( SVPAR_WORKING==eState, "Show nicht im Working-State - Das kann ins Auge gehen" );
    ViewShell *pOldVSh = CallEndAction();

    GetpApp()->Reschedule();

    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
    }

    // Die ViewShell nochmal holen, denn sie koennte im Reschedule
    // zerstoert wirden sein.
    ViewShell *pVSh = CallStartAction( pOldVSh );

    // ist der aktuelle Node nicht mehr sichtbar, dann benutzen wir
    // eine groessere Schrittweite
    if( pVSh )
    {
        nParaCnt = (pPam->GetPoint()->nNode.GetNode().IsInVisibleArea(pVSh))
            ? 5 : 50;
    }
}

void SwHTMLParser::ShowStatline()
{
    // Hier wird
    // - ein Reschedule gerufen, damit gescrollt werden kann
    // - die eiegen View-Shell wieder gesetzt
    // - ein Start/End-Action gerufen, wenn gescrollt wurde.

    OSL_ENSURE( SVPAR_WORKING==eState, "ShowStatLine nicht im Working-State - Das kann ins Auge gehen" );

    // Laufbalkenanzeige
    if( !GetMedium() || !GetMedium()->IsRemote() )
    {
        ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        CheckActionViewShell();
    }
    else
    {
        GetpApp()->Reschedule();

        if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
            || 1 == pDoc->getReferenceCount() )
            // wurde der Import vom SFX abgebrochen?
            eState = SVPAR_ERROR;

        ViewShell *pVSh = CheckActionViewShell();
        if( pVSh && pVSh->HasInvalidRect() )
        {
            CallEndAction( sal_False, sal_False );
            CallStartAction( pVSh, sal_False );
        }
    }
}

ViewShell *SwHTMLParser::CallStartAction( ViewShell *pVSh, sal_Bool bChkPtr )
{
    OSL_ENSURE( !pActionViewShell, "CallStartAction: ViewShell schon gesetzt" );

    if( !pVSh || bChkPtr )
    {
#if OSL_DEBUG_LEVEL > 0
        ViewShell *pOldVSh = pVSh;
#endif
        pDoc->GetEditShell( &pVSh );
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !pVSh || !pOldVSh || pOldVSh == pVSh, "CallStartAction: Wer hat die ViewShell ausgetauscht?" );
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

ViewShell *SwHTMLParser::CallEndAction( sal_Bool bChkAction, sal_Bool bChkPtr )
{
    if( bChkPtr )
    {
        ViewShell *pVSh = 0;
        pDoc->GetEditShell( &pVSh );
        OSL_ENSURE( !pVSh || pActionViewShell == pVSh,
                "CallEndAction: Wer hat die ViewShell ausgetauscht?" );
#if OSL_DEBUG_LEVEL > 0
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
                ((SwCrsrShell*)pSh)->SttEndDoc(sal_True);
            pSh = (ViewShell *)pSh->GetNext();
        } while( pSh != pActionViewShell );

        bSetCrsr = sal_False;
    }
    if( pActionViewShell->ISA( SwEditShell ) )
    {
        //Schon gescrollt?, dann dafuer sorgen, dass die View sich nicht bewegt!
        const sal_Bool bOldLock = pActionViewShell->IsViewLocked();
        pActionViewShell->LockView( sal_True );
        const sal_Bool bOldEndActionByVirDev = pActionViewShell->IsEndActionByVirDev();
        pActionViewShell->SetEndActionByVirDev( sal_True );;
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
            bChkJumpMark = sal_False;
        }
    }
    else
        pActionViewShell->EndAction();

    // sollte der Parser der Letzte sein, der das Doc haelt, dann kann
    // man hier abbrechen und einen Fehler setzen.
    if( 1 == pDoc->getReferenceCount() )
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
    OSL_ENSURE( !pVSh || pActionViewShell == pVSh,
            "CheckActionViewShell: Wer hat die ViewShell ausgetauscht?" );
#if OSL_DEBUG_LEVEL > 0
    if( pActionViewShell && !pVSh )
        pVSh = 0;
#endif
    if( pVSh != pActionViewShell )
        pActionViewShell = 0;

    return pActionViewShell;
}


void SwHTMLParser::_SetAttr( sal_Bool bChkEnd, sal_Bool bBeforeTable,
                             _HTMLAttrs *pPostIts )
{
    SwPaM* pAttrPam = new SwPaM( *pPam->GetPoint() );
    const SwNodeIndex& rEndIdx = pPam->GetPoint()->nNode;
    xub_StrLen nEndCnt = pPam->GetPoint()->nContent.GetIndex();
    _HTMLAttr* pAttr;
    SwCntntNode* pCNd;
    sal_uInt16 n;

    _HTMLAttrs aFields;

    for( n = aSetAttrTab.size(); n; )
    {
        pAttr = aSetAttrTab[ --n ];
        sal_uInt16 nWhich = pAttr->pItem->Which();

        sal_uLong nEndParaIdx = pAttr->GetEndParaIdx();
        sal_Bool bSetAttr;
        if( bChkEnd )
        {
            // Zechen-Attribute mit Ende moeglich frueh,
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
                         (isCHRATR(nWhich) || isTXTATR_WITHEND(nWhich)) ) ||
                       ( bBeforeTable &&
                         nEndParaIdx == rEndIdx.GetIndex() &&
                         !pAttr->GetEndCnt() );
        }
        else
        {
            // Attribiute im Content-Bereich duerfen nicht gesetzt
            // werden, wenn wir in einem Sonderbereich stehen, aber
            // umgekekehrt schon.
            sal_uLong nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bSetAttr = nEndParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nEndParaIdx <= nEndOfIcons;
        }

        if( bSetAttr )
        {
            // Das Attribute darf nicht in der liste der vorlaeufigen
            // Absatz-Attribute stehen, weil es sonst geloescht wurde.
            while( !aParaAttrs.empty() )
            {
                OSL_ENSURE( pAttr != aParaAttrs.back(),
                        "SetAttr: Attribut duerfte noch nicht gesetzt werden" );
                aParaAttrs.pop_back();
            }


            // dann also setzen
            aSetAttrTab.erase( aSetAttrTab.begin() + n );

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


                pCNd = pAttr->nSttPara.GetNode().GetCntntNode();
                if( !pCNd )
                {
                    // durch die elende Loescherei von Nodes kann auch mal
                    // ein Index auf einen End-Node zeigen :-(
                    if ( (pAttr->GetSttPara() == pAttr->GetEndPara()) &&
                         !isTXTATR_NOEND(nWhich) )
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
                        OSL_ENSURE( !this, "SetAttr: GoNext() failed!" );
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
                if ( (pAttr->GetSttPara() != pAttr->GetEndPara()) &&
                         !isTXTATR_NOEND(nWhich) )
                {
                    pCNd = pAttr->nEndPara.GetNode().GetCntntNode();
                    if( !pCNd )
                    {
                        pCNd = pDoc->GetNodes().GoPrevious( &(pAttr->nEndPara) );
                        if( pCNd )
                            pAttr->nEndCntnt = pCNd->Len();
                        else
                        {
                            OSL_ENSURE( !this, "SetAttr: GoPrevious() failed!" );
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
                         !isTXTATR_NOEND(nWhich) )
                    {
                        if( pAttrPam->GetMark()->nNode.GetIndex() !=
                            rEndIdx.GetIndex() )
                        {
                            OSL_ENSURE( !pAttrPam->GetPoint()->nContent.GetIndex(),
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
                case RES_FLTR_BOOKMARK: // insert bookmark
                    {
                        const String sName( ((SfxStringItem*)pAttr->pItem)->GetValue() );
                        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
                        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark( sName );
                        if( ppBkmk != pMarkAccess->getMarksEnd() &&
                            ppBkmk->get()->GetMarkStart() == *pAttrPam->GetPoint() )
                            break; // do not generate duplicates on this position
                        pAttrPam->DeleteMark();
                        const ::sw::mark::IMark* const pNewMark = pMarkAccess->makeMark(
                            *pAttrPam,
                            sName,
                            IDocumentMarkAccess::BOOKMARK );

                        // jump to bookmark
                        if( JUMPTO_MARK == eJumpTo && pNewMark->GetName() == OUString(sJmpMark) )
                        {
                            bChkJumpMark = sal_True;
                            eJumpTo = JUMPTO_NONE;
                        }
                    }
                    break;
                case RES_TXTATR_FIELD:
                    {
                        sal_uInt16 nFldWhich =
                            pPostIts ? ((const SwFmtFld *)pAttr->pItem)
                                            ->GetFld()->GetTyp()->Which() : 0;
                        if( pPostIts && (RES_POSTITFLD == nFldWhich ||
                                         RES_SCRIPTFLD == nFldWhich) )
                        {
                            pPostIts->push_front( pAttr );
                        }
                        else
                        {
                            aFields.push_back( pAttr);
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
                    OSL_ENSURE( !this,
                            "LRSpace ueber mehrere Absaetze gesetzt!" );
                    // kein break (hier sollen wir trotzdem nie hinkommen;
                default:

                    // ggfs. ein Bookmark anspringen
                    if( RES_TXTATR_INETFMT == nWhich &&
                        JUMPTO_MARK == eJumpTo &&
                        sJmpMark == ((SwFmtINetFmt*)pAttr->pItem)->GetName() )
                    {
                        bChkJumpMark = sal_True;
                        eJumpTo = JUMPTO_NONE;
                    }

                    pDoc->InsertPoolItem( *pAttrPam, *pAttr->pItem, nsSetAttrMode::SETATTR_DONTREPLACE );
                }
                pAttrPam->DeleteMark();

                delete pAttr;
                pAttr = pPrev;
            }
        }
    }

    for( n = aMoveFlyFrms.size(); n; )
    {
        SwFrmFmt *pFrmFmt = aMoveFlyFrms[ --n ];

        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        OSL_ENSURE( FLY_AT_PARA == rAnchor.GetAnchorId(),
                "Nur Auto-Rahmen brauchen eine Spezialbehandlung" );
        const SwPosition *pFlyPos = rAnchor.GetCntntAnchor();
        sal_uLong nFlyParaIdx = pFlyPos->nNode.GetIndex();
        sal_Bool bMoveFly;
        if( bChkEnd )
        {
            bMoveFly = nFlyParaIdx < rEndIdx.GetIndex() ||
                       ( nFlyParaIdx == rEndIdx.GetIndex() &&
                         aMoveFlyCnts[n] < nEndCnt );
        }
        else
        {
            sal_uLong nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
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
            aAnchor.SetType( FLY_AT_CHAR );
            aAnchor.SetAnchor( pAttrPam->GetPoint() );
            pFrmFmt->SetFmtAttr( aAnchor );

            const SwFmtHoriOrient& rHoriOri = pFrmFmt->GetHoriOrient();
            if( text::HoriOrientation::LEFT == rHoriOri.GetHoriOrient() )
            {
                SwFmtHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetRelationOrient( text::RelOrientation::CHAR );
                pFrmFmt->SetFmtAttr( aHoriOri );
            }
            const SwFmtVertOrient& rVertOri = pFrmFmt->GetVertOrient();
            if( text::VertOrientation::TOP == rVertOri.GetVertOrient() )
            {
                SwFmtVertOrient aVertOri( rVertOri );
                aVertOri.SetRelationOrient( text::RelOrientation::CHAR );
                pFrmFmt->SetFmtAttr( aVertOri );
            }

            pFrmFmt->MakeFrms();
            aMoveFlyFrms.erase( aMoveFlyFrms.begin() + n );
            aMoveFlyCnts.erase( aMoveFlyCnts.begin() + n );
        }
    }
    while( !aFields.empty() )
    {
        pAttr = aFields[0];

        pCNd = pAttr->nSttPara.GetNode().GetCntntNode();
        pAttrPam->GetPoint()->nNode = pAttr->nSttPara;
        pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nSttCntnt );

        if( bBeforeTable &&
            pAttrPam->GetPoint()->nNode.GetIndex() == rEndIdx.GetIndex() )
        {
            OSL_ENSURE( !bBeforeTable, "Aha, der Fall tritt also doch ein" );
            OSL_ENSURE( !pAttrPam->GetPoint()->nContent.GetIndex(),
                    "Content-Position vor Tabelle nicht 0???" );
            // !!!
            pAttrPam->Move( fnMoveBackward );
        }

        pDoc->InsertPoolItem( *pAttrPam, *pAttr->pItem, 0 );

        aFields.pop_front();
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
                            sal_Bool bChkEmpty )
{
    OSL_ENSURE( !ppDepAttr, "SwHTMLParser::EndAttr: ppDepAttr-Feature ungetestet?" );
    // Der Listenkopf ist im Attribut gespeichert
    _HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    // die aktuelle Position als Ende-Position merken
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

        OSL_ENSURE( pLast, "Attribut nicht in eigener Liste gefunden!" );
    }

    sal_Bool bMoveBack = sal_False;
    sal_uInt16 nWhich = pAttr->pItem->Which();
    if( !nEndCnt && RES_PARATR_BEGIN <= nWhich &&
        *pEndIdx != pAttr->GetSttPara() )
    {
        // dann eine Cntntnt Position zurueck!
        bMoveBack = pPam->Move( fnMoveBackward );
        nEndCnt = pPam->GetPoint()->nContent.GetIndex();
    }

    // nun das Attrubut beenden
    _HTMLAttr *pNext = pAttr->GetNext();


    sal_Bool bInsert;
    sal_uInt16 nScriptItem = 0;
    sal_Bool bScript = sal_False, bFont = sal_False;
    // ein Bereich ??
    if( !bChkEmpty || (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
        RES_PAGEDESC == nWhich || RES_BREAK == nWhich ||
        *pEndIdx != pAttr->GetSttPara() ||
        nEndCnt != pAttr->GetSttCnt() )
    {
        bInsert = sal_True;
        // We do some optimization for script depenedent attribtes here.
        if( *pEndIdx == pAttr->GetSttPara() )
        {
            lcl_swhtml_getItemInfo( *pAttr, bScript, bFont, nScriptItem );
        }
    }
    else
    {
        bInsert = sal_False;
    }

    if( bInsert && bScript )
    {
        const SwTxtNode *pTxtNd = pAttr->GetSttPara().GetNode()
                                            .GetTxtNode();
        OSL_ENSURE( pTxtNd, "No text node" );
        const String& rText = pTxtNd->GetTxt();
        sal_uInt16 nScriptTxt = g_pBreakIt->GetBreakIter()->getScriptType(
                        rText, pAttr->GetSttCnt() );
        xub_StrLen nScriptEnd = (xub_StrLen)g_pBreakIt->GetBreakIter()
                    ->endOfScript( rText, pAttr->GetSttCnt(), nScriptTxt );
        while( nScriptEnd < nEndCnt )
        {
            if( nScriptItem == nScriptTxt )
            {
                _HTMLAttr *pSetAttr = pAttr->Clone( *pEndIdx, nScriptEnd );
                pSetAttr->ClearPrev();
                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    if (pSetAttr->bInsAtStart)
                        aSetAttrTab.push_front( pSetAttr );
                    else
                        aSetAttrTab.push_back( pSetAttr );
                }
            }
            pAttr->nSttCntnt = nScriptEnd;
            nScriptTxt = g_pBreakIt->GetBreakIter()->getScriptType(
                            rText, nScriptEnd );
            nScriptEnd = (xub_StrLen)g_pBreakIt->GetBreakIter()
                    ->endOfScript( rText, nScriptEnd, nScriptTxt );
        }
        bInsert = nScriptItem == nScriptTxt;
    }
    if( bInsert )
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
                if (pAttr->bInsAtStart)
                    aSetAttrTab.push_front( pAttr );
                else
                    aSetAttrTab.push_back( pAttr );
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
                if (pPrev->bInsAtStart)
                    aSetAttrTab.push_front( pPrev );
                else
                    aSetAttrTab.push_back( pPrev );
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
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    // Der Listenkopf ist im Attribut gespeichert
    _HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

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

        OSL_ENSURE( pLast, "Attribut nicht in eigener Liste gefunden!" );
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
            if (pPrev->bInsAtStart)
                aSetAttrTab.push_front( pPrev );
            else
                aSetAttrTab.push_back( pPrev );
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
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;

    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
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
                                 sal_Bool bMoveEndBack )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    const SwNodeIndex& nSttIdx = pPam->GetPoint()->nNode;
    SwNodeIndex nEndIdx( nSttIdx );

    // alle noch offenen Attribute beenden und hinter der Tabelle
    // neu aufspannen
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;
    sal_Bool bSetAttr = sal_True;
    xub_StrLen nSttCnt = pPam->GetPoint()->nContent.GetIndex();
    xub_StrLen nEndCnt = nSttCnt;

    if( bMoveEndBack )
    {
        sal_uLong nOldEnd = nEndIdx.GetIndex();
        sal_uLong nTmpIdx;
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
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
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
                    if (pSetAttr->bInsAtStart)
                        aSetAttrTab.push_front( pSetAttr );
                    else
                        aSetAttrTab.push_back( pSetAttr );
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
                    if (pPrev->bInsAtStart)
                        aSetAttrTab.push_front( pPrev );
                    else
                        aSetAttrTab.push_back( pPrev );
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
                                   sal_Bool bSetNewStart )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;

    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
        nCnt--; (++pTbl, ++pSaveTbl) )
    {
        OSL_ENSURE( !*pTbl, "Die Attribut-Tabelle ist nicht leer!" );

        const SwPosition *pPos = pPam->GetPoint();
        const SwNodeIndex& rSttPara = pPos->nNode;
        xub_StrLen nSttCnt = pPos->nContent.GetIndex();

        *pTbl = *pSaveTbl;

        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            OSL_ENSURE( !pAttr->GetPrev() || !pAttr->GetPrev()->ppHead,
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

void SwHTMLParser::InsertAttr( const SfxPoolItem& rItem, sal_Bool bLikePara,
                               sal_Bool bInsAtStart )
{
    _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(),
                                     rItem );
    if( bLikePara )
        pTmp->SetLikePara();
    if (bInsAtStart)
        aSetAttrTab.push_front( pTmp );
    else
        aSetAttrTab.push_back( pTmp );
}

void SwHTMLParser::InsertAttrs( _HTMLAttrs& rAttrs )
{
    while( !rAttrs.empty() )
    {
        _HTMLAttr *pAttr = rAttrs.front();
        InsertAttr( pAttr->GetItem() );
        rAttrs.pop_front();
        delete pAttr;
    }
}

void SwHTMLParser::NewStdAttr( int nToken )
{
    String aId, aStyle, aLang, aDir;
    OUString aClass;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            if( HTML_SPAN_ON != nToken || aClass.isEmpty() ||
                !CreateContainer( aClass, aItemSet, aPropInfo, pCntxt ) )
                DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::NewStdAttr( int nToken,
                               _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               _HTMLAttr **ppAttr2, const SfxPoolItem *pItem2,
                               _HTMLAttr **ppAttr3, const SfxPoolItem *pItem3 )
{
    String aId, aStyle, aClass, aLang, aDir;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        aItemSet.Put( rItem );
        if( pItem2 )
            aItemSet.Put( *pItem2 );
        if( pItem3 )
            aItemSet.Put( *pItem3 );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
    }
    else
    {
        InsertAttr( ppAttr ,rItem, pCntxt );
        if( pItem2 )
        {
            OSL_ENSURE( ppAttr2, "missing table entry for item2" );
            InsertAttr( ppAttr2, *pItem2, pCntxt );
        }
        if( pItem3 )
        {
            OSL_ENSURE( ppAttr3, "missing table entry for item3" );
            InsertAttr( ppAttr3, *pItem3, pCntxt );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::EndTag( int nToken )
{
    // den Kontext holen
    _HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }
}


void SwHTMLParser::NewBasefontAttr()
{
    String aId, aStyle, aClass, aLang, aDir;
    sal_uInt16 nSize = 3;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_SIZE:
            nSize = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
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
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        //CJK has different defaults
        SvxFontHeightItem aFontHeight( aFontHeights[nSize-1], 100, RES_CHRATR_FONTSIZE );
        aItemSet.Put( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( aFontHeights[nSize-1], 100, RES_CHRATR_CJK_FONTSIZE );
        aItemSet.Put( aFontHeightCJK );
        //Complex type can contain so many types of letters,
        //that it's not really worthy to bother, IMO.
        //Still, I have set a default.
        SvxFontHeightItem aFontHeightCTL( aFontHeights[nSize-1], 100, RES_CHRATR_CTL_FONTSIZE );
        aItemSet.Put( aFontHeightCTL );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
    }
    else
    {
        SvxFontHeightItem aFontHeight( aFontHeights[nSize-1], 100, RES_CHRATR_FONTSIZE );
        InsertAttr( &aAttrTab.pFontHeight, aFontHeight, pCntxt );
        SvxFontHeightItem aFontHeightCJK( aFontHeights[nSize-1], 100, RES_CHRATR_CJK_FONTSIZE );
        InsertAttr( &aAttrTab.pFontHeightCJK, aFontHeightCJK, pCntxt );
        SvxFontHeightItem aFontHeightCTL( aFontHeights[nSize-1], 100, RES_CHRATR_CTL_FONTSIZE );
        InsertAttr( &aAttrTab.pFontHeightCJK, aFontHeightCTL, pCntxt );
    }

    // den Kontext merken
    PushContext( pCntxt );

    // die Font-Size merken
    aBaseFontStack.push_back( nSize );
}

void SwHTMLParser::EndBasefontAttr()
{
    EndTag( HTML_BASEFONT_ON );

    // Stack-Unterlauf in Tabellen vermeiden
    if( aBaseFontStack.size() > nBaseFontStMin )
        aBaseFontStack.erase( aBaseFontStack.begin() + aBaseFontStack.size() - 1 );
}

void SwHTMLParser::NewFontAttr( int nToken )
{
    sal_uInt16 nBaseSize =
        ( aBaseFontStack.size() > nBaseFontStMin
            ? (aBaseFontStack[aBaseFontStack.size()-1] & FONTSIZE_MASK)
            : 3 );
    sal_uInt16 nFontSize =
        ( aFontStack.size() > nFontStMin
            ? (aFontStack[aFontStack.size()-1] & FONTSIZE_MASK)
            : nBaseSize );

    String aFace, aId, aStyle, aClass, aLang, aDir;
    Color aColor;
    sal_uLong nFontHeight = 0;  // tatsaechlich einzustellende Font-Hoehe
    sal_uInt16 nSize = 0;       // Fontgroesse in Netscape-Notation (1-7)
    sal_Bool bColor = sal_False;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_SIZE:
            if( HTML_FONT_ON==nToken && rOption.GetString().Len() )
            {
                sal_Int32 nSSize;
                if( '+' == rOption.GetString().GetChar(0) ||
                    '-' == rOption.GetString().GetChar(0) )
                    nSSize = nBaseSize + rOption.GetSNumber();
                else
                    nSSize = (sal_Int32)rOption.GetNumber();

                if( nSSize < 1 )
                    nSSize = 1;
                else if( nSSize > 7 )
                    nSSize = 7;

                nSize = (sal_uInt16)nSSize;
                nFontHeight = aFontHeights[nSize-1];
            }
            break;
        case HTML_O_COLOR:
            if( HTML_FONT_ON==nToken )
            {
                rOption.GetColor( aColor );
                bColor = sal_True;
            }
            break;
        case HTML_O_FACE:
            if( HTML_FONT_ON==nToken )
                aFace = rOption.GetString();
            break;
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        }
    }

    if( HTML_FONT_ON != nToken )
    {
        // HTML_BIGPRINT_ON oder HTML_SMALLPRINT_ON

        // in Ueberschriften bestimmt die aktuelle Ueberschrift
        // die Font-Hoehe und nicht BASEFONT
        sal_uInt16 nPoolId = GetCurrFmtColl()->GetPoolFmtId();
        if( (nPoolId>=RES_POOLCOLL_HEADLINE1 &&
             nPoolId<=RES_POOLCOLL_HEADLINE6) )
        {
            // wenn die Schriftgroesse in der Ueberschrift noch
            // nicht veraendert ist, die aus der Vorlage nehmen
            if( nFontStHeadStart==aFontStack.size() )
                nFontSize = static_cast< sal_uInt16 >(6 - (nPoolId - RES_POOLCOLL_HEADLINE1));
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

    OSL_ENSURE( !nSize == !nFontHeight, "HTML-Font-Size != Font-Height" );

    String aFontName, aStyleName;
    FontFamily eFamily = FAMILY_DONTKNOW;   // Family und Pitch,
    FontPitch ePitch = PITCH_DONTKNOW;      // falls nicht gefunden
    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();

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

        sal_Bool bFound = sal_False;
        sal_Int32 nStrPos = 0;
        while( nStrPos!= -1 )
        {
            String aFName = aFace.GetToken( 0, ',', nStrPos );
            aFName = comphelper::string::strip(aFName, ' ');
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
                            bFound = sal_True;
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
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( nFontHeight )
        {
            SvxFontHeightItem aFontHeight( nFontHeight, 100, RES_CHRATR_FONTSIZE );
            aItemSet.Put( aFontHeight );
            SvxFontHeightItem aFontHeightCJK( nFontHeight, 100, RES_CHRATR_CJK_FONTSIZE );
            aItemSet.Put( aFontHeightCJK );
            SvxFontHeightItem aFontHeightCTL( nFontHeight, 100, RES_CHRATR_CTL_FONTSIZE );
            aItemSet.Put( aFontHeightCTL );
        }
        if( bColor )
            aItemSet.Put( SvxColorItem(aColor, RES_CHRATR_COLOR) );
        if( aFontName.Len() )
        {
            SvxFontItem aFont( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_FONT );
            aItemSet.Put( aFont );
            SvxFontItem aFontCJK( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CJK_FONT );
            aItemSet.Put( aFontCJK );
            SvxFontItem aFontCTL( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CTL_FONT );
            aItemSet.Put( aFontCTL );
        }


        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
    }
    else
    {
        if( nFontHeight )
        {
            SvxFontHeightItem aFontHeight( nFontHeight, 100, RES_CHRATR_FONTSIZE );
            InsertAttr( &aAttrTab.pFontHeight, aFontHeight, pCntxt );
            SvxFontHeightItem aFontHeightCJK( nFontHeight, 100, RES_CHRATR_CJK_FONTSIZE );
            InsertAttr( &aAttrTab.pFontHeight, aFontHeightCJK, pCntxt );
            SvxFontHeightItem aFontHeightCTL( nFontHeight, 100, RES_CHRATR_CTL_FONTSIZE );
            InsertAttr( &aAttrTab.pFontHeight, aFontHeightCTL, pCntxt );
        }
        if( bColor )
            InsertAttr( &aAttrTab.pFontColor, SvxColorItem(aColor, RES_CHRATR_COLOR), pCntxt );
        if( aFontName.Len() )
        {
            SvxFontItem aFont( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_FONT );
            InsertAttr( &aAttrTab.pFont, aFont, pCntxt );
            SvxFontItem aFontCJK( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CJK_FONT );
            InsertAttr( &aAttrTab.pFont, aFontCJK, pCntxt );
            SvxFontItem aFontCTL( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CTL_FONT );
            InsertAttr( &aAttrTab.pFont, aFontCTL, pCntxt );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );

    aFontStack.push_back( nSize );
}

void SwHTMLParser::EndFontAttr( int nToken )
{
    EndTag( nToken );

    // Stack-Unterlauf in Tabellen vermeiden
    if( aFontStack.size() > nFontStMin )
        aFontStack.erase( aFontStack.begin() + aFontStack.size() - 1 );
}


void SwHTMLParser::NewPara()
{
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    eParaAdjust = SVX_ADJUST_END;
    String aId, aStyle, aClass, aLang, aDir;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_ALIGN:
                eParaAdjust = (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(eParaAdjust) );
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
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
    if( HasStyleOptions( aStyle, aId, aEmptyStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    // und auf den Stack packen
    PushContext( pCntxt );

    // die aktuelle Vorlage oder deren Attribute setzen
    SetTxtCollAttrs( aClass.Len() ? pCntxt : 0 );

    // Laufbalkenanzeige
    ShowStatline();

    OSL_ENSURE( !nOpenParaToken, "Jetzt geht ein offenes Absatz-Element verloren" );
    nOpenParaToken = HTML_PARABREAK_ON;
}

void SwHTMLParser::EndPara( sal_Bool bReal )
{
    if( HTML_LI_ON==nOpenParaToken && pTable )
    {
#if OSL_DEBUG_LEVEL > 0
        const SwNumRule *pNumRule = pPam->GetNode()->GetTxtNode()->GetNumRule();
        OSL_ENSURE( pNumRule, "Wo ist die Numrule geblieben" );
#endif
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
        PopContext( static_cast< sal_uInt16 >(nOpenParaToken ? (nOpenParaToken & ~1)
                                   : HTML_PARABREAK_ON) );

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

    String aId, aStyle, aClass, aLang, aDir;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_ALIGN:
                eParaAdjust = (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(eParaAdjust) );
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    // die passende Vorlage suchen
    sal_uInt16 nTxtColl;
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
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nTxtColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    // udn auf den Stack packen
    PushContext( pCntxt );

    // und die Vorlage oder deren Attribute setzen
    SetTxtCollAttrs( pCntxt );

    nFontStHeadStart = aFontStack.size();

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
    sal_uInt16 nPos = aContexts.size();
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
            aContexts.erase( aContexts.begin() + nPos );
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


void SwHTMLParser::NewTxtFmtColl( int nToken, sal_uInt16 nColl )
{
    String aId, aStyle, aClass, aLang, aDir;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
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
        OSL_ENSURE( !this, "unbekannte Vorlage" );
        break;
    }
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // ... und in einem Kontext merken
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( !aClass.Len() || !pCSS1Parser->GetClass( aClass ),
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
        OSL_ENSURE( !this, "unbekannte Vorlage" );
        break;
    }
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );

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


void SwHTMLParser::NewDefList()
{
    String aId, aStyle, aClass, aLang, aDir;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    sal_Bool bSpace = (GetNumInfo().GetDepth() + nDefListDeep) == 0;
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    // ein Level mehr
    nDefListDeep++;


    sal_Bool bInDD = sal_False, bNotInDD = sal_False;
    sal_uInt16 nPos = aContexts.size();
    while( !bInDD && !bNotInDD && nPos>nContextStMin )
    {
        sal_uInt16 nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDD = sal_True;
            break;
        case HTML_DD_ON:
            bInDD = sal_True;
            break;
        }
    }


    // ... und in einem Kontext merken
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_DEFLIST_ON );

    // darin auch die Raender merken
    sal_uInt16 nLeft=0, nRight=0;
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
        nLeft = nLeft + static_cast< sal_uInt16 >(rLRSpace.GetTxtLeft());
    }

    pCntxt->SetMargins( nLeft, nRight, nIndent );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
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
    sal_Bool bSpace = (GetNumInfo().GetDepth() + nDefListDeep) == 1;
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
    sal_Bool bInDefList = sal_False, bNotInDefList = sal_False;
    sal_uInt16 nPos = aContexts.size();
    while( !bInDefList && !bNotInDefList && nPos>nContextStMin )
    {
        sal_uInt16 nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
            bInDefList = sal_True;
            break;
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDefList = sal_True;
            break;
        }
    }

    // wenn nicht, implizit eine neue DL aufmachen
    if( !bInDefList )
    {
        nDefListDeep++;
        OSL_ENSURE( !nOpenParaToken,
                "Jetzt geht ein offenes Absatz-Element verloren" );
        nOpenParaToken = static_cast< sal_uInt16 >(nToken);
    }

    NewTxtFmtColl( nToken, static_cast< sal_uInt16 >(nToken==HTML_DD_ON ? RES_POOLCOLL_HTML_DD
                                              : RES_POOLCOLL_HTML_DT) );
}

void SwHTMLParser::EndDefListItem( int nToken, sal_Bool bSetColl,
                                   sal_Bool /*bLastPara*/ )
{
    // einen neuen Absatz aufmachen
    if( !nToken && pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SOFTNOSPACE );

    // Kontext zu dem Token suchen und vom Stack holen
    nToken &= ~1;
    _HTMLAttrContext *pCntxt = 0;
    sal_uInt16 nPos = aContexts.size();
    while( !pCntxt && nPos>nContextStMin )
    {
        sal_uInt16 nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DD_ON:
        case HTML_DT_ON:
            if( !nToken || nToken == nCntxtToken  )
            {
                pCntxt = aContexts[nPos];
                aContexts.erase( aContexts.begin() + nPos );
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


sal_Bool SwHTMLParser::HasCurrentParaFlys( sal_Bool bNoSurroundOnly,
                                       sal_Bool bSurroundOnly ) const
{
    // bNoSurroundOnly:     Der Absatz enthaelt mindestens einen Rahmen
    //                      ohne Umlauf
    // bSurroundOnly:       Der Absatz enthaelt mindestens einen Rahmen
    //                      mit Umlauf aber keinen ohne Umlauf
    // sonst:               Der Absatz enthaelt irgendeinen Rahmen
    SwNodeIndex& rNodeIdx = pPam->GetPoint()->nNode;

    const SwFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

    sal_Bool bFound = sal_False;
    for ( sal_uInt16 i=0; i<rFrmFmtTbl.size(); i++ )
    {
        SwFrmFmt *const pFmt = rFrmFmtTbl[i];
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        // Ein Rahmen wurde gefunden, wenn
        // - er absatzgebunden ist, und
        // - im aktuellen Absatz verankert ist, und
        //   - jeder absatzgebunene Rahmen zaehlt, oder
        //   - (nur Rahmen oder umlauf zaehlen und ) der Rahmen keinen
        //     Umlauf besitzt
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            pAPos->nNode == rNodeIdx )
        {
            if( !(bNoSurroundOnly || bSurroundOnly) )
            {
                bFound = sal_True;
                break;
            }
            else
            {
                // Wenn Rahmen mit Umlauf gesucht sind,
                // auch keine mit Durchlauf beachten. Dabei handelt es
                // sich (noch) um HIDDEN-Controls, und denen weicht man
                // besser auch nicht aus.
                SwSurround eSurround = pFmt->GetSurround().GetSurround();
                if( bNoSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = sal_True;
                        break;
                    }
                }
                if( bSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = sal_False;
                        break;
                    }
                    else if( SURROUND_THROUGHT!=eSurround )
                    {
                        bFound = sal_True;
                        // weitersuchen: Es koennten ja noch welche ohne
                        // Umlauf kommen ...
                    }
                }
            }
        }
    }

    return bFound;
}


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
    sal_uInt16 nTopColl = pContext ? pContext->GetTxtFmtColl() : 0;
    const OUString& rTopClass = pContext ? pContext->GetClass() : aEmptyStr;
    sal_uInt16 nDfltColl = RES_POOLCOLL_TEXT;

    sal_Bool bInPRE=sal_False;                          // etwas Kontext Info

    sal_uInt16 nLeftMargin = 0, nRightMargin = 0;   // die Einzuege und
    short nFirstLineIndent = 0;                 // Abstaende
    sal_uInt16 i;

    for( i = nContextStAttrMin; i < aContexts.size(); i++ )
    {
        const _HTMLAttrContext *pCntxt = aContexts[i];

        sal_uInt16 nColl = pCntxt->GetTxtFmtColl();
        if( nColl )
        {
            // Es gibt eine Vorlage, die zu setzen ist. Dann
            // muss zunaechst einmal entschieden werden,
            // ob die Vorlage auch gesetzt werden kann
            sal_Bool bSetThis = sal_True;
            switch( nColl )
            {
            case sal_uInt16(RES_POOLCOLL_HTML_PRE):
                bInPRE = sal_True;
                break;
            case sal_uInt16(RES_POOLCOLL_TEXT):
                // <TD><P CLASS=xxx> muss TD.xxx werden
                if( nDfltColl==RES_POOLCOLL_TABLE ||
                    nDfltColl==RES_POOLCOLL_TABLE_HDLN )
                    nColl = nDfltColl;
                break;
            case sal_uInt16(RES_POOLCOLL_HTML_HR):
                // <HR> auch in <PRE> als Vorlage setzen, sonst kann man sie
                // nicht mehr exportieren
                break;
            default:
                if( bInPRE )
                    bSetThis = sal_False;
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
            sal_uInt16 nLeft=0, nRight=0;

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
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,sal_True, &pItem) )
        {
            const SvxLRSpaceItem *pLRItem =
                (const SvxLRSpaceItem *)pItem;

            sal_Int32 nLeft = pLRItem->GetTxtLeft();
            sal_Int32 nRight = pLRItem->GetRight();
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
            nLeftMargin = nLeftMargin + static_cast< sal_uInt16 >(nLeft);
            nRightMargin = nRightMargin + static_cast< sal_uInt16 >(nRight);

            pContext->SetMargins( nLeftMargin, nRightMargin,
                                  nFirstLineIndent );
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,sal_True, &pItem) )
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
            nLeftMargin = static_cast< sal_uInt16 >(rLRItem.GetTxtLeft());
        if( !nRightMargin )
            nRightMargin = static_cast< sal_uInt16 >(rLRItem.GetRight());
        if( !nFirstLineIndent )
            nFirstLineIndent = rLRItem.GetTxtFirstLineOfst();
    }

    // bisherige harte Attributierung des Absatzes entfernen
    if( !aParaAttrs.empty() )
    {
        for( i=0; i<aParaAttrs.size(); i++ )
            aParaAttrs[i]->Invalidate();

        aParaAttrs.clear();
    }

    // Die Vorlage setzen
    pDoc->SetTxtFmtColl( *pPam, pCollToSet );

    // ggf. noch den Absatz-Einzug korrigieren
    const SvxLRSpaceItem& rLRItem = pCollToSet->GetLRSpace();
    sal_Bool bSetLRSpace;

           bSetLRSpace = nLeftMargin != rLRItem.GetTxtLeft() ||
                      nFirstLineIndent != rLRItem.GetTxtFirstLineOfst() ||
                      nRightMargin != rLRItem.GetRight();

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
            aParaAttrs.push_back( aAttrTab.pLRSpace );
            EndAttr( aAttrTab.pLRSpace, 0, sal_False );
        }
    }

    // und nun noch die Attribute setzen
    if( pItemSet )
    {
        InsertParaAttrs( *pItemSet );
        delete pItemSet;
    }
}


void SwHTMLParser::NewCharFmt( int nToken )
{
    String aId, aStyle, aLang, aDir;
    OUString aClass;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_LANG:
            aLang = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        }
    }

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // die Vorlage setzen und im Kontext merken
    SwCharFmt* pCFmt = pCSS1Parser->GetChrFmt( static_cast< sal_uInt16 >(nToken), aClass );
    OSL_ENSURE( pCFmt, "keine Zeichenvorlage zu Token gefunden" );


    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
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



void SwHTMLParser::InsertSpacer()
{
    // und es ggf. durch die Optionen veraendern
    String aId;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    Size aSize( 0, 0);
    long nSize = 0;
    sal_Bool bPrcWidth = sal_False;
    sal_Bool bPrcHeight = sal_False;
    sal_uInt16 nType = HTML_SPTYPE_HORI;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_TYPE:
            rOption.GetEnum( nType, aHTMLSpacerTypeTable );
            break;
        case HTML_O_ALIGN:
            eVertOri =
                rOption.GetEnum( aHTMLImgVAlignTable,
                                  eVertOri );
            eHoriOri =
                rOption.GetEnum( aHTMLImgHAlignTable,
                                  eHoriOri );
            break;
        case HTML_O_WIDTH:
            // erstmal nur als Pixelwerte merken!
            bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            // erstmal nur als Pixelwerte merken!
            bPrcHeight = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            aSize.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_SIZE:
            // erstmal nur als Pixelwerte merken!
            nSize = rOption.GetNumber();
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
            SvxProtectItem aProtectItem( RES_PROTECT) ;
            aProtectItem.SetCntntProtect( sal_True );
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
                aULSpace.SetLower( aULSpace.GetLower() + (sal_uInt16)nSize );
                pTxtNode->SetAttr( aULSpace );
            }
            else
            {
                NewAttr( &aAttrTab.pULSpace, SvxULSpaceItem( 0, (sal_uInt16)nSize, RES_UL_SPACE ) );
                EndAttr( aAttrTab.pULSpace, 0, sal_False );

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
                sal_uInt16 nLeft=0, nRight=0;
                short nIndent = 0;

                GetMarginsFromContextWithNumBul( nLeft, nRight, nIndent );
                nIndent = nIndent + (short)nSize;

                SvxLRSpaceItem aLRItem( RES_LR_SPACE );
                aLRItem.SetTxtLeft( nLeft );
                aLRItem.SetRight( nRight );
                aLRItem.SetTxtFirstLineOfst( nIndent );

                NewAttr( &aAttrTab.pLRSpace, aLRItem );
                EndAttr( aAttrTab.pLRSpace, 0, sal_False );
            }
            else
            {
                NewAttr( &aAttrTab.pKerning, SvxKerningItem( (short)nSize, RES_CHRATR_KERNING ) );
                OUString aTmp( ' ' );
                pDoc->InsertString( *pPam, aTmp );
                EndAttr( aAttrTab.pKerning );
            }
        }
    }
}

sal_uInt16 SwHTMLParser::ToTwips( sal_uInt16 nPixel ) const
{
    if( nPixel && Application::GetDefaultDevice() )
    {
        long nTwips = Application::GetDefaultDevice()->PixelToLogic(
                    Size( nPixel, nPixel ), MapMode( MAP_TWIP ) ).Width();
        return nTwips <= USHRT_MAX ? (sal_uInt16)nTwips : USHRT_MAX;
    }
    else
        return nPixel;
}

SwTwips SwHTMLParser::GetCurrentBrowseWidth()
{
    const SwTwips nWidth = SwHTMLTableLayout::GetBrowseWidth( *pDoc );
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



void SwHTMLParser::InsertIDOption()
{
    String aId;
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        if( HTML_O_ID==rOption.GetToken() )
        {
            aId = rOption.GetString();
            break;
        }
    }

    if( aId.Len() )
        InsertBookmark( aId );
}




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
    sal_Bool bClearLeft = sal_False, bClearRight = sal_False;
    sal_Bool bCleared = sal_False;  // wurde ein CLEAR ausgefuehrt?

    // dann holen wir mal die Optionen
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_CLEAR:
                {
                    const String &aClear = rOption.GetString();
                    if( aClear.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_all ) )
                    {
                        bClearLeft = sal_True;
                        bClearRight = sal_True;
                    }
                    else if( aClear.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_left ) )
                        bClearLeft = sal_True;
                    else if( aClear.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_right ) )
                        bClearRight = sal_True;
                }
                break;
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
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
            const SwFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

            for( sal_uInt16 i=0; i<rFrmFmtTbl.size(); i++ )
            {
                SwFrmFmt *const pFmt = rFrmFmtTbl[i];
                SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    pAPos->nNode == rNodeIdx &&
                    pFmt->GetSurround().GetSurround() != SURROUND_NONE )
                {
                    sal_Int16 eHori = RES_DRAWFRMFMT == pFmt->Which()
                        ? text::HoriOrientation::LEFT
                        : pFmt->GetHoriOrient().GetHoriOrient();

                    SwSurround eSurround = SURROUND_PARALLEL;
                    if( pPam->GetPoint()->nContent.GetIndex() )
                    {
                        if( bClearLeft && text::HoriOrientation::LEFT==eHori )
                            eSurround = SURROUND_RIGHT;
                        else if( bClearRight && text::HoriOrientation::RIGHT==eHori )
                            eSurround = SURROUND_LEFT;
                    }
                    else if( (bClearLeft && text::HoriOrientation::LEFT==eHori) ||
                             (bClearRight && text::HoriOrientation::RIGHT==eHori) )
                    {
                        eSurround = SURROUND_NONE;
                    }

                    if( SURROUND_PARALLEL != eSurround )
                    {
                        SwFmtSurround aSurround( eSurround );
                        if( SURROUND_NONE != eSurround )
                            aSurround.SetAnchorOnly( sal_True );
                        pFmt->SetFmtAttr( aSurround );
                        bCleared = sal_True;
                    }
                } // Anker ist nicht im Node
            } // Schleife ueber Fly-Frames
        } // kein Text-Node
    } // kein CLEAR

    // Styles parsen
    SvxFmtBreakItem aBreakItem( SVX_BREAK_NONE, RES_BREAK );
    sal_Bool bBreakItem = sal_False;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            if( pCSS1Parser->SetFmtBreak( aItemSet, aPropInfo ) )
            {
                aBreakItem = (const SvxFmtBreakItem &)aItemSet.Get( RES_BREAK );
                bBreakItem = sal_True;
            }
            if( !aPropInfo.aId.isEmpty() )
                InsertBookmark( aPropInfo.aId );
        }
    }

    if( bBreakItem && SVX_BREAK_PAGE_AFTER==aBreakItem.GetBreak() )
    {
        NewAttr( &aAttrTab.pBreak, aBreakItem );
        EndAttr( aAttrTab.pBreak, 0, sal_False );
    }

    if( !bCleared && !bBreakItem )
    {
        // wenn kein CLEAR ausgefuehrt werden sollte oder konnte, wird
        // ein Zeilenumbruch eingef?gt
        OUString sTmp( (sal_Unicode)0x0a );   // make the Mac happy :-)
        pDoc->InsertString( *pPam, sTmp );
    }
    else if( pPam->GetPoint()->nContent.GetIndex() )
    {
        // wenn ein clear in einem nicht-leeren Absatz ausgefuehrt wurde,
        // muss anschliessen ein neuer Absatz aufgemacht werden
        // MIB 21.02.97: Eigentlich muesste man hier den unteren Absatz-
        // Absatnd auf 0 drehen. Das geht aber bei sowas wie <BR ..><P>
        // schief (>Netacpe). Deshalb lassen wir das erstmal.
        AppendTxtNode( AM_NOSPACE );
    }
    if( bBreakItem && SVX_BREAK_PAGE_BEFORE==aBreakItem.GetBreak() )
    {
        NewAttr( &aAttrTab.pBreak, aBreakItem );
        EndAttr( aAttrTab.pBreak, 0, sal_False );
    }
}

void SwHTMLParser::InsertHorzRule()
{
    sal_uInt16 nSize = 0;
    sal_uInt16 nWidth = 0;

    SvxAdjust eAdjust = SVX_ADJUST_END;

    sal_Bool bPrcWidth = sal_False;
    sal_Bool bNoShade = sal_False;
    sal_Bool bColor = sal_False;

    Color aColor;
    String aId;

    // dann holen wir mal die Optionen
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_SIZE:
            nSize = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_WIDTH:
            bPrcWidth = (rOption.GetString().Search('%') != STRING_NOTFOUND);
            nWidth = (sal_uInt16)rOption.GetNumber();
            if( bPrcWidth && nWidth>=100 )
            {
                // 100%-Linien sind der default-Fall (keine Attrs neotig)
                nWidth = 0;
                bPrcWidth = sal_False;
            }
            break;
        case HTML_O_ALIGN:
            eAdjust =
                (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
            break;
        case HTML_O_NOSHADE:
            bNoShade = sal_True;
            break;
        case HTML_O_COLOR:
            rOption.GetColor( aColor );
            bColor = sal_True;
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
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

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
            if ( !bNoShade )
            {
                aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            }
            aBorderLine.SetWidth( nPHeight );
        }
        else if( bNoShade )
        {
            aBorderLine.SetWidth( DEF_LINE_WIDTH_2 );
        }
        else
        {
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
        }

        SvxBoxItem aBoxItem(RES_BOX);
        aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
        _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(), aBoxItem );
        aSetAttrTab.push_back( pTmp );
    }
    if( nWidth )
    {
        // Wenn wir in keiner Tabelle sind, wird die Breitenangabe durch
        // Absatz-Einzuege "getuerkt". In einer Tabelle macht das wenig
        // Sinn. Um zu Vermeiden, dass die Linie bei der Breitenberechnung
        // beruecksichtigt wird, bekommt sie aber trotzdem entsprechendes
        // LRSpace-Item verpasst.
        if( !pTable )
        {
            // Laenge und Ausrichtung der Linie ueber Absatz-Einzuege "tuerken"
            long nBrowseWidth = GetCurrentBrowseWidth();
            nWidth = bPrcWidth ? (sal_uInt16)((nWidth*nBrowseWidth) / 100)
                               : ToTwips( (sal_uInt16)nBrowseWidth );
            if( nWidth < MINLAY )
                nWidth = MINLAY;

            if( (long)nWidth < nBrowseWidth )
            {
                const SwFmtColl *pColl = GetCurrFmtColl();
                SvxLRSpaceItem aLRItem( pColl->GetLRSpace() );
                long nDist = nBrowseWidth - nWidth;

                switch( eAdjust )
                {
                case SVX_ADJUST_RIGHT:
                    aLRItem.SetTxtLeft( (sal_uInt16)nDist );
                    break;
                case SVX_ADJUST_LEFT:
                    aLRItem.SetRight( (sal_uInt16)nDist );
                    break;
                case SVX_ADJUST_CENTER:
                default:
                    nDist /= 2;
                    aLRItem.SetTxtLeft( (sal_uInt16)nDist );
                    aLRItem.SetRight( (sal_uInt16)nDist );
                    break;
                }

                _HTMLAttr* pTmp = new _HTMLAttr( *pPam->GetPoint(), aLRItem );
                aSetAttrTab.push_back( pTmp );
            }
        }
    }

    // Bookmarks koennen nicht in Hyperlinks eingefueht werden
    if( aId.Len() )
        InsertBookmark( aId );

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pPoppedContext = PopContext( HTML_HORZRULE );
    OSL_ENSURE( pPoppedContext==pCntxt, "wo kommt denn da ein HR-Kontext her?" );
    delete pPoppedContext;

    pPam->Move( fnMoveForward );

    // und im Absatz danach die dort aktuelle Vorlage setzen
    SetTxtCollAttrs();
}

void SwHTMLParser::ParseMoreMetaOptions()
{
    String aName;
    OUString aContent;
    sal_Bool bHTTPEquiv = sal_False;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_NAME:
            aName = rOption.GetString();
            bHTTPEquiv = sal_False;
            break;
        case HTML_O_HTTPEQUIV:
            aName = rOption.GetString();
            bHTTPEquiv = sal_True;
            break;
        case HTML_O_CONTENT:
            aContent = rOption.GetString();
            break;
        }
    }

    // Hier wird es etwas tricky: Wir wissen genau, da? die Dok-Info
    // nicht geaendert wurde. Deshalb genuegt es, auf Generator und
    // auf refresh abzufragen, um noch nicht verarbeitete Token zu finden,
    // denn das sind die einzigen, die die Dok-Info nicht modifizieren.
    if( aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_generator ) ||
        aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_refresh ) ||
        aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_content_type ) ||
        aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_content_script_type ) )
        return;

    aContent = comphelper::string::remove(aContent, '\r');
    aContent = comphelper::string::remove(aContent, '\n');

    if( aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_sdendnote ) )
    {
        FillEndNoteInfo( aContent );
        return;
    }

    if( aName.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_sdfootnote ) )
    {
        FillFootNoteInfo( aContent );
        return;
    }

    OUStringBuffer sText;
    sText.append("HTML: <");
    sText.append(OOO_STRING_SVTOOLS_HTML_meta);
    sText.append(' ');
    if( bHTTPEquiv  )
        sText.append(OOO_STRING_SVTOOLS_HTML_O_httpequiv);
    else
        sText.append(OOO_STRING_SVTOOLS_HTML_O_name);
    sText.append("=\"");
    sText.append(aName);
    sText.append("\" ");
    sText.append(OOO_STRING_SVTOOLS_HTML_O_content);
    sText.append("=\"");
    sText.append(aContent);
    sText.append("\">");

    SwPostItField aPostItFld(
        (SwPostItFieldType*)pDoc->GetSysFldType( RES_POSTITFLD ),
        aEmptyStr, sText.makeStringAndClear(), aEmptyStr, aEmptyStr, DateTime( DateTime::SYSTEM ) );
    SwFmtFld aFmtFld( aPostItFld );
    InsertAttr( aFmtFld );
}


_HTMLAttr::_HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
                      _HTMLAttr **ppHd ) :
    nSttPara( rPos.nNode ),
    nEndPara( rPos.nNode ),
    nSttCntnt( rPos.nContent.GetIndex() ),
    nEndCntnt(rPos.nContent.GetIndex() ),
    bInsAtStart( sal_True ),
    bLikePara( sal_False ),
    bValid( sal_True ),
    nCount( 1 ),
    pNext( 0 ),
    pPrev( 0 ),
    ppHead( ppHd )
{
    pItem = rItem.Clone();
}

_HTMLAttr::_HTMLAttr( const _HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
                      sal_uInt16 nEndCnt, _HTMLAttr **ppHd ) :
    nSttPara( rAttr.nSttPara ),
    nEndPara( rEndPara ),
    nSttCntnt( rAttr.nSttCntnt ),
    nEndCntnt( nEndCnt ),
    bInsAtStart( rAttr.bInsAtStart ),
    bLikePara( rAttr.bLikePara ),
    bValid( rAttr.bValid ),
    nCount( rAttr.nCount ),
    pNext( 0 ),
    pPrev( 0 ),
    ppHead( ppHd )
{
    pItem = rAttr.pItem->Clone();
}

_HTMLAttr::~_HTMLAttr()
{
    delete pItem;
}

_HTMLAttr *_HTMLAttr::Clone( const SwNodeIndex& rEndPara, sal_uInt16 nEndCnt ) const
{
    // das Attribut mit der alten Start-Position neu anlegen
    _HTMLAttr *pNew = new _HTMLAttr( *this, rEndPara, nEndCnt, ppHead );

    // die Previous-Liste muss uebernommen werden, die Next-Liste nicht!
    pNew->pPrev = pPrev;

    return pNew;
}

void _HTMLAttr::Reset( const SwNodeIndex& rSttPara, sal_uInt16 nSttCnt,
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
    OSL_ENSURE( !pPrv->pNext || pPrv->pNext == this,
            "_HTMLAttr::InsertPrev: pNext falsch" );
    pPrv->pNext = 0;

    OSL_ENSURE( 0 == pPrv->ppHead || ppHead == pPrv->ppHead,
            "_HTMLAttr::InsertPrev: ppHead falsch" );
    pPrv->ppHead = 0;

    _HTMLAttr *pAttr = this;
    while( pAttr->GetPrev() )
        pAttr = pAttr->GetPrev();

    pAttr->pPrev = pPrv;
}

bool SwHTMLParser::ParseMetaOptions(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        SvKeyValueIterator *i_pHeader )
{
    // always call base ParseMetaOptions, it sets the encoding (#i96700#)
    bool ret( HTMLParser::ParseMetaOptions(i_xDocProps, i_pHeader) );
    if (!ret && IsNewDoc())
    {
        ParseMoreMetaOptions();
    }
    return ret;
}

// override so we can parse DOCINFO field subtypes INFO[1-4]
void SwHTMLParser::AddMetaUserDefined( OUString const & i_rMetaName )
{
    // unless we already have 4 names, append the argument to m_InfoNames
    OUString* pName // the first empty string in m_InfoNames
         (m_InfoNames[0].isEmpty() ? &m_InfoNames[0] :
         (m_InfoNames[1].isEmpty() ? &m_InfoNames[1] :
         (m_InfoNames[2].isEmpty() ? &m_InfoNames[2] :
         (m_InfoNames[3].isEmpty() ? &m_InfoNames[3] : 0 ))));
    if (pName)
    {
        (*pName) = i_rMetaName;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
