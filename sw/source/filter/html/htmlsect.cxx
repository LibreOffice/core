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

#include <rtl/uri.hxx>

#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <sfx2/linkmgr.hxx>

#include "hintids.hxx"
#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <fmtsrnd.hxx>
#include <fmtflcnt.hxx>
#include "frmatr.hxx"
#include "doc.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "section.hxx"
#include "poolfmt.hxx"
#include "pagedesc.hxx"
#include "swtable.hxx"
#include "viewsh.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"

#define CONTEXT_FLAGS_MULTICOL (HTML_CNTXT_STRIP_PARA |  \
                                HTML_CNTXT_KEEP_NUMRULE | \
                                HTML_CNTXT_KEEP_ATTRS)
#define CONTEXT_FLAGS_HDRFTR (CONTEXT_FLAGS_MULTICOL)
#define CONTEXT_FLAGS_FTN (CONTEXT_FLAGS_MULTICOL)

using namespace ::com::sun::star;

void SwHTMLParser::NewDivision( int nToken )
{
    OUString aId, aHRef;
    OUString aStyle, aLang, aDir;
    OUString aClass;
    SvxAdjust eAdjust = HTML_CENTER_ON==nToken ? SVX_ADJUST_CENTER
                                               : SVX_ADJUST_END;

    bool bHeader=false, bFooter=false;
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
            if( HTML_DIVISION_ON==nToken )
                eAdjust = (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                       static_cast< sal_uInt16 >(eAdjust) );
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
        case HTML_O_HREF:
            aHRef =  rOption.GetString();
            break;
        case HTML_O_TITLE:
            {
                const OUString& rType = rOption.GetString();
                if( rType.equalsIgnoreAsciiCase("header") )
                    bHeader = true;
                else if( rType.equalsIgnoreAsciiCase("footer") )
                    bFooter = true;
            }
        }
    }

    bool bAppended = false;
    if( pPam->GetPoint()->nContent.GetIndex() )
    {
        AppendTxtNode( bHeader||bFooter||!aId.isEmpty()|| !aHRef.isEmpty() ? AM_NORMAL
                                                                : AM_NOSPACE );
        bAppended = true;
    }

    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    bool bStyleParsed = false, bPositioned = false;
    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        bStyleParsed = ParseStyleOptions( aStyle, aId, aClass,
                                          aItemSet, aPropInfo, &aLang, &aDir );
        if( bStyleParsed )
        {
            if ( aPropInfo.nColumnCount >= 2 )
            {
                delete pCntxt;
                NewMultiCol( aPropInfo.nColumnCount );
                return;
            }
            bPositioned = HTML_DIVISION_ON == nToken && !aClass.isEmpty() &&
                          CreateContainer( aClass, aItemSet, aPropInfo,
                                           pCntxt );
            if( !bPositioned )
                bPositioned = DoPositioning( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( !bPositioned && (bHeader || bFooter) && IsNewDoc() )
    {
        SwPageDesc *pPageDesc = pCSS1Parser->GetMasterPageDesc();
        SwFrmFmt& rPageFmt = pPageDesc->GetMaster();

        SwFrmFmt *pHdFtFmt;
        bool bNew = false;
        sal_uInt16 nFlags = CONTEXT_FLAGS_HDRFTR;
        if( bHeader )
        {
            pHdFtFmt = const_cast<SwFrmFmt*>(rPageFmt.GetHeader().GetHeaderFmt());
            if( !pHdFtFmt )
            {
                // noch keine Header, dann erzeuge einen.
                rPageFmt.SetFmtAttr( SwFmtHeader( true ));
                pHdFtFmt = const_cast<SwFrmFmt*>(rPageFmt.GetHeader().GetHeaderFmt());
                bNew = true;
            }
            nFlags |= HTML_CNTXT_HEADER_DIST;
        }
        else
        {
            pHdFtFmt = const_cast<SwFrmFmt*>(rPageFmt.GetFooter().GetFooterFmt());
            if( !pHdFtFmt )
            {
                // noch keine Footer, dann erzeuge einen.
                rPageFmt.SetFmtAttr( SwFmtFooter( true ));
                pHdFtFmt = const_cast<SwFrmFmt*>(rPageFmt.GetFooter().GetFooterFmt());
                bNew = true;
            }
            nFlags |= HTML_CNTXT_FOOTER_DIST;
        }

        const SwFmtCntnt& rFlyCntnt = pHdFtFmt->GetCntnt();
        const SwNodeIndex& rCntntStIdx = *rFlyCntnt.GetCntntIdx();
        SwCntntNode *pCNd;

        if( bNew )
        {
            pCNd = pDoc->GetNodes()[rCntntStIdx.GetIndex()+1]
                       ->GetCntntNode();
        }
        else
        {
            // Einen neuen Node zu Beginn der Section anlegen
            SwNodeIndex aSttIdx( rCntntStIdx, 1 );
            pCNd = pDoc->GetNodes().MakeTxtNode( aSttIdx,
                            pCSS1Parser->GetTxtCollFromPool(RES_POOLCOLL_TEXT));

            // Den bisherigen Inhalt der Section loeschen
            SwPaM aDelPam( aSttIdx );
            aDelPam.SetMark();

            const SwStartNode *pStNd =
                static_cast<const SwStartNode *>( &rCntntStIdx.GetNode() );
            aDelPam.GetPoint()->nNode = pStNd->EndOfSectionIndex() - 1;

            pDoc->getIDocumentContentOperations().DelFullPara( aDelPam );

            // Die Seitenvorlage aktualisieren
            for( size_t i=0; i < pDoc->GetPageDescCnt(); i++ )
            {
                if( RES_POOLPAGE_HTML == pDoc->GetPageDesc(i).GetPoolFmtId() )
                {
                    pDoc->ChgPageDesc( i, *pPageDesc );
                    break;
                }
            }
        }

        SwPosition aNewPos( SwNodeIndex( rCntntStIdx, 1 ), SwIndex( pCNd, 0 ) );
        SaveDocContext( pCntxt, nFlags, &aNewPos );
    }
    else if( !bPositioned && aId.getLength() > 9 &&
             (aId[0] == 's' || aId[0] == 'S' ) &&
             (aId[1] == 'd' || aId[1] == 'D' ) )
    {
        bool bEndNote = false, bFootNote = false;
        if( aId.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote ) )
            bEndNote = true;
        else if( aId.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote ) )
            bFootNote = true;
        if( bFootNote || bEndNote )
        {
            SwNodeIndex *pStartNdIdx = GetFootEndNoteSection( aId );
            if( pStartNdIdx )
            {
                SwCntntNode *pCNd =
                    pDoc->GetNodes()[pStartNdIdx->GetIndex()+1]->GetCntntNode();
                SwNodeIndex aTmpSwNodeIndex = SwNodeIndex(*pCNd);
                SwPosition aNewPos( aTmpSwNodeIndex, SwIndex( pCNd, 0 ) );
                SaveDocContext( pCntxt, CONTEXT_FLAGS_FTN, &aNewPos );
                aId.clear();
                aPropInfo.aId.clear();
            }
        }
    }

    // Bereiche fuegen wir in Rahmen nur dann ein, wenn der Bereich gelinkt ist.
    if( (!aId.isEmpty() && !bPositioned) || !aHRef.isEmpty()  )
    {
        // Bereich einfuegen (muss vor dem Setzten von Attributen erfolgen,
        // weil die Section vor der PaM-Position eingefuegt.

        // wenn wir im ersten Node einer Section stehen, wir die neue
        // Section nicht in der aktuellen, sondern vor der aktuellen
        // Section eingefuegt. Deshalb muessen wir dann einen Node
        // einfuegen. UND IN LOESCHEN!!!
        if( !bAppended )
        {
            SwNodeIndex aPrvNdIdx( pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTxtNode();
                bAppended = true;
            }
        }
        _HTMLAttrs *pPostIts = bAppended ? 0 : new _HTMLAttrs;
        SetAttr( true, true, pPostIts );

        // Namen der Section eindeutig machen
        const OUString aName( pDoc->GetUniqueSectionName( !aId.isEmpty() ? &aId : 0 ) );

        if( !aHRef.isEmpty() )
        {
            sal_Unicode cDelim = 255U;
            sal_Int32 nPos = aHRef.lastIndexOf( cDelim );
            sal_Int32 nPos2 = -1;
            if( nPos != -1 )
            {
                nPos2 = aHRef.lastIndexOf( cDelim, nPos );
                if( nPos2 != -1 )
                {
                    sal_Int32 nTmp = nPos;
                    nPos = nPos2;
                    nPos2 = nTmp;
                }
            }
            OUString aURL;
            if( nPos == -1 )
            {
                aURL = URIHelper::SmartRel2Abs(INetURLObject( sBaseURL ), aHRef, Link<OUString *, bool>(), false);
            }
            else
            {
                aURL = URIHelper::SmartRel2Abs(INetURLObject( sBaseURL ), aHRef.copy( 0, nPos ), Link<OUString *, bool>(), false );
                aURL += OUString(sfx2::cTokenSeparator);
                if( nPos2 == -1 )
                {
                    aURL += aHRef.copy( nPos+1 );
                }
                else
                {
                    aURL += aHRef.copy( nPos+1, nPos2 - (nPos+1) );
                    aURL += OUString(sfx2::cTokenSeparator);
                    aURL += rtl::Uri::decode( aHRef.copy( nPos2+1 ),
                                              rtl_UriDecodeWithCharset,
                                              RTL_TEXTENCODING_ISO_8859_1 );
                }
            }
            aHRef = aURL;
        }

        SwSectionData aSection( (!aHRef.isEmpty()) ? FILE_LINK_SECTION
                                        : CONTENT_SECTION, aName );
        if( !aHRef.isEmpty() )
        {
            aSection.SetLinkFileName( aHRef );
            aSection.SetProtectFlag(true);
        }

        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        const SfxPoolItem *pItem;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SfxItemState::SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }

        pDoc->InsertSwSection( *pPam, aSection, 0, &aFrmItemSet, false );

        // ggfs. einen Bereich anspringen
        if( JUMPTO_REGION == eJumpTo && aName == sJmpMark )
        {
            bChkJumpMark = true;
            eJumpTo = JUMPTO_NONE;
        }

        SwTxtNode* pOldTxtNd =
            (bAppended) ? 0 : pPam->GetPoint()->nNode.GetNode().GetTxtNode();

        pPam->Move( fnMoveBackward );

        // PageDesc- und SwFmtBreak Attribute vom aktuellen Node in den
        // (ersten) Node des Bereich verschieben.
        if( pOldTxtNd )
            MovePageDescAttrs( pOldTxtNd, pPam->GetPoint()->nNode.GetIndex(),
                               true  );

        if( pPostIts )
        {
            // noch vorhandene PostIts in den ersten Absatz
            // der Tabelle setzen
            InsertAttrs( *pPostIts );
            delete pPostIts;
            pPostIts = 0;
        }

        pCntxt->SetSpansSection( true );

        // keine text::Bookmarks mit dem gleichen Namen wie Bereiche einfuegen
        if( !aPropInfo.aId.isEmpty() && aPropInfo.aId==aName )
            aPropInfo.aId.clear();
    }
    else
    {
        pCntxt->SetAppendMode( AM_NOSPACE );
    }

    if( SVX_ADJUST_END != eAdjust )
    {
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST), pCntxt );
    }

    // Style parsen
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, pCntxt, true );

    PushContext( pCntxt );
}

void SwHTMLParser::EndDivision( int /*nToken*/ )
{
    // Stack-Eintrag zu dem Token suchen (weil wir noch den Div-Stack
    // haben unterscheiden wir erst einmal nicht zwischen DIV und CENTER
    _HTMLAttrContext *pCntxt = 0;
    sal_uInt16 nPos = aContexts.size();
    while( !pCntxt && nPos>nContextStMin )
    {
        switch( aContexts[--nPos]->GetToken() )
        {
        case HTML_CENTER_ON:
        case HTML_DIVISION_ON:
            pCntxt = aContexts[nPos];
            aContexts.erase( aContexts.begin() + nPos );
            break;
        }
    }

    if( pCntxt )
    {
        // Attribute beenden
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen

        delete pCntxt;
    }
}

void SwHTMLParser::FixHeaderFooterDistance( bool bHeader,
                                            const SwPosition *pOldPos )
{
    SwPageDesc *pPageDesc = pCSS1Parser->GetMasterPageDesc();
    SwFrmFmt& rPageFmt = pPageDesc->GetMaster();

    SwFrmFmt *pHdFtFmt =
        bHeader ? const_cast<SwFrmFmt*>(rPageFmt.GetHeader().GetHeaderFmt())
                : const_cast<SwFrmFmt*>(rPageFmt.GetFooter().GetFooterFmt());
    OSL_ENSURE( pHdFtFmt, "Doch keine Kopf- oder Fusszeile" );

    const SwFmtCntnt& rFlyCntnt = pHdFtFmt->GetCntnt();
    const SwNodeIndex& rCntntStIdx = *rFlyCntnt.GetCntntIdx();

    sal_uLong nPrvNxtIdx;
    if( bHeader )
    {
        nPrvNxtIdx = rCntntStIdx.GetNode().EndOfSectionIndex()-1;
    }
    else
    {
        nPrvNxtIdx = pOldPos->nNode.GetIndex() - 1;
    }

    sal_uInt16 nSpace = 0;
    SwTxtNode *pTxtNode = pDoc->GetNodes()[nPrvNxtIdx]->GetTxtNode();
    if( pTxtNode )
    {
        const SvxULSpaceItem& rULSpace =
            static_cast<const SvxULSpaceItem&>(pTxtNode
                ->SwCntntNode::GetAttr( RES_UL_SPACE ));

        // Der untere Absatz-Abstand wird zum Abstand zur
        // Kopf- oder Fusszeile
        nSpace = rULSpace.GetLower();

        // und anschliessend auf einen vernuenftigen Wert
        // gesetzt
        const SvxULSpaceItem& rCollULSpace =
            pTxtNode->GetAnyFmtColl().GetULSpace();
        if( rCollULSpace.GetUpper() == rULSpace.GetUpper() )
            pTxtNode->ResetAttr( RES_UL_SPACE );
        else
            pTxtNode->SetAttr(
                SvxULSpaceItem( rULSpace.GetUpper(),
                                rCollULSpace.GetLower(), RES_UL_SPACE ) );
    }

    if( bHeader )
    {
        nPrvNxtIdx = pOldPos->nNode.GetIndex();
    }
    else
    {
        nPrvNxtIdx = rCntntStIdx.GetIndex() + 1;
    }

    pTxtNode = pDoc->GetNodes()[nPrvNxtIdx]
                    ->GetTxtNode();
    if( pTxtNode )
    {
        const SvxULSpaceItem& rULSpace =
            static_cast<const SvxULSpaceItem&>(pTxtNode
                ->SwCntntNode::GetAttr( RES_UL_SPACE ));

        // Der obere Absatz-Abstand wird zum Abstand zur
        // Kopf- oder Fusszeile, wenn er groesser ist als
        // der untere vom Absatz davor
        if( rULSpace.GetUpper() > nSpace )
            nSpace = rULSpace.GetUpper();

        // und anschliessend auf einen vernuenftigen Wert gesetzt
        const SvxULSpaceItem& rCollULSpace =
            pTxtNode->GetAnyFmtColl().GetULSpace();
        if( rCollULSpace.GetLower() == rULSpace.GetLower() )
            pTxtNode->ResetAttr( RES_UL_SPACE );
        else
            pTxtNode->SetAttr(
                SvxULSpaceItem( rCollULSpace.GetUpper(),
                                rULSpace.GetLower(), RES_UL_SPACE ) );
    }

    SvxULSpaceItem aULSpace( RES_UL_SPACE );
    if( bHeader )
        aULSpace.SetLower( nSpace );
    else
        aULSpace.SetUpper( nSpace );

    pHdFtFmt->SetFmtAttr( aULSpace );
}

bool SwHTMLParser::EndSection( bool bLFStripped )
{
    SwEndNode *pEndNd = pDoc->GetNodes()[pPam->GetPoint()->nNode.GetIndex()+1]
                            ->GetEndNode();
    if( pEndNd && pEndNd->StartOfSectionNode()->IsSectionNode() )
    {
        // den Bereich beenden
        if( !bLFStripped )
            StripTrailingPara();
        pPam->Move( fnMoveForward );
        return true;
    }

    OSL_ENSURE( false, "Falsche PaM Position Beenden eines Bereichs" );

    return false;
}

bool SwHTMLParser::EndSections( bool bLFStripped )
{
    bool bSectionClosed = false;
    sal_uInt16 nPos = aContexts.size();
    while( nPos>nContextStMin )
    {
        _HTMLAttrContext *pCntxt = aContexts[--nPos];
        if( pCntxt->GetSpansSection() && EndSection( bLFStripped ) )
        {
            bSectionClosed = true;
            pCntxt->SetSpansSection( false );
            bLFStripped = false;
        }
    }

    return bSectionClosed;
}

void SwHTMLParser::NewMultiCol( sal_uInt16 columnsFromCss )
{
    OUString aId;
    OUString aStyle, aClass, aLang, aDir;
    long nWidth = 100;
    sal_uInt16 nCols = columnsFromCss, nGutter = 10;
    bool bPrcWidth = true;

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
        case HTML_O_COLS:
            nCols = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_WIDTH:
            nWidth = rOption.GetNumber();
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            if( bPrcWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HTML_O_GUTTER:
            nGutter = (sal_uInt16)rOption.GetNumber();
            break;

        }
    }

    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_MULTICOL_ON );

    //.is the multicol elememt contained in a container? That may be the
    // case for 5.0 documents.
    bool bInCntnr = false;
    sal_uInt16 i = aContexts.size();
    while( !bInCntnr && i > nContextStMin )
        bInCntnr = 0 != aContexts[--i]->GetFrmItemSet();

    // Parse style sheets, but don't position anything by now.
    bool bStyleParsed = false;
    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
        bStyleParsed = ParseStyleOptions( aStyle, aId, aClass,
                                          aItemSet, aPropInfo, &aLang, &aDir );

    // Calculate width.
    sal_uInt8 nPrcWidth = bPrcWidth ? (sal_uInt8)nWidth : 0;
    sal_uInt16 nTwipWidth = 0;
    if( !bPrcWidth && nWidth && Application::GetDefaultDevice() )
    {
        nTwipWidth = (sal_uInt16)Application::GetDefaultDevice()
                             ->PixelToLogic( Size(nWidth, 0),
                                             MapMode(MAP_TWIP) ).Width();
    }

    if( !nPrcWidth && nTwipWidth < MINFLY )
        nTwipWidth = MINFLY;

    // Do positioning.
    bool bPositioned = false;
    if( bInCntnr || SwCSS1Parser::MayBePositioned( aPropInfo, true ) )
    {
        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, aItemSet, aPropInfo,
                                aFrmItemSet );

        // The width is either the WIDTH attribute's value or contained
        // in some style option.
        SetVarSize( aItemSet, aPropInfo, aFrmItemSet, nTwipWidth, nPrcWidth );

        SetSpace( Size(0,0), aItemSet, aPropInfo, aFrmItemSet );

        // Set some other frame attributes. If the background is set, its
        // it will be cleared here. That for, it won't be set at the section,
        // too.
        SetFrmFmtAttrs( aItemSet, aPropInfo,
                        HTML_FF_BOX|HTML_FF_BACKGROUND|HTML_FF_PADDING|HTML_FF_DIRECTION,
                        aFrmItemSet );

        // Insert fly frame. If the are columns, the fly frame's name is not
        // the sections name but a generated one.
        OUString aFlyName( aEmptyOUStr );
        if( nCols < 2 )
        {
            aFlyName = aId;
            aPropInfo.aId.clear();
        }

        InsertFlyFrame( aFrmItemSet, pCntxt, aFlyName, CONTEXT_FLAGS_ABSPOS );

        pCntxt->SetPopStack( true );
        bPositioned = true;
    }

    bool bAppended = false;
    if( !bPositioned )
    {
        if( pPam->GetPoint()->nContent.GetIndex() )
        {
            AppendTxtNode( AM_SPACE );
            bAppended = true;
        }
        else
        {
            AddParSpace();
        }
    }

    // If there are less then 2 columns, no section is inserted.
    if( nCols >= 2 )
    {
        if( !bAppended )
        {
            // If the pam is at the start of a section, a additional text
            // node must be inserted. Otherwise, the new section will be
            // inserted in front of the old one.
            SwNodeIndex aPrvNdIdx( pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTxtNode();
                bAppended = true;
            }
        }
        _HTMLAttrs *pPostIts = bAppended ? 0 : new _HTMLAttrs;
        SetAttr( true, true, pPostIts );

        // Make section name unique.
        OUString aName( pDoc->GetUniqueSectionName( !aId.isEmpty() ? &aId : 0 ) );
        SwSectionData aSection( CONTENT_SECTION, aName );

        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = (sal_uInt16)Application::GetDefaultDevice()
                             ->PixelToLogic( Size(nGutter, 0),
                                             MapMode(MAP_TWIP) ).Width();
        }

        SwFmtCol aFmtCol;
        nPrcWidth = 100;

        aFmtCol.Init( nCols, nGutter, USHRT_MAX );
        aFrmItemSet.Put( aFmtCol );

        const SfxPoolItem *pItem;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SfxItemState::SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }
        pDoc->InsertSwSection( *pPam, aSection, 0, &aFrmItemSet, false );

        // Jump to section, if this is requested.
        if( JUMPTO_REGION == eJumpTo && aName == sJmpMark )
        {
            bChkJumpMark = true;
            eJumpTo = JUMPTO_NONE;
        }

        SwTxtNode* pOldTxtNd =
            (bAppended) ? 0 : pPam->GetPoint()->nNode.GetNode().GetTxtNode();

        pPam->Move( fnMoveBackward );

        // Move PageDesc and SwFmtBreak attributes of the current node
        // to the section's first node.
        if( pOldTxtNd )
            MovePageDescAttrs( pOldTxtNd, pPam->GetPoint()->nNode.GetIndex(),
                               true  );

        if( pPostIts )
        {
            // Move pending PostIts into the section.
            InsertAttrs( *pPostIts );
            delete pPostIts;
            pPostIts = 0;
        }

        pCntxt->SetSpansSection( true );

        // Insert a bookmark if its name differs from the section's name only.
        if( !aPropInfo.aId.isEmpty() && aPropInfo.aId==aName )
            aPropInfo.aId.clear();
    }

    // Additional attributes must be set as hard ones.
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, pCntxt, true );

    PushContext( pCntxt );
}

void SwHTMLParser::InsertFlyFrame( const SfxItemSet& rItemSet,
                                   _HTMLAttrContext *pCntxt,
                                   const OUString& rName,
                                   sal_uInt16 nFlags )
{
    RndStdIds eAnchorId =
        static_cast<const SwFmtAnchor&>(rItemSet.Get( RES_ANCHOR )).GetAnchorId();

    // Den Rahmen anlegen
    SwFlyFrmFmt* pFlyFmt = pDoc->MakeFlySection( eAnchorId, pPam->GetPoint(),
                                                    &rItemSet );
    // Ggf. den Namen setzen
    if( !rName.isEmpty() )
        pFlyFmt->SetName( rName );

    RegisterFlyFrm( pFlyFmt );

    const SwFmtCntnt& rFlyCntnt = pFlyFmt->GetCntnt();
    const SwNodeIndex& rFlyCntIdx = *rFlyCntnt.GetCntntIdx();
    SwCntntNode *pCNd = pDoc->GetNodes()[rFlyCntIdx.GetIndex()+1]
                            ->GetCntntNode();

    SwPosition aNewPos( SwNodeIndex( rFlyCntIdx, 1 ), SwIndex( pCNd, 0 ) );
    SaveDocContext( pCntxt, nFlags, &aNewPos );
}

void SwHTMLParser::MovePageDescAttrs( SwNode *pSrcNd,
                                      sal_uLong nDestIdx,
                                      bool bFmtBreak )
{
    SwCntntNode* pDestCntntNd =
        pDoc->GetNodes()[nDestIdx]->GetCntntNode();

    OSL_ENSURE( pDestCntntNd, "Wieso ist das Ziel kein Content-Node?" );

    if( pSrcNd->IsCntntNode() )
    {
        SwCntntNode* pSrcCntntNd = pSrcNd->GetCntntNode();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pSrcCntntNd->GetSwAttrSet()
                .GetItemState( RES_PAGEDESC, false, &pItem ) &&
            static_cast<const SwFmtPageDesc *>(pItem)->GetPageDesc() )
        {
            pDestCntntNd->SetAttr( *pItem );
            pSrcCntntNd->ResetAttr( RES_PAGEDESC );
        }
        if( SfxItemState::SET == pSrcCntntNd->GetSwAttrSet()
                .GetItemState( RES_BREAK, false, &pItem ) )
        {
            switch( static_cast<const SvxFmtBreakItem *>(pItem)->GetBreak() )
            {
            case SVX_BREAK_PAGE_BEFORE:
            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
                if( bFmtBreak )
                    pDestCntntNd->SetAttr( *pItem );
                pSrcCntntNd->ResetAttr( RES_BREAK );
            default:
                ;
            }
        }
    }
    else if( pSrcNd->IsTableNode() )
    {
        SwFrmFmt *pFrmFmt = pSrcNd->GetTableNode()->GetTable().GetFrmFmt();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pFrmFmt->GetAttrSet().
                GetItemState( RES_PAGEDESC, false, &pItem ) )
        {
            pDestCntntNd->SetAttr( *pItem );
            pFrmFmt->ResetFmtAttr( RES_PAGEDESC );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
