/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <mdiexp.hxx>
#include <expfld.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
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

#include <statstr.hrc>
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


HTMLOptionEnum aHTMLPAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,  SVX_ADJUST_LEFT     },
    { OOO_STRING_SVTOOLS_HTML_AL_center,    SVX_ADJUST_CENTER   },
    { OOO_STRING_SVTOOLS_HTML_AL_middle,    SVX_ADJUST_CENTER   }, 
    { OOO_STRING_SVTOOLS_HTML_AL_right, SVX_ADJUST_RIGHT    },
    { OOO_STRING_SVTOOLS_HTML_AL_justify,   SVX_ADJUST_BLOCK    },
    { OOO_STRING_SVTOOLS_HTML_AL_char,  SVX_ADJUST_LEFT     },
    { 0,                0                   }
};


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

OUString HTMLReader::GetTemplateName() const
{
    const OUString sTemplateWithoutExt("internal/html");
    SvtPathOptions aPathOpt;

    
    
    OUString sTemplate( sTemplateWithoutExt + ".oth" );
    if (aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE ))
        return sTemplate;

        
        
    sTemplate = sTemplateWithoutExt + ".stw";
    if (aPathOpt.SearchFile( sTemplate, SvtPathOptions::PATH_TEMPLATE ))
        return sTemplate;

    OSL_ENSURE( !this, "The default HTML template cannot be found in the defined template directories!");

    return OUString();
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

    
sal_uLong HTMLReader::Read( SwDoc &rDoc, const OUString& rBaseURL, SwPaM &rPam, const OUString & rName )
{
    if( !pStrm )
    {
        OSL_ENSURE( pStrm, "HTML-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    if( !bInsertMode )
    {
        Reader::ResetFrmFmts( rDoc );

        
        
        if( !rDoc.get(IDocumentSettingAccess::HTML_MODE) )
        {
            rDoc.InsertPoolItem( rPam, SwFmtPageDesc(
                rDoc.GetPageDescFromPool( RES_POOLPAGE_HTML, false )), 0 );
        }
    }

    
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
        const OUString sErr(OUString::number((sal_Int32)xParser->GetLineNr())
            + "," + OUString::number((sal_Int32)xParser->GetLinePos()));

        
        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }


    return nRet;
}



SwHTMLParser::SwHTMLParser( SwDoc* pD, SwPaM& rCrsr, SvStream& rIn,
                            const OUString& rPath,
                            const OUString& rBaseURL,
                            bool bReadNewDoc,
                            SfxMedium* pMed, sal_Bool bReadUTF8,
                            sal_Bool bNoHTMLComments )
    : SfxHTMLParser( rIn, bReadNewDoc, pMed ),
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
    
    nContextStMin( 0 ),
    nContextStAttrMin( 0 ),
    nSelectEntryCnt( 0 ),
    nOpenParaToken( 0 ),
    eJumpTo( JUMPTO_NONE ),
#ifdef DBG_UTIL
    m_nContinue( 0 ),
#endif
    eParaAdjust( SVX_ADJUST_END ),
    bDocInitalized( false ),
    bSetModEnabled( false ),
    bInFloatingFrame( false ),
    bInField( false ),
    bCallNextToken( false ),
    bIgnoreRawData( false ),
    bLBEntrySelected ( false ),
    bTAIgnoreNewPara ( false ),
    bFixMarqueeWidth ( false ),
    bFixMarqueeHeight ( false ),
    bNoParSpace( false ),
    bInNoEmbed( false ),
    bInTitle( false ),
    bUpdateDocStat( false ),
    bFixSelectWidth( false ),
    bFixSelectHeight( false ),
    bTextArea( false ),
    bSelect( false ),
    bInFootEndNoteAnchor( false ),
    bInFootEndNoteSymbol( false ),
    bIgnoreHTMLComments( bNoHTMLComments ),
    bRemoveHidden( false ),
    pTempViewFrame(0)
{
    nEventId = 0;
    bUpperSpace = bViewCreated = bChkJumpMark =
    bSetCrsr = false;

    eScriptLang = HTML_SL_UNKNOWN;
    bAnyStarBasic = true;

    rCrsr.DeleteMark();
    pPam = &rCrsr; 
    memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));

    
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
        
        
        SvxFontHeightItem aFontHeight(aFontHeights[2], 100, RES_CHRATR_FONTSIZE);
        pDoc->SetDefault( aFontHeight );
        SvxFontHeightItem aFontHeightCJK(aFontHeights[2], 100, RES_CHRATR_CJK_FONTSIZE);
        pDoc->SetDefault( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL(aFontHeights[2], 100, RES_CHRATR_CTL_FONTSIZE);
        pDoc->SetDefault( aFontHeightCTL );

        
        
        pDoc->SetDefault( SwFmtFollowTextFlow(true) );
    }

    
    
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

    
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if( pDocSh )
    {
        bViewCreated = true;            

        

        if( pMed )
        {
            sJmpMark = pMed->GetURLObject().GetMark();
            if( !sJmpMark.isEmpty() )
            {
                eJumpTo = JUMPTO_MARK;
                sal_Int32 nLastPos = sJmpMark.lastIndexOf( cMarkSeparator );
                sal_Int32 nPos =  nLastPos != -1 ? nLastPos : 0;

                OUString sCmp;
                if (nPos)
                {
                    sCmp = comphelper::string::remove(
                        sJmpMark.copy(nPos + 1), ' ');
                }

                if( !sCmp.isEmpty() )
                {
                    sCmp = sCmp.toAsciiLowerCase();
                    if( sCmp == "region" )
                        eJumpTo = JUMPTO_REGION;
                    else if( sCmp == "table" )
                        eJumpTo = JUMPTO_TABLE;
                    else if( sCmp == "graphic" )
                        eJumpTo = JUMPTO_GRAPHIC;
                    else if( sCmp == "outline" ||
                            sCmp == "text" ||
                            sCmp == "frame" )
                        eJumpTo = JUMPTO_NONE;  
                    else
                        
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
    pDoc->SetInLoadAsynchron( false );
    pDoc->set(IDocumentSettingAccess::HTML_MODE, bOldIsHTMLMode);

    if( pDoc->GetDocShell() && nEventId )
        Application::RemoveUserEvent( nEventId );

    
    
    if( pDoc->GetDocShell() )
    {
        
        sal_uInt16 nLinkMode = pDoc->getLinkUpdateMode( true );
        if( nLinkMode != NEVER && bAsync &&
            SFX_CREATE_MODE_INTERNAL!=pDoc->GetDocShell()->GetCreateMode() )
            pDoc->GetLinkManager().UpdateAllLinks( nLinkMode == MANUAL,
                                                   true, false );

        if ( pDoc->GetDocShell()->IsLoading() )
        {
            
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
        
        delete pDoc;
        pDoc = NULL;
    }

    if ( pTempViewFrame )
    {
        pTempViewFrame->DoClose();

        
        if ( bRemoveHidden && pDoc && pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() )
            pDoc->GetDocShell()->GetMedium()->GetItemSet()->ClearItem( SID_HIDDEN );
    }
}

IMPL_LINK( SwHTMLParser, AsyncCallback, void*, /*pVoid*/ )
{
    nEventId=0;

    
    
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        
        eState = SVPAR_ERROR;
    }

    GetAsynchCallLink().Call(0);
    return 0;
}

SvParserState SwHTMLParser::CallParser()
{
    
    pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
    if( !IsNewDoc() )       
    {
        const SwPosition* pPos = pPam->GetPoint();

        pDoc->SplitNode( *pPos, false );

        *pSttNdIdx = pPos->nNode.GetIndex()-1;
        pDoc->SplitNode( *pPos, false );

        SwPaM aInsertionRangePam( *pPos );

        pPam->Move( fnMoveBackward );

        
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
            bViewCreated = true;
            nEventId = 0;
        }
    }

    
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

    
    
    
    OSL_ENSURE( SVPAR_ERROR!=eState,
            "SwHTMLParser::Continue: bereits ein Fehler gesetzt" );
    if( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        eState = SVPAR_ERROR;

    
    SwViewShell *pInitVSh = CallStartAction();

    if( SVPAR_ERROR != eState && GetMedium() && !bViewCreated )
    {
        
        
        
        
        eState = SVPAR_PENDING;
        bViewCreated = true;
        pDoc->SetInLoadAsynchron( true );

#ifdef DBG_UTIL
        m_nContinue--;
#endif

        return;
    }

    bSetModEnabled = false;
    if( pDoc->GetDocShell() &&
        (bSetModEnabled = pDoc->GetDocShell()->IsEnableSetModified()) )
    {
        pDoc->GetDocShell()->EnableSetModified( sal_False );
    }

    
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    sal_Bool bModified = pDoc->IsModified();
    bool const bWasUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);

    
    
    
    
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

    
    EndProgress( pDoc->GetDocShell() );

    sal_Bool bLFStripped = sal_False;
    if( SVPAR_PENDING != GetStatus() )
    {
        
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

            
            if( IsNewDoc() )
                bLFStripped = StripTrailingLF() > 0;

            
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

            
            pCSS1Parser->SetDelayedStyles();
        }

        
        if( !IsNewDoc() && pSttNdIdx->GetIndex() )
        {
            SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
            SwNodeIndex aNxtIdx( *pSttNdIdx );
            if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
            {
                const sal_Int32 nStt = pTxtNode->GetTxt().getLength();
                
                if( pPam->GetPoint()->nNode == aNxtIdx )
                {
                    pPam->GetPoint()->nNode = *pSttNdIdx;
                    pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
                }

#if OSL_DEBUG_LEVEL > 0

OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( true ).nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
OSL_ENSURE( pSttNdIdx->GetIndex()+1 != pPam->GetBound( false ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( true ).nNode.GetIndex() )
{
    const sal_Int32 nCntPos = pPam->GetBound( true ).nContent.GetIndex();
    pPam->GetBound( true ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().getLength() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( false ).nNode.GetIndex() )
{
    const sal_Int32 nCntPos = pPam->GetBound( false ).nContent.GetIndex();
    pPam->GetBound( false ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().getLength() + nCntPos );
}
#endif
                
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
            
            
            ConnectImageMaps();
        }

        
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
                        SwViewShell *pVSh = CheckActionViewShell();
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
                        pPam->GetBound(true).nContent.Assign( 0, 0 );
                        pPam->GetBound(false).nContent.Assign( 0, 0 );
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

        
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     
                pPam->Move( fnMoveForward, fnGoNode );  
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                
                
                

                
                
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->ResetAllAttr();

                if( pPrev->HasSwAttrSet() )
                    pTxtNode->SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(true).nNode.GetNode() == pPrev )
                    pPam->GetBound(true).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(false).nNode.GetNode() == pPrev )
                    pPam->GetBound(false).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }

        
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

    
    
    if( 1 < pDoc->getReferenceCount() )
    {
        if( bWasUndo )
        {
            pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
            pDoc->GetIDocumentUndoRedo().DoUndo(true);
        }
        else if( !pInitVSh )
        {
            
            
            
            
            SwViewShell *pTmpVSh = CheckActionViewShell();
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
            bSetModEnabled = false; 
        }
    }


    
    
    
    
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
            
            GetRegisteredInNonConst()->Remove( this );
            ReleaseRef();                   
        }
        break;
    }
}

void SwHTMLParser::DocumentDetected()
{
    OSL_ENSURE( !bDocInitalized, "DocumentDetected mehrfach aufgerufen" );
    bDocInitalized = true;
    if( IsNewDoc() )
    {
        if( IsInHeader() )
            FinishHeader( true );

        CallEndAction( sal_True, sal_True );

        pDoc->GetIDocumentUndoRedo().DoUndo(false);
        
        
        
        
        CallStartAction();
    }
}


void SwHTMLParser::NextToken( int nToken )
{
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        
        
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
            
        case HTML_TABLE_ON:
            
            
        case HTML_LINK:
            
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
                bInTitle = false;
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

    
    
    
    if( !bDocInitalized )
        DocumentDetected();

    sal_Bool bGetIDOption = sal_False, bInsertUnknown = sal_False;
    sal_Bool bUpperSpaceSave = bUpperSpace;
    bUpperSpace = false;

    
    
    if( !pPendStack )
    {
        if( bInFloatingFrame )
        {
            
            
            if( HTML_IFRAME_OFF == nToken )
            {
                bCallNextToken = false;
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
                bCallNextToken = false;
                bInNoEmbed = false;
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
            
            
            

            switch( nToken )
            {
            case HTML_APPLET_OFF:
                bCallNextToken = false;
                EndApplet();
                break;
            case HTML_OBJECT_OFF:
                bCallNextToken = false;
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
            
            

            switch( nToken )
            {
            case HTML_TEXTAREA_OFF:
                bCallNextToken = false;
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
            
            switch( nToken )
            {
            case HTML_SELECT_OFF:
                bCallNextToken = false;
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
                
                break;

            default:
                
                return;
            }
        }
        else if( pMarquee )
        {
            
            
            
            switch( nToken )
            {
            case HTML_MARQUEE_OFF:
                bCallNextToken = false;
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
                bCallNextToken = false;
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
                bCallNextToken = false;
                break;

            case HTML_TEXTTOKEN:
                InsertFootEndNoteText();
                break;
            }
            return;
        }
        else if( !aUnknownToken.isEmpty() )
        {
            
            
            if (!aToken.isEmpty() && !IsInHeader() )
            {
                if( !bDocInitalized )
                    DocumentDetected();
                pDoc->InsertString( *pPam, aToken );

                
                
                
                if( !aParaAttrs.empty() )
                    aParaAttrs.clear();

                SetAttr();
            }

            
            
            switch( nToken )
            {
            case HTML_UNKNOWNCONTROL_OFF:
                if( aUnknownToken != sSaveToken )
                    return;
            case HTML_FRAMESET_ON:
            case HTML_HEAD_OFF:
            case HTML_BODY_ON:
            case HTML_IMAGE:        
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
        bInTitle = true;
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
#if HAVE_FEATURE_JAVA
        NewObject();
        bCallNextToken = pAppletImpl!=0 && pTable!=0;
#endif
        break;

    case HTML_APPLET_ON:
#if HAVE_FEATURE_JAVA
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
            

    case HTML_NEWPARA:
        
        {
            if( HTML_NEWPARA==nToken ||
                pPam->GetPoint()->nContent.GetIndex() )
            {
                AppendTxtNode(); 
                                 
                SetTxtCollAttrs();
            }
            
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
        
        if( !aToken.isEmpty() && ' '==aToken[0] && !IsReadPRE() )
        {
            sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
            if( nPos )
            {
                const OUString& rText =
                    pPam->GetPoint()->nNode.GetNode().GetTxtNode()->GetTxt();
                sal_Unicode cLast = rText[--nPos];
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
        bInNoEmbed = true;
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
        EndDefListItem( 0, sal_False );
        NewDefListItem( nToken );
        break;

    case HTML_DD_OFF:
    case HTML_DT_OFF:
        
        
        
        EndDefListItem( nToken, sal_False );
        break;

    
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
        bNoParSpace = true; 
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
                    Show();     

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
            
            EndPara();
        }

        EndNumBulListItem( 0, sal_False );
        NewNumBulListItem( nToken );
        break;

    case HTML_LI_OFF:
    case HTML_LISTHEADER_OFF:
        EndNumBulListItem( nToken, sal_False );
        break;

    
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
        break;      
    case HTML_HTML_ON:
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                if( HTML_O_DIR == rOption.GetToken() )
                {
                    const OUString& rDir = rOption.GetString();
                    SfxItemSet aItemSet( pDoc->GetAttrPool(),
                                         pCSS1Parser->GetWhichMap() );
                    SvxCSS1PropertyInfo aPropInfo;
                    OUString aDummy;
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
            FinishHeader( true );
        bInsertUnknown = bKeepUnknown;
        break;

    case HTML_UNKNOWNCONTROL_ON:
        
        
        
        
        if( IsInHeader() && !IsReadPRE() && aUnknownToken.isEmpty() &&
            !sSaveToken.isEmpty() && '!' != sSaveToken[0] &&
            '%' != sSaveToken[0] )
            aUnknownToken = sSaveToken;
        

    default:
        bInsertUnknown = bKeepUnknown;
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
    
    
    sal_Int32 nLFStripped = StripTrailingLF();
    if( (AM_NOSPACE==eMode || AM_SOFTNOSPACE==eMode) && nLFStripped > 1 )
        eMode = AM_SPACE;

    
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

    
    
    const SwNodeIndex& rEndIdx = aOldPos.nNode;
    const sal_Int32 nEndCnt = aOldPos.nContent.GetIndex();
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

                    sal_Int32 nStt = pAttr->nSttCntnt;
                    sal_Bool bScript = sal_False, bFont = sal_False;
                    sal_uInt16 nScriptItem;
                    sal_Bool bInsert = sal_True;
                       lcl_swhtml_getItemInfo( *pAttr, bScript, bFont,
                                            nScriptItem );
                        
                    if( bInsert && bScript )
                    {
                        const SwTxtNode *pTxtNd =
                            pAttr->GetSttPara().GetNode().GetTxtNode();
                        OSL_ENSURE( pTxtNd, "No text node" );
                        if( pTxtNd )
                        {
                            const OUString& rText = pTxtNd->GetTxt();
                            sal_uInt16 nScriptTxt =
                                g_pBreakIt->GetBreakIter()->getScriptType(
                                            rText, pAttr->GetSttCnt() );
                            sal_Int32 nScriptEnd = g_pBreakIt->GetBreakIter()
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
                                nScriptEnd = g_pBreakIt->GetBreakIter()
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

    
    SetAttr();

    
    
    SwTxtNode *pTxtNd = rEndIdx.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "There is the txt node" );
    sal_uInt16 nCntAttr = (pTxtNd  && pTxtNd->GetpSwpHints())
                            ? pTxtNd->GetSwpHints().Count() : 0;
    if( nCntAttr )
    {
        
        
        
        
        
        
        
        sal_Int32 aEndPos[15] =
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        SwpHints& rHints = pTxtNd->GetSwpHints();
        for( sal_uInt16 i=0; i < nCntAttr; i++ )
        {
            SwTxtAttr *pHt = rHints.GetTextHint( i );
            sal_uInt16 nWhich = pHt->Which();
            sal_Int16 nIdx = -1;
            
            
            
            
            
            
            

            
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
                sal_Int32 nStt = *pHt->GetStart();
                if( nStt >= aEndPos[nIdx] )
                {
                    sal_Bool bFont = (nIdx % 5) == 0;
                    const SfxPoolItem& rItem =
                        ((const SwCntntNode *)pTxtNd)->GetAttr( nWhich );
                    if( bFont ? swhtml_css1atr_equalFontItems(rItem,pHt->GetAttr())
                              : rItem == pHt->GetAttr() )
                    {
                        
                        
                        
                        
                        
                        
                        pTxtNd->DeleteAttribute( pHt );
                        if( 1 == nCntAttr )
                            break;
                        i--;
                        nCntAttr--;
                    }
                    else
                    {
                        
                        
                        aEndPos[nIdx] = pHt->GetEnd() ? *pHt->GetEnd() : nStt;
                    }
                }
                else
                {
                    
                    
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
    
    if( !bNoParSpace )
        return;

    bNoParSpace = false;

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
    
    
    
    
    

    OSL_ENSURE( SVPAR_WORKING==eState, "Show nicht im Working-State - Das kann ins Auge gehen" );
    SwViewShell *pOldVSh = CallEndAction();

    GetpApp()->Reschedule();

    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        
        eState = SVPAR_ERROR;
    }

    
    
    SwViewShell *pVSh = CallStartAction( pOldVSh );

    
    
    if( pVSh )
    {
        nParaCnt = (pPam->GetPoint()->nNode.GetNode().IsInVisibleArea(pVSh))
            ? 5 : 50;
    }
}

void SwHTMLParser::ShowStatline()
{
    
    
    
    

    OSL_ENSURE( SVPAR_WORKING==eState, "ShowStatLine nicht im Working-State - Das kann ins Auge gehen" );

    
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
            
            eState = SVPAR_ERROR;

        SwViewShell *pVSh = CheckActionViewShell();
        if( pVSh && pVSh->HasInvalidRect() )
        {
            CallEndAction( sal_False, sal_False );
            CallStartAction( pVSh, sal_False );
        }
    }
}

SwViewShell *SwHTMLParser::CallStartAction( SwViewShell *pVSh, sal_Bool bChkPtr )
{
    OSL_ENSURE( !pActionViewShell, "CallStartAction: SwViewShell schon gesetzt" );

    if( !pVSh || bChkPtr )
    {
#if OSL_DEBUG_LEVEL > 0
        SwViewShell *pOldVSh = pVSh;
#endif
        pDoc->GetEditShell( &pVSh );
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !pVSh || !pOldVSh || pOldVSh == pVSh, "CallStartAction: Wer hat die SwViewShell ausgetauscht?" );
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

SwViewShell *SwHTMLParser::CallEndAction( sal_Bool bChkAction, sal_Bool bChkPtr )
{
    if( bChkPtr )
    {
        SwViewShell *pVSh = 0;
        pDoc->GetEditShell( &pVSh );
        OSL_ENSURE( !pVSh || pActionViewShell == pVSh,
                "CallEndAction: Wer hat die SwViewShell ausgetauscht?" );
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
        
        SwViewShell *pSh = pActionViewShell;
        do {
            if( pSh->IsA( TYPE( SwCrsrShell ) ) )
                ((SwCrsrShell*)pSh)->SttEndDoc(sal_True);
            pSh = (SwViewShell *)pSh->GetNext();
        } while( pSh != pActionViewShell );

        bSetCrsr = false;
    }
    if( pActionViewShell->ISA( SwEditShell ) )
    {
        
        const sal_Bool bOldLock = pActionViewShell->IsViewLocked();
        pActionViewShell->LockView( sal_True );
        const sal_Bool bOldEndActionByVirDev = pActionViewShell->IsEndActionByVirDev();
        pActionViewShell->SetEndActionByVirDev( sal_True );;
        ((SwEditShell*)pActionViewShell)->EndAction();
        pActionViewShell->SetEndActionByVirDev( bOldEndActionByVirDev );
        pActionViewShell->LockView( bOldLock );

        
        if( bChkJumpMark )
        {
            const Point aVisSttPos( DOCUMENTBORDER, DOCUMENTBORDER );
            if( GetMedium() && aVisSttPos == pActionViewShell->VisArea().Pos() )
                ::JumpToSwMark( pActionViewShell,
                                GetMedium()->GetURLObject().GetMark() );
            bChkJumpMark = false;
        }
    }
    else
        pActionViewShell->EndAction();

    
    
    if( 1 == pDoc->getReferenceCount() )
    {
        eState = SVPAR_ERROR;
    }

    SwViewShell *pVSh = pActionViewShell;
    pActionViewShell = 0;

    return pVSh;
}

SwViewShell *SwHTMLParser::CheckActionViewShell()
{
    SwViewShell *pVSh = 0;
    pDoc->GetEditShell( &pVSh );
    OSL_ENSURE( !pVSh || pActionViewShell == pVSh,
            "CheckActionViewShell: Wer hat die SwViewShell ausgetauscht?" );
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
    const sal_Int32 nEndCnt = pPam->GetPoint()->nContent.GetIndex();
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
            
            
            
            sal_uLong nEndOfIcons = pDoc->GetNodes().GetEndOfExtras().GetIndex();
            bSetAttr = nEndParaIdx < rEndIdx.GetIndex() ||
                       rEndIdx.GetIndex() > nEndOfIcons ||
                       nEndParaIdx <= nEndOfIcons;
        }

        if( bSetAttr )
        {
            
            
            while( !aParaAttrs.empty() )
            {
                OSL_ENSURE( pAttr != aParaAttrs.back(),
                        "SetAttr: Attribut duerfte noch nicht gesetzt werden" );
                aParaAttrs.pop_back();
            }


            
            aSetAttrTab.erase( aSetAttrTab.begin() + n );

            while( pAttr )
            {
                _HTMLAttr *pPrev = pAttr->GetPrev();
                if( !pAttr->bValid )
                {
                    
                    delete pAttr;
                    pAttr = pPrev;
                    continue; 
                }


                pCNd = pAttr->nSttPara.GetNode().GetCntntNode();
                if( !pCNd )
                {
                    
                    
                    if ( (pAttr->GetSttPara() == pAttr->GetEndPara()) &&
                         !isTXTATR_NOEND(nWhich) )
                    {
                        
                        
                        
                        delete pAttr;
                        pAttr = pPrev;
                        continue; 
                    }
                    pCNd = pDoc->GetNodes().GoNext( &(pAttr->nSttPara) );
                    if( pCNd )
                        pAttr->nSttCntnt = 0;
                    else
                    {
                        OSL_ENSURE( !this, "SetAttr: GoNext() failed!" );
                        delete pAttr;
                        pAttr = pPrev;
                        continue; 
                    }
                }
                pAttrPam->GetPoint()->nNode = pAttr->nSttPara;



                
                
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
                            continue; 
                        }
                    }

                    pAttrPam->GetPoint()->nNode = pAttr->nEndPara;
                }
                else if( pAttr->IsLikePara() )
                {
                    pAttr->nEndCntnt = pCNd->Len();
                }

                
                
                if( pAttr->nEndCntnt > pCNd->Len() )
                    pAttr->nEndCntnt = pCNd->Len();

                pAttrPam->GetPoint()->nContent.Assign( pCNd, pAttr->nEndCntnt );
                if( bBeforeTable &&
                    pAttrPam->GetPoint()->nNode.GetIndex() ==
                        rEndIdx.GetIndex() )
                {
                    
                    
                    
                    
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
                case RES_FLTR_BOOKMARK: 
                    {
                        const OUString sName( ((SfxStringItem*)pAttr->pItem)->GetValue() );
                        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
                        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark( sName );
                        if( ppBkmk != pMarkAccess->getAllMarksEnd() &&
                            ppBkmk->get()->GetMarkStart() == *pAttrPam->GetPoint() )
                            break; 
                        pAttrPam->DeleteMark();
                        const ::sw::mark::IMark* const pNewMark = pMarkAccess->makeMark(
                            *pAttrPam,
                            sName,
                            IDocumentMarkAccess::BOOKMARK );

                        
                        if( JUMPTO_MARK == eJumpTo && pNewMark->GetName() == sJmpMark )
                        {
                            bChkJumpMark = true;
                            eJumpTo = JUMPTO_NONE;
                        }
                    }
                    break;
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                case RES_TXTATR_INPUTFIELD:
                    {
                        sal_uInt16 nFldWhich =
                            pPostIts
                            ? ((const SwFmtFld *)pAttr->pItem)->GetField()->GetTyp()->Which()
                            : 0;
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
                        
                        
                        pCNd->SetAttr( *pAttr->pItem );
                        break;
                    }
                    OSL_ENSURE( !this,
                            "LRSpace ueber mehrere Absaetze gesetzt!" );
                    
                default:

                    
                    if( RES_TXTATR_INETFMT == nWhich &&
                        JUMPTO_MARK == eJumpTo &&
                        sJmpMark == ((SwFmtINetFmt*)pAttr->pItem)->GetName() )
                    {
                        bChkJumpMark = true;
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
    
    _HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    
    const SwNodeIndex* pEndIdx = &pPam->GetPoint()->nNode;
    sal_Int32 nEndCnt = pPam->GetPoint()->nContent.GetIndex();

    
    
    _HTMLAttr *pLast = 0;
    if( ppHead && pAttr != *ppHead )
    {
        

        
        
        
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
        
        bMoveBack = pPam->Move( fnMoveBackward );
        nEndCnt = pPam->GetPoint()->nContent.GetIndex();
    }

    
    _HTMLAttr *pNext = pAttr->GetNext();


    sal_Bool bInsert;
    sal_uInt16 nScriptItem = 0;
    sal_Bool bScript = sal_False, bFont = sal_False;
    
    if( !bChkEmpty || (RES_PARATR_BEGIN <= nWhich && bMoveBack) ||
        RES_PAGEDESC == nWhich || RES_BREAK == nWhich ||
        *pEndIdx != pAttr->GetSttPara() ||
        nEndCnt != pAttr->GetSttCnt() )
    {
        bInsert = sal_True;
        
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
        const OUString& rText = pTxtNd->GetTxt();
        sal_uInt16 nScriptTxt = g_pBreakIt->GetBreakIter()->getScriptType(
                        rText, pAttr->GetSttCnt() );
        sal_Int32 nScriptEnd = g_pBreakIt->GetBreakIter()
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
            nScriptEnd = g_pBreakIt->GetBreakIter()
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
            
            
            
            
            pNext->InsertPrev( pAttr );
        }
    }
    else
    {
        
        
        
        
        _HTMLAttr *pPrev = pAttr->GetPrev();
        delete pAttr;

        if( pPrev )
        {
            
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

    
    
    if( pLast )
        pLast->pNext = pNext;
    else if( ppHead )
        *ppHead = pNext;

    if( bMoveBack )
        pPam->Move( fnMoveForward );
}

void SwHTMLParser::DeleteAttr( _HTMLAttr* pAttr )
{
    
    
    OSL_ENSURE(aParaAttrs.empty(),
        "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    
    _HTMLAttr **ppHead = pAttr->ppHead;

    OSL_ENSURE( ppHead, "keinen Attributs-Listenkopf gefunden!" );

    
    
    _HTMLAttr *pLast = 0;
    if( ppHead && pAttr != *ppHead )
    {
        

        
        
        
        pLast = *ppHead;
        while( pLast && pLast->GetNext() != pAttr )
            pLast = pLast->GetNext();

        OSL_ENSURE( pLast, "Attribut nicht in eigener Liste gefunden!" );
    }

    
    _HTMLAttr *pNext = pAttr->GetNext();
    _HTMLAttr *pPrev = pAttr->GetPrev();
    delete pAttr;

    if( pPrev )
    {
        
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

    
    
    if( pLast )
        pLast->pNext = pNext;
    else if( ppHead )
        *ppHead = pNext;
}

void SwHTMLParser::SaveAttrTab( _HTMLAttrTable& rNewAttrTab )
{
    
    
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
    
    
    OSL_ENSURE(aParaAttrs.empty(),
            "Danger: there are non-final paragraph attributes");
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    const SwNodeIndex& nSttIdx = pPam->GetPoint()->nNode;
    SwNodeIndex nEndIdx( nSttIdx );

    
    
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    _HTMLAttr** pSaveTbl = (_HTMLAttr**)&rNewAttrTab;
    sal_Bool bSetAttr = sal_True;
    const sal_Int32 nSttCnt = pPam->GetPoint()->nContent.GetIndex();
    sal_Int32 nEndCnt = nSttCnt;

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
        const sal_Int32 nSttCnt = pPos->nContent.GetIndex();

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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    
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

    
    PushContext( pCntxt );
}

void SwHTMLParser::NewStdAttr( int nToken,
                               _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                               _HTMLAttr **ppAttr2, const SfxPoolItem *pItem2,
                               _HTMLAttr **ppAttr3, const SfxPoolItem *pItem3 )
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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    
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

    
    PushContext( pCntxt );
}

void SwHTMLParser::EndTag( int nToken )
{
    
    _HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );
    if( pCntxt )
    {
        
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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_BASEFONT_ON );

    
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        
        SvxFontHeightItem aFontHeight( aFontHeights[nSize-1], 100, RES_CHRATR_FONTSIZE );
        aItemSet.Put( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( aFontHeights[nSize-1], 100, RES_CHRATR_CJK_FONTSIZE );
        aItemSet.Put( aFontHeightCJK );
        
        
        
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

    
    PushContext( pCntxt );

    
    aBaseFontStack.push_back( nSize );
}

void SwHTMLParser::EndBasefontAttr()
{
    EndTag( HTML_BASEFONT_ON );

    
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

    OUString aFace, aId, aStyle, aClass, aLang, aDir;
    Color aColor;
    sal_uLong nFontHeight = 0;  
    sal_uInt16 nSize = 0;       
    sal_Bool bColor = sal_False;

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
        

        
        
        sal_uInt16 nPoolId = GetCurrFmtColl()->GetPoolFmtId();
        if( (nPoolId>=RES_POOLCOLL_HEADLINE1 &&
             nPoolId<=RES_POOLCOLL_HEADLINE6) )
        {
            
            
            if( nFontStHeadStart==aFontStack.size() )
                nFontSize = static_cast< sal_uInt16 >(6 - (nPoolId - RES_POOLCOLL_HEADLINE1));
        }
        else
            nPoolId = 0;

        if( HTML_BIGPRINT_ON == nToken )
            nSize = ( nFontSize<7 ? nFontSize+1 : 7 );
        else
            nSize = ( nFontSize>1 ? nFontSize-1 : 1 );

        
        
        if( nPoolId && nSize>=1 && nSize <=6 )
            nFontHeight =
                pCSS1Parser->GetTxtCollFromPool(
                    RES_POOLCOLL_HEADLINE1+6-nSize )->GetSize().GetHeight();
        else
            nFontHeight = aFontHeights[nSize-1];
    }

    OSL_ENSURE( !nSize == !nFontHeight, "HTML-Font-Size != Font-Height" );

    OUString aFontName, aStyleName;
    FontFamily eFamily = FAMILY_DONTKNOW;   
    FontPitch ePitch = PITCH_DONTKNOW;      
    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();

    if( !aFace.isEmpty() && !pCSS1Parser->IsIgnoreFontFamily() )
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
            OUString aFName = aFace.getToken( 0, ',', nStrPos );
            aFName = comphelper::string::strip(aFName, ' ');
            if( !aFName.isEmpty() )
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
                if( !aFontName.isEmpty() )
                    aFontName += ";";
                aFontName += aFName;
            }
        }
    }


    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    
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
        if( !aFontName.isEmpty() )
        {
            SvxFontItem aFont( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_FONT );
            InsertAttr( &aAttrTab.pFont, aFont, pCntxt );
            SvxFontItem aFontCJK( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CJK_FONT );
            InsertAttr( &aAttrTab.pFont, aFontCJK, pCntxt );
            SvxFontItem aFontCTL( eFamily, aFontName, aStyleName, ePitch, eEnc, RES_CHRATR_CTL_FONT );
            InsertAttr( &aAttrTab.pFont, aFontCTL, pCntxt );
        }
    }

    
    PushContext( pCntxt );

    aFontStack.push_back( nSize );
}

void SwHTMLParser::EndFontAttr( int nToken )
{
    EndTag( nToken );

    
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

    
    _HTMLAttrContext *pCntxt =
        !aClass.isEmpty() ? new _HTMLAttrContext( HTML_PARABREAK_ON,
                                             RES_POOLCOLL_TEXT, aClass )
                     : new _HTMLAttrContext( HTML_PARABREAK_ON );

    
    
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    
    PushContext( pCntxt );

    
    SetTxtCollAttrs( !aClass.isEmpty() ? pCntxt : 0 );

    
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

    
    if( bReal )
    {
        if( pPam->GetPoint()->nContent.GetIndex() )
            AppendTxtNode( AM_SPACE );
        else
            AddParSpace();
    }

    
    
    if( (nOpenParaToken==HTML_DT_ON || nOpenParaToken==HTML_DD_ON) &&
        nDefListDeep)
    {
        nDefListDeep--;
    }

    
    
    _HTMLAttrContext *pCntxt =
        PopContext( static_cast< sal_uInt16 >(nOpenParaToken ? (nOpenParaToken & ~1)
                                   : HTML_PARABREAK_ON) );

    
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  
        delete pCntxt;
    }

    
    if( bReal )
        SetTxtCollAttrs();

    nOpenParaToken = 0;
}


void SwHTMLParser::NewHeading( int nToken )
{
    eParaAdjust = SVX_ADJUST_END;

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

    
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    
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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nTxtColl, aClass );

    
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( SVX_ADJUST_END != eParaAdjust )
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eParaAdjust, RES_PARATR_ADJUST), pCntxt );

    
    PushContext( pCntxt );

    
    SetTxtCollAttrs( pCntxt );

    nFontStHeadStart = aFontStack.size();

    
    ShowStatline();
}

void SwHTMLParser::EndHeading()
{
    
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SPACE );
    else
        AddParSpace();

    
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

    
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  
        delete pCntxt;
    }

    
    SetTxtCollAttrs();

    nFontStHeadStart = nFontStMin;
}


void SwHTMLParser::NewTxtFmtColl( int nToken, sal_uInt16 nColl )
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

    
    SwHTMLAppendMode eMode = AM_NORMAL;
    switch( nToken )
    {
    case HTML_LISTING_ON:
    case HTML_XMP_ON:
        
        
        
        aClass = aEmptyOUStr;
    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE30_ON:
    case HTML_PREFORMTXT_ON:
        eMode = AM_SPACE;
        break;
    case HTML_ADDRESS_ON:
        eMode = AM_NOSPACE; 
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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken), nColl, aClass );

    
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    
    SetTxtCollAttrs( pCntxt );

    
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

    
    _HTMLAttrContext *pCntxt = PopContext( static_cast< sal_uInt16 >(nToken & ~1) );

    
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  
        delete pCntxt;
    }

    
    SetTxtCollAttrs();
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

    
    sal_Bool bSpace = (GetNumInfo().GetDepth() + nDefListDeep) == 0;
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( bSpace ? AM_SPACE : AM_SOFTNOSPACE );
    else if( bSpace )
        AddParSpace();

    
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


    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_DEFLIST_ON );

    
    sal_uInt16 nLeft=0, nRight=0;
    short nIndent=0;
    GetMarginsFromContext( nLeft, nRight, nIndent );

    
    
    
    
    if( !bInDD && nDefListDeep > 1 )
    {

        
        SvxLRSpaceItem rLRSpace =
            pCSS1Parser->GetTxtFmtColl( RES_POOLCOLL_HTML_DD, aEmptyOUStr )
                       ->GetLRSpace();
        nLeft = nLeft + static_cast< sal_uInt16 >(rLRSpace.GetTxtLeft());
    }

    pCntxt->SetMargins( nLeft, nRight, nIndent );

    
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

    
    if( nDefListDeep > 0 )
        nDefListDeep--;

    
    _HTMLAttrContext *pCntxt = PopContext( HTML_DEFLIST_ON );

    
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  
        delete pCntxt;
    }

    
    SetTxtCollAttrs();
}

void SwHTMLParser::NewDefListItem( int nToken )
{
    
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
    
    if( !nToken && pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_SOFTNOSPACE );

    
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
            
        case HTML_DIRLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
            
            nPos = nContextStMin;
            break;
        }
    }

    
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  
        delete pCntxt;
    }

    
    if( bSetColl )
        SetTxtCollAttrs();
}


sal_Bool SwHTMLParser::HasCurrentParaFlys( sal_Bool bNoSurroundOnly,
                                       sal_Bool bSurroundOnly ) const
{
    
    
    
    
    
    SwNodeIndex& rNodeIdx = pPam->GetPoint()->nNode;

    const SwFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

    sal_Bool bFound = sal_False;
    for ( sal_uInt16 i=0; i<rFrmFmtTbl.size(); i++ )
    {
        SwFrmFmt *const pFmt = rFrmFmtTbl[i];
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        
        
        
        
        
        
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
                        
                        
                    }
                }
            }
        }
    }

    return bFound;
}




const SwFmtColl *SwHTMLParser::GetCurrFmtColl() const
{
    const SwCntntNode* pCNd = pPam->GetCntntNode();
    return &pCNd->GetAnyFmtColl();
}


void SwHTMLParser::SetTxtCollAttrs( _HTMLAttrContext *pContext )
{
    SwTxtFmtColl *pCollToSet = 0;   
    SfxItemSet *pItemSet = 0;       
    sal_uInt16 nTopColl = pContext ? pContext->GetTxtFmtColl() : 0;
    const OUString& rTopClass = pContext ? pContext->GetClass() : aEmptyOUStr;
    sal_uInt16 nDfltColl = RES_POOLCOLL_TEXT;

    sal_Bool bInPRE=sal_False;                          

    sal_uInt16 nLeftMargin = 0, nRightMargin = 0;   
    short nFirstLineIndent = 0;                 
    sal_uInt16 i;

    for( i = nContextStAttrMin; i < aContexts.size(); i++ )
    {
        const _HTMLAttrContext *pCntxt = aContexts[i];

        sal_uInt16 nColl = pCntxt->GetTxtFmtColl();
        if( nColl )
        {
            
            
            
            sal_Bool bSetThis = sal_True;
            switch( nColl )
            {
            case sal_uInt16(RES_POOLCOLL_HTML_PRE):
                bInPRE = sal_True;
                break;
            case sal_uInt16(RES_POOLCOLL_TEXT):
                
                if( nDfltColl==RES_POOLCOLL_TABLE ||
                    nDfltColl==RES_POOLCOLL_TABLE_HDLN )
                    nColl = nDfltColl;
                break;
            case sal_uInt16(RES_POOLCOLL_HTML_HR):
                
                
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
                
                
                

                if( pCollToSet )
                {
                    
                    
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
                    
                    
                    
                    pItemSet->Differentiate( pNewColl->GetAttrSet() );
                }

                pCollToSet = pNewColl;
            }
            else
            {
                
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
            
            nColl = pCntxt->GetDfltTxtFmtColl();
            if( nColl )
                nDfltColl = nColl;
        }

        
        if( pCntxt->IsLRSpaceChanged() )
        {
            sal_uInt16 nLeft=0, nRight=0;

            pCntxt->GetMargins( nLeft, nRight, nFirstLineIndent );
            nLeftMargin = nLeft;
            nRightMargin = nRight;
        }
    }

    
    
    if( pContext && nTopColl )
    {
        
        if( nTopColl==RES_POOLCOLL_TEXT &&
            (nDfltColl==RES_POOLCOLL_TABLE ||
             nDfltColl==RES_POOLCOLL_TABLE_HDLN) )
            nTopColl = nDfltColl;

        const SwTxtFmtColl *pTopColl =
            pCSS1Parser->GetTxtFmtColl( nTopColl, rTopClass );
        const SfxItemSet& rItemSet = pTopColl->GetAttrSet();
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,true, &pItem) )
        {
            const SvxLRSpaceItem *pLRItem =
                (const SvxLRSpaceItem *)pItem;

            sal_Int32 nLeft = pLRItem->GetTxtLeft();
            sal_Int32 nRight = pLRItem->GetRight();
            nFirstLineIndent = pLRItem->GetTxtFirstLineOfst();

            
            
            if( RES_POOLCOLL_HTML_DD == nTopColl )
            {
                const SvxLRSpaceItem& rDTLRSpace = pCSS1Parser
                    ->GetTxtFmtColl( RES_POOLCOLL_HTML_DT, aEmptyOUStr )
                    ->GetLRSpace();
                nLeft -= rDTLRSpace.GetTxtLeft();
                nRight -= rDTLRSpace.GetRight();
            }
            else if( RES_POOLCOLL_HTML_DT == nTopColl )
            {
                nLeft = 0;
                nRight = 0;
            }

            
            nLeftMargin = nLeftMargin + static_cast< sal_uInt16 >(nLeft);
            nRightMargin = nRightMargin + static_cast< sal_uInt16 >(nRight);

            pContext->SetMargins( nLeftMargin, nRightMargin,
                                  nFirstLineIndent );
        }
        if( SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,true, &pItem) )
        {
            const SvxULSpaceItem *pULItem =
                (const SvxULSpaceItem *)pItem;
            pContext->SetULSpace( pULItem->GetUpper(), pULItem->GetLower() );
        }
    }

    
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

    
    if( !aParaAttrs.empty() )
    {
        for( i=0; i<aParaAttrs.size(); i++ )
            aParaAttrs[i]->Invalidate();

        aParaAttrs.clear();
    }

    
    pDoc->SetTxtFmtColl( *pPam, pCollToSet );

    
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

    
    if( pItemSet )
    {
        InsertParaAttrs( *pItemSet );
        delete pItemSet;
    }
}


void SwHTMLParser::NewCharFmt( int nToken )
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

    
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    
    SwCharFmt* pCFmt = pCSS1Parser->GetChrFmt( static_cast< sal_uInt16 >(nToken), aClass );
    OSL_ENSURE( pCFmt, "keine Zeichenvorlage zu Token gefunden" );


    
    if( HasStyleOptions( aStyle, aId, aEmptyOUStr, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aEmptyOUStr, aItemSet, aPropInfo, &aLang, &aDir ) )
        {
            OSL_ENSURE( aClass.isEmpty() || !pCSS1Parser->GetClass( aClass ),
                    "Class wird nicht beruecksichtigt" );
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );
        }
    }

    
    
    
    if( pCFmt )
        InsertAttr( &aAttrTab.pCharFmts, SwFmtCharFmt( pCFmt ), pCntxt );

    
    PushContext( pCntxt );
}



void SwHTMLParser::InsertSpacer()
{
    
    OUString aId;
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
            
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            aSize.Width() = (long)rOption.GetNumber();
            break;
        case HTML_O_HEIGHT:
            
            bPrcHeight = (rOption.GetString().indexOf('%') != -1);
            aSize.Height() = (long)rOption.GetNumber();
            break;
        case HTML_O_SIZE:
            
            nSize = rOption.GetNumber();
            break;
        }
    }

    switch( nType )
    {
    case HTML_SPTYPE_BLOCK:
        {
            

            
            SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
            if( !IsNewDoc() )
                Reader::ResetFrmFmtAttrs( aFrmSet );

            
            SetAnchorAndAdjustment( eVertOri, eHoriOri, aFrmSet );

            
            Size aDfltSz( MINFLY, MINFLY );
            Size aSpace( 0, 0 );
            SfxItemSet aDummyItemSet( pDoc->GetAttrPool(),
                                 pCSS1Parser->GetWhichMap() );
            SvxCSS1PropertyInfo aDummyPropInfo;

            SetFixSize( aSize, aDfltSz, bPrcWidth, bPrcHeight,
                        aDummyItemSet, aDummyPropInfo, aFrmSet );
            SetSpace( aSpace, aDummyItemSet, aDummyPropInfo, aFrmSet );

            
            SvxProtectItem aProtectItem( RES_PROTECT) ;
            aProtectItem.SetCntntProtect( sal_True );
            aFrmSet.Put( aProtectItem );

            
            RndStdIds eAnchorId =
                ((const SwFmtAnchor &)aFrmSet.Get(RES_ANCHOR)).GetAnchorId();
            SwFrmFmt *pFlyFmt = pDoc->MakeFlySection( eAnchorId,
                                            pPam->GetPoint(), &aFrmSet );
            
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

            
            SwTxtNode *pTxtNode = 0;
            if( !pPam->GetPoint()->nContent.GetIndex() )
            {
                
                

                SetAttr();  

                pTxtNode = pDoc->GetNodes()[pPam->GetPoint()->nNode.GetIndex()-1]
                               ->GetTxtNode();

                
                
                
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

                AppendTxtNode();    
            }
        }
        break;
    case HTML_SPTYPE_HORI:
        if( nSize > 0 )
        {
            
            

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
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    OUString aId, aStyle, aClass;             
    sal_Bool bClearLeft = sal_False, bClearRight = sal_False;
    sal_Bool bCleared = sal_False;  

    
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
                        bClearLeft = sal_True;
                        bClearRight = sal_True;
                    }
                    else if( rClear.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ) )
                        bClearLeft = sal_True;
                    else if( rClear.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right ) )
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
                } 
            } 
        } 
    } 

    
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
        
        
        OUString sTmp( (sal_Unicode)0x0a );   
        pDoc->InsertString( *pPam, sTmp );
    }
    else if( pPam->GetPoint()->nContent.GetIndex() )
    {
        
        
        
        
        
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
    OUString aId;

    
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

    
    _HTMLAttrContext *pCntxt =
        new _HTMLAttrContext( HTML_HORZRULE, RES_POOLCOLL_HTML_HR, aEmptyOUStr );

    PushContext( pCntxt );

    
    SetTxtCollAttrs( pCntxt );

    
    if( !aParaAttrs.empty() )
        aParaAttrs.clear();

    if( nSize>0 || bColor || bNoShade )
    {
        
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
        
        
        
        
        
        if( !pTable )
        {
            
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

    
    if( !aId.isEmpty() )
        InsertBookmark( aId );

    
    _HTMLAttrContext *pPoppedContext = PopContext( HTML_HORZRULE );
    OSL_ENSURE( pPoppedContext==pCntxt, "wo kommt denn da ein HR-Kontext her?" );
    delete pPoppedContext;

    pPam->Move( fnMoveForward );

    
    SetTxtCollAttrs();
}

void SwHTMLParser::ParseMoreMetaOptions()
{
    OUString aName, aContent;
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

    
    
    
    
    if( aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_generator ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_refresh ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_content_type ) ||
        aName.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_content_script_type ) )
        return;

    aContent = comphelper::string::remove(aContent, '\r');
    aContent = comphelper::string::remove(aContent, '\n');

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

    SwPostItField aPostItFld(
        (SwPostItFieldType*)pDoc->GetSysFldType( RES_POSTITFLD ),
        aEmptyOUStr, sText.makeStringAndClear(), aEmptyOUStr, aEmptyOUStr, DateTime( DateTime::SYSTEM ) );
    SwFmtFld aFmtFld( aPostItFld );
    InsertAttr( aFmtFld );
}


_HTMLAttr::_HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
                      _HTMLAttr **ppHd ) :
    nSttPara( rPos.nNode ),
    nEndPara( rPos.nNode ),
    nSttCntnt( rPos.nContent.GetIndex() ),
    nEndCntnt(rPos.nContent.GetIndex() ),
    bInsAtStart( true ),
    bLikePara( false ),
    bValid( true ),
    nCount( 1 ),
    pNext( 0 ),
    pPrev( 0 ),
    ppHead( ppHd )
{
    pItem = rItem.Clone();
}

_HTMLAttr::_HTMLAttr( const _HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
                      sal_Int32 nEndCnt, _HTMLAttr **ppHd ) :
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

_HTMLAttr *_HTMLAttr::Clone(const SwNodeIndex& rEndPara, sal_Int32 nEndCnt) const
{
    
    _HTMLAttr *pNew = new _HTMLAttr( *this, rEndPara, nEndCnt, ppHead );

    
    pNew->pPrev = pPrev;

    return pNew;
}

void _HTMLAttr::Reset(const SwNodeIndex& rSttPara, sal_Int32 nSttCnt,
                       _HTMLAttr **ppHd)
{
    
    nSttPara = rSttPara;
    nSttCntnt = nSttCnt;
    nEndPara = rSttPara;
    nEndCntnt = nSttCnt;

    
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
    
    bool ret( HTMLParser::ParseMetaOptions(i_xDocProps, i_pHeader) );
    if (!ret && IsNewDoc())
    {
        ParseMoreMetaOptions();
    }
    return ret;
}


void SwHTMLParser::AddMetaUserDefined( OUString const & i_rMetaName )
{
    
    OUString* pName 
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
