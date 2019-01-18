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

#include <unotools/charclass.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <editeng/unolingu.hxx>
#include <txtfld.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <docary.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <modeltoviewhelper.hxx>
#include <node.hxx>
#include <frmatr.hxx>
#include <pam.hxx>
#include <txttxmrk.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <txmsrt.hxx>
#include <ndtxt.hxx>
#include <txtatr.hxx>
#include <swtable.hxx>
#include <expfld.hxx>
#include <authfld.hxx>
#include <toxwrap.hxx>

#include <strings.hrc>
#include <numrule.hxx>
#include <reffld.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// Initialize strings
SwTOIOptions SwTOXSortTabBase::nOpt = SwTOIOptions::NONE;

SwTOXInternational::SwTOXInternational( LanguageType nLang, SwTOIOptions nOpt,
                                        const OUString& rSortAlgorithm ) :
    m_eLang( nLang ),
    m_sSortAlgorithm(rSortAlgorithm),
    m_nOptions( nOpt )
{
    Init();
}

SwTOXInternational::SwTOXInternational( const SwTOXInternational& rIntl ) :
    m_eLang( rIntl.m_eLang ),
    m_sSortAlgorithm(rIntl.m_sSortAlgorithm),
    m_nOptions( rIntl.m_nOptions )
{
  Init();
}

void SwTOXInternational::Init()
{
    m_pIndexWrapper.reset( new IndexEntrySupplierWrapper() );

    const lang::Locale aLcl( LanguageTag::convertToLocale( m_eLang ) );
    m_pIndexWrapper->SetLocale( aLcl );

    if(m_sSortAlgorithm.isEmpty())
    {
        Sequence < OUString > aSeq( m_pIndexWrapper->GetAlgorithmList( aLcl ));
        if(aSeq.getLength())
            m_sSortAlgorithm = aSeq.getConstArray()[0];
    }

    if ( m_nOptions & SwTOIOptions::CaseSensitive )
        m_pIndexWrapper->LoadAlgorithm( aLcl, m_sSortAlgorithm, 0 );
    else
        m_pIndexWrapper->LoadAlgorithm( aLcl, m_sSortAlgorithm, SW_COLLATOR_IGNORES );

    m_pCharClass.reset( new CharClass( LanguageTag( aLcl )) );

}

SwTOXInternational::~SwTOXInternational()
{
    m_pCharClass.reset();
    m_pIndexWrapper.reset();
}

OUString SwTOXInternational::ToUpper( const OUString& rStr, sal_Int32 nPos ) const
{
    return m_pCharClass->uppercase( rStr, nPos, 1 );
}

inline bool SwTOXInternational::IsNumeric( const OUString& rStr ) const
{
    return m_pCharClass->isNumeric( rStr );
}

sal_Int32 SwTOXInternational::Compare( const TextAndReading& rTaR1,
                                       const lang::Locale& rLocale1,
                                       const TextAndReading& rTaR2,
                                       const lang::Locale& rLocale2 ) const
{
    return m_pIndexWrapper->CompareIndexEntry( rTaR1.sText, rTaR1.sReading, rLocale1,
                                             rTaR2.sText, rTaR2.sReading, rLocale2 );
}

OUString SwTOXInternational::GetIndexKey( const TextAndReading& rTaR,
                                        const lang::Locale& rLocale ) const
{
    return m_pIndexWrapper->GetIndexKey( rTaR.sText, rTaR.sReading, rLocale );
}

OUString SwTOXInternational::GetFollowingText( bool bMorePages ) const
{
    return m_pIndexWrapper->GetFollowingText( bMorePages );
}

// SortElement for TOX entries
SwTOXSortTabBase::SwTOXSortTabBase( TOXSortType nTyp, const SwContentNode* pNd,
                                    const SwTextTOXMark* pMark,
                                    const SwTOXInternational* pInter,
                                    const lang::Locale* pLocale )
    : pTOXNd( nullptr ), pTextMark( pMark ), pTOXIntl( pInter ),
    nPos( 0 ), nCntPos( 0 ), nType( static_cast<sal_uInt16>(nTyp) )
    , m_bValidText( false )
{
    if ( pLocale )
        aLocale = *pLocale;

    if( pNd )
    {
        sal_Int32 n = 0;
        if( pTextMark )
            n = pTextMark->GetStart();
        SwTOXSource aTmp( pNd, n, pTextMark && pTextMark->GetTOXMark().IsMainEntry() );
        aTOXSources.push_back(aTmp);

        nPos = pNd->GetIndex();

        switch( nTyp )
        {
        case TOX_SORT_CONTENT:
        case TOX_SORT_PARA:
        case TOX_SORT_TABLE:
            // If they are in a special areas, we should get the position at the
            // body
            if( nPos < pNd->GetNodes().GetEndOfExtras().GetIndex() )
            {
                // Then get the 'anchor' (body) position
                Point aPt;
                std::pair<Point, bool> tmp(aPt, false);
                const SwContentFrame *const pFrame = pNd->getLayoutFrame(
                    pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(),
                    nullptr, &tmp);
                if( pFrame )
                {
                    SwPosition aPos( *pNd );
                    const SwDoc& rDoc = *pNd->GetDoc();
                    bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
                    OSL_ENSURE(bResult, "where is the text node");
                    nPos = aPos.nNode.GetIndex();
                    nCntPos = aPos.nContent.GetIndex();
                }
            }
            else
                nCntPos = n;
            break;
        default: break;
        }
    }
}

OUString SwTOXSortTabBase::GetURL() const
{
    return OUString();
}

bool SwTOXSortTabBase::IsFullPara() const
{
    return false;
}

void SwTOXSortTabBase::FillText( SwTextNode& rNd, const SwIndex& rInsPos,
                                    sal_uInt16, SwRootFrame const*const) const
{
    rNd.InsertText( GetText().sText, rInsPos );
}

bool SwTOXSortTabBase::equivalent(const SwTOXSortTabBase& rCmp)
{
    bool bRet = nPos == rCmp.nPos && nCntPos == rCmp.nCntPos &&
            (!aTOXSources[0].pNd || !rCmp.aTOXSources[0].pNd ||
            aTOXSources[0].pNd == rCmp.aTOXSources[0].pNd );

    if( TOX_SORT_CONTENT == nType )
    {
        bRet = bRet && pTextMark && rCmp.pTextMark &&
                pTextMark->GetStart() == rCmp.pTextMark->GetStart();

        if( bRet )
        {
            // Both pointers exist -> compare text
            // else -> compare AlternativeText
            const sal_Int32 *pEnd  = pTextMark->End();
            const sal_Int32 *pEndCmp = rCmp.pTextMark->End();

            bRet = ( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) ) &&
                    pTOXIntl->IsEqual( GetText(), GetLocale(),
                                       rCmp.GetText(), rCmp.GetLocale() );
        }
    }
    return bRet;
}

bool SwTOXSortTabBase::sort_lt(const SwTOXSortTabBase& rCmp)
{
    if( nPos < rCmp.nPos )
        return true;

    if( nPos == rCmp.nPos )
    {
        if( nCntPos < rCmp.nCntPos )
            return true;

        if( nCntPos == rCmp.nCntPos )
        {
            const SwNode* pFirst = aTOXSources[0].pNd;
            const SwNode* pNext = rCmp.aTOXSources[0].pNd;

            if( pFirst && pFirst == pNext )
            {
                if( TOX_SORT_CONTENT == nType && pTextMark && rCmp.pTextMark )
                {
                    if( pTextMark->GetStart() < rCmp.pTextMark->GetStart() )
                        return true;

                    if( pTextMark->GetStart() == rCmp.pTextMark->GetStart() )
                    {
                        const sal_Int32 *pEnd = pTextMark->End();
                        const sal_Int32 *pEndCmp = rCmp.pTextMark->End();

                        // Both pointers exist -> compare text
                        // else -> compare AlternativeText
                        if( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) )
                        {
                            return pTOXIntl->IsLess( GetText(), GetLocale(),
                                                      rCmp.GetText(), rCmp.GetLocale() );
                        }
                        if( pEnd && !pEndCmp )
                            return true;
                    }
                }
            }
            else if( pFirst && pFirst->IsTextNode() &&
                     pNext && pNext->IsTextNode() )
                    return ::IsFrameBehind( *static_cast<const SwTextNode*>(pNext), nCntPos,
                                            *static_cast<const SwTextNode*>(pFirst), nCntPos );
        }
    }
    return false;
}

// Sorted keyword entry
SwTOXIndex::SwTOXIndex( const SwTextNode& rNd,
                        const SwTextTOXMark* pMark, SwTOIOptions nOptions,
                        sal_uInt8 nKyLevel,
                        const SwTOXInternational& rIntl,
                        const lang::Locale& rLocale )
    : SwTOXSortTabBase( TOX_SORT_INDEX, &rNd, pMark, &rIntl, &rLocale ),
    nKeyLevel(nKyLevel)
{
    nPos = rNd.GetIndex();
    nOpt = nOptions;
}

// Compare keywords. Only relates to the text.

bool SwTOXIndex::equivalent(const SwTOXSortTabBase& rCmpBase)
{
    const SwTOXIndex& rCmp = static_cast<const SwTOXIndex&>(rCmpBase);

    // Respect case taking dependencies into account
    if(GetLevel() != rCmp.GetLevel() || nKeyLevel != rCmp.nKeyLevel)
        return false;

    OSL_ENSURE(pTextMark, "pTextMark == 0, No keyword");

    bool bRet = pTOXIntl->IsEqual( GetText(), GetLocale(),
                                   rCmp.GetText(), rCmp.GetLocale() );

    // If we don't summarize we need to evaluate the Pos
    if(bRet && !(GetOptions() & SwTOIOptions::SameEntry))
        bRet = nPos == rCmp.nPos;

    return bRet;
}

// operator, only depends on the text

bool SwTOXIndex::sort_lt(const SwTOXSortTabBase& rCmpBase)
{
    const SwTOXIndex& rCmp = static_cast<const SwTOXIndex&>(rCmpBase);

    OSL_ENSURE(pTextMark, "pTextMark == 0, No keyword");

    const TextAndReading aMyTaR(GetText());
    const TextAndReading aOtherTaR(rCmp.GetText());

    bool bRet = GetLevel() == rCmp.GetLevel() &&
                pTOXIntl->IsLess( aMyTaR, GetLocale(),
                                  aOtherTaR, rCmp.GetLocale() );

    // If we don't summarize we need to evaluate the Pos
    if( !bRet && !(GetOptions() & SwTOIOptions::SameEntry) )
    {
        bRet = pTOXIntl->IsEqual( aMyTaR, GetLocale(),
                                  aOtherTaR, rCmp.GetLocale() ) &&
               nPos < rCmp.nPos;
    }

    return bRet;
}

// The keyword itself

TextAndReading SwTOXIndex::GetText_Impl(SwRootFrame const*const pLayout) const
{
    OSL_ENSURE(pTextMark, "pTextMark == 0, No keyword");
    const SwTOXMark& rTOXMark = pTextMark->GetTOXMark();

    TextAndReading aRet;
    switch(nKeyLevel)
    {
        case FORM_PRIMARY_KEY    :
        {
            aRet.sText = rTOXMark.GetPrimaryKey();
            aRet.sReading = rTOXMark.GetPrimaryKeyReading();
        }
        break;
        case FORM_SECONDARY_KEY  :
        {
            aRet.sText = rTOXMark.GetSecondaryKey();
            aRet.sReading = rTOXMark.GetSecondaryKeyReading();
        }
        break;
        case FORM_ENTRY          :
        {
            aRet.sText = rTOXMark.GetText(pLayout);
            aRet.sReading = rTOXMark.GetTextReading();
        }
        break;
    }
    // if SwTOIOptions::InitialCaps is set, first character is to be capitalized
    if( SwTOIOptions::InitialCaps & nOpt && pTOXIntl && !aRet.sText.isEmpty())
    {
        aRet.sText = pTOXIntl->ToUpper( aRet.sText, 0 ) + aRet.sText.copy(1);
    }

    return aRet;
}

void SwTOXIndex::FillText( SwTextNode& rNd, const SwIndex& rInsPos, sal_uInt16,
        SwRootFrame const*const pLayout) const
{
    assert(!"sw_redlinehide: this is dead code, Bibliography only has SwTOXAuthority");
    const sal_Int32* pEnd = pTextMark->End();

    TextAndReading aRet;
    if( pEnd && !pTextMark->GetTOXMark().IsAlternativeText() &&
            !(GetOptions() & SwTOIOptions::KeyAsEntry))
    {
        aRet.sText = static_cast<const SwTextNode*>(aTOXSources[0].pNd)->GetExpandText(
                            pLayout,
                            pTextMark->GetStart(),
                            *pEnd - pTextMark->GetStart(),
                            false, false, false,
                            pLayout && pLayout->IsHideRedlines()
                                ? ExpandMode::HideDeletions
                                : ExpandMode(0));
        if(SwTOIOptions::InitialCaps & nOpt && pTOXIntl && !aRet.sText.isEmpty())
        {
            aRet.sText = pTOXIntl->ToUpper( aRet.sText, 0 ) + aRet.sText.copy(1);
        }
    }
    else
        aRet = GetText();

    rNd.InsertText( aRet.sText, rInsPos );
}

sal_uInt16 SwTOXIndex::GetLevel() const
{
    OSL_ENSURE(pTextMark, "pTextMark == 0, No keyword");

    sal_uInt16 nForm = FORM_PRIMARY_KEY;

    if( !(GetOptions() & SwTOIOptions::KeyAsEntry)&&
        !pTextMark->GetTOXMark().GetPrimaryKey().isEmpty() )
    {
        nForm = FORM_SECONDARY_KEY;
        if( !pTextMark->GetTOXMark().GetSecondaryKey().isEmpty() )
            nForm = FORM_ENTRY;
    }
    return nForm;
}

// Key and separator
SwTOXCustom::SwTOXCustom(const TextAndReading& rKey,
                         sal_uInt16 nLevel,
                         const SwTOXInternational& rIntl,
                         const lang::Locale& rLocale )
    : SwTOXSortTabBase( TOX_SORT_CUSTOM, nullptr, nullptr, &rIntl, &rLocale ),
    m_aKey(rKey), nLev(nLevel)
{
}

bool SwTOXCustom::equivalent(const SwTOXSortTabBase& rCmpBase)
{
    return GetLevel() == rCmpBase.GetLevel() &&
           pTOXIntl->IsEqual( GetText(), GetLocale(),
                              rCmpBase.GetText(), rCmpBase.GetLocale() );
}

bool SwTOXCustom::sort_lt(const SwTOXSortTabBase& rCmpBase)
{
    return  GetLevel() <= rCmpBase.GetLevel() &&
            pTOXIntl->IsLess( GetText(), GetLocale(),
                              rCmpBase.GetText(), rCmpBase.GetLocale() );
}

sal_uInt16 SwTOXCustom::GetLevel() const
{
    return nLev;
}

TextAndReading SwTOXCustom::GetText_Impl(SwRootFrame const*const) const
{
    return m_aKey;
}

// Sorts the TOX entries
SwTOXContent::SwTOXContent( const SwTextNode& rNd, const SwTextTOXMark* pMark,
                        const SwTOXInternational& rIntl)
    : SwTOXSortTabBase( TOX_SORT_CONTENT, &rNd, pMark, &rIntl )
{
}

// The content's text

TextAndReading SwTOXContent::GetText_Impl(SwRootFrame const*const pLayout) const
{
    const sal_Int32* pEnd = pTextMark->End();
    if( pEnd && !pTextMark->GetTOXMark().IsAlternativeText() )
    {
        return TextAndReading(
            static_cast<const SwTextNode*>(aTOXSources[0].pNd)->GetExpandText(
                                     pLayout,
                                     pTextMark->GetStart(),
                                     *pEnd - pTextMark->GetStart(),
                            false, false, false,
                            pLayout && pLayout->IsHideRedlines()
                                ? ExpandMode::HideDeletions
                                : ExpandMode(0)),
            pTextMark->GetTOXMark().GetTextReading());
    }

    return TextAndReading(pTextMark->GetTOXMark().GetAlternativeText(), OUString());
}

void SwTOXContent::FillText(SwTextNode& rNd, const SwIndex& rInsPos, sal_uInt16,
        SwRootFrame const*const pLayout) const
{
    assert(!"sw_redlinehide: this is dead code, Bibliography only has SwTOXAuthority");
    const sal_Int32* pEnd = pTextMark->End();
    if( pEnd && !pTextMark->GetTOXMark().IsAlternativeText() )
        // sw_redlinehide: this probably won't HideDeletions
        static_cast<const SwTextNode*>(aTOXSources[0].pNd)->CopyExpandText(
                rNd, &rInsPos, pTextMark->GetStart(),
                *pEnd - pTextMark->GetStart(), pLayout);
    else
    {
        rNd.InsertText( GetText().sText, rInsPos );
    }
}

// The level for displaying it

sal_uInt16 SwTOXContent::GetLevel() const
{
    return pTextMark->GetTOXMark().GetLevel();
}

// TOX assembled from paragraphs
// Watch out for OLE/graphics when sorting!
// The position must not come from the document, but from the "anchor"!
SwTOXPara::SwTOXPara( const SwContentNode& rNd, SwTOXElement eT, sal_uInt16 nLevel, const OUString& sSeqName )
    : SwTOXSortTabBase( TOX_SORT_PARA, &rNd, nullptr, nullptr ),
    eType( eT ),
    m_nLevel(nLevel),
    nStartIndex(0),
    nEndIndex(-1),
    m_sSequenceName( sSeqName )
{
}

TextAndReading SwTOXPara::GetText_Impl(SwRootFrame const*const pLayout) const
{
    const SwContentNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case SwTOXElement::Sequence:
        if (nStartIndex != 0 || nEndIndex != -1)
        {
            // sw_redlinehide: "captions" are a rather fuzzily defined concept anyway
            return TextAndReading(static_cast<const SwTextNode*>(pNd)->GetExpandText(
                        pLayout,
                        nStartIndex,
                        nEndIndex == -1 ? -1 : nEndIndex - nStartIndex,
                        false, false, false,
                        pLayout && pLayout->IsHideRedlines()
                            ? ExpandMode::HideDeletions
                            : ExpandMode(0)),
                    OUString());
        }
        BOOST_FALLTHROUGH;
    case SwTOXElement::Template:
    case SwTOXElement::OutlineLevel:
        {
            assert(nStartIndex == 0);
            assert(nEndIndex == -1);
            return TextAndReading(sw::GetExpandTextMerged(
                        pLayout, *static_cast<const SwTextNode*>(pNd),
                        false, false, ExpandMode(0)),
                    OUString());
        }
        break;

    case SwTOXElement::Ole:
    case SwTOXElement::Graphic:
    case SwTOXElement::Frame:
        {
            // Find the FlyFormat; the object/graphic name is there
            SwFrameFormat* pFly = pNd->GetFlyFormat();
            if( pFly )
                return TextAndReading(pFly->GetName(), OUString());

            OSL_ENSURE( false, "Graphic/object without name" );
            const char* pId = SwTOXElement::Ole == eType
                            ? STR_OBJECT_DEFNAME
                            : SwTOXElement::Graphic == eType
                                ? STR_GRAPHIC_DEFNAME
                                : STR_FRAME_DEFNAME;
            return TextAndReading(SwResId(pId), OUString());
        }
        break;
    default: break;
    }
    return TextAndReading();
}

void SwTOXPara::FillText( SwTextNode& rNd, const SwIndex& rInsPos, sal_uInt16,
        SwRootFrame const*const pLayout) const
{
    assert(!"sw_redlinehide: this is dead code, Bibliography only has SwTOXAuthority");
    if( SwTOXElement::Template == eType || SwTOXElement::Sequence == eType  || SwTOXElement::OutlineLevel == eType)
    {
        const SwTextNode* pSrc = static_cast<const SwTextNode*>(aTOXSources[0].pNd);
        if (SwTOXElement::Sequence == eType
            && (nStartIndex != 0 || nEndIndex != -1))
        {
            pSrc->CopyExpandText( rNd, &rInsPos, nStartIndex,
                    nEndIndex == -1 ? -1 : nEndIndex - nStartIndex,
                    pLayout, false, false, true );
        }
        else
        {
            assert(nStartIndex == 0);
            assert(nEndIndex == -1);
            // sw_redlinehide: this probably won't HideDeletions
            pSrc->CopyExpandText( rNd, &rInsPos, 0, -1,
                    pLayout, false, false, true );
            if (pLayout && pLayout->IsHideRedlines())
            {
                if (SwTextFrame const*const pFrame = static_cast<SwTextFrame*>(pSrc->getLayoutFrame(pLayout)))
                {
                    if (sw::MergedPara const*const pMerged = pFrame->GetMergedPara())
                    {
                        // pSrc already copied above
                        assert(pSrc == pMerged->pParaPropsNode);
                        for (sal_uLong i = pSrc->GetIndex() + 1;
                             i <= pMerged->pLastNode->GetIndex(); ++i)
                        {
                            SwNode *const pTmp(pSrc->GetNodes()[i]);
                            if (pTmp->GetRedlineMergeFlag() == SwNode::Merge::NonFirst)
                            {

                                pTmp->GetTextNode()->CopyExpandText(
                                        rNd, &rInsPos, 0, -1,
                                        pLayout, false, false, false );
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        rNd.InsertText( GetText().sText.replace('\t', ' '), rInsPos );
    }
}

sal_uInt16 SwTOXPara::GetLevel() const
{
    sal_uInt16 nRet = m_nLevel;
    const SwContentNode*  pNd = aTOXSources[0].pNd;

    if( SwTOXElement::OutlineLevel == eType && pNd->GetTextNode() )
    {
        const int nTmp = static_cast<const SwTextNode*>(pNd)->GetAttrOutlineLevel();
        if(nTmp != 0 )
            nRet = static_cast<sal_uInt16>(nTmp);
    }
    return nRet;
}

OUString SwTOXPara::GetURL() const
{
    OUString aText;
    const SwContentNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case SwTOXElement::Template:
    case SwTOXElement::OutlineLevel:
        {
            const SwTextNode * pTextNd = pNd->GetTextNode();

            SwDoc* pDoc = const_cast<SwDoc*>( pTextNd->GetDoc() );
            ::sw::mark::IMark const * const pMark = pDoc->getIDocumentMarkAccess()->getMarkForTextNode(
                                *pTextNd,
                                IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK);
            aText = "#" + pMark->GetName();
        }
        break;

    case SwTOXElement::Ole:
    case SwTOXElement::Graphic:
    case SwTOXElement::Frame:
        {
            // Find the FlyFormat; the object/graphic name is there
            SwFrameFormat* pFly = pNd->GetFlyFormat();
            if( pFly )
            {
                aText = "#" + pFly->GetName() + OUStringLiteral1(cMarkSeparator);
                const sal_Char* pStr;
                switch( eType )
                {
                case SwTOXElement::Ole:       pStr = "ole"; break;
                case SwTOXElement::Graphic:   pStr = "graphic"; break;
                case SwTOXElement::Frame:     pStr = "frame"; break;
                default:            pStr = nullptr;
                }
                if( pStr )
                    aText += OUString::createFromAscii( pStr );
            }
        }
        break;
    case SwTOXElement::Sequence:
        {
            aText = "#" + m_sSequenceName + OUStringLiteral1(cMarkSeparator)
                 + "sequence";
        }
        break;
    default: break;
    }
    return aText;
}

bool SwTOXPara::IsFullPara() const
{
    switch (eType)
    {
        case SwTOXElement::Sequence:
        case SwTOXElement::Template:
        case SwTOXElement::OutlineLevel:
            return nStartIndex == 0 && nEndIndex == -1;
        default:
            return false;
    }
}

// Table
SwTOXTable::SwTOXTable( const SwContentNode& rNd )
    : SwTOXSortTabBase( TOX_SORT_TABLE, &rNd, nullptr, nullptr ),
    nLevel(FORM_ALPHA_DELIMITTER)
{
}

TextAndReading SwTOXTable::GetText_Impl(SwRootFrame const*const) const
{
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd )
    {
        const SwTableNode* pTableNd =
            reinterpret_cast<const SwTableNode*>(pNd->FindTableNode());
        if (pTableNd)
        {
            return TextAndReading(pTableNd->GetTable().GetFrameFormat()->GetName(), OUString());
        }
    }

    OSL_ENSURE( false, "Where's my table?" );
    return TextAndReading(SwResId( STR_TABLE_DEFNAME ), OUString());
}

sal_uInt16 SwTOXTable::GetLevel() const
{
    return nLevel;
}

OUString SwTOXTable::GetURL() const
{
    const SwNode* pNd = aTOXSources[0].pNd;
    if (!pNd)
        return OUString();

    pNd = pNd->FindTableNode();
    if (!pNd)
        return OUString();

    const OUString sName = static_cast<const SwTableNode*>(pNd)->GetTable().GetFrameFormat()->GetName();
    if ( sName.isEmpty() )
        return OUString();

    return "#" + sName + OUStringLiteral1(cMarkSeparator) + "table";
}

SwTOXAuthority::SwTOXAuthority( const SwContentNode& rNd,
                SwFormatField& rField, const SwTOXInternational& rIntl ) :
    SwTOXSortTabBase( TOX_SORT_AUTHORITY, &rNd, nullptr, &rIntl ),
    m_rField(rField)
{
    if(rField.GetTextField())
        nCntPos = rField.GetTextField()->GetStart();
}

sal_uInt16 SwTOXAuthority::GetLevel() const
{
    OUString sText(static_cast<SwAuthorityField*>(m_rField.GetField())->GetFieldText(AUTH_FIELD_AUTHORITY_TYPE));
    //#i18655# the level '0' is the heading level therefore the values are incremented here
    sal_uInt16 nRet = 1;
    if( pTOXIntl->IsNumeric( sText ) )
    {
        nRet = sText.toUInt32();
        nRet++;
    }
    //illegal values are also set to 'ARTICLE' as non-numeric values are
    if(nRet > AUTH_TYPE_END)
        nRet = 1;
    return nRet;
}

static OUString lcl_GetText(SwFormatField const& rField, SwRootFrame const*const pLayout)
{
    return rField.GetField()->ExpandField(true, pLayout);
}

TextAndReading SwTOXAuthority::GetText_Impl(SwRootFrame const*const pLayout) const
{
    return TextAndReading(lcl_GetText(m_rField, pLayout), OUString());
}

void    SwTOXAuthority::FillText( SwTextNode& rNd,
                        const SwIndex& rInsPos, sal_uInt16 nAuthField,
        SwRootFrame const*const pLayout) const
{
    SwAuthorityField* pField = static_cast<SwAuthorityField*>(m_rField.GetField());
    OUString sText;
    if(AUTH_FIELD_IDENTIFIER == nAuthField)
    {
        sText = lcl_GetText(m_rField, pLayout);
        const SwAuthorityFieldType* pType = static_cast<const SwAuthorityFieldType*>(pField->GetTyp());
        sal_Unicode cChar = pType->GetPrefix();
        if(cChar && cChar != ' ')
            sText = sText.copy(1);
        cChar = pType->GetSuffix();
        if(cChar && cChar != ' ')
            sText = sText.copy(0, sText.getLength() - 1);
    }
    else if(AUTH_FIELD_AUTHORITY_TYPE == nAuthField)
    {
        sal_uInt16 nLevel = GetLevel();
        if(nLevel)
            sText = SwAuthorityFieldType::GetAuthTypeName(static_cast<ToxAuthorityType>(--nLevel));
    }
    else
        sText = pField->GetFieldText(static_cast<ToxAuthorityField>(nAuthField));
    rNd.InsertText( sText, rInsPos );
}

bool SwTOXAuthority::equivalent(const SwTOXSortTabBase& rCmp)
{
    return nType == rCmp.nType &&
            static_cast<SwAuthorityField*>(m_rField.GetField())->GetAuthEntry() ==
                static_cast<SwAuthorityField*>(static_cast<const SwTOXAuthority&>(rCmp).m_rField.GetField())->GetAuthEntry();
}

bool SwTOXAuthority::sort_lt(const SwTOXSortTabBase& rBase)
{
    bool bRet = false;
    SwAuthorityField* pField = static_cast<SwAuthorityField*>(m_rField.GetField());
    SwAuthorityFieldType* pType = static_cast<SwAuthorityFieldType*>(
                                                pField->GetTyp());
    if(pType->IsSortByDocument())
        bRet = SwTOXSortTabBase::sort_lt(rBase);
    else
    {
        SwAuthorityField* pCmpField =
            static_cast<SwAuthorityField*>(static_cast<const SwTOXAuthority&>(rBase).m_rField.GetField());

        for(sal_uInt16 i = 0; i < pType->GetSortKeyCount(); i++)
        {
            const SwTOXSortKey* pKey = pType->GetSortKey(i);
            const TextAndReading aMy(pField->GetFieldText(pKey->eField), OUString());
            const TextAndReading aOther(pCmpField->GetFieldText(pKey->eField), OUString());

            sal_Int32 nComp = pTOXIntl->Compare( aMy, GetLocale(),
                                                 aOther, rBase.GetLocale() );

            if( nComp )
            {
                bRet = (-1 == nComp) == pKey->bSortAscending;
                break;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
