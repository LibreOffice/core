/*************************************************************************
 *
 *  $RCSfile: reffld.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-04-03 11:52:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif

#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDPART_HPP_
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDSOURCE_HPP_
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

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
                pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
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

    // solange bis ein Frame ungleich ist ?
    while( nRefCnt && nCnt && aRefArr[ nRefCnt ] == aArr[ nCnt ] )
        --nCnt, --nRefCnt;

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
    BOOL bRefIsLower;
    if( ( FRM_COLUMN | FRM_CELL ) & pFldFrm->GetType() ||
        ( FRM_COLUMN | FRM_CELL ) & pRefFrm->GetType() )
    {
        if( pFldFrm->GetType() == pRefFrm->GetType() )
        {
            // hier ist die X-Pos wichtiger!
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
        bRefIsLower = pRefFrm->Frm().Top() < pFldFrm->Frm().Top() ||
                    ( pRefFrm->Frm().Top() == pFldFrm->Frm().Top() &&
                        pRefFrm->Frm().Left() < pFldFrm->Frm().Left() );
    return bRefIsLower;
}

/*--------------------------------------------------------------------
    Beschreibung: Referenzen holen
 --------------------------------------------------------------------*/


SwGetRefField::SwGetRefField( SwGetRefFieldType* pFldType,
                              const String& rSetRef, USHORT nSubTyp,
                              USHORT nSeqenceNo, ULONG nFmt )
    : SwField( pFldType, nFmt ), sSetRefName( rSetRef ),
    nSubType( nSubTyp ), nSeqNo( nSeqenceNo )
{
}

USHORT SwGetRefField::GetSubType() const
{
    return nSubType;
}

void SwGetRefField::SetSubType( USHORT n )
{
    nSubType = n;
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


void SwGetRefField::UpdateField()
{
    sTxt.Erase();

    SwDoc* pDoc = ((SwGetRefFieldType*)GetTyp())->GetDoc();
    USHORT nStt, nEnd;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor( pDoc, sSetRefName,
                                        nSubType, nSeqNo, &nStt, &nEnd );
    if( !pTxtNd )
        return ;

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
                        const SwTxtAttr* pTxtAttr = pTxtNd->GetTxtAttr( nStt,
                                                        RES_TXTATR_FIELD );
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
                sTxt = pTxtNd->GetExpandTxt( nStt, nEnd - nStt, FALSE );

                // alle Sonderzeichen entfernen (durch Blanks ersetzen):
                if( sTxt.Len() )
                    for( sal_Unicode* p = sTxt.GetBufferAccess(); *p; ++p )
                        if( *p < 0x20 )
                            *p = 0x20;
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
            const SwTxtFld* pTFld = 0;

            {
                SwClientIter aIter( *GetTyp() );
                for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE(SwFmtFld) );
                        pFld; pFld = (SwFmtFld*)aIter.Next() )
                    if( pFld->GetFld() == this )
                    {
                        pTFld = pFld->GetTxtFld();
                        break;
                    }
            }

            if( !pTFld || !pTFld->GetpTxtNode() )   // noch nicht im Node gestezt?
                break;

            LocaleDataWrapper aLocaleData(
                            ::comphelper::getProcessServiceFactory(),
                            SvxCreateLocale( GetLanguage() ) );

            // erstmal ein "Kurz" - Test - falls beide im selben
            // Node stehen!
            if( pTFld->GetpTxtNode() == pTxtNd )
            {
                sTxt = nStt < *pTFld->GetStart()
                            ? aLocaleData.getAboveWord()
                            : aLocaleData.getBelowWord();
                break;
            }

            sTxt = ::IsFrameBehind( *pTFld->GetpTxtNode(), *pTFld->GetStart(),
                                    *pTxtNd, nStt )
                        ? aLocaleData.getAboveWord()
                        : aLocaleData.getBelowWord();
        }
        break;
    }
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

/*-----------------06.03.98 13:34-------------------

--------------------------------------------------*/
BOOL SwGetRefField::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if(rProperty.EqualsAscii(UNO_NAME_REFERENCE_FIELD_PART))
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
        }
        rAny <<= nPart;
    }
    else if(rProperty.EqualsAscii(UNO_NAME_REFERENCE_FIELD_SOURCE))
    {
        sal_Int16 nSource = 0;
        switch(nSubType)
        {
            case  REF_SETREFATTR : nSource = ReferenceFieldSource::REFERENCE_MARK; break;
            case  REF_SEQUENCEFLD: nSource = ReferenceFieldSource::SEQUENCE_FIELD; break;
            case  REF_BOOKMARK   : nSource = ReferenceFieldSource::BOOKMARK; break;
            case  REF_OUTLINE    : DBG_ERROR("not implemented"); break;
            case  REF_FOOTNOTE   : nSource = ReferenceFieldSource::FOOTNOTE; break;
            case  REF_ENDNOTE    : nSource = ReferenceFieldSource::ENDNOTE; break;
        }
        rAny <<= nSource;
    }
    else if( rProperty.EqualsAscii(UNO_NAME_SOURCE_NAME ))
        rAny <<= rtl::OUString(GetPar1());
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
        rAny <<= rtl::OUString(Expand());
    else if(rProperty.EqualsAscii(UNO_NAME_SEQUENCE_NUMBER))
        rAny <<= (sal_Int16)nSeqNo;
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}
/*-----------------06.03.98 13:34-------------------

--------------------------------------------------*/
BOOL SwGetRefField::PutValue( const uno::Any& rAny, const String& rProperty )
{
    if(rProperty.EqualsAscii(UNO_NAME_REFERENCE_FIELD_PART))
    {
        sal_Int16 nPart;
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
            default: return FALSE;
        }
        SetFormat(nPart);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_REFERENCE_FIELD_SOURCE))
    {
        sal_Int16 nSource;
        rAny >>= nSource;
        switch(nSource)
        {
            case ReferenceFieldSource::REFERENCE_MARK : nSubType = REF_SETREFATTR ; break;
            case ReferenceFieldSource::SEQUENCE_FIELD : nSubType = REF_SEQUENCEFLD; break;
            case ReferenceFieldSource::BOOKMARK       : nSubType = REF_BOOKMARK   ; break;
            case ReferenceFieldSource::FOOTNOTE       : nSubType = REF_FOOTNOTE   ; break;
            case ReferenceFieldSource::ENDNOTE        : nSubType = REF_ENDNOTE    ; break;
        }
    }
    else if( rProperty.EqualsAscii(UNO_NAME_SOURCE_NAME ))
    {
        OUString uTmp;
        rAny >>= uTmp;
        SetPar1(uTmp);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_CURRENT_PRESENTATION))
    {
        OUString sVal;
        rAny >>= sVal;
        SetExpand(sVal);
    }
    else if(rProperty.EqualsAscii(UNO_NAME_SEQUENCE_NUMBER))
    {
        sal_Int16 nSetSeq;
        rAny >>= nSetSeq;
        if(nSetSeq >= 0)
            nSeqNo = nSetSeq;
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
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
                pGRef->SetLanguage( pTFld->GetpTxtNode()->GetLang(
                                                *pTFld->GetStart() ) );

            pGRef->UpdateField();
        }
    }
    // weiter an die Text-Felder, diese "Expandieren" den Text
    SwModify::Modify( pOld, pNew );
}

SwTxtNode* SwGetRefFieldType::FindAnchor( SwDoc* pDoc, const String& rRefMark,
                                        USHORT nSubType, USHORT nSeqNo,
                                        USHORT* pStt, USHORT* pEnd )
{
    ASSERT( pStt, "warum wird keine StartPos abgefragt?" );

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
            SwFieldType* pFldType = pDoc->GetFldType( RES_SETEXPFLD, rRefMark );
            if( pFldType && pFldType->GetDepends() &&
                GSE_SEQ & ((SwSetExpFieldType*)pFldType)->GetType() )
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
            USHORT nPos = pDoc->FindBookmark( rRefMark );
            if( USHRT_MAX != nPos )
            {
                const SwBookmark& rBkmk = *pDoc->GetBookmarks()[ nPos ];
                const SwPosition* pPos = &rBkmk.GetPos();
                if( rBkmk.GetOtherPos() && *pPos > *rBkmk.GetOtherPos() )
                    pPos = rBkmk.GetOtherPos();

                pTxtNd = pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
                *pStt = pPos->nContent.GetIndex();
                if( pEnd )
                {
                    if( !rBkmk.GetOtherPos() )
                        *pEnd = *pStt;
                    else if( rBkmk.GetOtherPos()->nNode == rBkmk.GetPos().nNode )
                    {
                        *pEnd = rBkmk.GetOtherPos() == pPos
                                ? rBkmk.GetPos().nContent.GetIndex()
                                : rBkmk.GetOtherPos()->nContent.GetIndex();
                    }
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
            if( 0 != ( pMod = rDestDoc.GetFldType( RES_SETEXPFLD, aName ) ))
            {
                SwClientIter aIter( *pMod );
                for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
                    pF = (SwFmtFld*)aIter.Next() )
                    if( pF->GetTxtFld() &&
                        0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
                        pNd->GetNodes().IsDocNodes() )
                        aIds.Insert( ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
            }
            if( 0 != ( pMod = rDoc.GetFldType( RES_SETEXPFLD, aName ) ))
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
            for( USHORT n = rDestDoc.GetFtnIdxs().Count(); n; )
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
            for( USHORT n = 0; n < aIds.Count(); ++n )
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
                SwModify* pMod = rDoc.GetFldType( RES_SETEXPFLD, aName );
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

