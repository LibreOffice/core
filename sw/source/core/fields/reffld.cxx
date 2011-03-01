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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
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
// --> OD 2007-10-18 #i81002#
#include <crossrefbookmark.hxx>
// <--
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using ::rtl::OUString;

extern void InsertSort( SvUShorts& rArr, USHORT nIdx, USHORT* pInsPos = 0 );

void lcl_GetLayTree( const SwFrm* pFrm, SvPtrarr& rArr )
{
    while( pFrm )
    {
        if( pFrm->IsBodyFrm() )     // soll uns nicht weiter interessieren
            pFrm = pFrm->GetUpper();
        else
        {
            void* p = (void*)pFrm;
            rArr.Insert( p, rArr.Count() );

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


BOOL IsFrameBehind( const SwTxtNode& rMyNd, USHORT nMySttPos,
                    const SwTxtNode& rBehindNd, USHORT nSttPos )
{
    const SwTxtFrm *pMyFrm = (SwTxtFrm*)rMyNd.GetFrm(0,0,FALSE),
                   *pFrm = (SwTxtFrm*)rBehindNd.GetFrm(0,0,FALSE);

    while( pFrm && !pFrm->IsInside( nSttPos ) )
        pFrm = (SwTxtFrm*)pFrm->GetFollow();
    while( pMyFrm && !pMyFrm->IsInside( nMySttPos ) )
        pMyFrm = (SwTxtFrm*)pMyFrm->GetFollow();

    if( !pFrm || !pMyFrm || pFrm == pMyFrm )
        return FALSE;

    SvPtrarr aRefArr( 10, 10 ), aArr( 10, 10 );
    ::lcl_GetLayTree( pFrm, aRefArr );
    ::lcl_GetLayTree( pMyFrm, aArr );

    USHORT nRefCnt = aRefArr.Count() - 1, nCnt = aArr.Count() - 1;
    BOOL bVert = FALSE;
    BOOL bR2L = FALSE;

    // solange bis ein Frame ungleich ist ?
    while( nRefCnt && nCnt && aRefArr[ nRefCnt ] == aArr[ nCnt ] )
    {
        const SwFrm* pTmpFrm = (const SwFrm*)aArr[ nCnt ];
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

    const SwFrm* pRefFrm = (const SwFrm*)aRefArr[ nRefCnt ];
    const SwFrm* pFldFrm = (const SwFrm*)aArr[ nCnt ];

    // unterschiedliche Frames, dann ueberpruefe deren Y-/X-Position
    BOOL bRefIsLower = FALSE;
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
            pFldFrm = (const SwFrm*)aArr[ nCnt - 1 ];
        else
            pRefFrm = (const SwFrm*)aRefArr[ nRefCnt - 1 ];
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
                              const String& rSetRef, USHORT nSubTyp,
                              USHORT nSeqenceNo, ULONG nFmt )
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

USHORT SwGetRefField::GetSubType() const
{
    return nSubType;
}

void SwGetRefField::SetSubType( USHORT n )
{
    nSubType = n;
}

// --> OD 2007-11-09 #i81002#
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
    USHORT nDummy = USHRT_MAX;
    return SwGetRefFieldType::FindAnchor( pDoc, sSetRefName, nSubType, nSeqNo, &nDummy );
}
// <--
// --> OD 2008-01-09 #i85090#
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


String SwGetRefField::GetCntnt(BOOL bName) const
{
    if( !bName )
        return Expand();

    String aStr(GetTyp()->GetName());
    aStr += ' ';
    aStr += sSetRefName;
    return aStr;
}

// --> OD 2007-09-07 #i81002# - parameter <pFldTxtAttr> added
void SwGetRefField::UpdateField( const SwTxtFld* pFldTxtAttr )
{
    sTxt.Erase();

    SwDoc* pDoc = ((SwGetRefFieldType*)GetTyp())->GetDoc();
    USHORT nStt = USHRT_MAX;
    USHORT nEnd = USHRT_MAX;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor( pDoc, sSetRefName,
                                        nSubType, nSeqNo, &nStt, &nEnd );
    if ( !pTxtNd )
    {
        sTxt = ViewShell::GetShellRes()->aGetRefFld_RefItemNotFound;
        return ;
    }

    switch( GetFormat() )
    {
    case REF_CONTENT:
    case REF_ONLYNUMBER:
    case REF_ONLYCAPTION:
    case REF_ONLYSEQNO:
        {
            switch( nSubType )
            {
            case REF_SEQUENCEFLD:
                nEnd = pTxtNd->GetTxt().Len();
                switch( GetFormat() )
                {
                case REF_ONLYNUMBER:
                    if( nStt + 1 < nEnd )
                        nEnd = nStt + 1;
                    nStt = 0;
                    break;

                case REF_ONLYCAPTION:
                    {
                        const SwTxtAttr* const pTxtAttr =
                            pTxtNd->GetTxtAttrForCharAt(nStt, RES_TXTATR_FIELD);
                        if( pTxtAttr )
                            nStt = SwGetExpField::GetReferenceTextPos(
                                                pTxtAttr->GetFld(), *pDoc );
                        else if( nStt + 1 < nEnd )
                            ++nStt;
                    }
                    break;

                case REF_ONLYSEQNO:
                    if( nStt + 1 < nEnd )
                        nEnd = nStt + 1;
                    break;

                default:
                    nStt = 0;
                    break;
                }
                break;

            case REF_BOOKMARK:
                if( USHRT_MAX == nEnd )
                {
                    // Text steht ueber verschiedene Nodes verteilt.
                    // Gesamten Text oder nur bis zum Ende vom Node?
                    nEnd = pTxtNd->GetTxt().Len();
                }
                break;

            case REF_OUTLINE:
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                {
                    // die Nummer oder den NumString besorgen
                    USHORT n, nFtnCnt = pDoc->GetFtnIdxs().Count();
                    SwTxtFtn* pFtnIdx;
                    for( n = 0; n < nFtnCnt; ++n )
                        if( nSeqNo == (pFtnIdx = pDoc->GetFtnIdxs()[ n ])->GetSeqRefNo() )
                        {
                            sTxt = pFtnIdx->GetFtn().GetViewNumStr( *pDoc );
                            break;
                        }
                    nStt = nEnd;        // kein Bereich, der String ist fertig
                }
                break;
            }

            if( nStt != nEnd )      // ein Bereich?
            {
                sTxt = pTxtNd->GetExpandTxt( nStt, nEnd - nStt );

                // alle Sonderzeichen entfernen (durch Blanks ersetzen):
                if( sTxt.Len() )
                {
                    sTxt.EraseAllChars( 0xad );
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
            const SwTxtFrm* pFrm = (SwTxtFrm*)pTxtNd->GetFrm(0,0,FALSE),
                        *pSave = pFrm;
            while( pFrm && !pFrm->IsInside( nStt ) )
                pFrm = (SwTxtFrm*)pFrm->GetFollow();

            if( pFrm || 0 != ( pFrm = pSave ))
            {
                USHORT nPageNo = pFrm->GetVirtPageNum();
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
            const SwFrm* pFrm = pTxtNd->GetFrm();
            if( pFrm )
            {
                SwChapterFieldType aFldTyp;
                SwChapterField aFld( &aFldTyp, 0 );
                aFld.SetLevel( MAXLEVEL - 1 );
                aFld.ChangeExpansion( pFrm, pTxtNd, TRUE );
                sTxt = aFld.GetNumber();
            }
        }
        break;

    case REF_UPDOWN:
        {
            // --> OD 2007-09-07 #i81002#
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
                sTxt = nStt < *pFldTxtAttr->GetStart()
                            ? aLocaleData.getAboveWord()
                            : aLocaleData.getBelowWord();
                break;
            }

            sTxt = ::IsFrameBehind( *pFldTxtAttr->GetpTxtNode(), *pFldTxtAttr->GetStart(),
                                    *pTxtNd, nStt )
                        ? aLocaleData.getAboveWord()
                        : aLocaleData.getBelowWord();
        }
        break;
    // --> OD 2007-08-24 #i81002#
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
    // <--
    default:
        OSL_FAIL("<SwGetRefField::UpdateField(..)> - unknown format type");
    }
}

// --> OD 2007-09-06 #i81002#
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
// <--

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


const String& SwGetRefField::GetPar1() const
{
    return sSetRefName;
}


void SwGetRefField::SetPar1( const String& rName )
{
    sSetRefName = rName;
}


String SwGetRefField::GetPar2() const
{
    return Expand();
}

bool SwGetRefField::QueryValue( uno::Any& rAny, USHORT nWhichId ) const
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
            // --> OD 2007-09-06 #i81002#
            case REF_NUMBER:              nPart = ReferenceFieldPart::NUMBER;              break;
            case REF_NUMBER_NO_CONTEXT:   nPart = ReferenceFieldPart::NUMBER_NO_CONTEXT;   break;
            case REF_NUMBER_FULL_CONTEXT: nPart = ReferenceFieldPart::NUMBER_FULL_CONTEXT; break;
            // <--
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

bool SwGetRefField::PutValue( const uno::Any& rAny, USHORT nWhichId )
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
            // --> OD 2007-09-06 #i81002#
            case ReferenceFieldPart::NUMBER:              nPart = REF_NUMBER;              break;
            case ReferenceFieldPart::NUMBER_NO_CONTEXT:   nPart = REF_NUMBER_NO_CONTEXT;   break;
            case ReferenceFieldPart::NUMBER_FULL_CONTEXT: nPart = REF_NUMBER_FULL_CONTEXT; break;
            // <--
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
            USHORT nResId = USHRT_MAX;
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
/*-----------------JP: 18.06.93 -------------------
 Get-Referenz-Type
 --------------------------------------------------*/


SwGetRefFieldType::SwGetRefFieldType( SwDoc* pDc )
    : SwFieldType( RES_GETREFFLD ), pDoc( pDc )
{}


SwFieldType* SwGetRefFieldType::Copy() const
{
    return new SwGetRefFieldType( pDoc );
}


void SwGetRefFieldType::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    // Update auf alle GetReferenz-Felder
    if( !pNew && !pOld )
    {
        SwClientIter aIter( *this );
        for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                        pFld; pFld = (SwFmtFld*)aIter.Next() )
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

            // --> OD 2007-09-06 #i81002#
            pGRef->UpdateField( pFld->GetTxtFld() );
            // <--
        }
    }
    // weiter an die Text-Felder, diese "Expandieren" den Text
    SwModify::Modify( pOld, pNew );
}

SwTxtNode* SwGetRefFieldType::FindAnchor( SwDoc* pDoc, const String& rRefMark,
                                        USHORT nSubType, USHORT nSeqNo,
                                        USHORT* pStt, USHORT* pEnd )
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
                SwClientIter aIter( *pFldType );
                for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                                pFld; pFld = (SwFmtFld*)aIter.Next() )
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

                pTxtNd = pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
                *pStt = pPos->nContent.GetIndex();
                if(pEnd)
                {
                    if(!pBkmk->IsExpanded())
                    {
                        *pEnd = *pStt;
                        // --> OD 2007-10-18 #i81002#
                        if(dynamic_cast< ::sw::mark::CrossRefBookmark const *>(pBkmk))
                        {
                            OSL_ENSURE( pTxtNd,
                                    "<SwGetRefFieldType::FindAnchor(..)> - node marked by cross-reference bookmark isn't a text node --> crash" );
                            *pEnd = pTxtNd->Len();
                        }
                        // <--
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
            USHORT n, nFtnCnt = pDoc->GetFtnIdxs().Count();
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
    String aName;
    SvUShortsSort aIds, aDstIds, aIdsMap;
    SvUShorts aMap;
    BOOL bInit;

    _RefIdsMap( const String& rName )
        : aName( rName ), aIds( 16, 16 ), aIdsMap( 16, 16 ), aMap( 16, 16 ),
        bInit( FALSE )
    {}

    void Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rFld,
                    BOOL bField = TRUE );

    BOOL IsInit() const { return bInit; }
};

SV_DECL_PTRARR_DEL( _RefIdsMaps, _RefIdsMap*, 5, 5 )
SV_IMPL_PTRARR( _RefIdsMaps, _RefIdsMap* )

void _RefIdsMap::Check( SwDoc& rDoc, SwDoc& rDestDoc, SwGetRefField& rFld,
                        BOOL bField )
{

    if( !bInit )
    {
        if( bField )
        {
            const SwTxtNode* pNd;
            SwModify* pMod;
            if( 0 != ( pMod = rDestDoc.GetFldType( RES_SETEXPFLD, aName, false ) ))
            {
                SwClientIter aIter( *pMod );
                for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
                    pF = (SwFmtFld*)aIter.Next() )
                    if( pF->GetTxtFld() &&
                        0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
                        pNd->GetNodes().IsDocNodes() )
                        aIds.Insert( ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
            }
            if( 0 != ( pMod = rDoc.GetFldType( RES_SETEXPFLD, aName, false ) ))
            {
                SwClientIter aIter( *pMod );
                for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
                        pF = (SwFmtFld*)aIter.Next() )
                    if( pF->GetTxtFld() &&
                        0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
                        pNd->GetNodes().IsDocNodes() )
                        aDstIds.Insert( ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
            }
        }
        else
        {
            USHORT n;

            for( n = rDestDoc.GetFtnIdxs().Count(); n; )
                aIds.Insert( rDestDoc.GetFtnIdxs()[ --n ]->GetSeqRefNo() );
            for( n = rDoc.GetFtnIdxs().Count(); n; )
                aDstIds.Insert( rDoc.GetFtnIdxs()[ --n ]->GetSeqRefNo() );
        }
        bInit = TRUE;
    }

    // dann teste mal, ob die Nummer schon vergeben ist
    // oder ob eine neue bestimmt werden muss.
    USHORT nPos, nSeqNo = rFld.GetSeqNo();
    if( aIds.Seek_Entry( nSeqNo ) && aDstIds.Seek_Entry( nSeqNo ))
    {
        // ist schon vergeben, also muss eine neue
        // erzeugt werden.
        if( aIdsMap.Seek_Entry( nSeqNo, &nPos ))
            rFld.SetSeqNo( aMap[ nPos ] );
        else
        {
            USHORT n;

            for( n = 0; n < aIds.Count(); ++n )
                if( n != aIds[ n ] )
                    break;

            // die neue SeqNo eintragen, damit die "belegt" ist
            aIds.Insert( n );
            aIdsMap.Insert( nSeqNo, nPos );
            aMap.Insert( n, nPos );
            rFld.SetSeqNo( n );

            // und noch die Felder oder Fuss-/EndNote auf die neue
            // Id umsetzen
            if( bField )
            {
                SwModify* pMod = rDoc.GetFldType( RES_SETEXPFLD, aName, false );
                if( pMod )
                {
                    SwClientIter aIter( *pMod );
                    for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
                            pF = (SwFmtFld*)aIter.Next() )
                        if( pF->GetTxtFld() && nSeqNo ==
                            ((SwSetExpField*)pF->GetFld())->GetSeqNumber() )
                            ((SwSetExpField*)pF->GetFld())->SetSeqNumber( n );
                }
            }
            else
            {
                SwTxtFtn* pFtnIdx;
                for( USHORT i = 0, nCnt = rDoc.GetFtnIdxs().Count(); i < nCnt; ++i )
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
        aIds.Insert( nSeqNo );
        aIdsMap.Insert( nSeqNo, nPos );
        aMap.Insert( nSeqNo, nPos );
    }
}


void SwGetRefFieldType::MergeWithOtherDoc( SwDoc& rDestDoc )
{
    if( &rDestDoc != pDoc &&
        rDestDoc.GetSysFldType( RES_GETREFFLD )->GetDepends() )
    {
        // dann gibt es im DestDoc RefFelder, also muessen im SourceDoc
        // alle RefFelder auf einduetige Ids in beiden Docs umgestellt
        // werden.
        _RefIdsMap aFntMap( aEmptyStr );
        _RefIdsMaps aFldMap;

        SwClientIter aIter( *this );
        for( SwClient* pFld = aIter.First( TYPE( SwFmtFld ));
                pFld; pFld = aIter.Next() )
        {
            SwGetRefField& rRefFld = *(SwGetRefField*)((SwFmtFld*)pFld)->GetFld();
            switch( rRefFld.GetSubType() )
            {
            case REF_SEQUENCEFLD:
                {
                    _RefIdsMap* pMap = 0;
                    for( USHORT n = aFldMap.Count(); n; )
                        if( aFldMap[ --n ]->aName == rRefFld.GetSetRefName() )
                        {
                            pMap = aFldMap[ n ];
                            break;
                        }
                    if( !pMap )
                    {
                        pMap = new _RefIdsMap( rRefFld.GetSetRefName() );
                        aFldMap.C40_INSERT( _RefIdsMap, pMap, aFldMap.Count() );
                    }

                    pMap->Check( *pDoc, rDestDoc, rRefFld, TRUE );
                }
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                aFntMap.Check( *pDoc, rDestDoc, rRefFld, FALSE );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
