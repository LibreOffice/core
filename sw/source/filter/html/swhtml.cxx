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
#include <svx/unobrushitemhelper.hxx>

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
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentState.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>
#include <expfld.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
#include <docufld.hxx>
#include <swcss1.hxx>
#include <fltini.hxx>
#include <htmltbl.hxx>
#include <htmlnum.hxx>
#include <swhtml.hxx>
#include <linkenum.hxx>
#include <breakit.hxx>
#include <SwAppletImpl.hxx>

#include <sfx2/viewfrm.hxx>

#include <statstr.hrc>
#include <swerror.h>
#include <css1atr.hxx>

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
    { nullptr,                0                   }
};

// <SPACER TYPE=...>
static HTMLOptionEnum aHTMLSpacerTypeTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_block,     HTML_SPTYPE_BLOCK       },
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_horizontal,    HTML_SPTYPE_HORI        },
    { OOO_STRING_SVTOOLS_HTML_SPTYPE_vertical,  HTML_SPTYPE_VERT        },
    { nullptr,                    0                       }
};

HTMLReader::HTMLReader()
{
    bTmplBrowseMode = true;
}

OUString HTMLReader::GetTemplateName() const
{
    const OUString sTemplateWithoutExt("internal/html");
    SvtPathOptions aPathOpt;

    // first search for OpenDocument Writer/Web template
    // OpenDocument Writer/Web template (extension .oth)
    OUString sTemplate( sTemplateWithoutExt + ".oth" );
    if (aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE ))
        return sTemplate;

        // no OpenDocument Writer/Web template found.
        // search for OpenOffice.org Writer/Web template
    sTemplate = sTemplateWithoutExt + ".stw";
    if (aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE ))
        return sTemplate;

    OSL_ENSURE( false, "The default HTML template cannot be found in the defined template directories!");

    return OUString();
}

bool HTMLReader::SetStrmStgPtr()
{
    OSL_ENSURE( pMedium, "Wo ist das Medium??" );

    if( pMedium->IsRemote() || !pMedium->IsStorage() )
    {
        pStrm = pMedium->GetInStream();
        return true;
    }
    return false;

}

    // Aufruf fuer die allg. Reader-Schnittstelle
sal_uLong HTMLReader::Read( SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPam, const OUString & rName )
{
    if( !pStrm )
    {
        OSL_ENSURE( pStrm, "HTML-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    if( !bInsertMode )
    {
        Reader::ResetFrameFormats( rDoc );

        // Die HTML-Seitenvorlage setzen, wenn des kein HTML-Dokument ist,
        // sonst ist sie schon gesetzt.
        if( !rDoc.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) )
        {
            rDoc.getIDocumentContentOperations().InsertPoolItem( rPam, SwFormatPageDesc(
                rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false )) );
        }
    }

    // damit keiner das Doc klaut!
    rDoc.acquire();
    sal_uLong nRet = 0;
    tools::SvRef<SvParser> xParser = new SwHTMLParser( &rDoc, rPam, *pStrm,
                                            rName, rBaseURL, !bInsertMode, pMedium,
                                            IsReadUTF8(),
                                            bIgnoreHTMLComments );

    SvParserState eState = xParser->CallParser();

    if( SVPAR_PENDING == eState )
        pStrm->ResetError();
    else if( SVPAR_ACCEPTED != eState )
    {
        const OUString sErr(OUString::number((sal_Int32)xParser->GetLineNr())
            + "," + OUString::number((sal_Int32)xParser->GetLinePos()));

        // den Stream als Fehlernummer Transporter benutzen
        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }

    return nRet;
}

SwHTMLParser::SwHTMLParser( SwDoc* pD, SwPaM& rCursor, SvStream& rIn,
                            const OUString& rPath,
                            const OUString& rBaseURL,
                            bool bReadNewDoc,
                            SfxMedium* pMed, bool bReadUTF8,
                            bool bNoHTMLComments )
    : SfxHTMLParser( rIn, bReadNewDoc, pMed ),
    SwClient( nullptr ),
    m_aPathToFile( rPath ),
    m_sBaseURL( rBaseURL ),
    m_pAppletImpl( nullptr ),
    m_pCSS1Parser( nullptr ),
    m_pNumRuleInfo( new SwHTMLNumRuleInfo ),
    m_pPendStack( nullptr ),
    m_pDoc( pD ),
    m_pActionViewShell( nullptr ),
    m_pSttNdIdx( nullptr ),
    m_pTable(nullptr),
    m_pFormImpl( nullptr ),
    m_pMarquee( nullptr ),
    m_pField( nullptr ),
    m_pImageMap( nullptr ),
    m_pImageMaps(nullptr),
    m_pFootEndNoteImpl( nullptr ),
    m_nScriptStartLineNr( 0 ),
    m_nBaseFontStMin( 0 ),
    m_nFontStMin( 0 ),
    m_nDefListDeep( 0 ),
    m_nFontStHeadStart( 0 ),
    m_nSBModuleCnt( 0 ),
    m_nMissingImgMaps( 0 ),
    m_nParaCnt( 5 ),
    // #i83625#
    m_nContextStMin( 0 ),
    m_nContextStAttrMin( 0 ),
    m_nSelectEntryCnt( 0 ),
    m_nOpenParaToken( 0 ),
    m_eJumpTo( JUMPTO_NONE ),
#ifdef DBG_UTIL
    m_nContinue( 0 ),
#endif
    m_eParaAdjust( SVX_ADJUST_END ),
    m_bDocInitalized( false ),
    m_bSetModEnabled( false ),
    m_bInFloatingFrame( false ),
    m_bInField( false ),
    m_bCallNextToken( false ),
    m_bIgnoreRawData( false ),
    m_bLBEntrySelected ( false ),
    m_bTAIgnoreNewPara ( false ),
    m_bFixMarqueeWidth ( false ),
    m_bNoParSpace( false ),
    m_bInNoEmbed( false ),
    m_bInTitle( false ),
    m_bUpdateDocStat( false ),
    m_bFixSelectWidth( false ),
    m_bFixSelectHeight( false ),
    m_bTextArea( false ),
    m_bSelect( false ),
    m_bInFootEndNoteAnchor( false ),
    m_bInFootEndNoteSymbol( false ),
    m_bIgnoreHTMLComments( bNoHTMLComments ),
    m_bRemoveHidden( false ),
    m_pTempViewFrame(nullptr)
{
    m_nEventId = nullptr;
    m_bUpperSpace = m_bViewCreated = m_bChkJumpMark = false;

    m_eScriptLang = HTML_SL_UNKNOWN;

    rCursor.DeleteMark();
    m_pPam = &rCursor; // re-use existing cursor: avoids spurious ~SwIndexReg assert
    memset( &m_aAttrTab, 0, sizeof( HTMLAttrTable ));

    // Die Font-Groessen 1-7 aus der INI-Datei lesen
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    m_aFontHeights[0] = rHtmlOptions.GetFontSize( 0 ) * 20;
    m_aFontHeights[1] = rHtmlOptions.GetFontSize( 1 ) * 20;
    m_aFontHeights[2] = rHtmlOptions.GetFontSize( 2 ) * 20;
    m_aFontHeights[3] = rHtmlOptions.GetFontSize( 3 ) * 20;
    m_aFontHeights[4] = rHtmlOptions.GetFontSize( 4 ) * 20;
    m_aFontHeights[5] = rHtmlOptions.GetFontSize( 5 ) * 20;
    m_aFontHeights[6] = rHtmlOptions.GetFontSize( 6 ) * 20;

    m_bKeepUnknown = rHtmlOptions.IsImportUnknown();

    if(bReadNewDoc)
    {
        //CJK has different defaults, so a different object should be used for this
        //RES_CHARTR_CJK_FONTSIZE is a valid value
        SvxFontHeightItem aFontHeight(m_aFontHeights[2], 100, RES_CHRATR_FONTSIZE);
        m_pDoc->SetDefault( aFontHeight );
        SvxFontHeightItem aFontHeightCJK(m_aFontHeights[2], 100, RES_CHRATR_CJK_FONTSIZE);
        m_pDoc->SetDefault( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL(m_aFontHeights[2], 100, RES_CHRATR_CTL_FONTSIZE);
        m_pDoc->SetDefault( aFontHeightCTL );

        // #i18732# - adjust default of option 'FollowTextFlow'
        // TODO: not sure what the appropriate default for HTML should be?
        m_pDoc->SetDefault( SwFormatFollowTextFlow(true) );
    }

    // Waehrend des Imports in den HTML-Modus schalten, damit die
    // richrigen Vorlagen angelegt werden
    m_bOldIsHTMLMode = m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE);
    m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, true);

    m_pCSS1Parser = new SwCSS1Parser( m_pDoc, m_aFontHeights, m_sBaseURL, IsNewDoc() );
    m_pCSS1Parser->SetIgnoreFontFamily( rHtmlOptions.IsIgnoreFontFamily() );

    if( bReadUTF8 )
    {
        SetSrcEncoding( RTL_TEXTENCODING_UTF8 );
    }
    else
    {
        SwDocShell *pDocSh = m_pDoc->GetDocShell();
        SvKeyValueIterator *pHeaderAttrs =
            pDocSh->GetHeaderAttributes();
        if( pHeaderAttrs )
            SetEncodingByHTTPHeader( pHeaderAttrs );
    }
    m_pCSS1Parser->SetDfltEncoding( osl_getThreadTextEncoding() );

    // Timer nur bei ganz normalen Dokumenten aufsetzen!
    SwDocShell* pDocSh = m_pDoc->GetDocShell();
    if( pDocSh )
    {
        m_bViewCreated = true;            // nicht, synchron laden

        // es ist ein Sprungziel vorgegeben.

        if( pMed )
        {
            m_sJmpMark = pMed->GetURLObject().GetMark();
            if( !m_sJmpMark.isEmpty() )
            {
                m_eJumpTo = JUMPTO_MARK;
                sal_Int32 nLastPos = m_sJmpMark.lastIndexOf( cMarkSeparator );
                sal_Int32 nPos =  nLastPos != -1 ? nLastPos : 0;

                OUString sCmp;
                if (nPos)
                {
                    sCmp = m_sJmpMark.copy(nPos + 1).replaceAll(" ", "");
                }

                if( !sCmp.isEmpty() )
                {
                    sCmp = sCmp.toAsciiLowerCase();
                    if( sCmp == "region" )
                        m_eJumpTo = JUMPTO_REGION;
                    else if( sCmp == "table" )
                        m_eJumpTo = JUMPTO_TABLE;
                    else if( sCmp == "graphic" )
                        m_eJumpTo = JUMPTO_GRAPHIC;
                    else if( sCmp == "outline" ||
                            sCmp == "text" ||
                            sCmp == "frame" )
                        m_eJumpTo = JUMPTO_NONE;  // das ist nichts gueltiges!
                    else
                        // ansonsten ist das ein normaler (Book)Mark
                        nPos = -1;
                }
                else
                    nPos = -1;

                if( nPos != -1 )
                    m_sJmpMark = m_sJmpMark.copy( 0, nPos );
                if( m_sJmpMark.isEmpty() )
                    m_eJumpTo = JUMPTO_NONE;
            }
        }
    }
}

SwHTMLParser::~SwHTMLParser()
{
#ifdef DBG_UTIL
    OSL_ENSURE( !m_nContinue, "DTOR im Continue!" );
#endif
    bool bAsync = m_pDoc->IsInLoadAsynchron();
    m_pDoc->SetInLoadAsynchron( false );
    m_pDoc->getIDocumentSettingAccess().set(DocumentSettingId::HTML_MODE, m_bOldIsHTMLMode);

    if( m_pDoc->GetDocShell() && m_nEventId )
        Application::RemoveUserEvent( m_nEventId );

    // das DocumentDetected kann ggfs. die DocShells loeschen, darum nochmals
    // abfragen
    if( m_pDoc->GetDocShell() )
    {
        // Gelinkte Bereiche updaten
        sal_uInt16 nLinkMode = m_pDoc->getIDocumentSettingAccess().getLinkUpdateMode( true );
        if( nLinkMode != NEVER && bAsync &&
            SfxObjectCreateMode::INTERNAL!=m_pDoc->GetDocShell()->GetCreateMode() )
            m_pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL );

        if ( m_pDoc->GetDocShell()->IsLoading() )
        {
            // #i59688#
            m_pDoc->GetDocShell()->LoadingFinished();
        }
    }

    delete m_pSttNdIdx;

    if( !m_aSetAttrTab.empty() )
    {
        OSL_ENSURE( m_aSetAttrTab.empty(),"Es stehen noch Attribute auf dem Stack" );
        for ( HTMLAttrs::const_iterator it = m_aSetAttrTab.begin();
              it != m_aSetAttrTab.end(); ++it )
            delete *it;
        m_aSetAttrTab.clear();
    }

    delete m_pCSS1Parser;
    delete m_pNumRuleInfo;
    DeleteFormImpl();
    DeleteFootEndNoteImpl();

    OSL_ENSURE( !m_pTable, "Es existiert noch eine offene Tabelle" );
    delete m_pImageMaps;

    OSL_ENSURE( !m_pPendStack,
            "SwHTMLParser::~SwHTMLParser: Hier sollte es keinen Pending-Stack mehr geben" );
    while( m_pPendStack )
    {
        SwPendingStack* pTmp = m_pPendStack;
        m_pPendStack = m_pPendStack->pNext;
        delete pTmp->pData;
        delete pTmp;
    }

    if( !m_pDoc->release() )
    {
        // keiner will mehr das Doc haben, also weg damit
        delete m_pDoc;
        m_pDoc = nullptr;
    }

    if ( m_pTempViewFrame )
    {
        m_pTempViewFrame->DoClose();

        // the temporary view frame is hidden, so the hidden flag might need to be removed
        if ( m_bRemoveHidden && m_pDoc && m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->GetMedium() )
            m_pDoc->GetDocShell()->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
    }
}

IMPL_LINK_NOARG_TYPED( SwHTMLParser, AsyncCallback, void*, void )
{
    m_nEventId=nullptr;

    // #i47907# - If the document has already been destructed,
    // the parser should be aware of this:
    if( ( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == m_pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
    }

    GetAsynchCallLink().Call(nullptr);
}

SvParserState SwHTMLParser::CallParser()
{
    // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
    m_pSttNdIdx = new SwNodeIndex( m_pDoc->GetNodes() );
    if( !IsNewDoc() )       // in ein Dokument einfuegen ?
    {
        const SwPosition* pPos = m_pPam->GetPoint();

        m_pDoc->getIDocumentContentOperations().SplitNode( *pPos, false );

        *m_pSttNdIdx = pPos->nNode.GetIndex()-1;
        m_pDoc->getIDocumentContentOperations().SplitNode( *pPos, false );

        SwPaM aInsertionRangePam( *pPos );

        m_pPam->Move( fnMoveBackward );

        // split any redline over the insertion point
        aInsertionRangePam.SetMark();
        *aInsertionRangePam.GetPoint() = *m_pPam->GetPoint();
        aInsertionRangePam.Move( fnMoveBackward );
        m_pDoc->getIDocumentRedlineAccess().SplitRedline( aInsertionRangePam );

        m_pDoc->SetTextFormatColl( *m_pPam,
                m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD ));
    }

    if( GetMedium() )
    {
        if( !m_bViewCreated )
        {
            m_nEventId = Application::PostUserEvent( LINK( this, SwHTMLParser, AsyncCallback ) );
        }
        else
        {
            m_bViewCreated = true;
            m_nEventId = nullptr;
        }
    }

    // Laufbalken anzeigen
    else if( !GetMedium() || !GetMedium()->IsRemote() )
    {
        rInput.Seek(STREAM_SEEK_TO_END);
        rInput.ResetError();
        ::StartProgress( STR_STATSTR_W4WREAD, 0, rInput.Tell(),
                         m_pDoc->GetDocShell() );
        rInput.Seek(STREAM_SEEK_TO_BEGIN);
        rInput.ResetError();
    }

    m_pDoc->GetPageDesc( 0 ).Add( this );

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
    if( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
        eState = SVPAR_ERROR;

    // Die SwViewShell vom Dokument holen, merken und als aktuelle setzen.
    SwViewShell *pInitVSh = CallStartAction();

    if( SVPAR_ERROR != eState && GetMedium() && !m_bViewCreated )
    {
        // Beim ersten Aufruf erstmal returnen, Doc anzeigen
        // und auf Timer Callback warten.
        // An dieser Stelle wurde im CallParser gerade mal ein Zeichen
        // gelesen und ein SaveState(0) gerufen.
        eState = SVPAR_PENDING;
        m_bViewCreated = true;
        m_pDoc->SetInLoadAsynchron( true );

#ifdef DBG_UTIL
        m_nContinue--;
#endif

        return;
    }

    m_bSetModEnabled = false;
    if( m_pDoc->GetDocShell() &&
        (m_bSetModEnabled = m_pDoc->GetDocShell()->IsEnableSetModified()) )
    {
        m_pDoc->GetDocShell()->EnableSetModified( false );
    }

    // waehrend des einlesens kein OLE-Modified rufen
    Link<bool,void> aOLELink( m_pDoc->GetOle2Link() );
    m_pDoc->SetOle2Link( Link<bool,void>() );

    bool bModified = m_pDoc->getIDocumentState().IsModified();
    bool const bWasUndo = m_pDoc->GetIDocumentUndoRedo().DoesUndo();
    m_pDoc->GetIDocumentUndoRedo().DoUndo(false);

    // Wenn der Import abgebrochen wird, kein Continue mehr rufen.
    // Falls ein Pending-Stack existiert aber durch einen Aufruf
    // von NextToken dafuer sorgen, dass der Pending-Stack noch
    // beendet wird.
    if( SVPAR_ERROR == eState )
    {
        OSL_ENSURE( !m_pPendStack || m_pPendStack->nToken,
                "SwHTMLParser::Continue: Pending-Stack ohne Token" );
        if( m_pPendStack && m_pPendStack->nToken )
            NextToken( m_pPendStack->nToken );
        OSL_ENSURE( !m_pPendStack,
                "SwHTMLParser::Continue: Es gibt wieder einen Pend-Stack" );
    }
    else
    {
        HTMLParser::Continue( m_pPendStack ? m_pPendStack->nToken : nToken );
    }

    // Laufbalken wieder abschalten
    EndProgress( m_pDoc->GetDocShell() );

    bool bLFStripped = false;
    if( SVPAR_PENDING != GetStatus() )
    {
        // noch die letzten Attribute setzen
        {
            if( !m_aScriptSource.isEmpty() )
            {
                SwScriptFieldType *pType =
                    static_cast<SwScriptFieldType*>(m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_SCRIPTFLD ));

                SwScriptField aField( pType, m_aScriptType, m_aScriptSource,
                                    false );
                InsertAttr( SwFormatField( aField ), false );
            }

            if( m_pAppletImpl )
            {
                if( m_pAppletImpl->GetApplet().is() )
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

            OSL_ENSURE( !m_nContextStMin, "Es gibt geschuetzte Kontexte" );
            m_nContextStMin = 0;
            while( m_aContexts.size() )
            {
                HTMLAttrContext *pCntxt = PopContext();
                if( pCntxt )
                {
                    EndContext( pCntxt );
                    delete pCntxt;
                }
            }

            if( !m_aParaAttrs.empty() )
                m_aParaAttrs.clear();

            SetAttr( false );

            // Noch die erst verzoegert gesetzten Styles setzen
            m_pCSS1Parser->SetDelayedStyles();
        }

        // den Start wieder korrigieren
        if( !IsNewDoc() && m_pSttNdIdx->GetIndex() )
        {
            SwTextNode* pTextNode = m_pSttNdIdx->GetNode().GetTextNode();
            SwNodeIndex aNxtIdx( *m_pSttNdIdx );
            if( pTextNode && pTextNode->CanJoinNext( &aNxtIdx ))
            {
                const sal_Int32 nStt = pTextNode->GetText().getLength();
                // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
                if( m_pPam->GetPoint()->nNode == aNxtIdx )
                {
                    m_pPam->GetPoint()->nNode = *m_pSttNdIdx;
                    m_pPam->GetPoint()->nContent.Assign( pTextNode, nStt );
                }

#if OSL_DEBUG_LEVEL > 0
// !!! sollte nicht moeglich sein, oder ??
OSL_ENSURE( m_pSttNdIdx->GetIndex()+1 != m_pPam->GetBound().nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
OSL_ENSURE( m_pSttNdIdx->GetIndex()+1 != m_pPam->GetBound( false ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( m_pSttNdIdx->GetIndex()+1 == m_pPam->GetBound().nNode.GetIndex() )
{
    const sal_Int32 nCntPos = m_pPam->GetBound().nContent.GetIndex();
    m_pPam->GetBound().nContent.Assign( pTextNode,
                    pTextNode->GetText().getLength() + nCntPos );
}
if( m_pSttNdIdx->GetIndex()+1 == m_pPam->GetBound( false ).nNode.GetIndex() )
{
    const sal_Int32 nCntPos = m_pPam->GetBound( false ).nContent.GetIndex();
    m_pPam->GetBound( false ).nContent.Assign( pTextNode,
                    pTextNode->GetText().getLength() + nCntPos );
}
#endif
                // Zeichen Attribute beibehalten!
                SwTextNode* pDelNd = aNxtIdx.GetNode().GetTextNode();
                if (pTextNode->GetText().getLength())
                    pDelNd->FormatToTextAttr( pTextNode );
                else
                    pTextNode->ChgFormatColl( pDelNd->GetTextColl() );
                pTextNode->JoinNext();
            }
        }
    }

    if( SVPAR_ACCEPTED == eState )
    {
        if( m_nMissingImgMaps )
        {
            // es fehlen noch ein paar Image-Map zuordungen.
            // vielleicht sind die Image-Maps ja jetzt da?
            ConnectImageMaps();
        }

        // jetzt noch den letzten ueberfluessigen Absatz loeschen
        SwPosition* pPos = m_pPam->GetPoint();
        if( !pPos->nContent.GetIndex() && !bLFStripped )
        {
            SwTextNode* pAktNd;
            sal_uLong nNodeIdx = pPos->nNode.GetIndex();

            bool bHasFlysOrMarks =
                HasCurrentParaFlys() || HasCurrentParaBookmarks( true );

            if( IsNewDoc() )
            {
                const SwNode *pPrev = m_pDoc->GetNodes()[nNodeIdx -1];
                if( !m_pPam->GetPoint()->nContent.GetIndex() &&
                    ( pPrev->IsContentNode() ||
                      (pPrev->IsEndNode() &&
                      pPrev->StartOfSectionNode()->IsSectionNode()) ) )
                {
                    SwContentNode* pCNd = m_pPam->GetContentNode();
                    if( pCNd && pCNd->StartOfSectionIndex()+2 <
                        pCNd->EndOfSectionIndex() && !bHasFlysOrMarks )
                    {
                        SwViewShell *pVSh = CheckActionViewShell();
                        SwCursorShell *pCursorSh = pVSh && dynamic_cast< const SwCursorShell *>( pVSh ) !=  nullptr
                                        ? static_cast < SwCursorShell * >( pVSh )
                                        : nullptr;
                        if( pCursorSh &&
                            pCursorSh->GetCursor()->GetPoint()
                                   ->nNode.GetIndex() == nNodeIdx )
                        {
                            pCursorSh->MovePara(fnParaPrev, fnParaEnd );
                            pCursorSh->SetMark();
                            pCursorSh->ClearMark();
                        }
                        m_pPam->GetBound().nContent.Assign( nullptr, 0 );
                        m_pPam->GetBound(false).nContent.Assign( nullptr, 0 );
                        m_pDoc->GetNodes().Delete( m_pPam->GetPoint()->nNode );
                    }
                }
            }
            else if( nullptr != ( pAktNd = m_pDoc->GetNodes()[ nNodeIdx ]->GetTextNode()) && !bHasFlysOrMarks )
            {
                if( pAktNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTextNode* pNextNd = pPos->nNode.GetNode().GetTextNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    m_pPam->SetMark(); m_pPam->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if (pAktNd->GetText().isEmpty())
                {
                    pPos->nContent.Assign( nullptr, 0 );
                    m_pPam->SetMark(); m_pPam->DeleteMark();
                    m_pDoc->GetNodes().Delete( pPos->nNode );
                    m_pPam->Move( fnMoveBackward );
                }
            }
        }

        // nun noch das SplitNode vom Anfang aufheben
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     // dann gabs am Ende kein <P>,
                m_pPam->Move( fnMoveForward, fnGoNode );  // als zum naechsten Node
            SwTextNode* pTextNode = pPos->nNode.GetNode().GetTextNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTextNode && pTextNode->CanJoinPrev( &aPrvIdx ) &&
                *m_pSttNdIdx <= aPrvIdx )
            {
                // eigentlich muss hier ein JoinNext erfolgen, aber alle Cursor
                // usw. sind im pTextNode angemeldet, so dass der bestehen
                // bleiben MUSS.

                // Absatz in Zeichen-Attribute umwandeln, aus dem Prev die
                // Absatzattribute und die Vorlage uebernehmen!
                SwTextNode* pPrev = aPrvIdx.GetNode().GetTextNode();
                pTextNode->ChgFormatColl( pPrev->GetTextColl() );
                pTextNode->FormatToTextAttr( pPrev );
                pTextNode->ResetAllAttr();

                if( pPrev->HasSwAttrSet() )
                    pTextNode->SetAttr( *pPrev->GetpSwAttrSet() );

                if( &m_pPam->GetBound().nNode.GetNode() == pPrev )
                    m_pPam->GetBound().nContent.Assign( pTextNode, 0 );
                if( &m_pPam->GetBound(false).nNode.GetNode() == pPrev )
                    m_pPam->GetBound(false).nContent.Assign( pTextNode, 0 );

                pTextNode->JoinPrev();
            }
        }

        // adjust AutoLoad in DocumentProperties
        if( IsNewDoc() )
        {
            SwDocShell *pDocShell(m_pDoc->GetDocShell());
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
                    xDocProps->setAutoloadURL(m_aPathToFile);
                }
            }
        }

        if( m_bUpdateDocStat )
        {
            m_pDoc->getIDocumentStatistics().UpdateDocStat( false, true );
        }
    }

    if( SVPAR_PENDING != GetStatus() )
    {
        delete m_pSttNdIdx;
        m_pSttNdIdx = nullptr;
    }

    // sollte der Parser der Letzte sein, der das Doc haelt, dann braucht
    // man hier auch nichts mehr tun, Doc wird gleich zerstoert!
    if( 1 < m_pDoc->getReferenceCount() )
    {
        if( bWasUndo )
        {
            m_pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
            m_pDoc->GetIDocumentUndoRedo().DoUndo(true);
        }
        else if( !pInitVSh )
        {
            // Wenn zu Beginn des Continue keine Shell vorhanden war,
            // kann trotzdem mitlerweile eine angelegt worden sein.
            // In dieses Fall stimmt das bWasUndo-Flag nicht und
            // wir muessen das Undo noch anschalten.
            SwViewShell *pTmpVSh = CheckActionViewShell();
            if( pTmpVSh )
            {
                m_pDoc->GetIDocumentUndoRedo().DoUndo(true);
            }
        }

        m_pDoc->SetOle2Link( aOLELink );
        if( !bModified )
            m_pDoc->getIDocumentState().ResetModified();
        if( m_bSetModEnabled && m_pDoc->GetDocShell() )
        {
            m_pDoc->GetDocShell()->EnableSetModified();
            m_bSetModEnabled = false; // this is unnecessary here
        }
    }

    // Wenn die Dokuemnt-SwViewShell noch existiert und eine Action
    // offen ist (muss bei Abbruch nicht sein), die Action beenden,
    // uns von der Shell abmelden und schliesslich die alte Shell
    // wieder rekonstruieren.
    CallEndAction( true );

#ifdef DBG_UTIL
    m_nContinue--;
#endif
}

void SwHTMLParser::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_OBJECTDYING:
        if (pOld && static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject == GetRegisteredIn())
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
    OSL_ENSURE( !m_bDocInitalized, "DocumentDetected mehrfach aufgerufen" );
    m_bDocInitalized = true;
    if( IsNewDoc() )
    {
        if( IsInHeader() )
            FinishHeader( true );

        CallEndAction( true );

        m_pDoc->GetIDocumentUndoRedo().DoUndo(false);
        // Durch das DocumentDetected wurde im allgemeinen eine
        // SwViewShell angelegt. Es kann aber auch sein, dass sie
        // erst spaeter angelegt wird, naemlich dann, wenn die UI
        // gecaptured ist.
        CallStartAction();
    }
}

// is called for every token that is recognised in CallParser
void SwHTMLParser::NextToken( int nToken )
{
    if( ( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == m_pDoc->getReferenceCount() )
    {
        // Was the import cancelled by SFX? If a pending stack
        // exists, clean it.
        eState = SVPAR_ERROR;
        OSL_ENSURE( !m_pPendStack || m_pPendStack->nToken,
                "SwHTMLParser::NextToken: Pending-Stack without token" );
        if( 1 == m_pDoc->getReferenceCount() || !m_pPendStack )
            return ;
    }

#if OSL_DEBUG_LEVEL > 0
    if( m_pPendStack )
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
            OSL_ENSURE( !m_pPendStack, "Unbekanntes Token fuer Pending-Stack" );
            break;
        }
    }
#endif

    // The following special cases have to be treated before the
    // filter detection, because Netscape doesn't reference the content
    // of the title for filter detection either.
    if( !m_pPendStack )
    {
        if( m_bInTitle )
        {
            switch( nToken )
            {
            case HTML_TITLE_OFF:
                if( IsNewDoc() && !m_sTitle.isEmpty() )
                {
                    if( m_pDoc->GetDocShell() ) {
                        uno::Reference<document::XDocumentPropertiesSupplier>
                            xDPS(m_pDoc->GetDocShell()->GetModel(),
                            uno::UNO_QUERY_THROW);
                        uno::Reference<document::XDocumentProperties> xDocProps(
                            xDPS->getDocumentProperties());
                        OSL_ENSURE(xDocProps.is(), "no DocumentProperties");
                        if (xDocProps.is()) {
                            xDocProps->setTitle(m_sTitle);
                        }

                        m_pDoc->GetDocShell()->SetTitle( m_sTitle );
                    }
                }
                m_bInTitle = false;
                m_sTitle.clear();
                break;

            case HTML_NONBREAKSPACE:
                m_sTitle += " ";
                break;

            case HTML_SOFTHYPH:
                m_sTitle += "-";
                break;

            case HTML_TEXTTOKEN:
                m_sTitle += aToken;
                break;

            default:
                m_sTitle += "<";
                if( (HTML_TOKEN_ONOFF & nToken) && (1 & nToken) )
                    m_sTitle += "/";
                m_sTitle += sSaveToken;
                if( !aToken.isEmpty() )
                {
                    m_sTitle += " ";
                    m_sTitle += aToken;
                }
                m_sTitle += ">";
                break;
            }

            return;
        }
    }

    // Find out what type of document it is if we don't know already.
    // For Controls this has to be finished before the control is inserted
    // because for inserting a View is needed.
    if( !m_bDocInitalized )
        DocumentDetected();

    bool bGetIDOption = false, bInsertUnknown = false;
    bool bUpperSpaceSave = m_bUpperSpace;
    m_bUpperSpace = false;

    // The following special cases may or have to be treated after the
    // filter detection
    if( !m_pPendStack )
    {
        if( m_bInFloatingFrame )
        {
            // <SCRIPT> is ignored here (from us), because it is ignored in
            // Applets as well
            if( HTML_IFRAME_OFF == nToken )
            {
                m_bCallNextToken = false;
                EndFloatingFrame();
            }

            return;
        }
        else if( m_bInNoEmbed )
        {
            switch( nToken )
            {
            case HTML_NOEMBED_OFF:
                m_aContents = convertLineEnd(m_aContents, GetSystemLineEnd());
                InsertComment( m_aContents, OOO_STRING_SVTOOLS_HTML_noembed );
                m_aContents.clear();
                m_bCallNextToken = false;
                m_bInNoEmbed = false;
                break;

            case HTML_RAWDATA:
                InsertCommentText( OOO_STRING_SVTOOLS_HTML_noembed );
                break;

            default:
                OSL_ENSURE( false, "SwHTMLParser::NextToken: invalid tag" );
                break;
            }

            return;
        }
        else if( m_pAppletImpl )
        {
            // in an applet only <PARAM> tags and the </APPLET> tag
            // are of interest for us (for the moment)
            // <SCRIPT> is ignored here (from Netscape)!

            switch( nToken )
            {
            case HTML_APPLET_OFF:
                m_bCallNextToken = false;
                EndApplet();
                break;
            case HTML_OBJECT_OFF:
                m_bCallNextToken = false;
                EndObject();
                break;

            case HTML_PARAM:
                InsertParam();
                break;
            }

            return;
        }
        else if( m_bTextArea )
        {
            // in a TextArea everything up to </TEXTAREA> is inserted as text.
            // <SCRIPT> is ignored here (from Netscape)!

            switch( nToken )
            {
            case HTML_TEXTAREA_OFF:
                m_bCallNextToken = false;
                EndTextArea();
                break;

            default:
                InsertTextAreaText( static_cast< sal_uInt16 >(nToken) );
                break;
            }

            return;
        }
        else if( m_bSelect )
        {
            // HAS to be treated after bNoScript!
            switch( nToken )
            {
            case HTML_SELECT_OFF:
                m_bCallNextToken = false;
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
        else if( m_pMarquee )
        {
            // in a TextArea everything up to </TEXTAREA> is inserted as text.
            // The <SCRIPT> tags are ignored from MS-IE, we ignore the whole
            // script.
            switch( nToken )
            {
            case HTML_MARQUEE_OFF:
                m_bCallNextToken = false;
                EndMarquee();
                break;

            case HTML_TEXTTOKEN:
                InsertMarqueeText();
                break;
            }

            return;
        }
        else if( m_bInField )
        {
            switch( nToken )
            {
            case HTML_SDFIELD_OFF:
                m_bCallNextToken = false;
                EndField();
                break;

            case HTML_TEXTTOKEN:
                InsertFieldText();
                break;
            }

            return;
        }
        else if( m_bInFootEndNoteAnchor || m_bInFootEndNoteSymbol )
        {
            switch( nToken )
            {
            case HTML_ANCHOR_OFF:
                EndAnchor();
                m_bCallNextToken = false;
                break;

            case HTML_TEXTTOKEN:
                InsertFootEndNoteText();
                break;
            }
            return;
        }
        else if( !m_aUnknownToken.isEmpty() )
        {
            // Paste content of unknown tags.
            // (but surely if we are not in the header section) fdo#36080 fdo#34666
            if (!aToken.isEmpty() && !IsInHeader() )
            {
                if( !m_bDocInitalized )
                    DocumentDetected();
                m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, aToken );

                // if there are temporary paragraph attributes and the
                // paragraph isn't empty then the paragraph attributes
                // are final.
                if( !m_aParaAttrs.empty() )
                    m_aParaAttrs.clear();

                SetAttr();
            }

            // Unknown token in the header are only closed by a matching
            // end-token, </HEAD> or <BODY>. Text inside is ignored.
            switch( nToken )
            {
            case HTML_UNKNOWNCONTROL_OFF:
                if( m_aUnknownToken != sSaveToken )
                    return;
                SAL_FALLTHROUGH;
            case HTML_FRAMESET_ON:
            case HTML_HEAD_OFF:
            case HTML_BODY_ON:
            case HTML_IMAGE:        // Don't know why Netscape acts this way.
                m_aUnknownToken.clear();
                break;
            case HTML_TEXTTOKEN:
                return;
            default:
                m_aUnknownToken.clear();
                break;
            }
        }
    }

    switch( nToken )
    {
    case HTML_BODY_ON:
        if( !m_aStyleSource.isEmpty() )
        {
            m_pCSS1Parser->ParseStyleSheet( m_aStyleSource );
            m_aStyleSource.clear();
        }
        if( IsNewDoc() )
        {
            InsertBodyOptions();
            // If there is a template for the first or the right page,
            // it is set here.
            const SwPageDesc *pPageDesc = nullptr;
            if( m_pCSS1Parser->IsSetFirstPageDesc() )
                pPageDesc = m_pCSS1Parser->GetFirstPageDesc();
            else if( m_pCSS1Parser->IsSetRightPageDesc() )
                pPageDesc = m_pCSS1Parser->GetRightPageDesc();

            if( pPageDesc )
            {
                m_pDoc->getIDocumentContentOperations().InsertPoolItem( *m_pPam, SwFormatPageDesc( pPageDesc ) );
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
                    m_sBaseURL = rOption.GetString();
                    break;
                case HTML_O_TARGET:
                    if( IsNewDoc() )
                    {
                        SwDocShell *pDocShell(m_pDoc->GetDocShell());
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
            SvKeyValueIterator *pHTTPHeader = nullptr;
            if( IsNewDoc() )
            {
                SwDocShell *pDocSh = m_pDoc->GetDocShell();
                if( pDocSh )
                    pHTTPHeader = pDocSh->GetHeaderAttributes();
            }
            SwDocShell *pDocShell(m_pDoc->GetDocShell());
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
        m_bInTitle = true;
        break;

    case HTML_SCRIPT_ON:
        NewScript();
        break;

    case HTML_SCRIPT_OFF:
        EndScript();
        break;

    case HTML_NOSCRIPT_ON:
    case HTML_NOSCRIPT_OFF:
        bInsertUnknown = true;
        break;

    case HTML_STYLE_ON:
        NewStyle();
        break;

    case HTML_STYLE_OFF:
        EndStyle();
        break;

    case HTML_RAWDATA:
        if( !m_bIgnoreRawData )
        {
            if( IsReadScript() )
            {
                AddScriptSource();
            }
            else if( IsReadStyle() )
            {
                if( !m_aStyleSource.isEmpty() )
                    m_aStyleSource += "\n";
                m_aStyleSource += aToken;
            }
        }
        break;

    case HTML_OBJECT_ON:
#if HAVE_FEATURE_JAVA
        NewObject();
        m_bCallNextToken = m_pAppletImpl!=nullptr && m_pTable!=nullptr;
#endif
        break;

    case HTML_APPLET_ON:
#if HAVE_FEATURE_JAVA
        InsertApplet();
        m_bCallNextToken = m_pAppletImpl!=nullptr && m_pTable!=nullptr;
#endif
        break;

    case HTML_IFRAME_ON:
        InsertFloatingFrame();
        m_bCallNextToken = m_bInFloatingFrame && m_pTable!=nullptr;
        break;

    case HTML_LINEBREAK:
        if( !IsReadPRE() )
        {
            InsertLineBreak();
            break;
        }
        else
            bGetIDOption = true;
            // <BR>s in <PRE> resemble true LFs, hence no break
        SAL_FALLTHROUGH;

    case HTML_NEWPARA:
        // CR in PRE/LISTING/XMP
        {
            if( HTML_NEWPARA==nToken ||
                m_pPam->GetPoint()->nContent.GetIndex() )
            {
                AppendTextNode(); // there is no LF at this place
                                 // therefore it will cause no problems
                SetTextCollAttrs();
            }
            // progress bar
            if( !GetMedium() || !GetMedium()->IsRemote() )
                ::SetProgressState( rInput.Tell(), m_pDoc->GetDocShell() );
        }
        break;

    case HTML_NONBREAKSPACE:
        m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, OUString(CHAR_HARDBLANK) );
        break;

    case HTML_SOFTHYPH:
        m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, OUString(CHAR_SOFTHYPHEN) );
        break;

    case HTML_LINEFEEDCHAR:
        if( m_pPam->GetPoint()->nContent.GetIndex() )
            AppendTextNode();
        if( !m_pTable && !m_pDoc->IsInHeaderFooter( m_pPam->GetPoint()->nNode ) )
        {
            NewAttr( &m_aAttrTab.pBreak, SvxFormatBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK) );
            EndAttr( m_aAttrTab.pBreak, nullptr, false );
        }
        break;

    case HTML_TEXTTOKEN:
        // insert string without spanning attributes at the end.
        if( !aToken.isEmpty() && ' '==aToken[0] && !IsReadPRE() )
        {
            sal_Int32 nPos = m_pPam->GetPoint()->nContent.GetIndex();
            if( nPos )
            {
                const OUString& rText =
                    m_pPam->GetPoint()->nNode.GetNode().GetTextNode()->GetText();
                sal_Unicode cLast = rText[--nPos];
                if( ' ' == cLast || '\x0a' == cLast)
                    aToken = aToken.copy(1);
            }
            else
                aToken = aToken.copy(1);

            if( aToken.isEmpty() )
            {
                m_bUpperSpace = bUpperSpaceSave;
                break;
            }
        }

        if( !aToken.isEmpty() )
        {
            if( !m_bDocInitalized )
                DocumentDetected();
            m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, aToken );

            // if there are temporary paragraph attributes and the
            // paragraph isn't empty then the paragraph attributes
            // are final.
            if( !m_aParaAttrs.empty() )
                m_aParaAttrs.clear();

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
        if( 1 == m_pDoc->getReferenceCount() )
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
        m_bInNoEmbed = true;
        m_bCallNextToken = m_pTable!=nullptr;
        ReadRawData( OOO_STRING_SVTOOLS_HTML_noembed );
        break;

    case HTML_DEFLIST_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewDefList();
        break;
    case HTML_DEFLIST_OFF:
        if( m_nOpenParaToken )
            EndPara();
        EndDefListItem( 0, 1==m_nDefListDeep );
        EndDefList();
        break;

    case HTML_DD_ON:
    case HTML_DT_ON:
        if( m_nOpenParaToken )
            EndPara();
        EndDefListItem();// close <DD>/<DT> and set no template
        NewDefListItem( nToken );
        break;

    case HTML_DD_OFF:
    case HTML_DT_OFF:
        // c.f. HTML_LI_OFF
        // Actually we should close a DD/DT now.
        // But neither Netscape nor Microsoft do this and so don't we.
        EndDefListItem( nToken );
        break;

    // divisions
    case HTML_DIVISION_ON:
    case HTML_CENTER_ON:
        if( m_nOpenParaToken )
        {
            if( IsReadPRE() )
                m_nOpenParaToken = 0;
            else
                EndPara();
        }
        NewDivision( nToken );
        break;

    case HTML_DIVISION_OFF:
    case HTML_CENTER_OFF:
        if( m_nOpenParaToken )
        {
            if( IsReadPRE() )
                m_nOpenParaToken = 0;
            else
                EndPara();
        }
        EndDivision( nToken );
        break;

    case HTML_MULTICOL_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewMultiCol();
        break;

    case HTML_MULTICOL_OFF:
        if( m_nOpenParaToken )
            EndPara();
        EndTag( HTML_MULTICOL_ON );
        break;

    case HTML_MARQUEE_ON:
        NewMarquee();
        m_bCallNextToken = m_pMarquee!=nullptr && m_pTable!=nullptr;
        break;

    case HTML_FORM_ON:
        NewForm();
        break;
    case HTML_FORM_OFF:
        EndForm();
        break;

    // templates
    case HTML_PARABREAK_ON:
        if( m_nOpenParaToken )
            EndPara( true );
        NewPara();
        break;

    case HTML_PARABREAK_OFF:
        EndPara( true );
        break;

    case HTML_ADDRESS_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewTextFormatColl( HTML_ADDRESS_ON, RES_POOLCOLL_SENDADRESS );
        break;

    case HTML_ADDRESS_OFF:
        if( m_nOpenParaToken )
            EndPara();
        EndTextFormatColl( HTML_ADDRESS_OFF );
        break;

    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE30_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewTextFormatColl( HTML_BLOCKQUOTE_ON, RES_POOLCOLL_HTML_BLOCKQUOTE );
        break;

    case HTML_BLOCKQUOTE_OFF:
    case HTML_BLOCKQUOTE30_OFF:
        if( m_nOpenParaToken )
            EndPara();
        EndTextFormatColl( HTML_BLOCKQUOTE_ON );
        break;

    case HTML_PREFORMTXT_ON:
    case HTML_LISTING_ON:
    case HTML_XMP_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewTextFormatColl( nToken, RES_POOLCOLL_HTML_PRE );
        break;

    case HTML_PREFORMTXT_OFF:
        m_bNoParSpace = true; // the last PRE-paragraph gets a spacing
        EndTextFormatColl( HTML_PREFORMTXT_OFF );
        break;

    case HTML_LISTING_OFF:
    case HTML_XMP_OFF:
        EndTextFormatColl( nToken );
        break;

    case HTML_HEAD1_ON:
    case HTML_HEAD2_ON:
    case HTML_HEAD3_ON:
    case HTML_HEAD4_ON:
    case HTML_HEAD5_ON:
    case HTML_HEAD6_ON:
        if( m_nOpenParaToken )
        {
            if( IsReadPRE() )
                m_nOpenParaToken = 0;
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
        if( m_pPendStack )
            BuildTable( SVX_ADJUST_END );
        else
        {
            if( m_nOpenParaToken )
                EndPara();
            OSL_ENSURE( !m_pTable, "table in table not allowed here" );
            if( !m_pTable && (IsNewDoc() || !m_pPam->GetNode().FindTableNode()) &&
                (m_pPam->GetPoint()->nNode.GetIndex() >
                            m_pDoc->GetNodes().GetEndOfExtras().GetIndex() ||
                !m_pPam->GetNode().FindFootnoteStartNode() ) )
            {
                if ( m_nParaCnt < 5 )
                    Show();     // show what we have up to here

                SvxAdjust eAdjust = m_aAttrTab.pAdjust
                    ? static_cast<const SvxAdjustItem&>(m_aAttrTab.pAdjust->GetItem()).
                                             GetAdjust()
                    : SVX_ADJUST_END;
                BuildTable( eAdjust );
            }
            else
                bInsertUnknown = m_bKeepUnknown;
        }
        break;

    // lists
    case HTML_DIRLIST_ON:
    case HTML_MENULIST_ON:
    case HTML_ORDERLIST_ON:
    case HTML_UNORDERLIST_ON:
        if( m_nOpenParaToken )
            EndPara();
        NewNumBulList( nToken );
        break;

    case HTML_DIRLIST_OFF:
    case HTML_MENULIST_OFF:
    case HTML_ORDERLIST_OFF:
    case HTML_UNORDERLIST_OFF:
        if( m_nOpenParaToken )
            EndPara();
        EndNumBulListItem( 0, true, GetNumInfo().GetDepth()==1 );
        EndNumBulList( nToken );
        break;

    case HTML_LI_ON:
    case HTML_LISTHEADER_ON:
        if( m_nOpenParaToken &&
            (m_pPam->GetPoint()->nContent.GetIndex()
            || HTML_PARABREAK_ON==m_nOpenParaToken) )
        {
            // only finish paragraph for <P><LI>, not for <DD><LI>
            EndPara();
        }

        EndNumBulListItem( 0, false );// close <LI>/<LH> and don't set a template
        NewNumBulListItem( nToken );
        break;

    case HTML_LI_OFF:
    case HTML_LISTHEADER_OFF:
        EndNumBulListItem( nToken, false );
        break;

    // Attribute :
    case HTML_ITALIC_ON:
        {
            SvxPostureItem aPosture( ITALIC_NORMAL, RES_CHRATR_POSTURE );
            SvxPostureItem aPostureCJK( ITALIC_NORMAL, RES_CHRATR_CJK_POSTURE );
            SvxPostureItem aPostureCTL( ITALIC_NORMAL, RES_CHRATR_CTL_POSTURE );
            NewStdAttr( HTML_ITALIC_ON,
                           &m_aAttrTab.pItalic, aPosture,
                           &m_aAttrTab.pItalicCJK, &aPostureCJK,
                           &m_aAttrTab.pItalicCTL, &aPostureCTL );
        }
        break;

    case HTML_BOLD_ON:
        {
            SvxWeightItem aWeight( WEIGHT_BOLD, RES_CHRATR_WEIGHT );
            SvxWeightItem aWeightCJK( WEIGHT_BOLD, RES_CHRATR_CJK_WEIGHT );
            SvxWeightItem aWeightCTL( WEIGHT_BOLD, RES_CHRATR_CTL_WEIGHT );
            NewStdAttr( HTML_BOLD_ON,
                        &m_aAttrTab.pBold, aWeight,
                        &m_aAttrTab.pBoldCJK, &aWeightCJK,
                        &m_aAttrTab.pBoldCTL, &aWeightCTL );
        }
        break;

    case HTML_STRIKE_ON:
    case HTML_STRIKETHROUGH_ON:
        {
            NewStdAttr( HTML_STRIKE_ON, &m_aAttrTab.pStrike,
                        SvxCrossedOutItem(STRIKEOUT_SINGLE, RES_CHRATR_CROSSEDOUT) );
        }
        break;

    case HTML_UNDERLINE_ON:
        {
            NewStdAttr( HTML_UNDERLINE_ON, &m_aAttrTab.pUnderline,
                        SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE) );
        }
        break;

    case HTML_SUPERSCRIPT_ON:
        {
            NewStdAttr( HTML_SUPERSCRIPT_ON, &m_aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUPER,HTML_ESC_PROP, RES_CHRATR_ESCAPEMENT) );
        }
        break;

    case HTML_SUBSCRIPT_ON:
        {
            NewStdAttr( HTML_SUBSCRIPT_ON, &m_aAttrTab.pEscapement,
                        SvxEscapementItem(HTML_ESC_SUB,HTML_ESC_PROP, RES_CHRATR_ESCAPEMENT) );
        }
        break;

    case HTML_BLINK_ON:
        {
            NewStdAttr( HTML_BLINK_ON, &m_aAttrTab.pBlink,
                        SvxBlinkItem( true, RES_CHRATR_BLINK ) );
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
        NewCharFormat( nToken );
        break;

    case HTML_SDFIELD_ON:
        NewField();
        m_bCallNextToken = m_bInField && m_pTable!=nullptr;
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
        if( !m_aStyleSource.isEmpty() )
        {
            m_pCSS1Parser->ParseStyleSheet( m_aStyleSource );
            m_aStyleSource.clear();
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
                    const OUString& rDir = rOption.GetString();
                    SfxItemSet aItemSet( m_pDoc->GetAttrPool(),
                                         m_pCSS1Parser->GetWhichMap() );
                    SvxCSS1PropertyInfo aPropInfo;
                    OUString aDummy;
                    ParseStyleOptions( aDummy, aDummy, aDummy, aItemSet,
                                       aPropInfo, nullptr, &rDir );

                    m_pCSS1Parser->SetPageDescAttrs( nullptr, &aItemSet );
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
        m_bCallNextToken = m_bTextArea && m_pTable!=nullptr;
        break;

    case HTML_SELECT_ON:
        NewSelect();
        m_bCallNextToken = m_bSelect && m_pTable!=nullptr;
        break;

    case HTML_ANCHOR_ON:
        NewAnchor();
        break;

    case HTML_ANCHOR_OFF:
        EndAnchor();
        break;

    case HTML_COMMENT:
        if( ( aToken.getLength() > 5 ) && ( ! m_bIgnoreHTMLComments ) )
        {
            // insert as Post-It
            // If there are no space characters right behind
            // the <!-- and on front of the -->, leave the comment untouched.
            if( ' ' == aToken[ 3 ] &&
                ' ' == aToken[ aToken.getLength()-3 ] )
            {
                OUString aComment( aToken.copy( 3, aToken.getLength()-5 ) );
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
        m_pImageMap = new ImageMap;
        if( ParseMapOptions( m_pImageMap) )
        {
            if (!m_pImageMaps)
                m_pImageMaps = new ImageMaps;
            m_pImageMaps->push_back(std::unique_ptr<ImageMap>(m_pImageMap));
        }
        else
        {
            delete m_pImageMap;
            m_pImageMap = nullptr;
        }
        break;

    case HTML_MAP_OFF:
        // there is no ImageMap anymore (don't delete IMap, because it's
        // already contained in the array!)
        m_pImageMap = nullptr;
        break;

    case HTML_AREA:
        if( m_pImageMap )
            ParseAreaOptions( m_pImageMap, m_sBaseURL, SFX_EVENT_MOUSEOVER_OBJECT,
                                         SFX_EVENT_MOUSEOUT_OBJECT );
        break;

    case HTML_FRAMESET_ON:
        bInsertUnknown = m_bKeepUnknown;
        break;

    case HTML_NOFRAMES_ON:
        if( IsInHeader() )
            FinishHeader( true );
        bInsertUnknown = m_bKeepUnknown;
        break;

    case HTML_UNKNOWNCONTROL_ON:
        // Ignore content of unknown token in the header, if the token
        // does not start with a '!'.
        // (but judging from the code, also if does not start with a '%')
        // (and also if we're not somewhere we consider PRE)
        if( IsInHeader() && !IsReadPRE() && m_aUnknownToken.isEmpty() &&
            !sSaveToken.isEmpty() && '!' != sSaveToken[0] &&
            '%' != sSaveToken[0] )
            m_aUnknownToken = sSaveToken;
        SAL_FALLTHROUGH;

    default:
        bInsertUnknown = m_bKeepUnknown;
        break;
    }

    if( bGetIDOption )
        InsertIDOption();

    if( bInsertUnknown )
    {
        OUString aComment("HTML: <");
        if( (HTML_TOKEN_ONOFF & nToken) != 0 && (1 & nToken) != 0 )
            aComment += "/";
        aComment += sSaveToken;
        if( !aToken.isEmpty() )
        {
            UnescapeToken();
            (aComment += " ") += aToken;
        }
        aComment += ">";
        InsertComment( aComment );
    }

    // if there are temporary paragraph attributes and the
    // paragraph isn't empty then the paragraph attributes are final.
    if( !m_aParaAttrs.empty() && m_pPam->GetPoint()->nContent.GetIndex() )
        m_aParaAttrs.clear();
}

static void lcl_swhtml_getItemInfo( const HTMLAttr& rAttr,
                                 bool& rScriptDependent, bool& rFont,
                                 sal_uInt16& rScriptType )
{
    switch( rAttr.GetItem().Which() )
    {
    case RES_CHRATR_FONT:
        rFont = true;
        SAL_FALLTHROUGH;
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        rScriptType = i18n::ScriptType::LATIN;
        rScriptDependent = true;
        break;
    case RES_CHRATR_CJK_FONT:
        rFont = true;
        SAL_FALLTHROUGH;
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        rScriptType = i18n::ScriptType::ASIAN;
        rScriptDependent = true;
        break;
    case RES_CHRATR_CTL_FONT:
        rFont = true;
        SAL_FALLTHROUGH;
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        rScriptType = i18n::ScriptType::COMPLEX;
        rScriptDependent = true;
        break;
    default:
        rScriptDependent = false;
        rFont = false;
        break;
    }
}

bool SwHTMLParser::AppendTextNode( SwHTMLAppendMode eMode, bool bUpdateNum )
{
    // Ein harter Zeilen-Umbruch am Ende muss immer entfernt werden.
    // Einen zweiten ersetzen wir durch einen Absatz-Abstand.
    sal_Int32 nLFStripped = StripTrailingLF();
    if( (AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode) && nLFStripped > 1 )
        eMode = AM_SPACE;

    // die harten Attribute an diesem Absatz werden nie mehr ungueltig
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    if( AM_SPACE==eMode || AM_NOSPACE==eMode )
    {
        SwTextNode *pTextNode =
            m_pPam->GetPoint()->nNode.GetNode().GetTextNode();

        const SvxULSpaceItem& rULSpace =
            static_cast<const SvxULSpaceItem&>(pTextNode->SwContentNode::GetAttr( RES_UL_SPACE ));

        bool bChange = AM_NOSPACE==eMode ? rULSpace.GetLower() > 0
                                         : rULSpace.GetLower() == 0;

        if( bChange )
        {
            const SvxULSpaceItem& rCollULSpace =
                pTextNode->GetAnyFormatColl().GetULSpace();

            bool bMayReset = AM_NOSPACE==eMode ? rCollULSpace.GetLower() == 0
                                               : rCollULSpace.GetLower() > 0;

            if( bMayReset &&
                rCollULSpace.GetUpper() == rULSpace.GetUpper() )
            {
                pTextNode->ResetAttr( RES_UL_SPACE );
            }
            else
            {
                pTextNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(),
                         AM_NOSPACE==eMode ? 0 : HTML_PARSPACE, RES_UL_SPACE ) );
            }
        }
    }
    m_bNoParSpace = AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode;

    SwPosition aOldPos( *m_pPam->GetPoint() );

    bool bRet = m_pDoc->getIDocumentContentOperations().AppendTextNode( *m_pPam->GetPoint() );

    // Zeichen-Attribute aufspalten und ggf keine setzen, die ueber den
    // ganzen Absatz gesetzt sind
    const SwNodeIndex& rEndIdx = aOldPos.nNode;
    const sal_Int32 nEndCnt = aOldPos.nContent.GetIndex();
    const SwPosition& rPos = *m_pPam->GetPoint();

    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(&m_aAttrTab);
    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        HTMLAttr *pAttr = *pHTMLAttributes;
        if( pAttr && pAttr->GetItem().Which() < RES_PARATR_BEGIN )
        {
            bool bWholePara = false;

            while( pAttr )
            {
                HTMLAttr *pNext = pAttr->GetNext();
                if( pAttr->GetSttParaIdx() < rEndIdx.GetIndex() ||
                    (!bWholePara &&
                     pAttr->GetSttPara() == rEndIdx &&
                     pAttr->GetSttCnt() != nEndCnt) )
                {
                    bWholePara =
                        pAttr->GetSttPara() == rEndIdx &&
                        pAttr->GetSttCnt() == 0;

                    sal_Int32 nStt = pAttr->nSttContent;
                    bool bScript = false, bFont = false;
                    sal_uInt16 nScriptItem;
                    bool bInsert = true;
                       lcl_swhtml_getItemInfo( *pAttr, bScript, bFont,
                                            nScriptItem );
                        // den besehrigen Teil setzen
                    if( bInsert && bScript )
                    {
                        const SwTextNode *pTextNd =
                            pAttr->GetSttPara().GetNode().GetTextNode();
                        OSL_ENSURE( pTextNd, "No text node" );
                        if( pTextNd )
                        {
                            const OUString& rText = pTextNd->GetText();
                            sal_uInt16 nScriptText =
                                g_pBreakIt->GetBreakIter()->getScriptType(
                                            rText, pAttr->GetSttCnt() );
                            sal_Int32 nScriptEnd = g_pBreakIt->GetBreakIter()
                                    ->endOfScript( rText, nStt, nScriptText );
                            while (nScriptEnd < nEndCnt && nScriptEnd != -1)
                            {
                                if( nScriptItem == nScriptText )
                                {
                                    HTMLAttr *pSetAttr =
                                        pAttr->Clone( rEndIdx, nScriptEnd );
                                    pSetAttr->nSttContent = nStt;
                                    pSetAttr->ClearPrev();
                                    if( !pNext || bWholePara )
                                    {
                                        if (pSetAttr->bInsAtStart)
                                            m_aSetAttrTab.push_front( pSetAttr );
                                        else
                                            m_aSetAttrTab.push_back( pSetAttr );
                                    }
                                    else
                                        pNext->InsertPrev( pSetAttr );
                                }
                                nStt = nScriptEnd;
                                nScriptText = g_pBreakIt->GetBreakIter()->getScriptType(
                                                rText, nStt );
                                nScriptEnd = g_pBreakIt->GetBreakIter()
                                    ->endOfScript( rText, nStt, nScriptText );
                            }
                            bInsert = nScriptItem == nScriptText;
                        }
                    }
                    if( bInsert )
                    {
                        HTMLAttr *pSetAttr =
                            pAttr->Clone( rEndIdx, nEndCnt );
                        pSetAttr->nSttContent = nStt;

                        // Wenn das Attribut den gesamten Absatz umspannt, werden
                        // alle auesseren Attribute nicht mehr beachtet. Deshalb
                        // darf es auch nicht in die Prev-Liste eines ausseren
                        // Attributs eingetragen werden, denn dieses wird ja
                        // erstmal nicht gesetzt. Das fuehrt zu verschiebenungen,
                        // wenn Felder ins Rennen kommen
                        if( !pNext || bWholePara )
                        {
                            if (pSetAttr->bInsAtStart)
                                m_aSetAttrTab.push_front( pSetAttr );
                            else
                                m_aSetAttrTab.push_back( pSetAttr );
                        }
                        else
                            pNext->InsertPrev( pSetAttr );
                    }
                    else
                    {
                        HTMLAttr *pPrev = pAttr->GetPrev();
                        if( pPrev )
                        {
                            // Die Previous-Attribute muessen trotzdem gesetzt werden.
                            if( !pNext || bWholePara )
                            {
                                if (pPrev->bInsAtStart)
                                    m_aSetAttrTab.push_front( pPrev );
                                else
                                    m_aSetAttrTab.push_back( pPrev );
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
            SetNodeNum( nLvl );
        }
        else
            m_pPam->GetNode().GetTextNode()->ResetAttr( RES_PARATR_NUMRULE );
    }

    // We must set the attribute of the paragraph before now (because of JavaScript)
    SetAttr();

    // Now it is time to get rid of all script dependent hints that are
    // equal to the settings in the style
    SwTextNode *pTextNd = rEndIdx.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "There is the txt node" );
    size_t nCntAttr = (pTextNd  && pTextNd->GetpSwpHints())
                            ? pTextNd->GetSwpHints().Count() : 0;
    if( nCntAttr )
    {
        // These are the end position of all script dependent hints.
        // If we find a hint that starts before the current end position,
        // we have to set it. If we find a hint that start behind or at
        // that position, we have to take the hint value into account.
        // If it is equal to the style, or in fact the paragarph value
        // for that hint, the hint is removed. Otherwise its end position
        // is remembered.
        sal_Int32 aEndPos[15] =
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        SwpHints& rHints = pTextNd->GetSwpHints();
        for( size_t i=0; i < nCntAttr; i++ )
        {
            SwTextAttr *pHt = rHints.Get( i );
            sal_uInt16 nWhich = pHt->Which();
            sal_Int16 nIdx = 0;
            bool bFont = false;
            switch( nWhich )
            {
                case RES_CHRATR_FONT:
                    nIdx = 0;
                    bFont = true;
                    break;
                case RES_CHRATR_FONTSIZE:
                    nIdx = 1;
                    break;
                case RES_CHRATR_LANGUAGE:
                    nIdx = 2;
                    break;
                case RES_CHRATR_POSTURE:
                    nIdx = 3;
                    break;
                case RES_CHRATR_WEIGHT:
                    nIdx = 4;
                    break;
                case RES_CHRATR_CJK_FONT:
                    nIdx = 5;
                    bFont = true;
                    break;
                case RES_CHRATR_CJK_FONTSIZE:
                    nIdx = 6;
                    break;
                case RES_CHRATR_CJK_LANGUAGE:
                    nIdx = 7;
                    break;
                case RES_CHRATR_CJK_POSTURE:
                    nIdx = 8;
                    break;
                case RES_CHRATR_CJK_WEIGHT:
                    nIdx = 9;
                    break;
                case RES_CHRATR_CTL_FONT:
                    nIdx = 10;
                    bFont = true;
                    break;
                case RES_CHRATR_CTL_FONTSIZE:
                    nIdx = 11;
                    break;
                case RES_CHRATR_CTL_LANGUAGE:
                    nIdx = 12;
                    break;
                case RES_CHRATR_CTL_POSTURE:
                    nIdx = 13;
                    break;
                case RES_CHRATR_CTL_WEIGHT:
                    nIdx = 14;
                    break;
                default:
                    // Skip to next attribute
                    continue;
            }
            const sal_Int32 nStt = pHt->GetStart();
            if( nStt >= aEndPos[nIdx] )
            {
                const SfxPoolItem& rItem =
                    static_cast<const SwContentNode *>(pTextNd)->GetAttr( nWhich );
                if( bFont ? swhtml_css1atr_equalFontItems(rItem,pHt->GetAttr())
                            : rItem == pHt->GetAttr() )
                {
                    // The hint is the same as set in the paragraph and
                    // therefore, it can be deleted
                    // CAUTION!!! This WILL delete the hint and it MAY
                    // also delete the SwpHints!!! To avoid any trouble
                    // we leave the loop immediately if this is the last
                    // hint.
                    pTextNd->DeleteAttribute( pHt );
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

    if( !m_pTable && !--m_nParaCnt )
        Show();

    return bRet;
}

void SwHTMLParser::AddParSpace()
{
    //If it already has ParSpace, return
    if( !m_bNoParSpace )
        return;

    m_bNoParSpace = false;

    sal_uLong nNdIdx = m_pPam->GetPoint()->nNode.GetIndex() - 1;

    SwTextNode *pTextNode = m_pDoc->GetNodes()[nNdIdx]->GetTextNode();
    if( !pTextNode )
        return;

    SvxULSpaceItem rULSpace =
        static_cast<const SvxULSpaceItem&>(pTextNode->SwContentNode::GetAttr( RES_UL_SPACE ));
    if( !rULSpace.GetLower() )
    {
        const SvxULSpaceItem& rCollULSpace =
            pTextNode->GetAnyFormatColl().GetULSpace();
        if( rCollULSpace.GetLower() &&
            rCollULSpace.GetUpper() == rULSpace.GetUpper() )
        {
            pTextNode->ResetAttr( RES_UL_SPACE );
        }
        else
        {
            //What I do here, is that I examine the attributes, and if
            //I find out, that it's CJK/CTL, then I set the paragraph space
            //to the value set in HTML_CJK_PARSPACE/HTML_CTL_PARSPACE.

            bool bIsCJK = false;
            bool bIsCTL = false;

            const size_t nCntAttr = (pTextNode  && pTextNode->GetpSwpHints())
                            ? pTextNode->GetSwpHints().Count() : 0;

            for(size_t i = 0; i < nCntAttr; ++i)
            {
                SwTextAttr *const pHt = pTextNode->GetSwpHints().Get(i);
                sal_uInt16 const nWhich = pHt->Which();
                if (RES_CHRATR_CJK_FONT == nWhich ||
                    RES_CHRATR_CJK_FONTSIZE == nWhich ||
                    RES_CHRATR_CJK_LANGUAGE == nWhich ||
                    RES_CHRATR_CJK_POSTURE == nWhich ||
                    RES_CHRATR_CJK_WEIGHT == nWhich)
                {
                    bIsCJK = true;
                    break;
                }
                if (RES_CHRATR_CTL_FONT == nWhich ||
                    RES_CHRATR_CTL_FONTSIZE == nWhich ||
                    RES_CHRATR_CTL_LANGUAGE == nWhich ||
                    RES_CHRATR_CTL_POSTURE == nWhich ||
                    RES_CHRATR_CTL_WEIGHT == nWhich)
                {
                    bIsCTL = true;
                    break;
                }
            }

            if( bIsCTL )
            {
                pTextNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(), HTML_CTL_PARSPACE, RES_UL_SPACE )  );
            }
            else if( bIsCJK )
            {
                pTextNode->SetAttr(
                    SvxULSpaceItem( rULSpace.GetUpper(), HTML_CJK_PARSPACE, RES_UL_SPACE )  );
            } else {
                pTextNode->SetAttr(
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
    SwViewShell *pOldVSh = CallEndAction();

    Application::Reschedule();

    if( ( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == m_pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
    }

    // Die SwViewShell nochmal holen, denn sie koennte im Reschedule
    // zerstoert wirden sein.
    SwViewShell *pVSh = CallStartAction( pOldVSh );

    // ist der aktuelle Node nicht mehr sichtbar, dann benutzen wir
    // eine groessere Schrittweite
    if( pVSh )
    {
        m_nParaCnt = (m_pPam->GetPoint()->nNode.GetNode().IsInVisibleArea(pVSh))
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
        ::SetProgressState( rInput.Tell(), m_pDoc->GetDocShell() );
        CheckActionViewShell();
    }
    else
    {
        Application::Reschedule();

        if( ( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
            || 1 == m_pDoc->getReferenceCount() )
            // wurde der Import vom SFX abgebrochen?
            eState = SVPAR_ERROR;

        SwViewShell *pVSh = CheckActionViewShell();
        if( pVSh && pVSh->HasInvalidRect() )
        {
            CallEndAction( false, false );
            CallStartAction( pVSh, false );
        }
    }
}

SwViewShell *SwHTMLParser::CallStartAction( SwViewShell *pVSh, bool bChkPtr )
{
    OSL_ENSURE( !m_pActionViewShell, "CallStartAction: SwViewShell schon gesetzt" );

    if( !pVSh || bChkPtr )
    {
#if OSL_DEBUG_LEVEL > 0
        SwViewShell *pOldVSh = pVSh;
#endif
        pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !pVSh || !pOldVSh || pOldVSh == pVSh, "CallStartAction: Wer hat die SwViewShell ausgetauscht?" );
        if( pOldVSh && !pVSh )
            pVSh = nullptr;
#endif
    }
    m_pActionViewShell = pVSh;

    if( m_pActionViewShell )
    {
        if( dynamic_cast< const SwEditShell *>( m_pActionViewShell ) !=  nullptr )
            static_cast<SwEditShell*>(m_pActionViewShell)->StartAction();
        else
            m_pActionViewShell->StartAction();
    }

    return m_pActionViewShell;
}

SwViewShell *SwHTMLParser::CallEndAction( bool bChkAction, bool bChkPtr )
{
    if( bChkPtr )
    {
        SwViewShell *pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
        OSL_ENSURE( !pVSh || m_pActionViewShell == pVSh,
                "CallEndAction: Wer hat die SwViewShell ausgetauscht?" );
#if OSL_DEBUG_LEVEL > 0
        if( m_pActionViewShell && !pVSh )
            pVSh = nullptr;
#endif
        if( pVSh != m_pActionViewShell )
            m_pActionViewShell = nullptr;
    }

    if( !m_pActionViewShell || (bChkAction && !m_pActionViewShell->ActionPend()) )
        return m_pActionViewShell;

    if( dynamic_cast< const SwEditShell *>( m_pActionViewShell ) !=  nullptr )
    {
        //Schon gescrollt?, dann dafuer sorgen, dass die View sich nicht bewegt!
        const bool bOldLock = m_pActionViewShell->IsViewLocked();
        m_pActionViewShell->LockView( true );
        const bool bOldEndActionByVirDev = m_pActionViewShell->IsEndActionByVirDev();
        m_pActionViewShell->SetEndActionByVirDev( true );
        static_cast<SwEditShell*>(m_pActionViewShell)->EndAction();
        m_pActionViewShell->SetEndActionByVirDev( bOldEndActionByVirDev );
        m_pActionViewShell->LockView( bOldLock );

        // bChkJumpMark ist nur gesetzt, wenn das Object auch gefunden wurde
        if( m_bChkJumpMark )
        {
            const Point aVisSttPos( DOCUMENTBORDER, DOCUMENTBORDER );
            if( GetMedium() && aVisSttPos == m_pActionViewShell->VisArea().Pos() )
                ::JumpToSwMark( m_pActionViewShell,
                                GetMedium()->GetURLObject().GetMark() );
            m_bChkJumpMark = false;
        }
    }
    else
        m_pActionViewShell->EndAction();

    // sollte der Parser der Letzte sein, der das Doc haelt, dann kann
    // man hier abbrechen und einen Fehler setzen.
    if( 1 == m_pDoc->getReferenceCount() )
    {
        eState = SVPAR_ERROR;
    }

    SwViewShell *pVSh = m_pActionViewShell;
    m_pActionViewShell = nullptr;

    return pVSh;
}

SwViewShell *SwHTMLParser::CheckActionViewShell()
{
    SwViewShell *pVSh = m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    OSL_ENSURE( !pVSh || m_pActionViewShell == pVSh,
            "CheckActionViewShell: Wer hat die SwViewShell ausgetauscht?" );
#if OSL_DEBUG_LEVEL > 0
    if( m_pActionViewShell && !pVSh )
        pVSh = nullptr;
#endif
    if( pVSh != m_pActionViewShell )
        m_pActionViewShell = nullptr;

    return m_pActionViewShell;
}

void SwHTMLParser::SetAttr_( bool bChkEnd, bool bBeforeTable,
                             HTMLAttrs *pPostIts )
{
    SwPaM* pAttrPam = new SwPaM( *m_pPam->GetPoint() );
    const SwNodeIndex& rEndIdx = m_pPam->GetPoint()->nNode;
    const sal_Int32 nEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
    HTMLAttr* pAttr;
    SwContentNode* pCNd;

    HTMLAttrs aFields;

    for( auto n = m_aSetAttrTab.size(); n; )
    {
        pAttr = m_aSetAttrTab[ --n ];
        sal_uInt16 nWhich = pAttr->pItem->Which();

        sal_uLong nEndParaIdx = pAttr->GetEndParaIdx();
        bool bSetAttr;
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
            // Attribute im Content-Bereich duerfen nicht gesetzt
            // werden, wenn wir in einem Sonderbereich stehen, aber
            // umgekekehrt schon.
            sal_uLong nEndOfIcons = m_pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bSetAttr = nEndParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nEndParaIdx <= nEndOfIcons;
        }

        if( bSetAttr )
        {
            // Das Attribute darf nicht in der liste der vorlaeufigen
            // Absatz-Attribute stehen, weil es sonst geloescht wurde.
            while( !m_aParaAttrs.empty() )
            {
                OSL_ENSURE( pAttr != m_aParaAttrs.back(),
                        "SetAttr: Attribut duerfte noch nicht gesetzt werden" );
                m_aParaAttrs.pop_back();
            }

            // dann also setzen
            m_aSetAttrTab.erase( m_aSetAttrTab.begin() + n );

            while( pAttr )
            {
                HTMLAttr *pPrev = pAttr->GetPrev();
                if( !pAttr->bValid )
                {
                    // ungueltige Attribute koennen gloescht werden
                    delete pAttr;
                    pAttr = pPrev;
                    continue;
                }

                pCNd = pAttr->nSttPara.GetNode().GetContentNode();
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
                        continue;
                    }
                    pCNd = m_pDoc->GetNodes().GoNext( &(pAttr->nSttPara) );
                    if( pCNd )
                        pAttr->nSttContent = 0;
                    else
                    {
                        OSL_ENSURE( false, "SetAttr: GoNext() failed!" );
                        delete pAttr;
                        pAttr = pPrev;
                        continue;
                    }
                }
                pAttrPam->GetPoint()->nNode = pAttr->nSttPara;

                // durch das Loeschen von BRs kann der Start-Index
                // auch mal hinter das Ende des Textes zeigen
                if( pAttr->nSttContent > pCNd->Len() )
                    pAttr->nSttContent = pCNd->Len();
                pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nSttContent );

                pAttrPam->SetMark();
                if ( (pAttr->GetSttPara() != pAttr->GetEndPara()) &&
                         !isTXTATR_NOEND(nWhich) )
                {
                    pCNd = pAttr->nEndPara.GetNode().GetContentNode();
                    if( !pCNd )
                    {
                        pCNd = SwNodes::GoPrevious( &(pAttr->nEndPara) );
                        if( pCNd )
                            pAttr->nEndContent = pCNd->Len();
                        else
                        {
                            OSL_ENSURE( false, "SetAttr: GoPrevious() failed!" );
                            pAttrPam->DeleteMark();
                            delete pAttr;
                            pAttr = pPrev;
                            continue;
                        }
                    }

                    pAttrPam->GetPoint()->nNode = pAttr->nEndPara;
                }
                else if( pAttr->IsLikePara() )
                {
                    pAttr->nEndContent = pCNd->Len();
                }

                // durch das Loeschen von BRs kann der End-Index
                // auch mal hinter das Ende des Textes zeigen
                if( pAttr->nEndContent > pCNd->Len() )
                    pAttr->nEndContent = pCNd->Len();

                pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nEndContent );
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
                        const OUString sName( static_cast<SfxStringItem*>(pAttr->pItem)->GetValue() );
                        IDocumentMarkAccess* const pMarkAccess = m_pDoc->getIDocumentMarkAccess();
                        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark( sName );
                        if( ppBkmk != pMarkAccess->getAllMarksEnd() &&
                            ppBkmk->get()->GetMarkStart() == *pAttrPam->GetPoint() )
                            break; // do not generate duplicates on this position
                        pAttrPam->DeleteMark();
                        const ::sw::mark::IMark* const pNewMark = pMarkAccess->makeMark(
                            *pAttrPam,
                            sName,
                            IDocumentMarkAccess::MarkType::BOOKMARK );

                        // jump to bookmark
                        if( JUMPTO_MARK == m_eJumpTo && pNewMark->GetName() == m_sJmpMark )
                        {
                            m_bChkJumpMark = true;
                            m_eJumpTo = JUMPTO_NONE;
                        }
                    }
                    break;
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                case RES_TXTATR_INPUTFIELD:
                    {
                        sal_uInt16 nFieldWhich =
                            pPostIts
                            ? static_cast<const SwFormatField *>(pAttr->pItem)->GetField()->GetTyp()->Which()
                            : 0;
                        if( pPostIts && (RES_POSTITFLD == nFieldWhich ||
                                         RES_SCRIPTFLD == nFieldWhich) )
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
                    OSL_ENSURE( false,
                            "LRSpace ueber mehrere Absaetze gesetzt!" );
                    SAL_FALLTHROUGH; // (shouldn't reach this point anyway)

                // tdf#94088 expand RES_BACKGROUND to the new fill attribute
                // definitions in the range [XATTR_FILL_FIRST .. XATTR_FILL_LAST].
                // This is the right place in the future if the adapted fill attributes
                // may be handled more directly in HTML import to handle them.
                case RES_BACKGROUND:
                {
                    const SvxBrushItem& rBrush = static_cast< SvxBrushItem& >(*pAttr->pItem);
                    SfxItemSet aNewSet(m_pDoc->GetAttrPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                    setSvxBrushItemAsFillAttributesToTargetSet(rBrush, aNewSet);
                    m_pDoc->getIDocumentContentOperations().InsertItemSet(*pAttrPam, aNewSet, SetAttrMode::DONTREPLACE);
                    break;
                }
                default:

                    // ggfs. ein Bookmark anspringen
                    if( RES_TXTATR_INETFMT == nWhich &&
                        JUMPTO_MARK == m_eJumpTo &&
                        m_sJmpMark == static_cast<SwFormatINetFormat*>(pAttr->pItem)->GetName() )
                    {
                        m_bChkJumpMark = true;
                        m_eJumpTo = JUMPTO_NONE;
                    }

                    m_pDoc->getIDocumentContentOperations().InsertPoolItem( *pAttrPam, *pAttr->pItem, SetAttrMode::DONTREPLACE );
                }
                pAttrPam->DeleteMark();

                delete pAttr;
                pAttr = pPrev;
            }
        }
    }

    for( auto n = m_aMoveFlyFrames.size(); n; )
    {
        SwFrameFormat *pFrameFormat = m_aMoveFlyFrames[ --n ];

        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        OSL_ENSURE( FLY_AT_PARA == rAnchor.GetAnchorId(),
                "Nur Auto-Rahmen brauchen eine Spezialbehandlung" );
        const SwPosition *pFlyPos = rAnchor.GetContentAnchor();
        sal_uLong nFlyParaIdx = pFlyPos->nNode.GetIndex();
        bool bMoveFly;
        if( bChkEnd )
        {
            bMoveFly = nFlyParaIdx < rEndIdx.GetIndex() ||
                       ( nFlyParaIdx == rEndIdx.GetIndex() &&
                         m_aMoveFlyCnts[n] < nEndCnt );
        }
        else
        {
            sal_uLong nEndOfIcons = m_pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bMoveFly = nFlyParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nFlyParaIdx <= nEndOfIcons;
        }
        if( bMoveFly )
        {
            pFrameFormat->DelFrames();
            *pAttrPam->GetPoint() = *pFlyPos;
            pAttrPam->GetPoint()->nContent.Assign( pAttrPam->GetContentNode(),
                                                   m_aMoveFlyCnts[n] );
            SwFormatAnchor aAnchor( rAnchor );
            aAnchor.SetType( FLY_AT_CHAR );
            aAnchor.SetAnchor( pAttrPam->GetPoint() );
            pFrameFormat->SetFormatAttr( aAnchor );

            const SwFormatHoriOrient& rHoriOri = pFrameFormat->GetHoriOrient();
            if( text::HoriOrientation::LEFT == rHoriOri.GetHoriOrient() )
            {
                SwFormatHoriOrient aHoriOri( rHoriOri );
                aHoriOri.SetRelationOrient( text::RelOrientation::CHAR );
                pFrameFormat->SetFormatAttr( aHoriOri );
            }
            const SwFormatVertOrient& rVertOri = pFrameFormat->GetVertOrient();
            if( text::VertOrientation::TOP == rVertOri.GetVertOrient() )
            {
                SwFormatVertOrient aVertOri( rVertOri );
                aVertOri.SetRelationOrient( text::RelOrientation::CHAR );
                pFrameFormat->SetFormatAttr( aVertOri );
            }

            pFrameFormat->MakeFrames();
            m_aMoveFlyFrames.erase( m_aMoveFlyFrames.begin() + n );
            m_aMoveFlyCnts.erase( m_aMoveFlyCnts.begin() + n );
        }
    }
    while( !aFields.empty() )
    {
        pAttr = aFields[0];

        pCNd = pAttr->nSttPara.GetNode().GetContentNode();
        pAttrPam->GetPoint()->nNode = pAttr->nSttPara;
        pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nSttContent );

        if( bBeforeTable &&
            pAttrPam->GetPoint()->nNode.GetIndex() == rEndIdx.GetIndex() )
        {
            OSL_ENSURE( !bBeforeTable, "Aha, der Fall tritt also doch ein" );
            OSL_ENSURE( !pAttrPam->GetPoint()->nContent.GetIndex(),
                    "Content-Position vor Tabelle nicht 0???" );
            // !!!
            pAttrPam->Move( fnMoveBackward );
        }

        m_pDoc->getIDocumentContentOperations().InsertPoolItem( *pAttrPam, *pAttr->pItem );

        aFields.pop_front();
        delete pAttr;
    }

    delete pAttrPam;
}

void SwHTMLParser::NewAttr( HTMLAttr **ppAttr, const SfxPoolItem& rItem )
{
    // Font-Hoehen und -Farben- sowie Escapement-Attribute duerfen nicht
    // zusammengefasst werden. Sie werden deshalb in einer Liste gespeichert,
    // in der das zuletzt aufgespannte Attribut vorne steht und der Count
    // immer 1 ist. Fuer alle anderen Attribute wird der Count einfach
    // hochgezaehlt.
    if( *ppAttr )
    {
        HTMLAttr *pAttr = new HTMLAttr( *m_pPam->GetPoint(), rItem,
                                            ppAttr );
        pAttr->InsertNext( *ppAttr );
        (*ppAttr) = pAttr;
    }
    else
        (*ppAttr) = new HTMLAttr( *m_pPam->GetPoint(), rItem, ppAttr );
}

bool SwHTMLParser::EndAttr( HTMLAttr* pAttr, HTMLAttr **ppDepAttr,
                            bool bChkEmpty )
{
    bool bRet = true;

    OSL_ENSURE( !ppDepAttr, "SwHTMLParser::EndAttr: ppDepAttr-Feature ungetestet?" );
    // Der Listenkopf ist im Attribut gespeichert
    HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    // die aktuelle Position als Ende-Position merken
    const SwNodeIndex* pEndIdx = &m_pPam->GetPoint()->nNode;
    sal_Int32 nEndCnt = m_pPam->GetPoint()->nContent.GetIndex();

    // WIrd das zueltzt gestartete oder ein frueher gestartetes Attribut
    // beendet?
    HTMLAttr *pLast = nullptr;
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

    bool bMoveBack = false;
    sal_uInt16 nWhich = pAttr->pItem->Which();
    if( !nEndCnt && RES_PARATR_BEGIN <= nWhich &&
        *pEndIdx != pAttr->GetSttPara() )
    {
        // dann eine Contentnt Position zurueck!
        bMoveBack = m_pPam->Move( fnMoveBackward );
        nEndCnt = m_pPam->GetPoint()->nContent.GetIndex();
    }

    // nun das Attrubut beenden
    HTMLAttr *pNext = pAttr->GetNext();

    bool bInsert;
    sal_uInt16 nScriptItem = 0;
    bool bScript = false;
    // ein Bereich ??
    if( !bChkEmpty || (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
        RES_PAGEDESC == nWhich || RES_BREAK == nWhich ||
        *pEndIdx != pAttr->GetSttPara() ||
        nEndCnt != pAttr->GetSttCnt() )
    {
        bInsert = true;
        // We do some optimization for script dependent attributes here.
        if( *pEndIdx == pAttr->GetSttPara() )
        {
            bool bFont = false;
            lcl_swhtml_getItemInfo( *pAttr, bScript, bFont, nScriptItem );
        }
    }
    else
    {
        bInsert = false;
    }

    if( bInsert && bScript )
    {
        const SwTextNode *pTextNd = pAttr->GetSttPara().GetNode()
                                            .GetTextNode();
        OSL_ENSURE( pTextNd, "No text node" );
        const OUString& rText = pTextNd->GetText();
        sal_uInt16 nScriptText = g_pBreakIt->GetBreakIter()->getScriptType(
                        rText, pAttr->GetSttCnt() );
        sal_Int32 nScriptEnd = g_pBreakIt->GetBreakIter()
                    ->endOfScript( rText, pAttr->GetSttCnt(), nScriptText );
        while (nScriptEnd < nEndCnt && nScriptEnd != -1)
        {
            if( nScriptItem == nScriptText )
            {
                HTMLAttr *pSetAttr = pAttr->Clone( *pEndIdx, nScriptEnd );
                pSetAttr->ClearPrev();
                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    if (pSetAttr->bInsAtStart)
                        m_aSetAttrTab.push_front( pSetAttr );
                    else
                        m_aSetAttrTab.push_back( pSetAttr );
                }
            }
            pAttr->nSttContent = nScriptEnd;
            nScriptText = g_pBreakIt->GetBreakIter()->getScriptType(
                            rText, nScriptEnd );
            nScriptEnd = g_pBreakIt->GetBreakIter()
                    ->endOfScript( rText, nScriptEnd, nScriptText );
        }
        bInsert = nScriptItem == nScriptText;
    }
    if( bInsert )
    {
        pAttr->nEndPara = *pEndIdx;
        pAttr->nEndContent = nEndCnt;
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
                    m_aSetAttrTab.push_front( pAttr );
                else
                    m_aSetAttrTab.push_back( pAttr );
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
        HTMLAttr *pPrev = pAttr->GetPrev();
        bRet = false;
        delete pAttr;

        if( pPrev )
        {
            // Die Previous-Attribute muessen trotzdem gesetzt werden.
            if( pNext )
                pNext->InsertPrev( pPrev );
            else
            {
                if (pPrev->bInsAtStart)
                    m_aSetAttrTab.push_front( pPrev );
                else
                    m_aSetAttrTab.push_back( pPrev );
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
        m_pPam->Move( fnMoveForward );

    return bRet;
}

void SwHTMLParser::DeleteAttr( HTMLAttr* pAttr )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    // Der Listenkopf ist im Attribut gespeichert
    HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    // Wird das zueltzt gestartete oder ein frueher gestartetes Attribut
    // entfernt?
    HTMLAttr *pLast = nullptr;
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
    HTMLAttr *pNext = pAttr->GetNext();
    HTMLAttr *pPrev = pAttr->GetPrev();
    delete pAttr;

    if( pPrev )
    {
        // Die Previous-Attribute muessen trotzdem gesetzt werden.
        if( pNext )
            pNext->InsertPrev( pPrev );
        else
        {
            if (pPrev->bInsAtStart)
                m_aSetAttrTab.push_front( pPrev );
            else
                m_aSetAttrTab.push_back( pPrev );
        }
    }

    // wenn das erste Attribut der Liste entfernt wurde muss noch der
    // Listenkopf korrigiert werden.
    if( pLast )
        pLast->pNext = pNext;
    else if( ppHead )
        *ppHead = pNext;
}

void SwHTMLParser::SaveAttrTab( HTMLAttrTable& rNewAttrTab )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(&m_aAttrTab);
    HTMLAttr** pSaveAttributes = reinterpret_cast<HTMLAttr**>(&rNewAttrTab);

    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes, ++pSaveAttributes)
    {
        *pSaveAttributes = *pHTMLAttributes;

        HTMLAttr *pAttr = *pSaveAttributes;
        while (pAttr)
        {
            pAttr->SetHead(pSaveAttributes);
            pAttr = pAttr->GetNext();
        }

        *pHTMLAttributes = nullptr;
    }
}

void SwHTMLParser::SplitAttrTab( HTMLAttrTable& rNewAttrTab,
                                 bool bMoveEndBack )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    const SwNodeIndex& nSttIdx = m_pPam->GetPoint()->nNode;
    SwNodeIndex nEndIdx( nSttIdx );

    // alle noch offenen Attribute beenden und hinter der Tabelle
    // neu aufspannen
    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(&m_aAttrTab);
    HTMLAttr** pSaveAttributes = reinterpret_cast<HTMLAttr**>(&rNewAttrTab);
    bool bSetAttr = true;
    const sal_Int32 nSttCnt = m_pPam->GetPoint()->nContent.GetIndex();
    sal_Int32 nEndCnt = nSttCnt;

    if( bMoveEndBack )
    {
        sal_uLong nOldEnd = nEndIdx.GetIndex();
        sal_uLong nTmpIdx;
        if( ( nTmpIdx = m_pDoc->GetNodes().GetEndOfExtras().GetIndex()) >= nOldEnd ||
            ( nTmpIdx = m_pDoc->GetNodes().GetEndOfAutotext().GetIndex()) >= nOldEnd )
        {
            nTmpIdx = m_pDoc->GetNodes().GetEndOfInserts().GetIndex();
        }
        SwContentNode* pCNd = SwNodes::GoPrevious(&nEndIdx);

        // keine Attribute setzen, wenn der PaM aus dem Content-Bereich
        // herausgeschoben wurde.
        bSetAttr = pCNd && nTmpIdx < nEndIdx.GetIndex();

        nEndCnt = (bSetAttr ? pCNd->Len() : 0);
    }
    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; (++pHTMLAttributes, ++pSaveAttributes))
    {
        HTMLAttr *pAttr = *pHTMLAttributes;
        *pSaveAttributes = nullptr;
        while( pAttr )
        {
            HTMLAttr *pNext = pAttr->GetNext();
            HTMLAttr *pPrev = pAttr->GetPrev();

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
                HTMLAttr *pSetAttr = pAttr->Clone( nEndIdx, nEndCnt );

                if( pNext )
                    pNext->InsertPrev( pSetAttr );
                else
                {
                    if (pSetAttr->bInsAtStart)
                        m_aSetAttrTab.push_front( pSetAttr );
                    else
                        m_aSetAttrTab.push_back( pSetAttr );
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
                        m_aSetAttrTab.push_front( pPrev );
                    else
                        m_aSetAttrTab.push_back( pPrev );
                }
            }

            // den Start des Attributs neu setzen und die Verkettungen
            // aufbrechen
            pAttr->Reset(nSttIdx, nSttCnt, pSaveAttributes);

            if (*pSaveAttributes)
            {
                HTMLAttr *pSAttr = *pSaveAttributes;
                while( pSAttr->GetNext() )
                    pSAttr = pSAttr->GetNext();
                pSAttr->InsertNext( pAttr );
            }
            else
                *pSaveAttributes = pAttr;

            pAttr = pNext;
        }

        *pHTMLAttributes = nullptr;
    }
}

void SwHTMLParser::RestoreAttrTab( HTMLAttrTable& rNewAttrTab )
{
    // preliminary paragraph attributes are not allowed here, they could
    // be set here and then the pointers become invalid!
    OSL_ENSURE(m_aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(&m_aAttrTab);
    HTMLAttr** pSaveAttributes = reinterpret_cast<HTMLAttr**>(&rNewAttrTab);

    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes, ++pSaveAttributes)
    {
        OSL_ENSURE(!*pHTMLAttributes, "Die Attribut-Tabelle ist nicht leer!");

        *pHTMLAttributes = *pSaveAttributes;

        HTMLAttr *pAttr = *pHTMLAttributes;
        while (pAttr)
        {
            OSL_ENSURE( !pAttr->GetPrev() || !pAttr->GetPrev()->ppHead,
                    "Previous-Attribut hat noch einen Header" );
            pAttr->SetHead(pHTMLAttributes);
            pAttr = pAttr->GetNext();
        }

        *pSaveAttributes = nullptr;
    }
}

void SwHTMLParser::InsertAttr( const SfxPoolItem& rItem, bool bInsAtStart )
{
    HTMLAttr* pTmp = new HTMLAttr( *m_pPam->GetPoint(),
                                     rItem );
    if (bInsAtStart)
        m_aSetAttrTab.push_front( pTmp );
    else
        m_aSetAttrTab.push_back( pTmp );
}

void SwHTMLParser::InsertAttrs( HTMLAttrs& rAttrs )
{
    while( !rAttrs.empty() )
    {
        HTMLAttr *pAttr = rAttrs.front();
        InsertAttr( pAttr->GetItem(), false );
        rAttrs.pop_front();
        delete pAttr;
    }
}

void SwHTMLParser::NewStdAttr( int nToken )
{
    OUString aId, aStyle, aLang, aDir;
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
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            if( HTML_SPAN_ON != nToken || aClass.isEmpty() ||
                !CreateContainer( aClass, aItemSet, aPropInfo, pCntxt ) )
                DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::NewStdAttr( int nToken,
                               HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               HTMLAttr **ppAttr2, const SfxPoolItem *pItem2,
                               HTMLAttr **ppAttr3, const SfxPoolItem *pItem3 )
{
    OUString aId, aStyle, aClass, aLang, aDir;

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
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        aItemSet.Put( rItem );
        if( pItem2 )
            aItemSet.Put( *pItem2 );
        if( pItem3 )
            aItemSet.Put( *pItem3 );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
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
    HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );
    if( pCntxt )
    {
        // und ggf. die Attribute beenden
        EndContext( pCntxt );
        delete pCntxt;
    }
}

void SwHTMLParser::NewBasefontAttr()
{
    OUString aId, aStyle, aClass, aLang, aDir;
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
    HTMLAttrContext *pCntxt = new HTMLAttrContext( HTML_BASEFONT_ON );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        //CJK has different defaults
        SvxFontHeightItem aFontHeight( m_aFontHeights[nSize-1], 100, RES_CHRATR_FONTSIZE );
        aItemSet.Put( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( m_aFontHeights[nSize-1], 100, RES_CHRATR_CJK_FONTSIZE );
        aItemSet.Put( aFontHeightCJK );
        //Complex type can contain so many types of letters,
        //that it's not really worthy to bother, IMO.
        //Still, I have set a default.
        SvxFontHeightItem aFontHeightCTL( m_aFontHeights[nSize-1], 100, RES_CHRATR_CTL_FONTSIZE );
        aItemSet.Put( aFontHeightCTL );

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
            DoPositioning( aItemSet, aPropInfo, pCntxt );

        InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
    }
    else
    {
        SvxFontHeightItem aFontHeight( m_aFontHeights[nSize-1], 100, RES_CHRATR_FONTSIZE );
        InsertAttr( &m_aAttrTab.pFontHeight, aFontHeight, pCntxt );
        SvxFontHeightItem aFontHeightCJK( m_aFontHeights[nSize-1], 100, RES_CHRATR_CJK_FONTSIZE );
        InsertAttr( &m_aAttrTab.pFontHeightCJK, aFontHeightCJK, pCntxt );
        SvxFontHeightItem aFontHeightCTL( m_aFontHeights[nSize-1], 100, RES_CHRATR_CTL_FONTSIZE );
        InsertAttr( &m_aAttrTab.pFontHeightCTL, aFontHeightCTL, pCntxt );
    }

    // den Kontext merken
    PushContext( pCntxt );

    // die Font-Size merken
    m_aBaseFontStack.push_back( nSize );
}

void SwHTMLParser::EndBasefontAttr()
{
    EndTag( HTML_BASEFONT_ON );

    // Stack-Unterlauf in Tabellen vermeiden
    if( m_aBaseFontStack.size() > m_nBaseFontStMin )
        m_aBaseFontStack.erase( m_aBaseFontStack.begin() + m_aBaseFontStack.size() - 1 );
}

void SwHTMLParser::NewFontAttr( int nToken )
{
    sal_uInt16 nBaseSize =
        ( m_aBaseFontStack.size() > m_nBaseFontStMin
            ? (m_aBaseFontStack[m_aBaseFontStack.size()-1] & FONTSIZE_MASK)
            : 3 );
    sal_uInt16 nFontSize =
        ( m_aFontStack.size() > m_nFontStMin
            ? (m_aFontStack[m_aFontStack.size()-1] & FONTSIZE_MASK)
            : nBaseSize );

    OUString aFace, aId, aStyle, aClass, aLang, aDir;
    Color aColor;
    sal_uLong nFontHeight = 0;  // tatsaechlich einzustellende Font-Hoehe
    sal_uInt16 nSize = 0;       // Fontgroesse in Netscape-Notation (1-7)
    bool bColor = false;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_SIZE:
            if( HTML_FONT_ON==nToken && !rOption.GetString().isEmpty() )
            {
                sal_Int32 nSSize;
                if( '+' == rOption.GetString()[0] ||
                    '-' == rOption.GetString()[0] )
                    nSSize = nBaseSize + rOption.GetSNumber();
                else
                    nSSize = (sal_Int32)rOption.GetNumber();

                if( nSSize < 1 )
                    nSSize = 1;
                else if( nSSize > 7 )
                    nSSize = 7;

                nSize = (sal_uInt16)nSSize;
                nFontHeight = m_aFontHeights[nSize-1];
            }
            break;
        case HTML_O_COLOR:
            if( HTML_FONT_ON==nToken )
            {
                rOption.GetColor( aColor );
                bColor = true;
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
        sal_uInt16 nPoolId = GetCurrFormatColl()->GetPoolFormatId();
        if( (nPoolId>=RES_POOLCOLL_HEADLINE1 &&
             nPoolId<=RES_POOLCOLL_HEADLINE6) )
        {
            // wenn die Schriftgroesse in der Ueberschrift noch
            // nicht veraendert ist, die aus der Vorlage nehmen
            if( m_nFontStHeadStart==m_aFontStack.size() )
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
                m_pCSS1Parser->GetTextCollFromPool(
                    RES_POOLCOLL_HEADLINE1+6-nSize )->GetSize().GetHeight();
        else
            nFontHeight = m_aFontHeights[nSize-1];
    }

    OSL_ENSURE( !nSize == !nFontHeight, "HTML-Font-Size != Font-Height" );

    OUString aFontName, aStyleName;
    FontFamily eFamily = FAMILY_DONTKNOW;   // Family und Pitch,
    FontPitch ePitch = PITCH_DONTKNOW;      // falls nicht gefunden
    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();

    if( !aFace.isEmpty() && !m_pCSS1Parser->IsIgnoreFontFamily() )
    {
        const FontList *pFList = nullptr;
        SwDocShell *pDocSh = m_pDoc->GetDocShell();
        if( pDocSh )
        {
            const SvxFontListItem *pFListItem =
               static_cast<const SvxFontListItem *>(pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
            if( pFListItem )
                pFList = pFListItem->GetFontList();
        }

        bool bFound = false;
        sal_Int32 nStrPos = 0;
        while( nStrPos!= -1 )
        {
            OUString aFName = aFace.getToken( 0, ',', nStrPos );
            aFName = comphelper::string::strip(aFName, ' ');
            if( !aFName.isEmpty() )
            {
                if( !bFound && pFList )
                {
                    sal_Handle hFont = pFList->GetFirstFontMetric( aFName );
                    if( nullptr != hFont )
                    {
                        const FontMetric& rFMetric = FontList::GetFontMetric( hFont );
                        if( RTL_TEXTENCODING_DONTKNOW != rFMetric.GetCharSet() )
                        {
                            bFound = true;
                            if( RTL_TEXTENCODING_SYMBOL == rFMetric.GetCharSet() )
                                eEnc = RTL_TEXTENCODING_SYMBOL;
                        }
                    }
                }
                if( !aFontName.isEmpty() )
                    aFontName += ";";
                aFontName += aFName;
            }
        }
    }

    // einen neuen Kontext anlegen
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
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
        if( !aFontName.isEmpty() )
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

        InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
    }
    else
    {
        if( nFontHeight )
        {
            SvxFontHeightItem aFontHeight( nFontHeight, 100, RES_CHRATR_FONTSIZE );
            InsertAttr( &m_aAttrTab.pFontHeight, aFontHeight, pCntxt );
            SvxFontHeightItem aFontHeightCJK( nFontHeight, 100, RES_CHRATR_CJK_FONTSIZE );
            InsertAttr( &m_aAttrTab.pFontHeight, aFontHeightCJK, pCntxt );
            SvxFontHeightItem aFontHeightCTL( nFontHeight, 100, RES_CHRATR_CTL_FONTSIZE );
            InsertAttr( &m_aAttrTab.pFontHeight, aFontHeightCTL, pCntxt );
        }
        if( bColor )
            InsertAttr( &m_aAttrTab.pFontColor, SvxColorItem(aColor, RES_CHRATR_COLOR), pCntxt );
        if( !aFontName.isEmpty() )
        {
            SvxFontItem aFont( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_FONT );
            InsertAttr( &m_aAttrTab.pFont, aFont, pCntxt );
            SvxFontItem aFontCJK( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CJK_FONT );
            InsertAttr( &m_aAttrTab.pFont, aFontCJK, pCntxt );
            SvxFontItem aFontCTL( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CTL_FONT );
            InsertAttr( &m_aAttrTab.pFont, aFontCTL, pCntxt );
        }
    }

    // den Kontext merken
    PushContext( pCntxt );

    m_aFontStack.push_back( nSize );
}

void SwHTMLParser::EndFontAttr( int nToken )
{
    EndTag( nToken );

    // Stack-Unterlauf in Tabellen vermeiden
    if( m_aFontStack.size() > m_nFontStMin )
        m_aFontStack.erase( m_aFontStack.begin() + m_aFontStack.size() - 1 );
}

void SwHTMLParser::NewPara()
{
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_SPACE );
    else
        AddParSpace();

    m_eParaAdjust = SVX_ADJUST_END;
    OUString aId, aStyle, aClass, aLang, aDir;

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
                m_eParaAdjust = (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(m_eParaAdjust) );
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
    HTMLAttrContext *pCntxt =
        !aClass.isEmpty() ? new HTMLAttrContext( HTML_PARABREAK_ON,
                                             RES_POOLCOLL_TEXT, aClass )
                     : new HTMLAttrContext( HTML_PARABREAK_ON );

    // Styles parsen (Class nicht beruecksichtigen. Das geht nur, solange
    // keine der CSS1-Properties der Klasse hart formatiert werden muss!!!)
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !m_pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != m_eParaAdjust )
        InsertAttr( &m_aAttrTab.pAdjust, SvxAdjustItem(m_eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    // und auf den Stack packen
    PushContext( pCntxt );

    // die aktuelle Vorlage oder deren Attribute setzen
    SetTextCollAttrs( !aClass.isEmpty() ? pCntxt : nullptr );

    // Laufbalkenanzeige
    ShowStatline();

    OSL_ENSURE( !m_nOpenParaToken, "Jetzt geht ein offenes Absatz-Element verloren" );
    m_nOpenParaToken = HTML_PARABREAK_ON;
}

void SwHTMLParser::EndPara( bool bReal )
{
    if( HTML_LI_ON==m_nOpenParaToken && m_pTable )
    {
#if OSL_DEBUG_LEVEL > 0
        const SwNumRule *pNumRule = m_pPam->GetNode().GetTextNode()->GetNumRule();
        OSL_ENSURE( pNumRule, "Wo ist die Numrule geblieben" );
#endif
    }

    // leere Absaetze werden von Netscape uebersprungen, von uns jetzt auch
    if( bReal )
    {
        if( m_pPam->GetPoint()->nContent.GetIndex() )
            AppendTextNode( AM_SPACE );
        else
            AddParSpace();
    }

    // wenn ein DD oder DT offen war, handelt es sich um eine
    // implizite Def-Liste, die jetzt beendet werden muss
    if( (m_nOpenParaToken==HTML_DT_ON || m_nOpenParaToken==HTML_DD_ON) &&
        m_nDefListDeep)
    {
        m_nDefListDeep--;
    }

    // den Kontext vom Stack holen. Er kann auch von einer implizit
    // geoeffneten Definitionsliste kommen
    HTMLAttrContext *pCntxt =
        PopContext( static_cast< sal_uInt16 >(m_nOpenParaToken ? (m_nOpenParaToken & ~1)
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
        SetTextCollAttrs();

    m_nOpenParaToken = 0;
}

void SwHTMLParser::NewHeading( int nToken )
{
    m_eParaAdjust = SVX_ADJUST_END;

    OUString aId, aStyle, aClass, aLang, aDir;

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
                m_eParaAdjust = (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(m_eParaAdjust) );
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
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_SPACE );
    else
        AddParSpace();

    // die passende Vorlage suchen
    sal_uInt16 nTextColl;
    switch( nToken )
    {
    case HTML_HEAD1_ON:         nTextColl = RES_POOLCOLL_HEADLINE1;  break;
    case HTML_HEAD2_ON:         nTextColl = RES_POOLCOLL_HEADLINE2;  break;
    case HTML_HEAD3_ON:         nTextColl = RES_POOLCOLL_HEADLINE3;  break;
    case HTML_HEAD4_ON:         nTextColl = RES_POOLCOLL_HEADLINE4;  break;
    case HTML_HEAD5_ON:         nTextColl = RES_POOLCOLL_HEADLINE5;  break;
    case HTML_HEAD6_ON:         nTextColl = RES_POOLCOLL_HEADLINE6;  break;
    default:                    nTextColl = RES_POOLCOLL_STANDARD;   break;
    }

    // den Kontext anlegen
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nTextColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !m_pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != m_eParaAdjust )
        InsertAttr( &m_aAttrTab.pAdjust, SvxAdjustItem(m_eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    // udn auf den Stack packen
    PushContext( pCntxt );

    // und die Vorlage oder deren Attribute setzen
    SetTextCollAttrs( pCntxt );

    m_nFontStHeadStart = m_aFontStack.size();

    // Laufbalkenanzeige
    ShowStatline();
}

void SwHTMLParser::EndHeading()
{
    // einen neuen Absatz aufmachen
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_SPACE );
    else
        AddParSpace();

    // Kontext zu dem Token suchen und vom Stack holen
    HTMLAttrContext *pCntxt = nullptr;
    auto nPos = m_aContexts.size();
    while( !pCntxt && nPos>m_nContextStMin )
    {
        switch( m_aContexts[--nPos]->GetToken() )
        {
        case HTML_HEAD1_ON:
        case HTML_HEAD2_ON:
        case HTML_HEAD3_ON:
        case HTML_HEAD4_ON:
        case HTML_HEAD5_ON:
        case HTML_HEAD6_ON:
            pCntxt = m_aContexts[nPos];
            m_aContexts.erase( m_aContexts.begin() + nPos );
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
    SetTextCollAttrs();

    m_nFontStHeadStart = m_nFontStMin;
}

void SwHTMLParser::NewTextFormatColl( int nToken, sal_uInt16 nColl )
{
    OUString aId, aStyle, aClass, aLang, aDir;

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
        aClass = aEmptyOUStr;
        SAL_FALLTHROUGH;
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
        OSL_ENSURE( false, "unbekannte Vorlage" );
        break;
    }
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // ... und in einem Kontext merken
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nColl, aClass );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !m_pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTextCollAttrs( pCntxt );

    // Laufbalkenanzeige aktualisieren
    ShowStatline();
}

void SwHTMLParser::EndTextFormatColl( int nToken )
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
        OSL_ENSURE( false, "unbekannte Vorlage" );
        break;
    }
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( eMode );
    else if( AM_SPACE==eMode )
        AddParSpace();

    // den aktuellen Kontext vom Stack holen
    HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage setzen
    SetTextCollAttrs();
}

void SwHTMLParser::NewDefList()
{
    OUString aId, aStyle, aClass, aLang, aDir;

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
    bool bSpace = (GetNumInfo().GetDepth() + m_nDefListDeep) == 0;
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    // ein Level mehr
    m_nDefListDeep++;

    bool bInDD = false, bNotInDD = false;
    auto nPos = m_aContexts.size();
    while( !bInDD && !bNotInDD && nPos>m_nContextStMin )
    {
        sal_uInt16 nCntxtToken = m_aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDD = true;
            break;
        case HTML_DD_ON:
            bInDD = true;
            break;
        }
    }

    // ... und in einem Kontext merken
    HTMLAttrContext *pCntxt = new HTMLAttrContext( HTML_DEFLIST_ON );

    // darin auch die Raender merken
    sal_uInt16 nLeft=0, nRight=0;
    short nIndent=0;
    GetMarginsFromContext( nLeft, nRight, nIndent );

    // Die Einrueckung, die sich schon aus einem DL-ergibt, entspricht der
    // eines DT auf dem aktuellen Level, und die entspricht der eines
    // DD auf dem Level davor. Fue einen Level >=2 muss also ein DD-Abstand
    // hinzugefuegt werden
    if( !bInDD && m_nDefListDeep > 1 )
    {

        // und den der DT-Vorlage des aktuellen Levels
        SvxLRSpaceItem rLRSpace =
            m_pCSS1Parser->GetTextFormatColl( RES_POOLCOLL_HTML_DD, aEmptyOUStr )
                       ->GetLRSpace();
        nLeft = nLeft + static_cast< sal_uInt16 >(rLRSpace.GetTextLeft());
    }

    pCntxt->SetMargins( nLeft, nRight, nIndent );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die Attribute der neuen Vorlage setzen
    if( m_nDefListDeep > 1 )
        SetTextCollAttrs( pCntxt );
}

void SwHTMLParser::EndDefList()
{
    bool bSpace = (GetNumInfo().GetDepth() + m_nDefListDeep) == 1;
    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    // ein Level weniger
    if( m_nDefListDeep > 0 )
        m_nDefListDeep--;

    // den aktuellen Kontext vom Stack holen
    HTMLAttrContext *pCntxt = PopContext( HTML_DEFLIST_ON );

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und Vorlage setzen
    SetTextCollAttrs();
}

void SwHTMLParser::NewDefListItem( int nToken )
{
    // festellen, ob das DD/DT in einer DL vorkommt
    bool bInDefList = false, bNotInDefList = false;
    auto nPos = m_aContexts.size();
    while( !bInDefList && !bNotInDefList && nPos>m_nContextStMin )
    {
        sal_uInt16 nCntxtToken = m_aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DEFLIST_ON:
            bInDefList = true;
            break;
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            bNotInDefList = true;
            break;
        }
    }

    // wenn nicht, implizit eine neue DL aufmachen
    if( !bInDefList )
    {
        m_nDefListDeep++;
        OSL_ENSURE( !m_nOpenParaToken,
                "Jetzt geht ein offenes Absatz-Element verloren" );
        m_nOpenParaToken = static_cast< sal_uInt16 >(nToken);
    }

    NewTextFormatColl( nToken, static_cast< sal_uInt16 >(nToken==HTML_DD_ON ? RES_POOLCOLL_HTML_DD
                                              : RES_POOLCOLL_HTML_DT) );
}

void SwHTMLParser::EndDefListItem( int nToken, bool /*bLastPara*/ )
{
    // einen neuen Absatz aufmachen
    if( !nToken && m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_SOFTNOSPACE );

    // Kontext zu dem Token suchen und vom Stack holen
    nToken &= ~1;
    HTMLAttrContext *pCntxt = nullptr;
    auto nPos = m_aContexts.size();
    while( !pCntxt && nPos>m_nContextStMin )
    {
        sal_uInt16 nCntxtToken = m_aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_DD_ON:
        case HTML_DT_ON:
            if( !nToken || nToken == nCntxtToken  )
            {
                pCntxt = m_aContexts[nPos];
                m_aContexts.erase( m_aContexts.begin() + nPos );
            }
            break;
        case HTML_DEFLIST_ON:
            // keine DD/DT ausserhalb der aktuelen DefListe betrachten
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            // und auch nicht ausserhalb einer anderen Liste
            nPos = m_nContextStMin;
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
}

bool SwHTMLParser::HasCurrentParaFlys( bool bNoSurroundOnly,
                                       bool bSurroundOnly ) const
{
    // bNoSurroundOnly:     Der Absatz enthaelt mindestens einen Rahmen
    //                      ohne Umlauf
    // bSurroundOnly:       Der Absatz enthaelt mindestens einen Rahmen
    //                      mit Umlauf aber keinen ohne Umlauf
    // sonst:               Der Absatz enthaelt irgendeinen Rahmen
    SwNodeIndex& rNodeIdx = m_pPam->GetPoint()->nNode;

    const SwFrameFormats& rFrameFormatTable = *m_pDoc->GetSpzFrameFormats();

    bool bFound = false;
    for ( size_t i=0; i<rFrameFormatTable.size(); i++ )
    {
        const SwFrameFormat *const pFormat = rFrameFormatTable[i];
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        // Ein Rahmen wurde gefunden, wenn
        // - er absatzgebunden ist, und
        // - im aktuellen Absatz verankert ist, und
        //   - jeder absatzgebunene Rahmen zaehlt, oder
        //   - (nur Rahmen oder umlauf zaehlen und ) der Rahmen keinen
        //     Umlauf besitzt
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            pAPos->nNode == rNodeIdx )
        {
            if( !(bNoSurroundOnly || bSurroundOnly) )
            {
                bFound = true;
                break;
            }
            else
            {
                // Wenn Rahmen mit Umlauf gesucht sind,
                // auch keine mit Durchlauf beachten. Dabei handelt es
                // sich (noch) um HIDDEN-Controls, und denen weicht man
                // besser auch nicht aus.
                SwSurround eSurround = pFormat->GetSurround().GetSurround();
                if( bNoSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = true;
                        break;
                    }
                }
                if( bSurroundOnly )
                {
                    if( SURROUND_NONE==eSurround )
                    {
                        bFound = false;
                        break;
                    }
                    else if( SURROUND_THROUGHT!=eSurround )
                    {
                        bFound = true;
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

const SwFormatColl *SwHTMLParser::GetCurrFormatColl() const
{
    const SwContentNode* pCNd = m_pPam->GetContentNode();
    return &pCNd->GetAnyFormatColl();
}

void SwHTMLParser::SetTextCollAttrs( HTMLAttrContext *pContext )
{
    SwTextFormatColl *pCollToSet = nullptr;   // die zu setzende Vorlage
    SfxItemSet *pItemSet = nullptr;       // der Set fuer harte Attrs
    sal_uInt16 nTopColl = pContext ? pContext->GetTextFormatColl() : 0;
    const OUString& rTopClass = pContext ? pContext->GetClass() : aEmptyOUStr;
    sal_uInt16 nDfltColl = RES_POOLCOLL_TEXT;

    bool bInPRE=false;                          // etwas Kontext Info

    sal_uInt16 nLeftMargin = 0, nRightMargin = 0;   // die Einzuege und
    short nFirstLineIndent = 0;                 // Abstaende

    for( auto i = m_nContextStAttrMin; i < m_aContexts.size(); ++i )
    {
        const HTMLAttrContext *pCntxt = m_aContexts[i];

        sal_uInt16 nColl = pCntxt->GetTextFormatColl();
        if( nColl )
        {
            // Es gibt eine Vorlage, die zu setzen ist. Dann
            // muss zunaechst einmal entschieden werden,
            // ob die Vorlage auch gesetzt werden kann
            bool bSetThis = true;
            switch( nColl )
            {
            case RES_POOLCOLL_HTML_PRE:
                bInPRE = true;
                break;
            case RES_POOLCOLL_TEXT:
                // <TD><P CLASS=xxx> muss TD.xxx werden
                if( nDfltColl==RES_POOLCOLL_TABLE ||
                    nDfltColl==RES_POOLCOLL_TABLE_HDLN )
                    nColl = nDfltColl;
                break;
            case RES_POOLCOLL_HTML_HR:
                // <HR> auch in <PRE> als Vorlage setzen, sonst kann man sie
                // nicht mehr exportieren
                break;
            default:
                if( bInPRE )
                    bSetThis = false;
                break;
            }

            SwTextFormatColl *pNewColl =
                m_pCSS1Parser->GetTextFormatColl( nColl, pCntxt->GetClass() );

            if( bSetThis )
            {
                // wenn jetzt eine andere Vorlage gesetzt werden soll als
                // bisher, muss die bishere Vorlage durch harte Attributierung
                // ersetzt werden

                if( pCollToSet )
                {
                    // die Attribute, die bisherige Vorlage setzt
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
            nColl = pCntxt->GetDfltTextFormatColl();
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

        const SwTextFormatColl *pTopColl =
            m_pCSS1Parser->GetTextFormatColl( nTopColl, rTopClass );
        const SfxItemSet& rItemSet = pTopColl->GetAttrSet();
        const SfxPoolItem *pItem;
        if( SfxItemState::SET == rItemSet.GetItemState(RES_LR_SPACE,true, &pItem) )
        {
            const SvxLRSpaceItem *pLRItem =
                static_cast<const SvxLRSpaceItem *>(pItem);

            sal_Int32 nLeft = pLRItem->GetTextLeft();
            sal_Int32 nRight = pLRItem->GetRight();
            nFirstLineIndent = pLRItem->GetTextFirstLineOfst();

            // In Definitions-Listen enthalten die Abstaende auch die der
            // vorhergehenden Level
            if( RES_POOLCOLL_HTML_DD == nTopColl )
            {
                const SvxLRSpaceItem& rDTLRSpace = m_pCSS1Parser
                    ->GetTextFormatColl( RES_POOLCOLL_HTML_DT, aEmptyOUStr )
                    ->GetLRSpace();
                nLeft -= rDTLRSpace.GetTextLeft();
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
        if( SfxItemState::SET == rItemSet.GetItemState(RES_UL_SPACE,true, &pItem) )
        {
            const SvxULSpaceItem *pULItem =
                static_cast<const SvxULSpaceItem *>(pItem);
            pContext->SetULSpace( pULItem->GetUpper(), pULItem->GetLower() );
        }
    }

    // wenn gar keine Vorlage im Kontext gesetzt ist, Textkoerper nehmen
    if( !pCollToSet )
    {
        pCollToSet = m_pCSS1Parser->GetTextCollFromPool( nDfltColl );
        const SvxLRSpaceItem& rLRItem = pCollToSet->GetLRSpace();
        if( !nLeftMargin )
            nLeftMargin = static_cast< sal_uInt16 >(rLRItem.GetTextLeft());
        if( !nRightMargin )
            nRightMargin = static_cast< sal_uInt16 >(rLRItem.GetRight());
        if( !nFirstLineIndent )
            nFirstLineIndent = rLRItem.GetTextFirstLineOfst();
    }

    // bisherige harte Attributierung des Absatzes entfernen
    if( !m_aParaAttrs.empty() )
    {
        for( auto pParaAttr : m_aParaAttrs )
            pParaAttr->Invalidate();

        m_aParaAttrs.clear();
    }

    // Die Vorlage setzen
    m_pDoc->SetTextFormatColl( *m_pPam, pCollToSet );

    // ggf. noch den Absatz-Einzug korrigieren
    const SvxLRSpaceItem& rLRItem = pCollToSet->GetLRSpace();
    bool bSetLRSpace;

           bSetLRSpace = nLeftMargin != rLRItem.GetTextLeft() ||
                      nFirstLineIndent != rLRItem.GetTextFirstLineOfst() ||
                      nRightMargin != rLRItem.GetRight();

    if( bSetLRSpace )
    {
        SvxLRSpaceItem aLRItem( rLRItem );
        aLRItem.SetTextLeft( nLeftMargin );
        aLRItem.SetRight( nRightMargin );
        aLRItem.SetTextFirstLineOfst( nFirstLineIndent );
        if( pItemSet )
            pItemSet->Put( aLRItem );
        else
        {
            NewAttr( &m_aAttrTab.pLRSpace, aLRItem );
            m_aAttrTab.pLRSpace->SetLikePara();
            m_aParaAttrs.push_back( m_aAttrTab.pLRSpace );
            EndAttr( m_aAttrTab.pLRSpace, nullptr, false );
        }
    }

    // und nun noch die Attribute setzen
    if( pItemSet )
    {
        InsertParaAttrs( *pItemSet );
        delete pItemSet;
    }
}

void SwHTMLParser::NewCharFormat( int nToken )
{
    OUString aId, aStyle, aLang, aDir;
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
    HTMLAttrContext *pCntxt = new HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    // die Vorlage setzen und im Kontext merken
    SwCharFormat* pCFormat = m_pCSS1Parser->GetChrFormat( static_cast< sal_uInt16 >(nToken), aClass );
    OSL_ENSURE( pCFormat, "keine Zeichenvorlage zu Token gefunden" );

    // Styles parsen (zu Class siehe auch NewPara)
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !m_pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, true );
        }
    }

    // Zeichen-Vorlagen werden in einem eigenen Stack gehalten und
    // koennen nie durch Styles eingefuegt werden. Das Attribut ist deshalb
    // auch gar nicht im CSS1-Which-Range enthalten
    if( pCFormat )
        InsertAttr( &m_aAttrTab.pCharFormats, SwFormatCharFormat( pCFormat ), pCntxt );

    // den Kontext merken
    PushContext( pCntxt );
}

void SwHTMLParser::InsertSpacer()
{
    // und es ggf. durch die Optionen veraendern
    OUString aId;
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    Size aSize( 0, 0);
    long nSize = 0;
    bool bPrcWidth = false;
    bool bPrcHeight = false;
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
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            // erstmal nur als Pixelwerte merken!
            bPrcHeight = (rOption.GetString().indexOf('%') != -1);
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
            SfxItemSet aFrameSet( m_pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
            if( !IsNewDoc() )
                Reader::ResetFrameFormatAttrs( aFrameSet );

            // den Anker und die Ausrichtung setzen
            SetAnchorAndAdjustment( eVertOri, eHoriOri, aFrameSet );

            // und noch die Groesse des Rahmens
            Size aDfltSz( MINFLY, MINFLY );
            Size aSpace( 0, 0 );
            SfxItemSet aDummyItemSet( m_pDoc->GetAttrPool(),
                                 m_pCSS1Parser->GetWhichMap() );
            SvxCSS1PropertyInfo aDummyPropInfo;

            SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight,
                        aDummyItemSet, aDummyPropInfo, aFrameSet );
            SetSpace( aSpace, aDummyItemSet, aDummyPropInfo, aFrameSet );

            // den Inhalt schuetzen
            SvxProtectItem aProtectItem( RES_PROTECT) ;
            aProtectItem.SetContentProtect( true );
            aFrameSet.Put( aProtectItem );

            // der Rahmen anlegen
            RndStdIds eAnchorId =
                static_cast<const SwFormatAnchor &>(aFrameSet.Get(RES_ANCHOR)).GetAnchorId();
            SwFrameFormat *pFlyFormat = m_pDoc->MakeFlySection( eAnchorId,
                                            m_pPam->GetPoint(), &aFrameSet );
            // Ggf Frames anlegen und auto-geb. Rahmen registrieren
            RegisterFlyFrame( pFlyFormat );
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
            SwTextNode *pTextNode = nullptr;
            if( !m_pPam->GetPoint()->nContent.GetIndex() )
            {
                // den unteren Absatz-Abstand des vorherigen Nodes aendern,
                // wenn moeglich

                SetAttr();  // noch offene Absatz-Attribute setzen

                pTextNode = m_pDoc->GetNodes()[m_pPam->GetPoint()->nNode.GetIndex()-1]
                               ->GetTextNode();

                // Wenn der Abstz davor kein Textenode ist, dann wird jetzt
                // ein leere Absatz angelegt, der eh schon eine Zeilenhoehe
                // Abstand erzeugt.
                if( !pTextNode )
                    nSize = nSize>HTML_PARSPACE ? nSize-HTML_PARSPACE : 0;
            }

            if( pTextNode )
            {
                SvxULSpaceItem aULSpace( static_cast<const SvxULSpaceItem&>(pTextNode
                    ->SwContentNode::GetAttr( RES_UL_SPACE )) );
                aULSpace.SetLower( aULSpace.GetLower() + (sal_uInt16)nSize );
                pTextNode->SetAttr( aULSpace );
            }
            else
            {
                NewAttr( &m_aAttrTab.pULSpace, SvxULSpaceItem( 0, (sal_uInt16)nSize, RES_UL_SPACE ) );
                EndAttr( m_aAttrTab.pULSpace, nullptr, false );

                AppendTextNode();    // nicht am Abstand drehen!
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

            if( !m_pPam->GetPoint()->nContent.GetIndex() )
            {
                sal_uInt16 nLeft=0, nRight=0;
                short nIndent = 0;

                GetMarginsFromContextWithNumBul( nLeft, nRight, nIndent );
                nIndent = nIndent + (short)nSize;

                SvxLRSpaceItem aLRItem( RES_LR_SPACE );
                aLRItem.SetTextLeft( nLeft );
                aLRItem.SetRight( nRight );
                aLRItem.SetTextFirstLineOfst( nIndent );

                NewAttr( &m_aAttrTab.pLRSpace, aLRItem );
                EndAttr( m_aAttrTab.pLRSpace, nullptr, false );
            }
            else
            {
                NewAttr( &m_aAttrTab.pKerning, SvxKerningItem( (short)nSize, RES_CHRATR_KERNING ) );
                OUString aTmp( ' ' );
                m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, aTmp );
                EndAttr( m_aAttrTab.pKerning );
            }
        }
    }
}

sal_uInt16 SwHTMLParser::ToTwips( sal_uInt16 nPixel )
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
    const SwTwips nWidth = SwHTMLTableLayout::GetBrowseWidth( *m_pDoc );
    if( nWidth )
        return nWidth;

    if( !m_aHTMLPageSize.Width() )
    {
        const SwFrameFormat& rPgFormat = m_pCSS1Parser->GetMasterPageDesc()->GetMaster();

        const SwFormatFrameSize& rSz   = rPgFormat.GetFrameSize();
        const SvxLRSpaceItem& rLR = rPgFormat.GetLRSpace();
        const SvxULSpaceItem& rUL = rPgFormat.GetULSpace();
        const SwFormatCol& rCol = rPgFormat.GetCol();

        m_aHTMLPageSize.Width() = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();
        m_aHTMLPageSize.Height() = rSz.GetHeight() - rUL.GetUpper() - rUL.GetLower();

        if( 1 < rCol.GetNumCols() )
            m_aHTMLPageSize.Width() /= rCol.GetNumCols();
    }

    return m_aHTMLPageSize.Width();
}

void SwHTMLParser::InsertIDOption()
{
    OUString aId;
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

    if( !aId.isEmpty() )
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

    OUString aId, aStyle, aClass;             // die ID der Bookmark
    bool bClearLeft = false, bClearRight = false;
    bool bCleared = false;  // wurde ein CLEAR ausgefuehrt?

    // dann holen wir mal die Optionen
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
            case HTML_O_CLEAR:
                {
                    const OUString &rClear = rOption.GetString();
                    if( rClear.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_all ) )
                    {
                        bClearLeft = true;
                        bClearRight = true;
                    }
                    else if( rClear.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ) )
                        bClearLeft = true;
                    else if( rClear.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right ) )
                        bClearRight = true;
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
        SwNodeIndex& rNodeIdx = m_pPam->GetPoint()->nNode;
        SwTextNode* pTextNd = rNodeIdx.GetNode().GetTextNode();
        if( pTextNd )
        {
            const SwFrameFormats& rFrameFormatTable = *m_pDoc->GetSpzFrameFormats();

            for( size_t i=0; i<rFrameFormatTable.size(); i++ )
            {
                SwFrameFormat *const pFormat = rFrameFormatTable[i];
                SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetContentAnchor();
                if (pAPos &&
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    pAPos->nNode == rNodeIdx &&
                    pFormat->GetSurround().GetSurround() != SURROUND_NONE )
                {
                    sal_Int16 eHori = RES_DRAWFRMFMT == pFormat->Which()
                        ? text::HoriOrientation::LEFT
                        : pFormat->GetHoriOrient().GetHoriOrient();

                    SwSurround eSurround = SURROUND_PARALLEL;
                    if( m_pPam->GetPoint()->nContent.GetIndex() )
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
                        SwFormatSurround aSurround( eSurround );
                        if( SURROUND_NONE != eSurround )
                            aSurround.SetAnchorOnly( true );
                        pFormat->SetFormatAttr( aSurround );
                        bCleared = true;
                    }
                } // Anker ist nicht im Node
            } // Schleife ueber Fly-Frames
        } // kein Text-Node
    } // kein CLEAR

    // Styles parsen
    SvxFormatBreakItem aBreakItem( SVX_BREAK_NONE, RES_BREAK );
    bool bBreakItem = false;
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            if( m_pCSS1Parser->SetFormatBreak( aItemSet, aPropInfo ) )
            {
                aBreakItem = static_cast<const SvxFormatBreakItem &>(aItemSet.Get( RES_BREAK ));
                bBreakItem = true;
            }
            if( !aPropInfo.aId.isEmpty() )
                InsertBookmark( aPropInfo.aId );
        }
    }

    if( bBreakItem && SVX_BREAK_PAGE_AFTER==aBreakItem.GetBreak() )
    {
        NewAttr( &m_aAttrTab.pBreak, aBreakItem );
        EndAttr( m_aAttrTab.pBreak, nullptr, false );
    }

    if( !bCleared && !bBreakItem )
    {
        // wenn kein CLEAR ausgefuehrt werden sollte oder konnte, wird
        // ein Zeilenumbruch eingef?gt
        OUString sTmp( (sal_Unicode)0x0a );   // make the Mac happy :-)
        m_pDoc->getIDocumentContentOperations().InsertString( *m_pPam, sTmp );
    }
    else if( m_pPam->GetPoint()->nContent.GetIndex() )
    {
        // wenn ein clear in einem nicht-leeren Absatz ausgefuehrt wurde,
        // muss anschliessen ein neuer Absatz aufgemacht werden
        // MIB 21.02.97: Eigentlich muesste man hier den unteren Absatz-
        // Absatnd auf 0 drehen. Das geht aber bei sowas wie <BR ..><P>
        // schief (>Netscape). Deshalb lassen wir das erstmal.
        AppendTextNode( AM_NOSPACE );
    }
    if( bBreakItem && SVX_BREAK_PAGE_BEFORE==aBreakItem.GetBreak() )
    {
        NewAttr( &m_aAttrTab.pBreak, aBreakItem );
        EndAttr( m_aAttrTab.pBreak, nullptr, false );
    }
}

void SwHTMLParser::InsertHorzRule()
{
    sal_uInt16 nSize = 0;
    sal_uInt16 nWidth = 0;

    SvxAdjust eAdjust = SVX_ADJUST_END;

    bool bPrcWidth = false;
    bool bNoShade = false;
    bool bColor = false;

    Color aColor;
    OUString aId;

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
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            nWidth = (sal_uInt16)rOption.GetNumber();
            if( bPrcWidth && nWidth>=100 )
            {
                // 100%-Linien sind der default-Fall (keine Attrs neotig)
                nWidth = 0;
                bPrcWidth = false;
            }
            break;
        case HTML_O_ALIGN:
            eAdjust =
                (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
            break;
        case HTML_O_NOSHADE:
            bNoShade = true;
            break;
        case HTML_O_COLOR:
            rOption.GetColor( aColor );
            bColor = true;
            break;
        }
    }

    if( m_pPam->GetPoint()->nContent.GetIndex() )
        AppendTextNode( AM_NOSPACE );
    if( m_nOpenParaToken )
        EndPara();
    AppendTextNode();
    m_pPam->Move( fnMoveBackward );

    // ... und in einem Kontext merken
    HTMLAttrContext *pCntxt =
        new HTMLAttrContext( HTML_HORZRULE, RES_POOLCOLL_HTML_HR, aEmptyOUStr );

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTextCollAttrs( pCntxt );

    // die harten Attribute an diesem Absatz werden nie mehr ungueltig
    if( !m_aParaAttrs.empty() )
        m_aParaAttrs.clear();

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
        aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::BOTTOM );
        HTMLAttr* pTmp = new HTMLAttr( *m_pPam->GetPoint(), aBoxItem );
        m_aSetAttrTab.push_back( pTmp );
    }
    if( nWidth )
    {
        // Wenn wir in keiner Tabelle sind, wird die Breitenangabe durch
        // Absatz-Einzuege "getuerkt". In einer Tabelle macht das wenig
        // Sinn. Um zu Vermeiden, dass die Linie bei der Breitenberechnung
        // beruecksichtigt wird, bekommt sie aber trotzdem entsprechendes
        // LRSpace-Item verpasst.
        if( !m_pTable )
        {
            // Laenge und Ausrichtung der Linie ueber Absatz-Einzuege "tuerken"
            long nBrowseWidth = GetCurrentBrowseWidth();
            nWidth = bPrcWidth ? (sal_uInt16)((nWidth*nBrowseWidth) / 100)
                               : ToTwips( (sal_uInt16)nBrowseWidth );
            if( nWidth < MINLAY )
                nWidth = MINLAY;

            if( (long)nWidth < nBrowseWidth )
            {
                const SwFormatColl *pColl = GetCurrFormatColl();
                SvxLRSpaceItem aLRItem( pColl->GetLRSpace() );
                long nDist = nBrowseWidth - nWidth;

                switch( eAdjust )
                {
                case SVX_ADJUST_RIGHT:
                    aLRItem.SetTextLeft( (sal_uInt16)nDist );
                    break;
                case SVX_ADJUST_LEFT:
                    aLRItem.SetRight( (sal_uInt16)nDist );
                    break;
                case SVX_ADJUST_CENTER:
                default:
                    nDist /= 2;
                    aLRItem.SetTextLeft( (sal_uInt16)nDist );
                    aLRItem.SetRight( (sal_uInt16)nDist );
                    break;
                }

                HTMLAttr* pTmp = new HTMLAttr( *m_pPam->GetPoint(), aLRItem );
                m_aSetAttrTab.push_back( pTmp );
            }
        }
    }

    // Bookmarks koennen nicht in Hyperlinks eingefueht werden
    if( !aId.isEmpty() )
        InsertBookmark( aId );

    // den aktuellen Kontext vom Stack holen
    HTMLAttrContext *pPoppedContext = PopContext( HTML_HORZRULE );
    OSL_ENSURE( pPoppedContext==pCntxt, "wo kommt denn da ein HR-Kontext her?" );
    delete pPoppedContext;

    m_pPam->Move( fnMoveForward );

    // und im Absatz danach die dort aktuelle Vorlage setzen
    SetTextCollAttrs();
}

void SwHTMLParser::ParseMoreMetaOptions()
{
    OUString aName, aContent;
    bool bHTTPEquiv = false;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_NAME:
            aName = rOption.GetString();
            bHTTPEquiv = false;
            break;
        case HTML_O_HTTPEQUIV:
            aName = rOption.GetString();
            bHTTPEquiv = true;
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
    if( aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_generator ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_refresh ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_content_type ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_content_script_type ) )
        return;

    aContent = aContent.replaceAll("\r", "").replaceAll("\n", "");

    if( aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_sdendnote ) )
    {
        FillEndNoteInfo( aContent );
        return;
    }

    if( aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_sdfootnote ) )
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

    SwPostItField aPostItField(
        static_cast<SwPostItFieldType*>(m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_POSTITFLD )),
        aEmptyOUStr, sText.makeStringAndClear(), aEmptyOUStr, aEmptyOUStr, DateTime( DateTime::SYSTEM ) );
    SwFormatField aFormatField( aPostItField );
    InsertAttr( aFormatField,  false );
}

HTMLAttr::HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
                      HTMLAttr **ppHd ) :
    nSttPara( rPos.nNode ),
    nEndPara( rPos.nNode ),
    nSttContent( rPos.nContent.GetIndex() ),
    nEndContent(rPos.nContent.GetIndex() ),
    bInsAtStart( true ),
    bLikePara( false ),
    bValid( true ),
    pNext( nullptr ),
    pPrev( nullptr ),
    ppHead( ppHd )
{
    pItem = rItem.Clone();
}

HTMLAttr::HTMLAttr( const HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
                      sal_Int32 nEndCnt, HTMLAttr **ppHd ) :
    nSttPara( rAttr.nSttPara ),
    nEndPara( rEndPara ),
    nSttContent( rAttr.nSttContent ),
    nEndContent( nEndCnt ),
    bInsAtStart( rAttr.bInsAtStart ),
    bLikePara( rAttr.bLikePara ),
    bValid( rAttr.bValid ),
    pNext( nullptr ),
    pPrev( nullptr ),
    ppHead( ppHd )
{
    pItem = rAttr.pItem->Clone();
}

HTMLAttr::~HTMLAttr()
{
    delete pItem;
}

HTMLAttr *HTMLAttr::Clone(const SwNodeIndex& rEndPara, sal_Int32 nEndCnt) const
{
    // das Attribut mit der alten Start-Position neu anlegen
    HTMLAttr *pNew = new HTMLAttr( *this, rEndPara, nEndCnt, ppHead );

    // die Previous-Liste muss uebernommen werden, die Next-Liste nicht!
    pNew->pPrev = pPrev;

    return pNew;
}

void HTMLAttr::Reset(const SwNodeIndex& rSttPara, sal_Int32 nSttCnt,
                       HTMLAttr **ppHd)
{
    // den Anfang (und das Ende) neu setzen
    nSttPara = rSttPara;
    nSttContent = nSttCnt;
    nEndPara = rSttPara;
    nEndContent = nSttCnt;

    // den Head korrigieren und die Verkettungen aufheben
    pNext = nullptr;
    pPrev = nullptr;
    ppHead = ppHd;
}

void HTMLAttr::InsertPrev( HTMLAttr *pPrv )
{
    OSL_ENSURE( !pPrv->pNext || pPrv->pNext == this,
            "HTMLAttr::InsertPrev: pNext falsch" );
    pPrv->pNext = nullptr;

    OSL_ENSURE( nullptr == pPrv->ppHead || ppHead == pPrv->ppHead,
            "HTMLAttr::InsertPrev: ppHead falsch" );
    pPrv->ppHead = nullptr;

    HTMLAttr *pAttr = this;
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
         (m_InfoNames[3].isEmpty() ? &m_InfoNames[3] : nullptr ))));
    if (pName)
    {
        (*pName) = i_rMetaName;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
