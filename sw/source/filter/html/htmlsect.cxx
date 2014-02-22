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

    sal_Bool bHeader=sal_False, bFooter=sal_False;
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
                    bHeader = sal_True;
                else if( rType.equalsIgnoreAsciiCase("footer") )
                    bFooter = sal_True;
            }
        }
    }

    sal_Bool bAppended = sal_False;
    if( pPam->GetPoint()->nContent.GetIndex() )
    {
        AppendTxtNode( bHeader||bFooter||!aId.isEmpty()|| !aHRef.isEmpty() ? AM_NORMAL
                                                                : AM_NOSPACE );
        bAppended = sal_True;
    }

    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( static_cast< sal_uInt16 >(nToken) );

    sal_Bool bStyleParsed = sal_False, bPositioned = sal_False;
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
        sal_Bool bNew = sal_False;
        sal_uInt16 nFlags = CONTEXT_FLAGS_HDRFTR;
        if( bHeader )
        {
            pHdFtFmt = (SwFrmFmt*)rPageFmt.GetHeader().GetHeaderFmt();
            if( !pHdFtFmt )
            {
                
                rPageFmt.SetFmtAttr( SwFmtHeader( sal_True ));
                pHdFtFmt = (SwFrmFmt*)rPageFmt.GetHeader().GetHeaderFmt();
                bNew = sal_True;
            }
            nFlags |= HTML_CNTXT_HEADER_DIST;
        }
        else
        {
            pHdFtFmt = (SwFrmFmt*)rPageFmt.GetFooter().GetFooterFmt();
            if( !pHdFtFmt )
            {
                
                rPageFmt.SetFmtAttr( SwFmtFooter( sal_True ));
                pHdFtFmt = (SwFrmFmt*)rPageFmt.GetFooter().GetFooterFmt();
                bNew = sal_True;
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
            
            SwNodeIndex aSttIdx( rCntntStIdx, 1 );
            pCNd = pDoc->GetNodes().MakeTxtNode( aSttIdx,
                            pCSS1Parser->GetTxtCollFromPool(RES_POOLCOLL_TEXT));

            
            SwPaM aDelPam( aSttIdx );
            aDelPam.SetMark();

            const SwStartNode *pStNd =
                (const SwStartNode *) &rCntntStIdx.GetNode();
            aDelPam.GetPoint()->nNode = pStNd->EndOfSectionIndex() - 1;

            pDoc->DelFullPara( aDelPam );

            
            for( sal_uInt16 i=0; i < pDoc->GetPageDescCnt(); i++ )
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
        sal_Bool bEndNote = sal_False, bFootNote = sal_False;
        if( aId.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdendnote ) )
            bEndNote = sal_True;
        else if( aId.startsWithIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_sdfootnote ) )
            bFootNote = sal_True;
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
                aId = aPropInfo.aId = OUString();
            }
        }
    }

    
    if( (!aId.isEmpty() && !bPositioned) || !aHRef.isEmpty()  )
    {
        
        

        
        
        
        
        if( !bAppended )
        {
            SwNodeIndex aPrvNdIdx( pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTxtNode();
                bAppended = sal_True;
            }
        }
        _HTMLAttrs *pPostIts = bAppended ? 0 : new _HTMLAttrs;
        SetAttr( sal_True, sal_True, pPostIts );

        
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
                aURL = URIHelper::SmartRel2Abs(INetURLObject( sBaseURL ), aHRef, Link(), false);
            }
            else
            {
                aURL = URIHelper::SmartRel2Abs(INetURLObject( sBaseURL ), aHRef.copy( 0, nPos ), Link(), false );
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
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }

        pDoc->InsertSwSection( *pPam, aSection, 0, &aFrmItemSet, false );

        
        if( JUMPTO_REGION == eJumpTo && aName == sJmpMark )
        {
            bChkJumpMark = true;
            eJumpTo = JUMPTO_NONE;
        }

        SwTxtNode* pOldTxtNd =
            (bAppended) ? 0 : pPam->GetPoint()->nNode.GetNode().GetTxtNode();

        pPam->Move( fnMoveBackward );

        
        
        if( pOldTxtNd )
            MovePageDescAttrs( pOldTxtNd, pPam->GetPoint()->nNode.GetIndex(),
                               sal_True  );

        if( pPostIts )
        {
            
            
            InsertAttrs( *pPostIts );
            delete pPostIts;
            pPostIts = 0;
        }

        pCntxt->SetSpansSection( sal_True );

        
        if( !aPropInfo.aId.isEmpty() && aPropInfo.aId==aName )
            aPropInfo.aId = "";
    }
    else
    {
        pCntxt->SetAppendMode( AM_NOSPACE );
    }

    if( SVX_ADJUST_END != eAdjust )
    {
        InsertAttr( &aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST), pCntxt );
    }

    
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );

    PushContext( pCntxt );
}

void SwHTMLParser::EndDivision( int /*nToken*/ )
{
    
    
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
        
        EndContext( pCntxt );
        SetAttr();  

        delete pCntxt;
    }
}

void SwHTMLParser::FixHeaderFooterDistance( sal_Bool bHeader,
                                            const SwPosition *pOldPos )
{
    SwPageDesc *pPageDesc = pCSS1Parser->GetMasterPageDesc();
    SwFrmFmt& rPageFmt = pPageDesc->GetMaster();

    SwFrmFmt *pHdFtFmt =
        bHeader ? (SwFrmFmt*)rPageFmt.GetHeader().GetHeaderFmt()
                : (SwFrmFmt*)rPageFmt.GetFooter().GetFooterFmt();
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
            ((const SvxULSpaceItem&)pTxtNode
                ->SwCntntNode::GetAttr( RES_UL_SPACE ));

        
        
        nSpace = rULSpace.GetLower();

        
        
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
            ((const SvxULSpaceItem&)pTxtNode
                ->SwCntntNode::GetAttr( RES_UL_SPACE ));

        
        
        
        if( rULSpace.GetUpper() > nSpace )
            nSpace = rULSpace.GetUpper();

        
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

sal_Bool SwHTMLParser::EndSection( sal_Bool bLFStripped )
{
    SwEndNode *pEndNd = pDoc->GetNodes()[pPam->GetPoint()->nNode.GetIndex()+1]
                            ->GetEndNode();
    if( pEndNd && pEndNd->StartOfSectionNode()->IsSectionNode() )
    {
        
        if( !bLFStripped )
            StripTrailingPara();
        pPam->Move( fnMoveForward );
        return sal_True;
    }

    OSL_ENSURE( !this, "Falsche PaM Position Beenden eines Bereichs" );

    return sal_False;
}

sal_Bool SwHTMLParser::EndSections( sal_Bool bLFStripped )
{
    sal_Bool bSectionClosed = sal_False;
    sal_uInt16 nPos = aContexts.size();
    while( nPos>nContextStMin )
    {
        _HTMLAttrContext *pCntxt = aContexts[--nPos];
        if( pCntxt->GetSpansSection() && EndSection( bLFStripped ) )
        {
            bSectionClosed = sal_True;
            pCntxt->SetSpansSection( sal_False );
            bLFStripped = sal_False;
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
    sal_Bool bPrcWidth = sal_True;

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

    
    
    sal_Bool bInCntnr = sal_False;
    sal_uInt16 i = aContexts.size();
    while( !bInCntnr && i > nContextStMin )
        bInCntnr = 0 != aContexts[--i]->GetFrmItemSet();

    
    sal_Bool bStyleParsed = sal_False;
    SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
        bStyleParsed = ParseStyleOptions( aStyle, aId, aClass,
                                          aItemSet, aPropInfo, &aLang, &aDir );

    
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

    
    sal_Bool bPositioned = sal_False;
    if( bInCntnr || pCSS1Parser->MayBePositioned( aPropInfo, sal_True ) )
    {
        SfxItemSet aFrmItemSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
        if( !IsNewDoc() )
            Reader::ResetFrmFmtAttrs(aFrmItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, aItemSet, aPropInfo,
                                aFrmItemSet );

        
        
        SetVarSize( aItemSet, aPropInfo, aFrmItemSet, nTwipWidth, nPrcWidth );

        SetSpace( Size(0,0), aItemSet, aPropInfo, aFrmItemSet );

        
        
        
        SetFrmFmtAttrs( aItemSet, aPropInfo,
                        HTML_FF_BOX|HTML_FF_BACKGROUND|HTML_FF_PADDING|HTML_FF_DIRECTION,
                        aFrmItemSet );

        
        
        OUString aFlyName( aEmptyOUStr );
        if( nCols < 2 )
        {
            aFlyName = aId;
            aPropInfo.aId = "";
        }

        InsertFlyFrame( aFrmItemSet, pCntxt, aFlyName, CONTEXT_FLAGS_ABSPOS );

        pCntxt->SetPopStack( sal_True );
        bPositioned = sal_True;
    }

    sal_Bool bAppended = sal_False;
    if( !bPositioned )
    {
        if( pPam->GetPoint()->nContent.GetIndex() )
        {
            AppendTxtNode( AM_SPACE );
            bAppended = sal_True;
        }
        else
        {
            AddParSpace();
        }
    }

    
    if( nCols >= 2 )
    {
        if( !bAppended )
        {
            
            
            
            SwNodeIndex aPrvNdIdx( pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTxtNode();
                bAppended = sal_True;
            }
        }
        _HTMLAttrs *pPostIts = bAppended ? 0 : new _HTMLAttrs;
        SetAttr( sal_True, sal_True, pPostIts );

        
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

        aFmtCol.Init( nCols, nGutter, nPrcWidth ? USHRT_MAX : nTwipWidth );
        aFrmItemSet.Put( aFmtCol );

        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrmItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }
        pDoc->InsertSwSection( *pPam, aSection, 0, &aFrmItemSet, false );

        
        if( JUMPTO_REGION == eJumpTo && aName == sJmpMark )
        {
            bChkJumpMark = true;
            eJumpTo = JUMPTO_NONE;
        }

        SwTxtNode* pOldTxtNd =
            (bAppended) ? 0 : pPam->GetPoint()->nNode.GetNode().GetTxtNode();

        pPam->Move( fnMoveBackward );

        
        
        if( pOldTxtNd )
            MovePageDescAttrs( pOldTxtNd, pPam->GetPoint()->nNode.GetIndex(),
                               sal_True  );

        if( pPostIts )
        {
            
            InsertAttrs( *pPostIts );
            delete pPostIts;
            pPostIts = 0;
        }

        pCntxt->SetSpansSection( sal_True );

        
        if( !aPropInfo.aId.isEmpty() && aPropInfo.aId==aName )
            aPropInfo.aId = "";
    }

    
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, pCntxt, sal_True );

    PushContext( pCntxt );
}


void SwHTMLParser::InsertFlyFrame( const SfxItemSet& rItemSet,
                                   _HTMLAttrContext *pCntxt,
                                   const OUString& rName,
                                   sal_uInt16 nFlags )
{
    RndStdIds eAnchorId =
        ((const SwFmtAnchor&)rItemSet.Get( RES_ANCHOR )).GetAnchorId();

    
    SwFlyFrmFmt* pFlyFmt = pDoc->MakeFlySection( eAnchorId, pPam->GetPoint(),
                                                    &rItemSet );
    
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
                                      sal_Bool bFmtBreak )
{
    SwCntntNode* pDestCntntNd =
        pDoc->GetNodes()[nDestIdx]->GetCntntNode();

    OSL_ENSURE( pDestCntntNd, "Wieso ist das Ziel kein Content-Node?" );

    if( pSrcNd->IsCntntNode() )
    {
        SwCntntNode* pSrcCntntNd = pSrcNd->GetCntntNode();

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pSrcCntntNd->GetSwAttrSet()
                .GetItemState( RES_PAGEDESC, false, &pItem ) &&
            ((SwFmtPageDesc *)pItem)->GetPageDesc() )
        {
            pDestCntntNd->SetAttr( *pItem );
            pSrcCntntNd->ResetAttr( RES_PAGEDESC );
        }
        if( SFX_ITEM_SET == pSrcCntntNd->GetSwAttrSet()
                .GetItemState( RES_BREAK, false, &pItem ) )
        {
            switch( ((SvxFmtBreakItem *)pItem)->GetBreak() )
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
        if( SFX_ITEM_SET == pFrmFmt->GetAttrSet().
                GetItemState( RES_PAGEDESC, false, &pItem ) )
        {
            pDestCntntNd->SetAttr( *pItem );
            pFrmFmt->ResetFmtAttr( RES_PAGEDESC );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
