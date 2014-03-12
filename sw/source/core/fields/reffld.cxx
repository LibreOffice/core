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

#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/unolingu.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <docary.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <fmtftn.hxx>
#include <ndtxt.hxx>
#include <chpfld.hxx>
#include <reffld.hxx>
#include <expfld.hxx>
#include <txtfrm.hxx>
#include <flyfrm.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <crossrefbookmark.hxx>
#include <ftnidx.hxx>
#include <viewsh.hxx>
#include <unofldmid.h>
#include <SwStyleNameMapper.hxx>
#include <shellres.hxx>
#include <poolfmt.hxx>
#include <poolfmt.hrc>
#include <comcore.hrc>
#include <numrule.hxx>
#include <SwNodeNum.hxx>
#include <switerator.hxx>

#include <set>
#include <map>
#include <algorithm>

#include <sfx2/childwin.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;

extern void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx, sal_uInt16* pInsPos = 0 );

static void lcl_GetLayTree( const SwFrm* pFrm, std::vector<const SwFrm*>& rArr )
{
    while( pFrm )
    {
        if( pFrm->IsBodyFrm() ) // unspectacular
            pFrm = pFrm->GetUpper();
        else
        {
            rArr.push_back( pFrm );

            // this is the last page
            if( pFrm->IsPageFrm() )
                break;

            if( pFrm->IsFlyFrm() )
                pFrm = ((SwFlyFrm*)pFrm)->GetAnchorFrm();
            else
                pFrm = pFrm->GetUpper();
        }
    }
}

bool IsFrameBehind( const SwTxtNode& rMyNd, sal_Int32 nMySttPos,
                    const SwTxtNode& rBehindNd, sal_Int32 nSttPos )
{
    const SwTxtFrm *pMyFrm = (SwTxtFrm*)rMyNd.getLayoutFrm( rMyNd.GetDoc()->GetCurrentLayout(), 0, 0, false),
                   *pFrm = (SwTxtFrm*)rBehindNd.getLayoutFrm( rBehindNd.GetDoc()->GetCurrentLayout(), 0, 0, false);

    while( pFrm && !pFrm->IsInside( nSttPos ) )
        pFrm = (SwTxtFrm*)pFrm->GetFollow();
    while( pMyFrm && !pMyFrm->IsInside( nMySttPos ) )
        pMyFrm = (SwTxtFrm*)pMyFrm->GetFollow();

    if( !pFrm || !pMyFrm || pFrm == pMyFrm )
        return false;

    std::vector<const SwFrm*> aRefArr, aArr;
    ::lcl_GetLayTree( pFrm, aRefArr );
    ::lcl_GetLayTree( pMyFrm, aArr );

    size_t nRefCnt = aRefArr.size() - 1, nCnt = aArr.size() - 1;
    sal_Bool bVert = sal_False;
    sal_Bool bR2L = sal_False;

    // Loop as long as a frame does not equal?
    while( nRefCnt && nCnt && aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        const SwFrm* pTmpFrm = aArr[ nCnt ];
        bVert = pTmpFrm->IsVertical();
        bR2L = pTmpFrm->IsRightToLeft();
        --nCnt, --nRefCnt;
    }

    // If a counter overflows?
    if( aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        if( nCnt )
            --nCnt;
        else
            --nRefCnt;
    }

    const SwFrm* pRefFrm = aRefArr[ nRefCnt ];
    const SwFrm* pFldFrm = aArr[ nCnt ];

    // different frames, check their Y-/X-position
    bool bRefIsLower = false;
    if( ( FRM_COLUMN | FRM_CELL ) & pFldFrm->GetType() ||
        ( FRM_COLUMN | FRM_CELL ) & pRefFrm->GetType() )
    {
        if( pFldFrm->GetType() == pRefFrm->GetType() )
        {
            // here, the X-pos is more important
            if( bVert )
            {
                if( bR2L )
                    bRefIsLower = pRefFrm->Frm().Top() < pFldFrm->Frm().Top() ||
                            ( pRefFrm->Frm().Top() == pFldFrm->Frm().Top() &&
                              pRefFrm->Frm().Left() < pFldFrm->Frm().Left() );
                else
                    bRefIsLower = pRefFrm->Frm().Top() < pFldFrm->Frm().Top() ||
                            ( pRefFrm->Frm().Top() == pFldFrm->Frm().Top() &&
                              pRefFrm->Frm().Left() > pFldFrm->Frm().Left() );
            }
            else if( bR2L )
                bRefIsLower = pRefFrm->Frm().Left() > pFldFrm->Frm().Left() ||
                            ( pRefFrm->Frm().Left() == pFldFrm->Frm().Left() &&
                              pRefFrm->Frm().Top() < pFldFrm->Frm().Top() );
            else
                bRefIsLower = pRefFrm->Frm().Left() < pFldFrm->Frm().Left() ||
                            ( pRefFrm->Frm().Left() == pFldFrm->Frm().Left() &&
                              pRefFrm->Frm().Top() < pFldFrm->Frm().Top() );
            pRefFrm = 0;
        }
        else if( ( FRM_COLUMN | FRM_CELL ) & pFldFrm->GetType() )
            pFldFrm = aArr[ nCnt - 1 ];
        else
            pRefFrm = aRefArr[ nRefCnt - 1 ];
    }

    if( pRefFrm ) // misuse as flag
    {
        if( bVert )
        {
            if( bR2L )
                bRefIsLower = pRefFrm->Frm().Left() < pFldFrm->Frm().Left() ||
                            ( pRefFrm->Frm().Left() == pFldFrm->Frm().Left() &&
                                pRefFrm->Frm().Top() < pFldFrm->Frm().Top() );
            else
                bRefIsLower = pRefFrm->Frm().Left() > pFldFrm->Frm().Left() ||
                            ( pRefFrm->Frm().Left() == pFldFrm->Frm().Left() &&
                                pRefFrm->Frm().Top() < pFldFrm->Frm().Top() );
        }
        else if( bR2L )
            bRefIsLower = pRefFrm->Frm().Top() < pFldFrm->Frm().Top() ||
                        ( pRefFrm->Frm().Top() == pFldFrm->Frm().Top() &&
                            pRefFrm->Frm().Left() > pFldFrm->Frm().Left() );
        else
            bRefIsLower = pRefFrm->Frm().Top() < pFldFrm->Frm().Top() ||
                        ( pRefFrm->Frm().Top() == pFldFrm->Frm().Top() &&
                            pRefFrm->Frm().Left() < pFldFrm->Frm().Left() );
    }
    return bRefIsLower;
}

/// get references
SwGetRefField::SwGetRefField( SwGetRefFieldType* pFldType,
                              const OUString& rSetRef, sal_uInt16 nSubTyp,
                              sal_uInt16 nSeqenceNo, sal_uLong nFmt )
    : SwField( pFldType, nFmt ),
      sSetRefName( rSetRef ),
      nSubType( nSubTyp ),
      nSeqNo( nSeqenceNo )
{
}

SwGetRefField::~SwGetRefField()
{
}

OUString SwGetRefField::GetDescription() const
{
    return SW_RES(STR_REFERENCE);
}

sal_uInt16 SwGetRefField::GetSubType() const
{
    return nSubType;
}

void SwGetRefField::SetSubType( sal_uInt16 n )
{
    nSubType = n;
}

// #i81002#
bool SwGetRefField::IsRefToHeadingCrossRefBookmark() const
{
    return GetSubType() == REF_BOOKMARK &&
        ::sw::mark::CrossRefHeadingBookmark::IsLegalName(sSetRefName);
}

bool SwGetRefField::IsRefToNumItemCrossRefBookmark() const
{
    return GetSubType() == REF_BOOKMARK &&
        ::sw::mark::CrossRefNumItemBookmark::IsLegalName(sSetRefName);
}

const SwTxtNode* SwGetRefField::GetReferencedTxtNode() const
{
    SwDoc* pDoc = dynamic_cast<SwGetRefFieldType*>(GetTyp())->GetDoc();
    sal_Int32 nDummy = -1;
    return SwGetRefFieldType::FindAnchor( pDoc, sSetRefName, nSubType, nSeqNo, &nDummy );
}

// #i85090#
OUString SwGetRefField::GetExpandedTxtOfReferencedTxtNode() const
{
    const SwTxtNode* pReferencedTxtNode( GetReferencedTxtNode() );
    return pReferencedTxtNode
           ? OUString(pReferencedTxtNode->GetExpandTxt( 0, -1, true, true, false, false ))
           : OUString();
}

OUString SwGetRefField::Expand() const
{
    return sTxt;
}

OUString SwGetRefField::GetFieldName() const
{
    const OUString aName = GetTyp()->GetName();
    if ( !aName.isEmpty() || !sSetRefName.isEmpty() )
    {
        return aName + " " + sSetRefName;
    }
    return Expand();
}

// #i81002# - parameter <pFldTxtAttr> added
void SwGetRefField::UpdateField( const SwTxtFld* pFldTxtAttr )
{
    sTxt = OUString();

    SwDoc* pDoc = ((SwGetRefFieldType*)GetTyp())->GetDoc();
    // finding the reference target (the number)
    sal_Int32 nNumStart = -1;
    sal_Int32 nNumEnd = -1;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor(
        pDoc, sSetRefName, nSubType, nSeqNo, &nNumStart, &nNumEnd
    );
    // not found?
    if ( !pTxtNd )
    {
        sTxt = SwViewShell::GetShellRes()->aGetRefFld_RefItemNotFound;
        return ;
    }
    // where is the category name (e.g. "Illustration")?
    OUString const sText = pTxtNd->GetTxt();
    const sal_Int32 nCatStart = sText.indexOf(sSetRefName);
    const bool bHasCat = nCatStart>=0;
    const sal_Int32 nCatEnd = bHasCat ? nCatStart + sSetRefName.getLength() : -1;

    // length of the referenced text
    const sal_Int32 nLen = sText.getLength();

    // which format?
    switch( GetFormat() )
    {
    case REF_CONTENT:
    case REF_ONLYNUMBER:
    case REF_ONLYCAPTION:
    case REF_ONLYSEQNO:
        {
            // needed part of Text
            sal_Int32 nStart;
            sal_Int32 nEnd;

            switch( nSubType )
            {
            case REF_SEQUENCEFLD:

                switch( GetFormat() )
                {
                // "Category and Number"
                case REF_ONLYNUMBER:
                    if (bHasCat) {
                        nStart = std::min(nNumStart, nCatStart);
                        nEnd = std::max(nNumEnd, nCatEnd);
                    } else {
                        nStart = nNumStart;
                        nEnd = nNumEnd;
                    }
                    break;

                // "Caption Text"
                case REF_ONLYCAPTION: {
                    // next alphanumeric character after category+number
                    if (const SwTxtAttr* const pTxtAttr =
                        pTxtNd->GetTxtAttrForCharAt(nNumStart, RES_TXTATR_FIELD)
                    ) {
                        // start searching from nFrom
                        const sal_Int32 nFrom = bHasCat
                            ? std::max(nNumStart + 1, nCatEnd)
                            : nNumStart + 1;
                        nStart = SwGetExpField::GetReferenceTextPos( pTxtAttr->GetFmtFld(), *pDoc, nFrom );
                    } else {
                        nStart = bHasCat ? std::max(nNumEnd, nCatEnd) : nNumEnd;
                    }
                    nEnd = nLen;
                    break;
                }

                // "Numbering"
                case REF_ONLYSEQNO:
                    nStart = nNumStart;
                    nEnd = std::min(nStart + 1, nLen);
                    break;

                // "Reference" (whole Text)
                default:
                    nStart = 0;
                    nEnd = nLen;
                    break;
                }
                break;

            case REF_BOOKMARK:
                nStart = nNumStart;
                // Text steht ueber verschiedene Nodes verteilt.
                // Gesamten Text oder nur bis zum Ende vom Node?
                nEnd = nNumEnd<0 ? nLen : nNumEnd;
                break;

            case REF_OUTLINE:
                nStart = nNumStart;
                nEnd = nNumEnd;
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                // die Nummer oder den NumString besorgen
                for( unsigned i = 0; i < pDoc->GetFtnIdxs().size(); ++i )
                {
                    SwTxtFtn* const pFtnIdx = pDoc->GetFtnIdxs()[i];
                    if( nSeqNo == pFtnIdx->GetSeqRefNo() )
                    {
                        sTxt = pFtnIdx->GetFtn().GetViewNumStr( *pDoc );
                        break;
                    }
                }
                return;

            default:
                nStart = nNumStart;
                nEnd = nNumEnd;
                break;
            }

            if( nStart != nEnd ) // a section?
            {
                sTxt = pTxtNd->GetExpandTxt( nStart, nEnd - nStart );

                // remove all special characters (replace them with blanks)
                if( !sTxt.isEmpty() )
                {
                    sTxt = comphelper::string::remove(sTxt, 0xad);
                    OUStringBuffer aBuf(sTxt);
                    const sal_Int32 l = aBuf.getLength();
                    for (sal_Int32 i=0; i<l; ++i)
                    {
                        if (aBuf[i]<' ')
                        {
                            aBuf[i]=' ';
                        }
                        else if (aBuf[i]==0x2011)
                        {
                            aBuf[i]='-';
                        }
                    }
                    sTxt = aBuf.makeStringAndClear();
                }
            }
        }
        break;

    case REF_PAGE:
    case REF_PAGE_PGDESC:
        {
            const SwTxtFrm* pFrm = (SwTxtFrm*)pTxtNd->getLayoutFrm( pDoc->GetCurrentLayout(), 0, 0, false),
                        *pSave = pFrm;
            while( pFrm && !pFrm->IsInside( nNumStart ) )
                pFrm = (SwTxtFrm*)pFrm->GetFollow();

            if( pFrm || 0 != ( pFrm = pSave ))
            {
                sal_uInt16 nPageNo = pFrm->GetVirtPageNum();
                const SwPageFrm *pPage;
                if( REF_PAGE_PGDESC == GetFormat() &&
                    0 != ( pPage = pFrm->FindPageFrm() ) &&
                    pPage->GetPageDesc() )
                    sTxt = pPage->GetPageDesc()->GetNumType().GetNumStr( nPageNo );
                else
                    sTxt = OUString::number(nPageNo);
            }
        }
        break;

    case REF_CHAPTER:
        {
            // a bit tricky: search any frame
            const SwFrm* pFrm = pTxtNd->getLayoutFrm( pDoc->GetCurrentLayout() );
            if( pFrm )
            {
                SwChapterFieldType aFldTyp;
                SwChapterField aFld( &aFldTyp, 0 );
                aFld.SetLevel( MAXLEVEL - 1 );
                aFld.ChangeExpansion( pFrm, pTxtNd, sal_True );
                sTxt = aFld.GetNumber();
            }
        }
        break;

    case REF_UPDOWN:
        {
            // #i81002#
            // simplified: use parameter <pFldTxtAttr>
            if( !pFldTxtAttr || !pFldTxtAttr->GetpTxtNode() )
                break;

            LanguageTag aLanguageTag( GetLanguage());
            LocaleDataWrapper aLocaleData( aLanguageTag );

            // first a "short" test - in case both are in the same node
            if( pFldTxtAttr->GetpTxtNode() == pTxtNd )
            {
                sTxt = nNumStart < *pFldTxtAttr->GetStart()
                            ? aLocaleData.getAboveWord()
                            : aLocaleData.getBelowWord();
                break;
            }

            sTxt = ::IsFrameBehind( *pFldTxtAttr->GetpTxtNode(), *pFldTxtAttr->GetStart(),
                                    *pTxtNd, nNumStart )
                        ? aLocaleData.getAboveWord()
                        : aLocaleData.getBelowWord();
        }
        break;
    // #i81002#
    case REF_NUMBER:
    case REF_NUMBER_NO_CONTEXT:
    case REF_NUMBER_FULL_CONTEXT:
        {
            if ( pFldTxtAttr && pFldTxtAttr->GetpTxtNode() )
            {
                sTxt = MakeRefNumStr( pFldTxtAttr->GetTxtNode(), *pTxtNd, GetFormat() );
            }
        }
        break;

    default:
        OSL_FAIL("<SwGetRefField::UpdateField(..)> - unknown format type");
    }
}

// #i81002#
OUString SwGetRefField::MakeRefNumStr( const SwTxtNode& rTxtNodeOfField,
                                     const SwTxtNode& rTxtNodeOfReferencedItem,
                                     const sal_uInt32 nRefNumFormat ) const
{
    if ( rTxtNodeOfReferencedItem.HasNumber() &&
         rTxtNodeOfReferencedItem.IsCountedInList() )
    {
        OSL_ENSURE( rTxtNodeOfReferencedItem.GetNum(),
                "<SwGetRefField::MakeRefNumStr(..)> - referenced paragraph has number, but no <SwNodeNum> instance --> please inform OD!" );

        // Determine, up to which level the superior list labels have to be
        // included - default is to include all superior list labels.
        sal_uInt8 nRestrictInclToThisLevel( 0 );
        // Determine for format REF_NUMBER the level, up to which the superior
        // list labels have to be restricted, if the text node of the reference
        // field and the text node of the referenced item are in the same
        // document context.
        if ( nRefNumFormat == REF_NUMBER &&
             rTxtNodeOfField.FindFlyStartNode()
                            == rTxtNodeOfReferencedItem.FindFlyStartNode() &&
             rTxtNodeOfField.FindFootnoteStartNode()
                            == rTxtNodeOfReferencedItem.FindFootnoteStartNode() &&
             rTxtNodeOfField.FindHeaderStartNode()
                            == rTxtNodeOfReferencedItem.FindHeaderStartNode() &&
             rTxtNodeOfField.FindFooterStartNode()
                            == rTxtNodeOfReferencedItem.FindFooterStartNode() )
        {
            const SwNodeNum* pNodeNumForTxtNodeOfField( 0 );
            if ( rTxtNodeOfField.HasNumber() &&
                 rTxtNodeOfField.GetNumRule() == rTxtNodeOfReferencedItem.GetNumRule() )
            {
                pNodeNumForTxtNodeOfField = rTxtNodeOfField.GetNum();
            }
            else
            {
                pNodeNumForTxtNodeOfField =
                    rTxtNodeOfReferencedItem.GetNum()->GetPrecedingNodeNumOf( rTxtNodeOfField );
            }
            if ( pNodeNumForTxtNodeOfField )
            {
                const SwNumberTree::tNumberVector rFieldNumVec = pNodeNumForTxtNodeOfField->GetNumberVector();
                const SwNumberTree::tNumberVector rRefItemNumVec = rTxtNodeOfReferencedItem.GetNum()->GetNumberVector();
                sal_uInt8 nLevel( 0 );
                while ( nLevel < rFieldNumVec.size() && nLevel < rRefItemNumVec.size() )
                {
                    if ( rRefItemNumVec[nLevel] == rFieldNumVec[nLevel] )
                    {
                        nRestrictInclToThisLevel = nLevel + 1;
                    }
                    else
                    {
                        break;
                    }
                    ++nLevel;
                }
            }
        }

        // Determine, if superior list labels have to be included
        const bool bInclSuperiorNumLabels(
            ( nRestrictInclToThisLevel < rTxtNodeOfReferencedItem.GetActualListLevel() &&
              ( nRefNumFormat == REF_NUMBER || nRefNumFormat == REF_NUMBER_FULL_CONTEXT ) ) );

        OSL_ENSURE( rTxtNodeOfReferencedItem.GetNumRule(),
                "<SwGetRefField::MakeRefNumStr(..)> - referenced numbered paragraph has no numbering rule set --> please inform OD!" );
        return rTxtNodeOfReferencedItem.GetNumRule()->MakeRefNumString(
                                            *(rTxtNodeOfReferencedItem.GetNum()),
                                            bInclSuperiorNumLabels,
                                            nRestrictInclToThisLevel );
    }

    return OUString();
}

SwField* SwGetRefField::Copy() const
{
    SwGetRefField* pFld = new SwGetRefField( (SwGetRefFieldType*)GetTyp(),
                                                sSetRefName, nSubType,
                                                nSeqNo, GetFormat() );
    pFld->sTxt = sTxt;
    return pFld;
}

/// get reference name
OUString SwGetRefField::GetPar1() const
{
    return sSetRefName;
}

/// set reference name
void SwGetRefField::SetPar1( const OUString& rName )
{
    sSetRefName = rName;
}

OUString SwGetRefField::GetPar2() const
{
    return Expand();
}

bool SwGetRefField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nPart = 0;
            switch(GetFormat())
            {
            case REF_PAGE       : nPart = ReferenceFieldPart::PAGE                ; break;
            case REF_CHAPTER    : nPart = ReferenceFieldPart::CHAPTER             ; break;
            case REF_CONTENT    : nPart = ReferenceFieldPart::TEXT                ; break;
            case REF_UPDOWN     : nPart = ReferenceFieldPart::UP_DOWN             ; break;
            case REF_PAGE_PGDESC: nPart = ReferenceFieldPart::PAGE_DESC           ; break;
            case REF_ONLYNUMBER : nPart = ReferenceFieldPart::CATEGORY_AND_NUMBER ; break;
            case REF_ONLYCAPTION: nPart = ReferenceFieldPart::ONLY_CAPTION        ; break;
            case REF_ONLYSEQNO  : nPart = ReferenceFieldPart::ONLY_SEQUENCE_NUMBER; break;
            // #i81002#
            case REF_NUMBER:              nPart = ReferenceFieldPart::NUMBER;              break;
            case REF_NUMBER_NO_CONTEXT:   nPart = ReferenceFieldPart::NUMBER_NO_CONTEXT;   break;
            case REF_NUMBER_FULL_CONTEXT: nPart = ReferenceFieldPart::NUMBER_FULL_CONTEXT; break;
            }
            rAny <<= nPart;
        }
        break;
    case FIELD_PROP_USHORT2:
        {
            sal_Int16 nSource = 0;
            switch(nSubType)
            {
            case  REF_SETREFATTR : nSource = ReferenceFieldSource::REFERENCE_MARK; break;
            case  REF_SEQUENCEFLD: nSource = ReferenceFieldSource::SEQUENCE_FIELD; break;
            case  REF_BOOKMARK   : nSource = ReferenceFieldSource::BOOKMARK; break;
            case  REF_OUTLINE    : OSL_FAIL("not implemented"); break;
            case  REF_FOOTNOTE   : nSource = ReferenceFieldSource::FOOTNOTE; break;
            case  REF_ENDNOTE    : nSource = ReferenceFieldSource::ENDNOTE; break;
            }
            rAny <<= nSource;
        }
        break;
    case FIELD_PROP_PAR1:
    {
        OUString sTmp(GetPar1());
        if(REF_SEQUENCEFLD == nSubType)
        {
            sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sTmp, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            switch( nPoolId )
            {
                case RES_POOLCOLL_LABEL_ABB:
                case RES_POOLCOLL_LABEL_TABLE:
                case RES_POOLCOLL_LABEL_FRAME:
                case RES_POOLCOLL_LABEL_DRAWING:
                    SwStyleNameMapper::FillProgName(nPoolId, sTmp) ;
                break;
            }
        }
        rAny <<= sTmp;
    }
    break;
    case FIELD_PROP_PAR3:
        rAny <<= Expand();
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= (sal_Int16)nSeqNo;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwGetRefField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nPart = 0;
            rAny >>= nPart;
            switch(nPart)
            {
            case ReferenceFieldPart::PAGE:                  nPart = REF_PAGE; break;
            case ReferenceFieldPart::CHAPTER:               nPart = REF_CHAPTER; break;
            case ReferenceFieldPart::TEXT:                  nPart = REF_CONTENT; break;
            case ReferenceFieldPart::UP_DOWN:               nPart = REF_UPDOWN; break;
            case ReferenceFieldPart::PAGE_DESC:             nPart = REF_PAGE_PGDESC; break;
            case ReferenceFieldPart::CATEGORY_AND_NUMBER:   nPart = REF_ONLYNUMBER; break;
            case ReferenceFieldPart::ONLY_CAPTION:          nPart = REF_ONLYCAPTION; break;
            case ReferenceFieldPart::ONLY_SEQUENCE_NUMBER : nPart = REF_ONLYSEQNO; break;
            // #i81002#
            case ReferenceFieldPart::NUMBER:              nPart = REF_NUMBER;              break;
            case ReferenceFieldPart::NUMBER_NO_CONTEXT:   nPart = REF_NUMBER_NO_CONTEXT;   break;
            case ReferenceFieldPart::NUMBER_FULL_CONTEXT: nPart = REF_NUMBER_FULL_CONTEXT; break;
            default: return false;
            }
            SetFormat(nPart);
        }
        break;
    case FIELD_PROP_USHORT2:
        {
            sal_Int16 nSource = 0;
            rAny >>= nSource;
            switch(nSource)
            {
            case ReferenceFieldSource::REFERENCE_MARK : nSubType = REF_SETREFATTR ; break;
            case ReferenceFieldSource::SEQUENCE_FIELD :
            {
                if(REF_SEQUENCEFLD == nSubType)
                    break;
                nSubType = REF_SEQUENCEFLD;
                ConvertProgrammaticToUIName();
            }
            break;
            case ReferenceFieldSource::BOOKMARK       : nSubType = REF_BOOKMARK   ; break;
            case ReferenceFieldSource::FOOTNOTE       : nSubType = REF_FOOTNOTE   ; break;
            case ReferenceFieldSource::ENDNOTE        : nSubType = REF_ENDNOTE    ; break;
            }
        }
        break;
    case FIELD_PROP_PAR1:
    {
        OUString sTmpStr;
        rAny >>= sTmpStr;
        SetPar1(sTmpStr);
        ConvertProgrammaticToUIName();
    }
    break;
    case FIELD_PROP_PAR3:
        {
            OUString sTmpStr;
            rAny >>= sTmpStr;
            SetExpand( sTmpStr );
        }
        break;
    case FIELD_PROP_SHORT1:
        {
            sal_Int16 nSetSeq = 0;
            rAny >>= nSetSeq;
            if(nSetSeq >= 0)
                nSeqNo = nSetSeq;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

void SwGetRefField::ConvertProgrammaticToUIName()
{
    if(GetTyp() && REF_SEQUENCEFLD == nSubType)
    {
        SwDoc* pDoc = ((SwGetRefFieldType*)GetTyp())->GetDoc();
        const OUString rPar1 = GetPar1();
        // don't convert when the name points to an existing field type
        if(!pDoc->GetFldType(RES_SETEXPFLD, rPar1, false))
        {
            sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromProgName( rPar1, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            sal_uInt16 nResId = USHRT_MAX;
            switch( nPoolId )
            {
                case RES_POOLCOLL_LABEL_ABB:
                    nResId = STR_POOLCOLL_LABEL_ABB;
                break;
                case RES_POOLCOLL_LABEL_TABLE:
                    nResId = STR_POOLCOLL_LABEL_TABLE;
                break;
                case RES_POOLCOLL_LABEL_FRAME:
                    nResId = STR_POOLCOLL_LABEL_FRAME;
                break;
                case RES_POOLCOLL_LABEL_DRAWING:
                    nResId = STR_POOLCOLL_LABEL_DRAWING;
                break;
            }
            if( nResId != USHRT_MAX )
                SetPar1(SW_RESSTR( nResId ));
        }
    }
}

SwGetRefFieldType::SwGetRefFieldType( SwDoc* pDc )
    : SwFieldType( RES_GETREFFLD ), pDoc( pDc )
{}

SwFieldType* SwGetRefFieldType::Copy() const
{
    return new SwGetRefFieldType( pDoc );
}

void SwGetRefFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    // update to all GetReference fields
    if( !pNew && !pOld )
    {
        SwIterator<SwFmtFld,SwFieldType> aIter( *this );
        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
        {
            // update only the GetRef fields
            //JP 3.4.2001: Task 71231 - we need the correct language
            SwGetRefField* pGRef = (SwGetRefField*)pFmtFld->GetField();
            const SwTxtFld* pTFld;
            if( !pGRef->GetLanguage() &&
                0 != ( pTFld = pFmtFld->GetTxtFld()) &&
                pTFld->GetpTxtNode() )
            {
                pGRef->SetLanguage( pTFld->GetpTxtNode()->GetLang(
                                                *pTFld->GetStart() ) );
            }

            // #i81002#
            pGRef->UpdateField( pFmtFld->GetTxtFld() );
        }
    }
    // forward to text fields, they "expand" the text
    NotifyClients( pOld, pNew );
}

SwTxtNode* SwGetRefFieldType::FindAnchor( SwDoc* pDoc, const OUString& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_Int32* pStt, sal_Int32* pEnd )
{
    OSL_ENSURE( pStt, "Why did noone check the StartPos?" );

    SwTxtNode* pTxtNd = 0;
    switch( nSubType )
    {
    case REF_SETREFATTR:
        {
            const SwFmtRefMark *pRef = pDoc->GetRefMark( rRefMark );
            if( pRef && pRef->GetTxtRefMark() )
            {
                pTxtNd = (SwTxtNode*)&pRef->GetTxtRefMark()->GetTxtNode();
                *pStt = *pRef->GetTxtRefMark()->GetStart();
                if( pEnd )
                    *pEnd = *pRef->GetTxtRefMark()->GetAnyEnd();
            }
        }
        break;

    case REF_SEQUENCEFLD:
        {
            SwFieldType* pFldType = pDoc->GetFldType( RES_SETEXPFLD, rRefMark, false );
            if( pFldType && pFldType->GetDepends() &&
                nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType*)pFldType)->GetType() )
            {
                SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
                for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
                {
                    if( pFmtFld->GetTxtFld() && nSeqNo ==
                        ((SwSetExpField*)pFmtFld->GetField())->GetSeqNumber() )
                    {
                        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
                        pTxtNd = (SwTxtNode*)pTxtFld->GetpTxtNode();
                        *pStt = *pTxtFld->GetStart();
                        if( pEnd )
                            *pEnd = (*pStt) + 1;
                        break;
                    }
                }
            }
        }
        break;

    case REF_BOOKMARK:
        {
            IDocumentMarkAccess::const_iterator_t ppMark = pDoc->getIDocumentMarkAccess()->findMark(rRefMark);
            if(ppMark != pDoc->getIDocumentMarkAccess()->getAllMarksEnd())
            {
                const ::sw::mark::IMark* pBkmk = ppMark->get();
                const SwPosition* pPos = &pBkmk->GetMarkStart();

                pTxtNd = pPos->nNode.GetNode().GetTxtNode();
                *pStt = pPos->nContent.GetIndex();
                if(pEnd)
                {
                    if(!pBkmk->IsExpanded())
                    {
                        *pEnd = *pStt;
                        // #i81002#
                        if(dynamic_cast< ::sw::mark::CrossRefBookmark const *>(pBkmk))
                        {
                            OSL_ENSURE( pTxtNd,
                                    "<SwGetRefFieldType::FindAnchor(..)> - node marked by cross-reference bookmark isn't a text node --> crash" );
                            *pEnd = pTxtNd->Len();
                        }
                    }
                    else if(pBkmk->GetOtherMarkPos().nNode == pBkmk->GetMarkPos().nNode)
                        *pEnd = pBkmk->GetMarkEnd().nContent.GetIndex();
                    else
                        *pEnd = -1;
                }
            }
        }
        break;

    case REF_OUTLINE:
        break;

    case REF_FOOTNOTE:
    case REF_ENDNOTE:
        {
            sal_uInt16 n, nFtnCnt = pDoc->GetFtnIdxs().size();
            SwTxtFtn* pFtnIdx;
            for( n = 0; n < nFtnCnt; ++n )
                if( nSeqNo == (pFtnIdx = pDoc->GetFtnIdxs()[ n ])->GetSeqRefNo() )
                {
                    SwNodeIndex* pIdx = pFtnIdx->GetStartNode();
                    if( pIdx )
                    {
                        SwNodeIndex aIdx( *pIdx, 1 );
                        if( 0 == ( pTxtNd = aIdx.GetNode().GetTxtNode()))
                            pTxtNd = (SwTxtNode*)pDoc->GetNodes().GoNext( &aIdx );
                    }
                    *pStt = 0;
                    if( pEnd )
                        *pEnd = 0;
                    break;
                }
        }
        break;
    }

    return pTxtNd;
}

struct _RefIdsMap
{
private:
    OUString aName;
    std::set<sal_uInt16> aIds;
    std::set<sal_uInt16> aDstIds;
    std::map<sal_uInt16, sal_uInt16> sequencedIds; /// ID numbers sorted by sequence number.
    bool bInit;

    void       Init(SwDoc& rDoc, SwDoc& rDestDoc, bool bField );
    void       GetNoteIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       AddId( sal_uInt16 id, sal_uInt16 seqNum );
    sal_uInt16 GetFirstUnusedId( std::set<sal_uInt16> &rIds );

public:
    _RefIdsMap( const OUString& rName ) : aName( rName ), bInit( false ) {}

    void Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rFld, bool bField );

    OUString GetName() { return aName; }
};

typedef boost::ptr_vector<_RefIdsMap> _RefIdsMaps;

/// Get a sorted list of the field IDs from a document.
/// @param[in]     rDoc The document to search.
/// @param[in,out] rIds The list of IDs found in the document.
void _RefIdsMap::GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds)
{
    SwFieldType *const pType = rDoc.GetFldType(RES_SETEXPFLD, aName, false);

    if (!pType)
        return;

    SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
    for (SwFmtFld const* pF = aIter.First(); pF; pF = aIter.Next())
    {
        if (pF->GetTxtFld())
        {
            SwTxtNode const*const pNd = pF->GetTxtFld()->GetpTxtNode();
            if (pNd && pNd->GetNodes().IsDocNodes())
            {
                rIds.insert(static_cast<SwSetExpField const*>(pF->GetField())
                                ->GetSeqNumber());
            }
        }
    }
}

/// Get a sorted list of the footnote/endnote IDs from a document.
/// @param[in]     rDoc The document to search.
/// @param[in,out] rIds The list of IDs found in the document.
void _RefIdsMap::GetNoteIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds)
{
    for( sal_uInt16 n = rDoc.GetFtnIdxs().size(); n; )
        rIds.insert( rDoc.GetFtnIdxs()[ --n ]->GetSeqRefNo() );
}

/// Initialise the aIds and aDestIds collections from the source documents.
/// @param[in] rDoc     The source document.
/// @param[in] rDestDoc The destination document.
/// @param[in] bField   True if we're interested in all fields, false for footnotes.
void _RefIdsMap::Init( SwDoc& rDoc, SwDoc& rDestDoc, bool bField )
{
    if( bInit )
        return;

    if( bField )
    {
        GetFieldIdsFromDoc( rDestDoc, aIds );
        GetFieldIdsFromDoc( rDoc, aDstIds );

        // Map all the new src fields to the next available unused id
        for ( std::set<sal_uInt16>::iterator pIt = aDstIds.begin(); pIt != aDstIds.end(); ++pIt )
            AddId( GetFirstUnusedId(aIds), *pIt );

        // Change the Sequence number of all SetExp fields in the source document
        SwFieldType* pType = rDoc.GetFldType( RES_SETEXPFLD, aName, false );
        if( pType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
            for( SwFmtFld* pF = aIter.First(); pF; pF = aIter.Next() )
                if( pF->GetTxtFld() )
                {
                    SwSetExpField *const pSetExp(
                            static_cast<SwSetExpField *>(pF->GetField()));
                    sal_uInt16 const n = pSetExp->GetSeqNumber();
                    pSetExp->SetSeqNumber( sequencedIds[n] );
                }
        }
    }
    else
    {
        GetNoteIdsFromDoc( rDestDoc, aIds );
        GetNoteIdsFromDoc( rDoc, aDstIds );
    }
    bInit = true;
}

/// Get the lowest number unused in the passed set.
/// @param[in] rIds The set of used ID numbers.
/// @returns The lowest number unused by the passed set
sal_uInt16 _RefIdsMap::GetFirstUnusedId( std::set<sal_uInt16> &rIds )
{
    sal_uInt16 num(0);
    std::set<sal_uInt16>::iterator it;

    for( it = rIds.begin(); it != rIds.end(); ++it )
    {
        if( num != *it )
        {
            return num;
        }
        ++num;
    }
    return num;
}

/// Add a new ID and sequence number to the "occupied" collection.
/// @param[in] id     The ID number.
/// @param[in] seqNum The sequence number.
void _RefIdsMap::AddId( sal_uInt16 id, sal_uInt16 seqNum )
{
    aIds.insert( id );
    sequencedIds[ seqNum ] = id;
}

void _RefIdsMap::Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rFld,
                        bool bField )
{
    Init( rDoc, rDestDoc, bField);

    sal_uInt16 nSeqNo = rFld.GetSeqNo();

    // Check if the number is used in both documents
    // Note: For fields, aIds contains both the ids of SetExp from rDestDoc
    // and the targets of the already remapped ones from rDoc.
    // It is possible that aDstIds contains numbers that aIds does not contain!
    // For example, copying a selection to clipboard that does not contain
    // the first SwSetExpField will result in id 0 missing, then pasting that
    // into empty document gives a mapping 1->0 ... N->N-1 (fdo#63553).
    if (aIds.count(nSeqNo) || aDstIds.count(nSeqNo))
    {
        // Number already taken, so need a new one.
        if( sequencedIds.count(nSeqNo) )
            rFld.SetSeqNo( sequencedIds[nSeqNo] );
        else
        {
            assert(!bField || !aDstIds.count(nSeqNo)); // postcond of Init

            sal_uInt16 n = GetFirstUnusedId( aIds );

            // register the new SeqNo, so that it's "in use"
            AddId( n, nSeqNo );
            rFld.SetSeqNo( n );

            // and move the footnotes/endnotes to the new ID
            if( !bField )
            {
                SwTxtFtn* pFtnIdx;
                for( sal_uInt16 i = 0, nCnt = rDoc.GetFtnIdxs().size(); i < nCnt; ++i )
                    if( nSeqNo == (pFtnIdx = rDoc.GetFtnIdxs()[ i ])->GetSeqRefNo() )
                    {
                        pFtnIdx->SetSeqNo( n );
                        break;
                    }
            }
        }
    }
    else
    {
        AddId( nSeqNo, nSeqNo ); // this requires that nSeqNo is unused in both!
    }
}

void SwGetRefFieldType::MergeWithOtherDoc( SwDoc& rDestDoc )
{
    if( &rDestDoc != pDoc )
    {
        // then there are RefFields in the DescDox - so all RefFields in the SourceDoc
        // need to be converted to have unique IDs for both documents
        _RefIdsMap aFntMap( aEmptyOUStr );
        _RefIdsMaps aFldMap;

        SwIterator<SwFmtFld,SwFieldType> aIter( *this );
        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
        {
            SwGetRefField& rRefFld = *(SwGetRefField*)pFld->GetField();
            switch( rRefFld.GetSubType() )
            {
            case REF_SEQUENCEFLD:
                {
                    _RefIdsMap* pMap = 0;
                    for( sal_uInt16 n = aFldMap.size(); n; )
                    {
                        if( aFldMap[ --n ].GetName()==rRefFld.GetSetRefName() )
                        {
                            pMap = &aFldMap[ n ];
                            break;
                        }
                    }
                    if( !pMap )
                    {
                        pMap = new _RefIdsMap( rRefFld.GetSetRefName() );
                        aFldMap.push_back( pMap );
                    }

                    pMap->Check( *pDoc, rDestDoc, rRefFld, true );
                }
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                aFntMap.Check( *pDoc, rDestDoc, rRefFld, false );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
