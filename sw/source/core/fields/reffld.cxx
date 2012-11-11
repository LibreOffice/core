/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <algorithm> // min, max

#include <sfx2/childwin.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using ::rtl::OUString;

extern void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx, sal_uInt16* pInsPos = 0 );

static void lcl_GetLayTree( const SwFrm* pFrm, std::vector<const SwFrm*>& rArr )
{
    while( pFrm )
    {
        if( pFrm->IsBodyFrm() )     // soll uns nicht weiter interessieren
            pFrm = pFrm->GetUpper();
        else
        {
            rArr.push_back( pFrm );

            // bei der Seite ist schluss
            if( pFrm->IsPageFrm() )
                break;

            if( pFrm->IsFlyFrm() )
                pFrm = ((SwFlyFrm*)pFrm)->GetAnchorFrm();
            else
                pFrm = pFrm->GetUpper();
        }
    }
}


bool IsFrameBehind( const SwTxtNode& rMyNd, sal_uInt16 nMySttPos,
                    const SwTxtNode& rBehindNd, sal_uInt16 nSttPos )
{
    const SwTxtFrm *pMyFrm = (SwTxtFrm*)rMyNd.getLayoutFrm( rMyNd.GetDoc()->GetCurrentLayout(), 0,0,sal_False),
                   *pFrm = (SwTxtFrm*)rBehindNd.getLayoutFrm( rBehindNd.GetDoc()->GetCurrentLayout(), 0,0,sal_False);

    while( pFrm && !pFrm->IsInside( nSttPos ) )
        pFrm = (SwTxtFrm*)pFrm->GetFollow();
    while( pMyFrm && !pMyFrm->IsInside( nMySttPos ) )
        pMyFrm = (SwTxtFrm*)pMyFrm->GetFollow();

    if( !pFrm || !pMyFrm || pFrm == pMyFrm )
        return false;

    std::vector<const SwFrm*> aRefArr, aArr;
    ::lcl_GetLayTree( pFrm, aRefArr );
    ::lcl_GetLayTree( pMyFrm, aArr );

    sal_uInt16 nRefCnt = aRefArr.size() - 1, nCnt = aArr.size() - 1;
    sal_Bool bVert = sal_False;
    sal_Bool bR2L = sal_False;

    // solange bis ein Frame ungleich ist ?
    while( nRefCnt && nCnt && aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        const SwFrm* pTmpFrm = aArr[ nCnt ];
        bVert = pTmpFrm->IsVertical();
        bR2L = pTmpFrm->IsRightToLeft();
        --nCnt, --nRefCnt;
    }

    // sollte einer der Counter ueberlaeufen?
    if( aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        if( nCnt )
            --nCnt;
        else
            --nRefCnt;
    }

    const SwFrm* pRefFrm = aRefArr[ nRefCnt ];
    const SwFrm* pFldFrm = aArr[ nCnt ];

    // unterschiedliche Frames, dann ueberpruefe deren Y-/X-Position
    bool bRefIsLower = false;
    if( ( FRM_COLUMN | FRM_CELL ) & pFldFrm->GetType() ||
        ( FRM_COLUMN | FRM_CELL ) & pRefFrm->GetType() )
    {
        if( pFldFrm->GetType() == pRefFrm->GetType() )
        {
            // hier ist die X-Pos wichtiger!
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

    if( pRefFrm )               // als Flag missbrauchen
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

/*--------------------------------------------------------------------
    Beschreibung: Referenzen holen
 --------------------------------------------------------------------*/


SwGetRefField::SwGetRefField( SwGetRefFieldType* pFldType,
                              const String& rSetRef, sal_uInt16 nSubTyp,
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

String SwGetRefField::GetDescription() const
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
    sal_uInt16 nDummy = USHRT_MAX;
    return SwGetRefFieldType::FindAnchor( pDoc, sSetRefName, nSubType, nSeqNo, &nDummy );
}
// #i85090#
String SwGetRefField::GetExpandedTxtOfReferencedTxtNode() const
{
    const SwTxtNode* pReferencedTxtNode( GetReferencedTxtNode() );
    return pReferencedTxtNode
           ? pReferencedTxtNode->GetExpandTxt( 0, STRING_LEN, true, true )
           : aEmptyStr;
}

String SwGetRefField::Expand() const
{
    return sTxt;
}


String SwGetRefField::GetFieldName() const
{
    if ( GetTyp()->GetName().getLength() > 0 || sSetRefName.getLength() > 0 )
    {
        String aStr(GetTyp()->GetName());
        aStr += ' ';
        aStr += sSetRefName;
        return aStr;
    }
    else
        return Expand();
}

// #i81002# - parameter <pFldTxtAttr> added
void SwGetRefField::UpdateField( const SwTxtFld* pFldTxtAttr )
{
    sTxt.Erase();

    SwDoc* pDoc = ((SwGetRefFieldType*)GetTyp())->GetDoc();
    // finding the reference target (the number)
    sal_uInt16 nNumStart, nNumEnd;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor(
        pDoc, sSetRefName, nSubType, nSeqNo, &nNumStart, &nNumEnd
    );
    // not found?
    if ( !pTxtNd )
    {
        sTxt = ViewShell::GetShellRes()->aGetRefFld_RefItemNotFound;
        return ;
    }
    // where is the category name (e.g. "Illustration")?
    rtl::OUString const Text = pTxtNd->GetTxt();
    unsigned const nCatStart = Text.indexOf(sSetRefName);
    unsigned const nCatEnd = nCatStart == unsigned(-1) ?
        unsigned(-1) : nCatStart + sSetRefName.getLength();
    bool const bHasCat = nCatStart != unsigned(-1);

    // length of the referenced text
    unsigned const nLen = Text.getLength();

    // which format?
    switch( GetFormat() )
    {
    case REF_CONTENT:
    case REF_ONLYNUMBER:
    case REF_ONLYCAPTION:
    case REF_ONLYSEQNO:
        {
            // needed part of Text
            unsigned nStart, nEnd;

            switch( nSubType )
            {
            case REF_SEQUENCEFLD:

                switch( GetFormat() )
                {
                // "Category and Number"
                case REF_ONLYNUMBER:
                    if (bHasCat) {
                        nStart = std::min<unsigned>(nNumStart, nCatStart);
                        nEnd = std::max<unsigned>(nNumEnd, nCatEnd);
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
                        unsigned const nFrom = bHasCat ?
                            std::max<unsigned>(nNumStart + 1, nCatEnd) : nNumStart + 1;
                        nStart = SwGetExpField::GetReferenceTextPos(
                            pTxtAttr->GetFld(), *pDoc, nFrom
                        );
                    } else {
                        nStart = bHasCat ?
                            std::max<unsigned>(nNumEnd, nCatEnd) : nNumEnd;
                    }
                    nEnd = nLen;
                    break;
                }

                // "Numbering"
                case REF_ONLYSEQNO:
                    nStart = nNumStart;
                    nEnd = std::min<unsigned>(nStart + 1, nLen);
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
                nEnd = nNumEnd == USHRT_MAX ? nLen : nNumEnd;
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

            if( nStart != nEnd ) // ein Bereich?
            {
                sTxt = pTxtNd->GetExpandTxt( nStart, nEnd - nStart );

                // alle Sonderzeichen entfernen (durch Blanks ersetzen):
                if( sTxt.Len() )
                {
                    sTxt = comphelper::string::remove(sTxt, 0xad);
                    for( sal_Unicode* p = sTxt.GetBufferAccess(); *p; ++p )
                    {
                        if( *p < 0x20 )
                            *p = 0x20;
                        else if(*p == 0x2011)
                            *p = '-';
                    }
                }
            }
        }
        break;

    case REF_PAGE:
    case REF_PAGE_PGDESC:
        {
            const SwTxtFrm* pFrm = (SwTxtFrm*)pTxtNd->getLayoutFrm( pDoc->GetCurrentLayout(), 0,0,sal_False),
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
                    sTxt = String::CreateFromInt32(nPageNo);
            }
        }
        break;

    case REF_CHAPTER:
        {
            // ein bischen trickreich: suche irgend einen Frame
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

            LocaleDataWrapper aLocaleData(
                            ::comphelper::getProcessServiceFactory(),
                            SvxCreateLocale( GetLanguage() ) );

            // erstmal ein "Kurz" - Test - falls beide im selben
            // Node stehen!
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
String SwGetRefField::MakeRefNumStr( const SwTxtNode& rTxtNodeOfField,
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

    return String();
}

SwField* SwGetRefField::Copy() const
{
    SwGetRefField* pFld = new SwGetRefField( (SwGetRefFieldType*)GetTyp(),
                                                sSetRefName, nSubType,
                                                nSeqNo, GetFormat() );
    pFld->sTxt = sTxt;
    return pFld;
}

/*--------------------------------------------------------------------
    Beschreibung: ReferenzName holen
 --------------------------------------------------------------------*/


const rtl::OUString& SwGetRefField::GetPar1() const
{
    return sSetRefName;
}


void SwGetRefField::SetPar1( const rtl::OUString& rName )
{
    sSetRefName = rName;
}


rtl::OUString SwGetRefField::GetPar2() const
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
        String  sTmp(GetPar1());
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
        rAny <<= rtl::OUString(sTmp);
    }
    break;
    case FIELD_PROP_PAR3:
        rAny <<= rtl::OUString(Expand());
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
    String sTmp;
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
        SetExpand( ::GetString( rAny, sTmp ));
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
        const String& rPar1 = GetPar1();
        //don't convert when the name points to an existing field type
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
    // Update auf alle GetReferenz-Felder
    if( !pNew && !pOld )
    {
        SwIterator<SwFmtFld,SwFieldType> aIter( *this );
        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
        {
            // nur die GetRef-Felder Updaten
            //JP 3.4.2001: Task 71231 - we need the correct language
            SwGetRefField* pGRef = (SwGetRefField*)pFld->GetFld();
            const SwTxtFld* pTFld;
            if( !pGRef->GetLanguage() &&
                0 != ( pTFld = pFld->GetTxtFld()) &&
                pTFld->GetpTxtNode() )
            {
                pGRef->SetLanguage( pTFld->GetpTxtNode()->GetLang(
                                                *pTFld->GetStart() ) );
            }

            // #i81002#
            pGRef->UpdateField( pFld->GetTxtFld() );
        }
    }
    // weiter an die Text-Felder, diese "Expandieren" den Text
    NotifyClients( pOld, pNew );
}

SwTxtNode* SwGetRefFieldType::FindAnchor( SwDoc* pDoc, const String& rRefMark,
                                        sal_uInt16 nSubType, sal_uInt16 nSeqNo,
                                        sal_uInt16* pStt, sal_uInt16* pEnd )
{
    OSL_ENSURE( pStt, "warum wird keine StartPos abgefragt?" );

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
                for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
                {
                    if( pFld->GetTxtFld() && nSeqNo ==
                        ((SwSetExpField*)pFld->GetFld())->GetSeqNumber() )
                    {
                        SwTxtFld* pTxtFld = pFld->GetTxtFld();
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
            if(ppMark != pDoc->getIDocumentMarkAccess()->getMarksEnd())
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
                        *pEnd = USHRT_MAX;
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
    String aName;
    std::set<sal_uInt16> aIds;
    std::set<sal_uInt16> aDstIds;
    std::map<sal_uInt16, sal_uInt16> sequencedIds; /// ID numbers sorted by sequence number.
    sal_Bool bInit;

    void       Init(SwDoc& rDoc, SwDoc& rDestDoc, sal_Bool bField );
    void       GetNoteIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds );
    void       AddId( sal_uInt16 id, sal_uInt16 seqNum );
    sal_uInt16 GetFirstUnusedId( std::set<sal_uInt16> &rIds );

public:
    _RefIdsMap( const String& rName ) : aName( rName ), bInit( sal_False ) {}

    void Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rFld, sal_Bool bField );

    String GetName() { return aName; }
};

typedef boost::ptr_vector<_RefIdsMap> _RefIdsMaps;

/// Get a sorted list of the field IDs from a document.
/// @param[in]     rDoc The document to search.
/// @param[in,out] rIds The list of IDs found in the document.
void _RefIdsMap::GetFieldIdsFromDoc( SwDoc& rDoc, std::set<sal_uInt16> &rIds)
{
    const SwTxtNode* pNd;
    SwFieldType* pType;

    if( 0 != ( pType = rDoc.GetFldType( RES_SETEXPFLD, aName, false ) ))
    {
        SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
        for( SwFmtFld* pF = aIter.First(); pF; pF = aIter.Next() )
            if( pF->GetTxtFld() &&
                0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
                pNd->GetNodes().IsDocNodes() )
                rIds.insert( ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
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
void _RefIdsMap::Init( SwDoc& rDoc, SwDoc& rDestDoc, sal_Bool bField )
{
    if( bInit )
        return;

    if( bField )
    {
        GetFieldIdsFromDoc( rDestDoc, aIds );
        GetFieldIdsFromDoc( rDoc, aDstIds );

        // Define the mappings now
        sal_uInt16 nMaxDstId = -1;
        if ( !aIds.empty() )
            nMaxDstId = *aIds.rbegin();

        // Map all the src fields to their value + nMaxDstId
        for ( std::set<sal_uInt16>::iterator pIt = aDstIds.begin(); pIt != aDstIds.end(); ++pIt )
            AddId( ++nMaxDstId, *pIt );

        // Change the Sequence number of all the SetExp fields in the destination document
        SwFieldType* pType = rDoc.GetFldType( RES_SETEXPFLD, aName, false );
        if( pType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
            for( SwFmtFld* pF = aIter.First(); pF; pF = aIter.Next() )
                if( pF->GetTxtFld() )
                {
                    sal_uInt16 n = ((SwSetExpField*)pF->GetFld())->GetSeqNumber( );
                    ((SwSetExpField*)pF->GetFld())->SetSeqNumber( sequencedIds[ n ] );
                }
        }
    }
    else
    {
        GetNoteIdsFromDoc( rDestDoc, aIds );
        GetNoteIdsFromDoc( rDoc, aDstIds );
    }
    bInit = sal_True;
}

/// Get the lowest unused ID in the passed set.
/// @param[in] rIds The set of used ID numbers.
/// @returns The lowest unused ID.
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
                        sal_Bool bField )
{
    Init( rDoc, rDestDoc, bField);

    // dann teste mal, ob die Nummer schon vergeben ist
    // oder ob eine neue bestimmt werden muss.
    sal_uInt16 nSeqNo = rFld.GetSeqNo();
    if( aIds.count( nSeqNo ) && aDstIds.count( nSeqNo ))
    {
        // Number already taken, so need a new one.
        if( sequencedIds.count(nSeqNo) )
            rFld.SetSeqNo( sequencedIds[nSeqNo] );
        else
        {
            sal_uInt16 n = GetFirstUnusedId( aIds );

            // die neue SeqNo eintragen, damit die "belegt" ist
            AddId( n, nSeqNo );
            rFld.SetSeqNo( n );

            // und noch die Fuss-/EndNote auf die neue Id umsetzen
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
        AddId( nSeqNo, nSeqNo );
    }
}


void SwGetRefFieldType::MergeWithOtherDoc( SwDoc& rDestDoc )
{
    if( &rDestDoc != pDoc )
    {
        // dann gibt es im DestDoc RefFelder, also muessen im SourceDoc
        // alle RefFelder auf einduetige Ids in beiden Docs umgestellt
        // werden.
        _RefIdsMap aFntMap( aEmptyStr );
        _RefIdsMaps aFldMap;

        SwIterator<SwFmtFld,SwFieldType> aIter( *this );
        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
        {
            SwGetRefField& rRefFld = *(SwGetRefField*)pFld->GetFld();
            switch( rRefFld.GetSubType() )
            {
            case REF_SEQUENCEFLD:
                {
                    _RefIdsMap* pMap = 0;
                    for( sal_uInt16 n = aFldMap.size(); n; )
                    {
                        if( aFldMap[ --n ].GetName().Equals(rRefFld.GetSetRefName()) )
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

                    pMap->Check( *pDoc, rDestDoc, rRefFld, sal_True );
                }
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                aFntMap.Check( *pDoc, rDestDoc, rRefFld, sal_False );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
