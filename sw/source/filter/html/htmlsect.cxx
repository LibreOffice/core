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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <rtl/uri.hxx>

#include <svl/urihelper.hxx>
#include <vcl/svapp.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <sfx2/linkmgr.hxx>
#include <osl/diagnose.h>

#include <hintids.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtclds.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <section.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <swtable.hxx>
#include "swcss1.hxx"
#include "swhtml.hxx"


using namespace ::com::sun::star;

void SwHTMLParser::NewDivision( HtmlTokenId nToken )
{
    OUString aId, aHRef;
    OUString aStyle, aLang, aDir;
    OUString aClass;
    SvxAdjust eAdjust = HtmlTokenId::CENTER_ON==nToken ? SvxAdjust::Center
                                               : SvxAdjust::End;

    bool bHeader=false, bFooter=false;
    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::ALIGN:
            if( HtmlTokenId::DIVISION_ON==nToken )
                eAdjust = rOption.GetEnum( aHTMLPAlignTable, eAdjust );
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::LANG:
            aLang = rOption.GetString();
            break;
        case HtmlOptionId::DIR:
            aDir = rOption.GetString();
            break;
        case HtmlOptionId::HREF:
            aHRef =  rOption.GetString();
            break;
        case HtmlOptionId::TITLE:
            {
                const OUString& rType = rOption.GetString();
                if( rType.equalsIgnoreAsciiCase("header") )
                    bHeader = true;
                else if( rType.equalsIgnoreAsciiCase("footer") )
                    bFooter = true;
            }
            break;
        default: break;
        }
    }

    bool bAppended = false;
    if( m_pPam->GetPoint()->nContent.GetIndex() )
    {
        AppendTextNode( bHeader||bFooter||!aId.isEmpty()|| !aHRef.isEmpty() ? AM_NORMAL
                                                                : AM_NOSPACE );
        bAppended = true;
    }

    std::unique_ptr<HTMLAttrContext> xCntxt(new HTMLAttrContext(nToken));

    bool bStyleParsed = false, bPositioned = false;
    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        bStyleParsed = ParseStyleOptions( aStyle, aId, aClass,
                                          aItemSet, aPropInfo, &aLang, &aDir );
        if( bStyleParsed )
        {
            if ( aPropInfo.m_nColumnCount >= 2 )
            {
                xCntxt.reset();
                NewMultiCol( aPropInfo.m_nColumnCount );
                return;
            }
            bPositioned = HtmlTokenId::DIVISION_ON == nToken && !aClass.isEmpty() &&
                          CreateContainer(aClass, aItemSet, aPropInfo,
                                          xCntxt.get());
            if( !bPositioned )
            {
                if (aPropInfo.m_bVisible && m_aContexts.size())
                {
                    const std::unique_ptr<HTMLAttrContext>& pParent
                        = m_aContexts[m_aContexts.size() - 1];
                    if (!pParent->IsVisible())
                    {
                        // If the parent context is hidden, we are not visible, either.
                        aPropInfo.m_bVisible = false;
                    }
                }
                bPositioned = DoPositioning(aItemSet, aPropInfo, xCntxt.get());
            }
        }
    }

    if (!bPositioned && (bHeader || bFooter) && IsNewDoc() && !m_bReadingHeaderOrFooter)
    {
        m_bReadingHeaderOrFooter = true;
        xCntxt->SetHeaderOrFooter(true);

        SwPageDesc *pPageDesc = m_pCSS1Parser->GetMasterPageDesc();
        SwFrameFormat& rPageFormat = pPageDesc->GetMaster();

        SwFrameFormat *pHdFtFormat;
        bool bNew = false;
        HtmlContextFlags nFlags = HtmlContextFlags::MultiColMask;
        if( bHeader )
        {
            pHdFtFormat = const_cast<SwFrameFormat*>(rPageFormat.GetHeader().GetHeaderFormat());
            if( !pHdFtFormat )
            {
                // still no header, then create one
                rPageFormat.SetFormatAttr( SwFormatHeader( true ));
                pHdFtFormat = const_cast<SwFrameFormat*>(rPageFormat.GetHeader().GetHeaderFormat());
                bNew = true;
            }
            nFlags |= HtmlContextFlags::HeaderDist;
        }
        else
        {
            pHdFtFormat = const_cast<SwFrameFormat*>(rPageFormat.GetFooter().GetFooterFormat());
            if( !pHdFtFormat )
            {
                // still no footer, then create one
                rPageFormat.SetFormatAttr( SwFormatFooter( true ));
                pHdFtFormat = const_cast<SwFrameFormat*>(rPageFormat.GetFooter().GetFooterFormat());
                bNew = true;
            }
            nFlags |= HtmlContextFlags::FooterDist;
        }

        const SwFormatContent& rFlyContent = pHdFtFormat->GetContent();
        const SwNodeIndex& rContentStIdx = *rFlyContent.GetContentIdx();
        SwContentNode *pCNd;

        if( bNew )
        {
            pCNd = m_xDoc->GetNodes()[rContentStIdx.GetIndex()+1]
                       ->GetContentNode();
        }
        else
        {
            // Create a new node at the beginning of the section
            SwNodeIndex aSttIdx( rContentStIdx, 1 );
            pCNd = m_xDoc->GetNodes().MakeTextNode( aSttIdx,
                            m_pCSS1Parser->GetTextCollFromPool(RES_POOLCOLL_TEXT));

            // delete the current content of the section
            SwPaM aDelPam( aSttIdx );
            aDelPam.SetMark();

            const SwStartNode *pStNd =
                static_cast<const SwStartNode *>( &rContentStIdx.GetNode() );
            aDelPam.GetPoint()->nNode = pStNd->EndOfSectionIndex() - 1;

            if (!PendingObjectsInPaM(aDelPam))
            {
                ClearFootnotesMarksInRange(aDelPam.GetMark()->nNode, aDelPam.GetPoint()->nNode);
                m_xDoc->getIDocumentContentOperations().DelFullPara(aDelPam);
            }

            // update page style
            for( size_t i=0; i < m_xDoc->GetPageDescCnt(); i++ )
            {
                if( RES_POOLPAGE_HTML == m_xDoc->GetPageDesc(i).GetPoolFormatId() )
                {
                    m_xDoc->ChgPageDesc( i, *pPageDesc );
                    break;
                }
            }
        }

        SwPosition aNewPos( SwNodeIndex( rContentStIdx, 1 ), SwIndex( pCNd, 0 ) );
        SaveDocContext(xCntxt.get(), nFlags, &aNewPos);
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
                SwContentNode *pCNd =
                    m_xDoc->GetNodes()[pStartNdIdx->GetIndex()+1]->GetContentNode();
                SwNodeIndex aTmpSwNodeIndex(*pCNd);
                SwPosition aNewPos( aTmpSwNodeIndex, SwIndex( pCNd, 0 ) );
                SaveDocContext(xCntxt.get(), HtmlContextFlags::MultiColMask, &aNewPos);
                aId.clear();
                aPropInfo.m_aId.clear();
            }
        }
    }

    // We only insert sections into frames if the section is linked.
    if( (!aId.isEmpty() && !bPositioned) || !aHRef.isEmpty()  )
    {
        // Insert section (has to be done before setting of attributes,
        // because the section is inserted before the PaM position.

        // If we are in the first node of a section, we insert the section
        // before the current section and not in the current section.
        // Therefore we have to add a node and delete it again!
        if( !bAppended )
        {
            SwNodeIndex aPrvNdIdx( m_pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTextNode();
                bAppended = true;
            }
        }
        std::unique_ptr<std::deque<std::unique_ptr<HTMLAttr>>> pPostIts(bAppended ? nullptr : new std::deque<std::unique_ptr<HTMLAttr>>);
        SetAttr( true, true, pPostIts.get() );

        // make name of section unique
        const OUString aName( m_xDoc->GetUniqueSectionName( !aId.isEmpty() ? &aId : nullptr ) );

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
                aURL = URIHelper::SmartRel2Abs(INetURLObject( m_sBaseURL ), aHRef, Link<OUString *, bool>(), false);
            }
            else
            {
                aURL = URIHelper::SmartRel2Abs(INetURLObject( m_sBaseURL ), aHRef.copy( 0, nPos ), Link<OUString *, bool>(), false )
                    + OUStringChar(sfx2::cTokenSeparator);
                if( nPos2 == -1 )
                {
                    aURL += aHRef.copy( nPos+1 );
                }
                else
                {
                    aURL += aHRef.copy( nPos+1, nPos2 - (nPos+1) )
                        + OUStringChar(sfx2::cTokenSeparator)
                        + rtl::Uri::decode( aHRef.copy( nPos2+1 ),
                                              rtl_UriDecodeWithCharset,
                                              RTL_TEXTENCODING_ISO_8859_1 );
                }
            }
            aHRef = aURL;
        }

        SwSectionData aSection( (!aHRef.isEmpty()) ? SectionType::FileLink
                                        : SectionType::Content, aName );
        if( !aHRef.isEmpty() )
        {
            aSection.SetLinkFileName( aHRef );
            aSection.SetProtectFlag(true);
        }

        SfxItemSet aFrameItemSet( m_xDoc->GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs(aFrameItemSet );

        const SfxPoolItem *pItem;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrameItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SfxItemState::SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrameItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }

        m_xDoc->InsertSwSection( *m_pPam, aSection, nullptr, &aFrameItemSet, false );

        // maybe jump to section
        if( JumpToMarks::Region == m_eJumpTo && aName == m_sJmpMark )
        {
            m_bChkJumpMark = true;
            m_eJumpTo = JumpToMarks::NONE;
        }

        SwTextNode* pOldTextNd =
            bAppended ? nullptr : m_pPam->GetPoint()->nNode.GetNode().GetTextNode();

        m_pPam->Move( fnMoveBackward );

        // move PageDesc and SwFormatBreak attribute from current node into
        // (first) node of the section
        if( pOldTextNd )
            MovePageDescAttrs( pOldTextNd, m_pPam->GetPoint()->nNode.GetIndex(),
                               true  );

        if( pPostIts )
        {
            // move still existing PostIts in the first paragraph of the table
            InsertAttrs( std::move(*pPostIts) );
            pPostIts.reset();
        }

        xCntxt->SetSpansSection( true );

        // don't insert Bookmarks with same name as sections
        if( !aPropInfo.m_aId.isEmpty() && aPropInfo.m_aId==aName )
            aPropInfo.m_aId.clear();
    }
    else
    {
        xCntxt->SetAppendMode( AM_NOSPACE );
    }

    if( SvxAdjust::End != eAdjust )
    {
        InsertAttr(&m_xAttrTab->pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST), xCntxt.get());
    }

    // parse style
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, xCntxt.get(), true );

    xCntxt->SetVisible(aPropInfo.m_bVisible);
    PushContext(xCntxt);
}

void SwHTMLParser::EndDivision()
{
    // search for the stack entry of the token (because we still have the div stack
    // we don't make a difference between DIV and CENTER)
    std::unique_ptr<HTMLAttrContext> xCntxt;
    auto nPos = m_aContexts.size();
    while (!xCntxt && nPos>m_nContextStMin)
    {
        switch( m_aContexts[--nPos]->GetToken() )
        {
        case HtmlTokenId::CENTER_ON:
        case HtmlTokenId::DIVISION_ON:
            xCntxt = std::move(m_aContexts[nPos]);
            m_aContexts.erase( m_aContexts.begin() + nPos );
            break;
        default: break;
        }
    }

    if (xCntxt)
    {
        // close attribute
        EndContext(xCntxt.get());
        SetAttr();  // set paragraph attributes really fast because of JavaScript
        if (xCntxt->IsHeaderOrFooter())
            m_bReadingHeaderOrFooter = false;
    }
}

void SwHTMLParser::FixHeaderFooterDistance( bool bHeader,
                                            const SwPosition *pOldPos )
{
    SwPageDesc *pPageDesc = m_pCSS1Parser->GetMasterPageDesc();
    SwFrameFormat& rPageFormat = pPageDesc->GetMaster();

    SwFrameFormat *pHdFtFormat =
        bHeader ? const_cast<SwFrameFormat*>(rPageFormat.GetHeader().GetHeaderFormat())
                : const_cast<SwFrameFormat*>(rPageFormat.GetFooter().GetFooterFormat());
    OSL_ENSURE( pHdFtFormat, "No header or footer" );

    const SwFormatContent& rFlyContent = pHdFtFormat->GetContent();
    const SwNodeIndex& rContentStIdx = *rFlyContent.GetContentIdx();

    sal_uLong nPrvNxtIdx;
    if( bHeader )
    {
        nPrvNxtIdx = rContentStIdx.GetNode().EndOfSectionIndex()-1;
    }
    else
    {
        nPrvNxtIdx = pOldPos->nNode.GetIndex() - 1;
    }

    sal_uInt16 nSpace = 0;
    SwTextNode *pTextNode = m_xDoc->GetNodes()[nPrvNxtIdx]->GetTextNode();
    if( pTextNode )
    {
        const SvxULSpaceItem& rULSpace =
            static_cast<const SvxULSpaceItem&>(pTextNode
                ->SwContentNode::GetAttr( RES_UL_SPACE ));

        // The bottom paragraph padding becomes the padding
        // to header or footer
        nSpace = rULSpace.GetLower();

        // and afterwards set to a valid value
        const SvxULSpaceItem& rCollULSpace =
            pTextNode->GetAnyFormatColl().GetULSpace();
        if( rCollULSpace.GetUpper() == rULSpace.GetUpper() )
            pTextNode->ResetAttr( RES_UL_SPACE );
        else
            pTextNode->SetAttr(
                SvxULSpaceItem( rULSpace.GetUpper(),
                                rCollULSpace.GetLower(), RES_UL_SPACE ) );
    }

    if( bHeader )
    {
        nPrvNxtIdx = pOldPos->nNode.GetIndex();
    }
    else
    {
        nPrvNxtIdx = rContentStIdx.GetIndex() + 1;
    }

    pTextNode = m_xDoc->GetNodes()[nPrvNxtIdx]
                    ->GetTextNode();
    if( pTextNode )
    {
        const SvxULSpaceItem& rULSpace =
            static_cast<const SvxULSpaceItem&>(pTextNode
                ->SwContentNode::GetAttr( RES_UL_SPACE ));

        // The top paragraph padding becomes the padding
        // to headline or footer if it is greater than the
        // bottom padding of the paragraph beforehand
        if( rULSpace.GetUpper() > nSpace )
            nSpace = rULSpace.GetUpper();

        // and afterwards set to a valid value
        const SvxULSpaceItem& rCollULSpace =
            pTextNode->GetAnyFormatColl().GetULSpace();
        if( rCollULSpace.GetLower() == rULSpace.GetLower() )
            pTextNode->ResetAttr( RES_UL_SPACE );
        else
            pTextNode->SetAttr(
                SvxULSpaceItem( rCollULSpace.GetUpper(),
                                rULSpace.GetLower(), RES_UL_SPACE ) );
    }

    SvxULSpaceItem aULSpace( RES_UL_SPACE );
    if( bHeader )
        aULSpace.SetLower( nSpace );
    else
        aULSpace.SetUpper( nSpace );

    pHdFtFormat->SetFormatAttr( aULSpace );
}

bool SwHTMLParser::EndSection( bool bLFStripped )
{
    SwEndNode *pEndNd = m_xDoc->GetNodes()[m_pPam->GetPoint()->nNode.GetIndex()+1]
                            ->GetEndNode();
    if( pEndNd && pEndNd->StartOfSectionNode()->IsSectionNode() )
    {
        // close the section
        if( !bLFStripped )
            StripTrailingPara();
        m_pPam->Move( fnMoveForward );
        return true;
    }

    OSL_ENSURE( false, "Wrong PaM position at end of section" );

    return false;
}

bool SwHTMLParser::EndSections( bool bLFStripped )
{
    bool bSectionClosed = false;
    auto nPos = m_aContexts.size();
    while( nPos>m_nContextStMin )
    {
        HTMLAttrContext *pCntxt = m_aContexts[--nPos].get();
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
    bool bPercentWidth = true;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (size_t i = rHTMLOptions.size(); i; )
    {
        const HTMLOption& rOption = rHTMLOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::LANG:
            aLang = rOption.GetString();
            break;
        case HtmlOptionId::DIR:
            aDir = rOption.GetString();
            break;
        case HtmlOptionId::COLS:
            nCols = static_cast<sal_uInt16>(rOption.GetNumber());
            break;
        case HtmlOptionId::WIDTH:
            nWidth = rOption.GetNumber();
            bPercentWidth = (rOption.GetString().indexOf('%') != -1);
            if( bPercentWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HtmlOptionId::GUTTER:
            nGutter = static_cast<sal_uInt16>(rOption.GetNumber());
            break;
        default: break;
        }
    }

    std::unique_ptr<HTMLAttrContext> xCntxt(new HTMLAttrContext(HtmlTokenId::MULTICOL_ON));

    //.is the multicol element contained in a container? That may be the
    // case for 5.0 documents.
    bool bInCntnr = false;
    auto i = m_aContexts.size();
    while( !bInCntnr && i > m_nContextStMin )
        bInCntnr = nullptr != m_aContexts[--i]->GetFrameItemSet();

    // Parse style sheets, but don't position anything by now.
    bool bStyleParsed = false;
    SfxItemSet aItemSet( m_xDoc->GetAttrPool(), m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
        bStyleParsed = ParseStyleOptions( aStyle, aId, aClass,
                                          aItemSet, aPropInfo, &aLang, &aDir );

    // Calculate width.
    sal_uInt8 nPercentWidth = bPercentWidth ? static_cast<sal_uInt8>(nWidth) : 0;
    SwTwips nTwipWidth = 0;
    if( !bPercentWidth && nWidth && Application::GetDefaultDevice() )
    {
        nTwipWidth = Application::GetDefaultDevice()
                             ->PixelToLogic( Size(nWidth, 0),
                                             MapMode(MapUnit::MapTwip) ).Width();
    }

    if( !nPercentWidth && nTwipWidth < MINFLY )
        nTwipWidth = MINFLY;

    // Do positioning.
    bool bPositioned = false;
    if( bInCntnr || SwCSS1Parser::MayBePositioned( aPropInfo, true ) )
    {
        SfxItemSet aFrameItemSet( m_xDoc->GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs(aFrameItemSet );

        SetAnchorAndAdjustment( text::VertOrientation::NONE, text::HoriOrientation::NONE, aPropInfo,
                                aFrameItemSet );

        // The width is either the WIDTH attribute's value or contained
        // in some style option.
        SetVarSize( aPropInfo, aFrameItemSet, nTwipWidth, nPercentWidth );

        SetSpace( Size(0,0), aItemSet, aPropInfo, aFrameItemSet );

        // Set some other frame attributes. If the background is set, its
        // it will be cleared here. That for, it won't be set at the section,
        // too.
        SetFrameFormatAttrs( aItemSet,
                        HtmlFrameFormatFlags::Box|HtmlFrameFormatFlags::Background|HtmlFrameFormatFlags::Padding|HtmlFrameFormatFlags::Direction,
                        aFrameItemSet );

        // Insert fly frame. If the are columns, the fly frame's name is not
        // the sections name but a generated one.
        OUString aFlyName;
        if( nCols < 2 )
        {
            aFlyName = aId;
            aPropInfo.m_aId.clear();
        }

        InsertFlyFrame(aFrameItemSet, xCntxt.get(), aFlyName);

        xCntxt->SetPopStack( true );
        bPositioned = true;
    }

    bool bAppended = false;
    if( !bPositioned )
    {
        if( m_pPam->GetPoint()->nContent.GetIndex() )
        {
            AppendTextNode( AM_SPACE );
            bAppended = true;
        }
        else
        {
            AddParSpace();
        }
    }

    // If there are less than 2 columns, no section is inserted.
    if( nCols >= 2 )
    {
        if( !bAppended )
        {
            // If the pam is at the start of a section, an additional text
            // node must be inserted. Otherwise, the new section will be
            // inserted in front of the old one.
            SwNodeIndex aPrvNdIdx( m_pPam->GetPoint()->nNode, -1 );
            if (aPrvNdIdx.GetNode().IsSectionNode())
            {
                AppendTextNode();
                bAppended = true;
            }
        }
        std::unique_ptr<std::deque<std::unique_ptr<HTMLAttr>>> pPostIts(bAppended ? nullptr : new std::deque<std::unique_ptr<HTMLAttr>>);
        SetAttr( true, true, pPostIts.get() );

        // Make section name unique.
        OUString aName( m_xDoc->GetUniqueSectionName( !aId.isEmpty() ? &aId : nullptr ) );
        SwSectionData aSection( SectionType::Content, aName );

        SfxItemSet aFrameItemSet( m_xDoc->GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
        if( !IsNewDoc() )
            Reader::ResetFrameFormatAttrs(aFrameItemSet );

        if( nGutter && Application::GetDefaultDevice() )
        {
            nGutter = static_cast<sal_uInt16>(Application::GetDefaultDevice()
                             ->PixelToLogic( Size(nGutter, 0),
                                             MapMode(MapUnit::MapTwip) ).Width());
        }

        SwFormatCol aFormatCol;

        aFormatCol.Init( nCols, nGutter, USHRT_MAX );
        aFrameItemSet.Put( aFormatCol );

        const SfxPoolItem *pItem;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            aFrameItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_BACKGROUND );
        }
        if( SfxItemState::SET == aItemSet.GetItemState( RES_FRAMEDIR, false,
                                                   &pItem ) )
        {
            aFrameItemSet.Put( *pItem );
            aItemSet.ClearItem( RES_FRAMEDIR );
        }
        m_xDoc->InsertSwSection( *m_pPam, aSection, nullptr, &aFrameItemSet, false );

        // Jump to section, if this is requested.
        if( JumpToMarks::Region == m_eJumpTo && aName == m_sJmpMark )
        {
            m_bChkJumpMark = true;
            m_eJumpTo = JumpToMarks::NONE;
        }

        SwTextNode* pOldTextNd =
            bAppended ? nullptr : m_pPam->GetPoint()->nNode.GetNode().GetTextNode();

        m_pPam->Move( fnMoveBackward );

        // Move PageDesc and SwFormatBreak attributes of the current node
        // to the section's first node.
        if( pOldTextNd )
            MovePageDescAttrs( pOldTextNd, m_pPam->GetPoint()->nNode.GetIndex(),
                               true  );

        if( pPostIts )
        {
            // Move pending PostIts into the section.
            InsertAttrs( std::move(*pPostIts) );
            pPostIts.reset();
        }

        xCntxt->SetSpansSection( true );

        // Insert a bookmark if its name differs from the section's name only.
        if( !aPropInfo.m_aId.isEmpty() && aPropInfo.m_aId==aName )
            aPropInfo.m_aId.clear();
    }

    // Additional attributes must be set as hard ones.
    if( bStyleParsed )
        InsertAttrs( aItemSet, aPropInfo, xCntxt.get(), true );

    PushContext(xCntxt);
}

void SwHTMLParser::InsertFlyFrame( const SfxItemSet& rItemSet,
                                   HTMLAttrContext *pCntxt,
                                   const OUString& rName )
{
    RndStdIds eAnchorId =
        rItemSet.Get( RES_ANCHOR ).GetAnchorId();

    // create frame
    SwFlyFrameFormat* pFlyFormat = m_xDoc->MakeFlySection( eAnchorId, m_pPam->GetPoint(),
                                                    &rItemSet );
    if( !rName.isEmpty() )
        pFlyFormat->SetName( rName );

    RegisterFlyFrame( pFlyFormat );

    const SwFormatContent& rFlyContent = pFlyFormat->GetContent();
    const SwNodeIndex& rFlyCntIdx = *rFlyContent.GetContentIdx();
    SwContentNode *pCNd = m_xDoc->GetNodes()[rFlyCntIdx.GetIndex()+1]
                            ->GetContentNode();

    SwPosition aNewPos( SwNodeIndex( rFlyCntIdx, 1 ), SwIndex( pCNd, 0 ) );
    const HtmlContextFlags nFlags = HtmlContextFlags::ProtectStack|HtmlContextFlags::StripPara;
    SaveDocContext( pCntxt, nFlags, &aNewPos );
}

void SwHTMLParser::MovePageDescAttrs( SwNode *pSrcNd,
                                      sal_uLong nDestIdx,
                                      bool bFormatBreak )
{
    SwContentNode* pDestContentNd =
        m_xDoc->GetNodes()[nDestIdx]->GetContentNode();

    OSL_ENSURE( pDestContentNd, "Why is the target not a Content-Node?" );

    if( pSrcNd->IsContentNode() )
    {
        SwContentNode* pSrcContentNd = pSrcNd->GetContentNode();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pSrcContentNd->GetSwAttrSet()
                .GetItemState( RES_PAGEDESC, false, &pItem ) &&
            static_cast<const SwFormatPageDesc *>(pItem)->GetPageDesc() )
        {
            pDestContentNd->SetAttr( *pItem );
            pSrcContentNd->ResetAttr( RES_PAGEDESC );
        }
        if( SfxItemState::SET == pSrcContentNd->GetSwAttrSet()
                .GetItemState( RES_BREAK, false, &pItem ) )
        {
            switch( static_cast<const SvxFormatBreakItem *>(pItem)->GetBreak() )
            {
            case SvxBreak::PageBefore:
            case SvxBreak::PageAfter:
            case SvxBreak::PageBoth:
                if( bFormatBreak )
                    pDestContentNd->SetAttr( *pItem );
                pSrcContentNd->ResetAttr( RES_BREAK );
                break;
            default:
                break;
            }
        }
    }
    else if( pSrcNd->IsTableNode() )
    {
        SwFrameFormat *pFrameFormat = pSrcNd->GetTableNode()->GetTable().GetFrameFormat();

        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pFrameFormat->GetAttrSet().
                GetItemState( RES_PAGEDESC, false, &pItem ) )
        {
            if (pDestContentNd)
                pDestContentNd->SetAttr(*pItem);
            pFrameFormat->ResetFormatAttr( RES_PAGEDESC );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
