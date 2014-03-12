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

#include <tools/resid.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <editeng/unolingu.hxx>
#include <txtfld.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <cntfrm.hxx>
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

#include <comcore.hrc>
#include <numrule.hxx>

extern bool IsFrameBehind( const SwTxtNode& rMyNd, sal_Int32 nMySttPos,
                           const SwTxtNode& rBehindNd, sal_Int32 nSttPos );

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
/*--------------------------------------------------------------------
  Initialize strings
 --------------------------------------------------------------------*/

sal_uInt16 SwTOXSortTabBase::nOpt = 0;

SwTOXInternational::SwTOXInternational( LanguageType nLang, sal_uInt16 nOpt,
                                        const OUString& rSortAlgorithm ) :
    eLang( nLang ),
    sSortAlgorithm(rSortAlgorithm),
    nOptions( nOpt )
{
    Init();
}

SwTOXInternational::SwTOXInternational( const SwTOXInternational& rIntl ) :
    eLang( rIntl.eLang ),
    sSortAlgorithm(rIntl.sSortAlgorithm),
    nOptions( rIntl.nOptions )
{
  Init();
}

void SwTOXInternational::Init()
{
    pIndexWrapper = new IndexEntrySupplierWrapper();

    const lang::Locale aLcl( LanguageTag::convertToLocale( eLang ) );
    pIndexWrapper->SetLocale( aLcl );

    if(sSortAlgorithm.isEmpty())
    {
        Sequence < OUString > aSeq( pIndexWrapper->GetAlgorithmList( aLcl ));
        if(aSeq.getLength())
            sSortAlgorithm = aSeq.getConstArray()[0];
    }

    if ( nOptions & nsSwTOIOptions::TOI_CASE_SENSITIVE )
        pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, 0 );
    else
        pIndexWrapper->LoadAlgorithm( aLcl, sSortAlgorithm, SW_COLLATOR_IGNORES );

    pCharClass = new CharClass( LanguageTag( aLcl ));

}

SwTOXInternational::~SwTOXInternational()
{
    delete pCharClass;
    delete pIndexWrapper;
}

OUString SwTOXInternational::ToUpper( const OUString& rStr, sal_Int32 nPos ) const
{
    return pCharClass->uppercase( rStr, nPos, 1 );
}

inline bool SwTOXInternational::IsNumeric( const OUString& rStr ) const
{
    return pCharClass->isNumeric( rStr );
}

sal_Int32 SwTOXInternational::Compare( const TextAndReading& rTaR1,
                                       const lang::Locale& rLocale1,
                                       const TextAndReading& rTaR2,
                                       const lang::Locale& rLocale2 ) const
{
    return pIndexWrapper->CompareIndexEntry( rTaR1.sText, rTaR1.sReading, rLocale1,
                                             rTaR2.sText, rTaR2.sReading, rLocale2 );
}

OUString SwTOXInternational::GetIndexKey( const TextAndReading& rTaR,
                                        const lang::Locale& rLocale ) const
{
    return pIndexWrapper->GetIndexKey( rTaR.sText, rTaR.sReading, rLocale );
}

OUString SwTOXInternational::GetFollowingText( bool bMorePages ) const
{
    return pIndexWrapper->GetFollowingText( bMorePages );
}

/*--------------------------------------------------------------------
  SortElement for TOX entries
 --------------------------------------------------------------------*/

SwTOXSortTabBase::SwTOXSortTabBase( TOXSortType nTyp, const SwCntntNode* pNd,
                                    const SwTxtTOXMark* pMark,
                                    const SwTOXInternational* pInter,
                                    const lang::Locale* pLocale )
    : pTOXNd( 0 ), pTxtMark( pMark ), pTOXIntl( pInter ),
    nPos( 0 ), nCntPos( 0 ), nType( static_cast<sal_uInt16>(nTyp) ), bValidTxt( false )
{
    if ( pLocale )
        aLocale = *pLocale;

    if( pNd )
    {
        sal_Int32 n = 0;
        if( pTxtMark )
            n = *pTxtMark->GetStart();
        SwTOXSource aTmp( pNd, n, pTxtMark && pTxtMark->GetTOXMark().IsMainEntry() );
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
                const SwCntntFrm* pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aPt, 0, false );
                if( pFrm )
                {
                    SwPosition aPos( *pNd );
                    const SwDoc& rDoc = *pNd->GetDoc();
                    bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
                    OSL_ENSURE(bResult, "where is the text node");
                    (void) bResult; // unused in non-debug
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

void SwTOXSortTabBase::FillText( SwTxtNode& rNd, const SwIndex& rInsPos,
                                    sal_uInt16 ) const
{
    rNd.InsertText( GetTxt().sText, rInsPos );
}

bool SwTOXSortTabBase::operator==( const SwTOXSortTabBase& rCmp )
{
    bool bRet = nPos == rCmp.nPos && nCntPos == rCmp.nCntPos &&
            (!aTOXSources[0].pNd || !rCmp.aTOXSources[0].pNd ||
            aTOXSources[0].pNd == rCmp.aTOXSources[0].pNd );

    if( TOX_SORT_CONTENT == nType )
    {
        bRet = bRet && pTxtMark && rCmp.pTxtMark &&
                *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart();

        if( bRet )
        {
            // Both pointers exist -> compare text
            // else -> compare AlternativeText
            const sal_Int32 *pEnd  = pTxtMark->End();
            const sal_Int32 *pEndCmp = rCmp.pTxtMark->End();

            bRet = ( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) ) &&
                    pTOXIntl->IsEqual( GetTxt(), GetLocale(),
                                       rCmp.GetTxt(), rCmp.GetLocale() );
        }
    }
    return bRet;
}

bool SwTOXSortTabBase::operator<( const SwTOXSortTabBase& rCmp )
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
                if( TOX_SORT_CONTENT == nType && pTxtMark && rCmp.pTxtMark )
                {
                    if( *pTxtMark->GetStart() < *rCmp.pTxtMark->GetStart() )
                        return true;

                    if( *pTxtMark->GetStart() == *rCmp.pTxtMark->GetStart() )
                    {
                        const sal_Int32 *pEnd = pTxtMark->End();
                        const sal_Int32 *pEndCmp = rCmp.pTxtMark->End();

                        // Both pointers exist -> compare text
                        // else -> compare AlternativeText
                        if( ( pEnd && pEndCmp ) || ( !pEnd && !pEndCmp ) )
                            pTOXIntl->IsEqual( GetTxt(), GetLocale(),
                                               rCmp.GetTxt(), rCmp.GetLocale() );

                        if( pEnd && !pEndCmp )
                            return true;
                    }
                }
            }
            else if( pFirst && pFirst->IsTxtNode() &&
                     pNext && pNext->IsTxtNode() )
                    return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntPos,
                                            *(SwTxtNode*)pFirst, nCntPos );
        }
    }
    return false;
}

/*--------------------------------------------------------------------
   Sorted keyword entry
 --------------------------------------------------------------------*/

SwTOXIndex::SwTOXIndex( const SwTxtNode& rNd,
                        const SwTxtTOXMark* pMark, sal_uInt16 nOptions,
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

bool SwTOXIndex::operator==( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    // Respect case taking dependencies into account
    if(GetLevel() != rCmp.GetLevel() || nKeyLevel != rCmp.nKeyLevel)
        return false;

    OSL_ENSURE(pTxtMark, "pTxtMark == 0, No keyword");

    bool bRet = pTOXIntl->IsEqual( GetTxt(), GetLocale(),
                                   rCmp.GetTxt(), rCmp.GetLocale() );

    // If we don't summarize we need to evaluate the Pos
    if(bRet && !(GetOptions() & nsSwTOIOptions::TOI_SAME_ENTRY))
        bRet = nPos == rCmp.nPos;

    return bRet;
}

// operator, only depends on the text

bool SwTOXIndex::operator<( const SwTOXSortTabBase& rCmpBase )
{
    SwTOXIndex& rCmp = (SwTOXIndex&)rCmpBase;

    OSL_ENSURE(pTxtMark, "pTxtMark == 0, No keyword");

    const TextAndReading aMyTaR(GetTxt());
    const TextAndReading aOtherTaR(rCmp.GetTxt());

    bool bRet = GetLevel() == rCmp.GetLevel() &&
                pTOXIntl->IsLess( aMyTaR, GetLocale(),
                                  aOtherTaR, rCmp.GetLocale() );

    // If we don't summarize we need to evaluate the Pos
    if( !bRet && !(GetOptions() & nsSwTOIOptions::TOI_SAME_ENTRY) )
    {
        bRet = pTOXIntl->IsEqual( aMyTaR, GetLocale(),
                                  aOtherTaR, rCmp.GetLocale() ) &&
               nPos < rCmp.nPos;
    }

    return bRet;
}

// The keyword itself

TextAndReading SwTOXIndex::GetText_Impl() const
{
    OSL_ENSURE(pTxtMark, "pTxtMark == 0, No keyword");
    const SwTOXMark& rTOXMark = pTxtMark->GetTOXMark();

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
            aRet.sText = rTOXMark.GetText();
            aRet.sReading = rTOXMark.GetTextReading();
        }
        break;
    }
    // if TOI_INITIAL_CAPS is set, first character is to be capitalized
    if( nsSwTOIOptions::TOI_INITIAL_CAPS & nOpt && pTOXIntl && !aRet.sText.isEmpty())
    {
        aRet.sText = pTOXIntl->ToUpper( aRet.sText, 0 ) + aRet.sText.copy(1);
    }

    return aRet;
}

void SwTOXIndex::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    const sal_Int32* pEnd = pTxtMark->End();

    TextAndReading aRet;
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() &&
            0 == (GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY))
    {
        aRet.sText = ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                            *pTxtMark->GetStart(),
                            *pEnd - *pTxtMark->GetStart());
        if(nsSwTOIOptions::TOI_INITIAL_CAPS & nOpt && pTOXIntl && !aRet.sText.isEmpty())
        {
            aRet.sText = pTOXIntl->ToUpper( aRet.sText, 0 ) + aRet.sText.copy(1);
        }
    }
    else
        aRet = GetTxt();

    rNd.InsertText( aRet.sText, rInsPos );
}

sal_uInt16 SwTOXIndex::GetLevel() const
{
    OSL_ENSURE(pTxtMark, "pTxtMark == 0, No keyword");

    sal_uInt16 nForm = FORM_PRIMARY_KEY;

    if( 0 == (GetOptions() & nsSwTOIOptions::TOI_KEY_AS_ENTRY)&&
        !pTxtMark->GetTOXMark().GetPrimaryKey().isEmpty() )
    {
        nForm = FORM_SECONDARY_KEY;
        if( !pTxtMark->GetTOXMark().GetSecondaryKey().isEmpty() )
            nForm = FORM_ENTRY;
    }
    return nForm;
}

/*--------------------------------------------------------------------
  Key and separator
 --------------------------------------------------------------------*/

SwTOXCustom::SwTOXCustom(const TextAndReading& rKey,
                         sal_uInt16 nLevel,
                         const SwTOXInternational& rIntl,
                         const lang::Locale& rLocale )
    : SwTOXSortTabBase( TOX_SORT_CUSTOM, 0, 0, &rIntl, &rLocale ),
    m_aKey(rKey), nLev(nLevel)
{
}

bool SwTOXCustom::operator==(const SwTOXSortTabBase& rCmpBase)
{
    return GetLevel() == rCmpBase.GetLevel() &&
           pTOXIntl->IsEqual( GetTxt(), GetLocale(),
                              rCmpBase.GetTxt(), rCmpBase.GetLocale() );
}

bool SwTOXCustom::operator < (const SwTOXSortTabBase& rCmpBase)
{
    return  GetLevel() <= rCmpBase.GetLevel() &&
            pTOXIntl->IsLess( GetTxt(), GetLocale(),
                              rCmpBase.GetTxt(), rCmpBase.GetLocale() );
}

sal_uInt16 SwTOXCustom::GetLevel() const
{
    return nLev;
}

TextAndReading SwTOXCustom::GetText_Impl() const
{
    return m_aKey;
}

/*--------------------------------------------------------------------
   Sorts the TOX entries
 --------------------------------------------------------------------*/

SwTOXContent::SwTOXContent( const SwTxtNode& rNd, const SwTxtTOXMark* pMark,
                        const SwTOXInternational& rIntl)
    : SwTOXSortTabBase( TOX_SORT_CONTENT, &rNd, pMark, &rIntl )
{
}

// The content's text

TextAndReading SwTOXContent::GetText_Impl() const
{
    const sal_Int32* pEnd = pTxtMark->End();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
    {
        return TextAndReading(
            ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt(
                                     *pTxtMark->GetStart(),
                                     *pEnd - *pTxtMark->GetStart() ),
            pTxtMark->GetTOXMark().GetTextReading());
    }

    return TextAndReading(pTxtMark->GetTOXMark().GetAlternativeText(), OUString());
}

void SwTOXContent::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    const sal_Int32* pEnd = pTxtMark->End();
    if( pEnd && !pTxtMark->GetTOXMark().IsAlternativeText() )
        ((SwTxtNode*)aTOXSources[0].pNd)->GetExpandTxt( rNd, &rInsPos,
                                    *pTxtMark->GetStart(),
                                    *pEnd - *pTxtMark->GetStart() );
    else
    {
        rNd.InsertText( GetTxt().sText, rInsPos );
    }
}

// The level for displaying it

sal_uInt16 SwTOXContent::GetLevel() const
{
    return pTxtMark->GetTOXMark().GetLevel();
}

/*--------------------------------------------------------------------
   TOX assembled from paragraphs
   Watch out for OLE/graphics when sorting!
   The position must not come from the document, but from the "anchor"!
 --------------------------------------------------------------------*/

SwTOXPara::SwTOXPara( const SwCntntNode& rNd, SwTOXElement eT, sal_uInt16 nLevel, OUString sSeqName )
    : SwTOXSortTabBase( TOX_SORT_PARA, &rNd, 0, 0 ),
    eType( eT ),
    m_nLevel(nLevel),
    nStartIndex(0),
    nEndIndex(-1),
    m_sSequenceName( sSeqName )
{
}

TextAndReading SwTOXPara::GetText_Impl() const
{
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case nsSwTOXElement::TOX_SEQUENCE:
    case nsSwTOXElement::TOX_TEMPLATE:
    case nsSwTOXElement::TOX_OUTLINELEVEL:
        {
            return TextAndReading(((SwTxtNode*)pNd)->GetExpandTxt(
                    nStartIndex,
                    nEndIndex == -1 ? -1 : nEndIndex - nStartIndex),
                    OUString());
        }
        break;

    case nsSwTOXElement::TOX_OLE:
    case nsSwTOXElement::TOX_GRAPHIC:
    case nsSwTOXElement::TOX_FRAME:
        {
            // Find the FlyFormat; the object/graphic name is there
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
                return TextAndReading(pFly->GetName(), OUString());

            OSL_ENSURE( !this, "Graphic/object without name" );
            sal_uInt16 nId = nsSwTOXElement::TOX_OLE == eType
                            ? STR_OBJECT_DEFNAME
                            : nsSwTOXElement::TOX_GRAPHIC == eType
                                ? STR_GRAPHIC_DEFNAME
                                : STR_FRAME_DEFNAME;
            return TextAndReading(SW_RESSTR( nId ), OUString());
        }
        break;
    default: break;
    }
    return TextAndReading();
}

void SwTOXPara::FillText( SwTxtNode& rNd, const SwIndex& rInsPos, sal_uInt16 ) const
{
    if( nsSwTOXElement::TOX_TEMPLATE == eType || nsSwTOXElement::TOX_SEQUENCE == eType  || nsSwTOXElement::TOX_OUTLINELEVEL == eType)
    {
        SwTxtNode* pSrc = (SwTxtNode*)aTOXSources[0].pNd;
        pSrc->GetExpandTxt( rNd, &rInsPos, nStartIndex,
                nEndIndex == -1 ? -1 : nEndIndex - nStartIndex,
                false, false, true );
    }
    else
    {
        rNd.InsertText( GetTxt().sText.replace('\t', ' '), rInsPos );
    }
}

sal_uInt16 SwTOXPara::GetLevel() const
{
    sal_uInt16 nRet = m_nLevel;
    const SwCntntNode*  pNd = aTOXSources[0].pNd;

    if( nsSwTOXElement::TOX_OUTLINELEVEL == eType && pNd->GetTxtNode() )
    {
        const int nTmp = ((SwTxtNode*)pNd)->GetAttrOutlineLevel();
        if(nTmp != 0 )
            nRet = static_cast<sal_uInt16>(nTmp);
    }
    return nRet;
}

OUString SwTOXPara::GetURL() const
{
    OUString aTxt;
    const SwCntntNode* pNd = aTOXSources[0].pNd;
    switch( eType )
    {
    case nsSwTOXElement::TOX_TEMPLATE:
    case nsSwTOXElement::TOX_OUTLINELEVEL:
        {
            const SwTxtNode * pTxtNd = static_cast<const SwTxtNode *>(pNd);

            SwDoc* pDoc = const_cast<SwDoc*>( pTxtNd->GetDoc() );
            ::sw::mark::IMark const * const pMark = pDoc->getIDocumentMarkAccess()->getMarkForTxtNode(
                                *(pTxtNd),
                                IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK);
            aTxt = "#" + pMark->GetName();
        }
        break;

    case nsSwTOXElement::TOX_OLE:
    case nsSwTOXElement::TOX_GRAPHIC:
    case nsSwTOXElement::TOX_FRAME:
        {
            // Find the FlyFormat; the object/graphic name is there
            SwFrmFmt* pFly = pNd->GetFlyFmt();
            if( pFly )
            {
                aTxt = "#" + pFly->GetName() + OUString(cMarkSeparator);
                const sal_Char* pStr;
                switch( eType )
                {
                case nsSwTOXElement::TOX_OLE:       pStr = "ole"; break;
                case nsSwTOXElement::TOX_GRAPHIC:   pStr = "graphic"; break;
                case nsSwTOXElement::TOX_FRAME:     pStr = "frame"; break;
                default:            pStr = 0;
                }
                if( pStr )
                    aTxt += OUString::createFromAscii( pStr );
            }
        }
        break;
    case nsSwTOXElement::TOX_SEQUENCE:
        {
            aTxt = "#" + m_sSequenceName + OUString(cMarkSeparator)
                 + "sequence";
        }
        break;
    default: break;
    }
    return aTxt;
}

/*--------------------------------------------------------------------
  Table
 --------------------------------------------------------------------*/

SwTOXTable::SwTOXTable( const SwCntntNode& rNd )
    : SwTOXSortTabBase( TOX_SORT_TABLE, &rNd, 0, 0 ),
    nLevel(FORM_ALPHA_DELIMITTER)
{
}

TextAndReading SwTOXTable::GetText_Impl() const
{
    const SwNode* pNd = aTOXSources[0].pNd;
    if( pNd )
    {
        const SwTableNode* pTableNd =
            reinterpret_cast<const SwTableNode*>(pNd->FindTableNode());
        if (pTableNd)
        {
            return TextAndReading(pTableNd->GetTable().GetFrmFmt()->GetName(), OUString());
        }
    }

    OSL_ENSURE( !this, "Where's my table?" );
    return TextAndReading(SW_RESSTR( STR_TABLE_DEFNAME ), OUString());
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

    const OUString sName = ((SwTableNode*)pNd)->GetTable().GetFrmFmt()->GetName();
    if ( sName.isEmpty() )
        return OUString();

    return "#" + sName + OUString(cMarkSeparator) + "table";
}

SwTOXAuthority::SwTOXAuthority( const SwCntntNode& rNd,
                SwFmtFld& rField, const SwTOXInternational& rIntl ) :
    SwTOXSortTabBase( TOX_SORT_AUTHORITY, &rNd, 0, &rIntl ),
    m_rField(rField)
{
    if(rField.GetTxtFld())
        nCntPos = *rField.GetTxtFld()->GetStart();
}

sal_uInt16 SwTOXAuthority::GetLevel() const
{
    OUString sText(((SwAuthorityField*)m_rField.GetField())->GetFieldText(AUTH_FIELD_AUTHORITY_TYPE));
    //#i18655# the level '0' is the heading level therefor the values are incremented here
    sal_uInt16 nRet = 1;
    if( pTOXIntl->IsNumeric( sText ) )
    {
        nRet = (sal_uInt16)sText.toInt32();
        nRet++;
    }
    //illegal values are also set to 'ARTICLE' as non-numeric values are
    if(nRet > AUTH_TYPE_END)
        nRet = 1;
    return nRet;
}

static OUString lcl_GetText(SwFmtFld const& rField)
{
    return rField.GetField()->ExpandField(true);
}

TextAndReading SwTOXAuthority::GetText_Impl() const
{
    return TextAndReading(lcl_GetText(m_rField), OUString());
}

void    SwTOXAuthority::FillText( SwTxtNode& rNd,
                        const SwIndex& rInsPos, sal_uInt16 nAuthField ) const
{
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetField();
    OUString sText;
    if(AUTH_FIELD_IDENTIFIER == nAuthField)
    {
        sText = lcl_GetText(m_rField);
        const SwAuthorityFieldType* pType = (const SwAuthorityFieldType*)pField->GetTyp();
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
            sText = SwAuthorityFieldType::GetAuthTypeName((ToxAuthorityType) --nLevel);
    }
    else
        sText = (pField->GetFieldText((ToxAuthorityField) nAuthField));
    rNd.InsertText( sText, rInsPos );
}

bool    SwTOXAuthority::operator==( const SwTOXSortTabBase& rCmp)
{
    return nType == rCmp.nType &&
            ((SwAuthorityField*)m_rField.GetField())->GetHandle() ==
                ((SwAuthorityField*)((SwTOXAuthority&)rCmp).m_rField.GetField())->GetHandle();
}

bool    SwTOXAuthority::operator<( const SwTOXSortTabBase& rBase)
{
    bool bRet = false;
    SwAuthorityField* pField = (SwAuthorityField*)m_rField.GetField();
    SwAuthorityFieldType* pType = (SwAuthorityFieldType*)
                                                pField->GetTyp();
    if(pType->IsSortByDocument())
        bRet = SwTOXSortTabBase::operator<(rBase);
    else
    {
        SwAuthorityField* pCmpField =
            (SwAuthorityField*)((SwTOXAuthority&)rBase).m_rField.GetField();

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
