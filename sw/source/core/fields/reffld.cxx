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
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <calbck.hxx>

#include <sfx2/childwin.hxx>

#include <memory>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;

static void lcl_GetLayTree( const SwFrame* pFrame, std::vector<const SwFrame*>& rArr )
{
    while( pFrame )
    {
        if( pFrame->IsBodyFrame() ) // unspectacular
            pFrame = pFrame->GetUpper();
        else
        {
            rArr.push_back( pFrame );

            // this is the last page
            if( pFrame->IsPageFrame() )
                break;

            if( pFrame->IsFlyFrame() )
                pFrame = static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame();
            else
                pFrame = pFrame->GetUpper();
        }
    }
}

bool IsFrameBehind( const SwTextNode& rMyNd, sal_Int32 nMySttPos,
                    const SwTextNode& rBehindNd, sal_Int32 nSttPos )
{
    const SwTextFrame *pMyFrame = static_cast<SwTextFrame*>(rMyNd.getLayoutFrame( rMyNd.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr, false) ),
                   *pFrame = static_cast<SwTextFrame*>(rBehindNd.getLayoutFrame( rBehindNd.GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr, false) );

    while( pFrame && !pFrame->IsInside( nSttPos ) )
        pFrame = pFrame->GetFollow();
    while( pMyFrame && !pMyFrame->IsInside( nMySttPos ) )
        pMyFrame = pMyFrame->GetFollow();

    if( !pFrame || !pMyFrame || pFrame == pMyFrame )
        return false;

    std::vector<const SwFrame*> aRefArr, aArr;
    ::lcl_GetLayTree( pFrame, aRefArr );
    ::lcl_GetLayTree( pMyFrame, aArr );

    size_t nRefCnt = aRefArr.size() - 1, nCnt = aArr.size() - 1;
    bool bVert = false;
    bool bR2L = false;

    // Loop as long as a frame does not equal?
    while( nRefCnt && nCnt && aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        const SwFrame* pTmpFrame = aArr[ nCnt ];
        bVert = pTmpFrame->IsVertical();
        bR2L = pTmpFrame->IsRightToLeft();
        --nCnt;
        --nRefCnt;
    }

    // If a counter overflows?
    if( aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        if( nCnt )
            --nCnt;
        else
            --nRefCnt;
    }

    const SwFrame* pRefFrame = aRefArr[ nRefCnt ];
    const SwFrame* pFieldFrame = aArr[ nCnt ];

    // different frames, check their Y-/X-position
    bool bRefIsLower = false;
    if( ( FRM_COLUMN | FRM_CELL ) & pFieldFrame->GetType() ||
        ( FRM_COLUMN | FRM_CELL ) & pRefFrame->GetType() )
    {
        if( pFieldFrame->GetType() == pRefFrame->GetType() )
        {
            // here, the X-pos is more important
            if( bVert )
            {
                if( bR2L )
                    bRefIsLower = pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() ||
                            ( pRefFrame->Frame().Top() == pFieldFrame->Frame().Top() &&
                              pRefFrame->Frame().Left() < pFieldFrame->Frame().Left() );
                else
                    bRefIsLower = pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() ||
                            ( pRefFrame->Frame().Top() == pFieldFrame->Frame().Top() &&
                              pRefFrame->Frame().Left() > pFieldFrame->Frame().Left() );
            }
            else if( bR2L )
                bRefIsLower = pRefFrame->Frame().Left() > pFieldFrame->Frame().Left() ||
                            ( pRefFrame->Frame().Left() == pFieldFrame->Frame().Left() &&
                              pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() );
            else
                bRefIsLower = pRefFrame->Frame().Left() < pFieldFrame->Frame().Left() ||
                            ( pRefFrame->Frame().Left() == pFieldFrame->Frame().Left() &&
                              pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() );
            pRefFrame = nullptr;
        }
        else if( ( FRM_COLUMN | FRM_CELL ) & pFieldFrame->GetType() )
            pFieldFrame = aArr[ nCnt - 1 ];
        else
            pRefFrame = aRefArr[ nRefCnt - 1 ];
    }

    if( pRefFrame ) // misuse as flag
    {
        if( bVert )
        {
            if( bR2L )
                bRefIsLower = pRefFrame->Frame().Left() < pFieldFrame->Frame().Left() ||
                            ( pRefFrame->Frame().Left() == pFieldFrame->Frame().Left() &&
                                pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() );
            else
                bRefIsLower = pRefFrame->Frame().Left() > pFieldFrame->Frame().Left() ||
                            ( pRefFrame->Frame().Left() == pFieldFrame->Frame().Left() &&
                                pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() );
        }
        else if( bR2L )
            bRefIsLower = pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() ||
                        ( pRefFrame->Frame().Top() == pFieldFrame->Frame().Top() &&
                            pRefFrame->Frame().Left() > pFieldFrame->Frame().Left() );
        else
            bRefIsLower = pRefFrame->Frame().Top() < pFieldFrame->Frame().Top() ||
                        ( pRefFrame->Frame().Top() == pFieldFrame->Frame().Top() &&
                            pRefFrame->Frame().Left() < pFieldFrame->Frame().Left() );
    }
    return bRefIsLower;
}

/// get references
SwGetRefField::SwGetRefField( SwGetRefFieldType* pFieldType,
                              const OUString& rSetRef, sal_uInt16 nSubTyp,
                              sal_uInt16 nSeqenceNo, sal_uLong nFormat )
    : SwField( pFieldType, nFormat ),
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

const SwTextNode* SwGetRefField::GetReferencedTextNode() const
{
    SwGetRefFieldType *pTyp = dynamic_cast<SwGetRefFieldType*>(GetTyp());
    if (!pTyp)
        return nullptr;
    sal_Int32 nDummy = -1;
    return SwGetRefFieldType::FindAnchor( pTyp->GetDoc(), sSetRefName, nSubType, nSeqNo, &nDummy );
}

// #i85090#
OUString SwGetRefField::GetExpandedTextOfReferencedTextNode() const
{
    const SwTextNode* pReferencedTextNode( GetReferencedTextNode() );
    return pReferencedTextNode
           ? OUString(pReferencedTextNode->GetExpandText( 0, -1, true, true, false, false ))
           : OUString();
}

OUString SwGetRefField::Expand() const
{
    return sText;
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

// #i81002# - parameter <pFieldTextAttr> added
void SwGetRefField::UpdateField( const SwTextField* pFieldTextAttr )
{
    sText.clear();

    SwDoc* pDoc = static_cast<SwGetRefFieldType*>(GetTyp())->GetDoc();
    // finding the reference target (the number)
    sal_Int32 nNumStart = -1;
    sal_Int32 nNumEnd = -1;
    SwTextNode* pTextNd = SwGetRefFieldType::FindAnchor(
        pDoc, sSetRefName, nSubType, nSeqNo, &nNumStart, &nNumEnd
    );
    // not found?
    if ( !pTextNd )
    {
        sText = SwViewShell::GetShellRes()->aGetRefField_RefItemNotFound;
        return ;
    }
    // where is the category name (e.g. "Illustration")?
    const OUString aText = pTextNd->GetText();
    const sal_Int32 nCatStart = aText.indexOf(sSetRefName);
    const bool bHasCat = nCatStart>=0;
    const sal_Int32 nCatEnd = bHasCat ? nCatStart + sSetRefName.getLength() : -1;

    // length of the referenced text
    const sal_Int32 nLen = aText.getLength();

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
                    if (const SwTextAttr* const pTextAttr =
                        pTextNd->GetTextAttrForCharAt(nNumStart, RES_TXTATR_FIELD)
                    ) {
                        // start searching from nFrom
                        const sal_Int32 nFrom = bHasCat
                            ? std::max(nNumStart + 1, nCatEnd)
                            : nNumStart + 1;
                        nStart = SwGetExpField::GetReferenceTextPos( pTextAttr->GetFormatField(), *pDoc, nFrom );
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
                // text is spread across multiple nodes - get whole text or only until end of node?
                nEnd = nNumEnd<0 ? nLen : nNumEnd;
                break;

            case REF_OUTLINE:
                nStart = nNumStart;
                nEnd = nNumEnd;
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                // get number or numString
                for( size_t i = 0; i < pDoc->GetFootnoteIdxs().size(); ++i )
                {
                    SwTextFootnote* const pFootnoteIdx = pDoc->GetFootnoteIdxs()[i];
                    if( nSeqNo == pFootnoteIdx->GetSeqRefNo() )
                    {
                        sText = pFootnoteIdx->GetFootnote().GetViewNumStr( *pDoc );
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
                sText = pTextNd->GetExpandText( nStart, nEnd - nStart, false, false, false, false );

                // remove all special characters (replace them with blanks)
                if( !sText.isEmpty() )
                {
                    sText = comphelper::string::remove(sText, 0xad);
                    OUStringBuffer aBuf(sText);
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
                    sText = aBuf.makeStringAndClear();
                }
            }
        }
        break;

    case REF_PAGE:
    case REF_PAGE_PGDESC:
        {
            const SwTextFrame* pFrame = static_cast<SwTextFrame*>(pTextNd->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr, false)),
                        *pSave = pFrame;
            while( pFrame && !pFrame->IsInside( nNumStart ) )
                pFrame = pFrame->GetFollow();

            if( pFrame || nullptr != ( pFrame = pSave ))
            {
                sal_uInt16 nPageNo = pFrame->GetVirtPageNum();
                const SwPageFrame *pPage;
                if( REF_PAGE_PGDESC == GetFormat() &&
                    nullptr != ( pPage = pFrame->FindPageFrame() ) &&
                    pPage->GetPageDesc() )
                    sText = pPage->GetPageDesc()->GetNumType().GetNumStr( nPageNo );
                else
                    sText = OUString::number(nPageNo);
            }
        }
        break;

    case REF_CHAPTER:
        {
            // a bit tricky: search any frame
            const SwFrame* pFrame = pTextNd->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
            if( pFrame )
            {
                SwChapterFieldType aFieldTyp;
                SwChapterField aField( &aFieldTyp, 0 );
                aField.SetLevel( MAXLEVEL - 1 );
                aField.ChangeExpansion( pFrame, pTextNd, true );
                sText = aField.GetNumber();
            }
        }
        break;

    case REF_UPDOWN:
        {
            // #i81002#
            // simplified: use parameter <pFieldTextAttr>
            if( !pFieldTextAttr || !pFieldTextAttr->GetpTextNode() )
                break;

            LanguageTag aLanguageTag( GetLanguage());
            LocaleDataWrapper aLocaleData( aLanguageTag );

            // first a "short" test - in case both are in the same node
            if( pFieldTextAttr->GetpTextNode() == pTextNd )
            {
                sText = nNumStart < pFieldTextAttr->GetStart()
                            ? aLocaleData.getAboveWord()
                            : aLocaleData.getBelowWord();
                break;
            }

            sText = ::IsFrameBehind( *pFieldTextAttr->GetpTextNode(), pFieldTextAttr->GetStart(),
                                    *pTextNd, nNumStart )
                        ? aLocaleData.getAboveWord()
                        : aLocaleData.getBelowWord();
        }
        break;
    // #i81002#
    case REF_NUMBER:
    case REF_NUMBER_NO_CONTEXT:
    case REF_NUMBER_FULL_CONTEXT:
        {
            if ( pFieldTextAttr && pFieldTextAttr->GetpTextNode() )
            {
                sText = MakeRefNumStr( pFieldTextAttr->GetTextNode(), *pTextNd, GetFormat() );
            }
        }
        break;

    default:
        OSL_FAIL("<SwGetRefField::UpdateField(..)> - unknown format type");
    }
}

// #i81002#
OUString SwGetRefField::MakeRefNumStr( const SwTextNode& rTextNodeOfField,
                                     const SwTextNode& rTextNodeOfReferencedItem,
                                     const sal_uInt32 nRefNumFormat )
{
    if ( rTextNodeOfReferencedItem.HasNumber() &&
         rTextNodeOfReferencedItem.IsCountedInList() )
    {
        OSL_ENSURE( rTextNodeOfReferencedItem.GetNum(),
                "<SwGetRefField::MakeRefNumStr(..)> - referenced paragraph has number, but no <SwNodeNum> instance!" );

        // Determine, up to which level the superior list labels have to be
        // included - default is to include all superior list labels.
        sal_uInt8 nRestrictInclToThisLevel( 0 );
        // Determine for format REF_NUMBER the level, up to which the superior
        // list labels have to be restricted, if the text node of the reference
        // field and the text node of the referenced item are in the same
        // document context.
        if ( nRefNumFormat == REF_NUMBER &&
             rTextNodeOfField.FindFlyStartNode()
                            == rTextNodeOfReferencedItem.FindFlyStartNode() &&
             rTextNodeOfField.FindFootnoteStartNode()
                            == rTextNodeOfReferencedItem.FindFootnoteStartNode() &&
             rTextNodeOfField.FindHeaderStartNode()
                            == rTextNodeOfReferencedItem.FindHeaderStartNode() &&
             rTextNodeOfField.FindFooterStartNode()
                            == rTextNodeOfReferencedItem.FindFooterStartNode() )
        {
            const SwNodeNum* pNodeNumForTextNodeOfField( nullptr );
            if ( rTextNodeOfField.HasNumber() &&
                 rTextNodeOfField.GetNumRule() == rTextNodeOfReferencedItem.GetNumRule() )
            {
                pNodeNumForTextNodeOfField = rTextNodeOfField.GetNum();
            }
            else
            {
                pNodeNumForTextNodeOfField =
                    rTextNodeOfReferencedItem.GetNum()->GetPrecedingNodeNumOf( rTextNodeOfField );
            }
            if ( pNodeNumForTextNodeOfField )
            {
                const SwNumberTree::tNumberVector rFieldNumVec = pNodeNumForTextNodeOfField->GetNumberVector();
                const SwNumberTree::tNumberVector rRefItemNumVec = rTextNodeOfReferencedItem.GetNum()->GetNumberVector();
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
            ( nRestrictInclToThisLevel < rTextNodeOfReferencedItem.GetActualListLevel() &&
              ( nRefNumFormat == REF_NUMBER || nRefNumFormat == REF_NUMBER_FULL_CONTEXT ) ) );

        OSL_ENSURE( rTextNodeOfReferencedItem.GetNumRule(),
                "<SwGetRefField::MakeRefNumStr(..)> - referenced numbered paragraph has no numbering rule set!" );
        return rTextNodeOfReferencedItem.GetNumRule()->MakeRefNumString(
                                            *(rTextNodeOfReferencedItem.GetNum()),
                                            bInclSuperiorNumLabels,
                                            nRestrictInclToThisLevel );
    }

    return OUString();
}

SwField* SwGetRefField::Copy() const
{
    SwGetRefField* pField = new SwGetRefField( static_cast<SwGetRefFieldType*>(GetTyp()),
                                                sSetRefName, nSubType,
                                                nSeqNo, GetFormat() );
    pField->sText = sText;
    return pField;
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
        SwDoc* pDoc = static_cast<SwGetRefFieldType*>(GetTyp())->GetDoc();
        const OUString rPar1 = GetPar1();
        // don't convert when the name points to an existing field type
        if(!pDoc->getIDocumentFieldsAccess().GetFieldType(RES_SETEXPFLD, rPar1, false))
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
        SwIterator<SwFormatField,SwFieldType> aIter( *this );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
        {
            // update only the GetRef fields
            //JP 3.4.2001: Task 71231 - we need the correct language
            SwGetRefField* pGRef = static_cast<SwGetRefField*>(pFormatField->GetField());
            const SwTextField* pTField;
            if( !pGRef->GetLanguage() &&
                nullptr != ( pTField = pFormatField->GetTextField()) &&
                pTField->GetpTextNode() )
            {
                pGRef->SetLanguage( pTField->GetpTextNode()->GetLang(
                                                pTField->GetStart() ) );
            }

            // #i81002#
            pGRef->UpdateField( pFormatField->GetTextField() );
        }
    }
    // forward to text fields, they "expand" the text
    NotifyClients( pOld, pNew );
}

SwTextNode* SwGetRefFieldType::FindAnchor( SwDoc* pDoc, const OUString& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_Int32* pStt, sal_Int32* pEnd )
{
    OSL_ENSURE( pStt, "Why did no one check the StartPos?" );

    SwTextNode* pTextNd = nullptr;
    switch( nSubType )
    {
    case REF_SETREFATTR:
        {
            const SwFormatRefMark *pRef = pDoc->GetRefMark( rRefMark );
            if( pRef && pRef->GetTextRefMark() )
            {
                pTextNd = const_cast<SwTextNode*>(&pRef->GetTextRefMark()->GetTextNode());
                *pStt = pRef->GetTextRefMark()->GetStart();
                if( pEnd )
                    *pEnd = *pRef->GetTextRefMark()->GetAnyEnd();
            }
        }
        break;

    case REF_SEQUENCEFLD:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetFieldType( RES_SETEXPFLD, rRefMark, false );
            if( pFieldType && pFieldType->HasWriterListeners() &&
                nsSwGetSetExpType::GSE_SEQ & static_cast<SwSetExpFieldType*>(pFieldType)->GetType() )
            {
                SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
                for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
                {
                    if( pFormatField->GetTextField() && nSeqNo ==
                        static_cast<SwSetExpField*>(pFormatField->GetField())->GetSeqNumber() )
                    {
                        SwTextField* pTextField = pFormatField->GetTextField();
                        pTextNd = pTextField->GetpTextNode();
                        *pStt = pTextField->GetStart();
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

                pTextNd = pPos->nNode.GetNode().GetTextNode();
                *pStt = pPos->nContent.GetIndex();
                if(pEnd)
                {
                    if(!pBkmk->IsExpanded())
                    {
                        *pEnd = *pStt;
                        // #i81002#
                        if(dynamic_cast< ::sw::mark::CrossRefBookmark const *>(pBkmk))
                        {
                            OSL_ENSURE( pTextNd,
                                    "<SwGetRefFieldType::FindAnchor(..)> - node marked by cross-reference bookmark isn't a text node --> crash" );
                            *pEnd = pTextNd->Len();
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
            for( auto pFootnoteIdx : pDoc->GetFootnoteIdxs() )
                if( nSeqNo == pFootnoteIdx->GetSeqRefNo() )
                {
                    SwNodeIndex* pIdx = pFootnoteIdx->GetStartNode();
                    if( pIdx )
                    {
                        SwNodeIndex aIdx( *pIdx, 1 );
                        if( nullptr == ( pTextNd = aIdx.GetNode().GetTextNode()))
                            pTextNd = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext( &aIdx ));
                    }
                    *pStt = 0;
                    if( pEnd )
                        *pEnd = 0;
                    break;
                }
        }
        break;
    }

    return pTextNd;
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
    static void GetNoteIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       AddId( sal_uInt16 id, sal_uInt16 seqNum );
    static sal_uInt16 GetFirstUnusedId( std::set<sal_uInt16> &rIds );

public:
    explicit _RefIdsMap( const OUString& rName ) : aName( rName ), bInit( false ) {}

    void Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rField, bool bField );

    OUString GetName() { return aName; }
};

/// Get a sorted list of the field IDs from a document.
/// @param[in]     rDoc The document to search.
/// @param[in,out] rIds The list of IDs found in the document.
void _RefIdsMap::GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds)
{
    SwFieldType *const pType = rDoc.getIDocumentFieldsAccess().GetFieldType(RES_SETEXPFLD, aName, false);

    if (!pType)
        return;

    SwIterator<SwFormatField,SwFieldType> aIter( *pType );
    for (SwFormatField const* pF = aIter.First(); pF; pF = aIter.Next())
    {
        if (pF->GetTextField())
        {
            SwTextNode const*const pNd = pF->GetTextField()->GetpTextNode();
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
    for( auto n = rDoc.GetFootnoteIdxs().size(); n; )
        rIds.insert( rDoc.GetFootnoteIdxs()[ --n ]->GetSeqRefNo() );
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
        SwFieldType* pType = rDoc.getIDocumentFieldsAccess().GetFieldType( RES_SETEXPFLD, aName, false );
        if( pType )
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *pType );
            for( SwFormatField* pF = aIter.First(); pF; pF = aIter.Next() )
                if( pF->GetTextField() )
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

        for (std::set<sal_uInt16>::iterator pIt = aDstIds.begin(); pIt != aDstIds.end(); ++pIt)
            AddId( GetFirstUnusedId(aIds), *pIt );

        // Change the footnotes/endnotes in the source doc to the new ID
        for ( const auto pFootnoteIdx : rDoc.GetFootnoteIdxs() )
        {
            sal_uInt16 const n = pFootnoteIdx->GetSeqRefNo();
            pFootnoteIdx->SetSeqNo(sequencedIds[n]);
        }
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

void _RefIdsMap::Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rField,
                        bool bField )
{
    Init( rDoc, rDestDoc, bField);

    sal_uInt16 const nSeqNo = rField.GetSeqNo();

    // check if it needs to be remapped
    // if sequencedIds doesn't contain the number, it means there is no
    // SetExp field / footnote in the source document: do not modify
    // the number, which works well for copy from/paste to same document
    // (and if it is not the same document, there's no "correct" result anyway)
    if (sequencedIds.count(nSeqNo))
    {
        rField.SetSeqNo( sequencedIds[nSeqNo] );
    }
}

/// 1. if _both_ SetExp + GetExp / Footnote + GetExp field are copied,
///    ensure that both get a new unused matching number
/// 2. if only SetExp / Footnote is copied, it gets a new unused number
/// 3. if only GetExp field is copied, for the case of copy from / paste to
///    same document it's desirable to keep the same number;
///    for other cases of copy/paste or master documents it's not obvious
///    what is most desirable since it's going to be wrong anyway
void SwGetRefFieldType::MergeWithOtherDoc( SwDoc& rDestDoc )
{
    if( &rDestDoc != pDoc )
    {
        if (rDestDoc.IsClipBoard())
        {
            // when copying _to_ clipboard, expectation is that no fields exist
            // so no re-mapping is required to avoid collisions
            assert(!rDestDoc.getIDocumentFieldsAccess().GetSysFieldType(RES_GETREFFLD)->HasWriterListeners());
            return; // don't modify the fields in the source doc
        }

        // then there are RefFields in the DescDox - so all RefFields in the SourceDoc
        // need to be converted to have unique IDs for both documents
        _RefIdsMap aFntMap( aEmptyOUStr );
        std::vector<std::unique_ptr<_RefIdsMap>> aFieldMap;

        SwIterator<SwFormatField,SwFieldType> aIter( *this );
        for( SwFormatField* pField = aIter.First(); pField; pField = aIter.Next() )
        {
            SwGetRefField& rRefField = *static_cast<SwGetRefField*>(pField->GetField());
            switch( rRefField.GetSubType() )
            {
            case REF_SEQUENCEFLD:
                {
                    _RefIdsMap* pMap = nullptr;
                    for( auto n = aFieldMap.size(); n; )
                    {
                        if (aFieldMap[ --n ]->GetName() == rRefField.GetSetRefName())
                        {
                            pMap = aFieldMap[ n ].get();
                            break;
                        }
                    }
                    if( !pMap )
                    {
                        pMap = new _RefIdsMap( rRefField.GetSetRefName() );
                        aFieldMap.push_back(std::unique_ptr<_RefIdsMap>(pMap));
                    }

                    pMap->Check( *pDoc, rDestDoc, rRefField, true );
                }
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                aFntMap.Check( *pDoc, rDestDoc, rRefField, false );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
