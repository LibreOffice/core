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


#include <string.h>         // for strchr()

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/WordType.hpp>

#include <unotools/charclass.hxx>

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <mvsave.hxx>       // structures for Saving by Move/Delete
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <rubylist.hxx>
#include <pam.hxx>
#include <swundo.hxx>       // for the UndoIds
#include <breakit.hxx>
#include <crsskip.hxx>

using namespace ::com::sun::star::i18n;


/*
 * Members in the list:
 *   - String - the orig text
 *   - SwFmtRuby - the ruby attribut
 *
 *
 */
sal_uInt16 SwDoc::FillRubyList( const SwPaM& rPam, SwRubyList& rList,
                            sal_uInt16 nMode )
{
    const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
                *__pStartCrsr = _pStartCrsr;
    sal_Bool bCheckEmpty = &rPam != _pStartCrsr;
    do {
        const SwPosition* pStt = _pStartCrsr->Start(),
                        * pEnd = pStt == _pStartCrsr->GetPoint()
                                                ? _pStartCrsr->GetMark()
                                                : _pStartCrsr->GetPoint();
        if( !bCheckEmpty || ( pStt != pEnd && *pStt != *pEnd ))
        {
            SwPaM aPam( *pStt );
            do {
                SwRubyListEntry* pNew = new SwRubyListEntry;
                if( pEnd != pStt )
                {
                    aPam.SetMark();
                    *aPam.GetMark() = *pEnd;
                }
                if( _SelectNextRubyChars( aPam, *pNew, nMode ))
                {
                    rList.push_back( pNew );
                    aPam.DeleteMark();
                }
                else
                {
                    delete pNew;
                     if( *aPam.GetPoint() < *pEnd )
                     {
                        // goto next paragraph
                        aPam.DeleteMark();
                        aPam.Move( fnMoveForward, fnGoNode );
                     }
                     else
                        break;
                }
            } while( 30 > rList.size() && *aPam.GetPoint() < *pEnd );
        }
    } while( 30 > rList.size() &&
        (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

    return rList.size();
}

sal_uInt16 SwDoc::SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
                            sal_uInt16 nMode )
{
    GetIDocumentUndoRedo().StartUndo( UNDO_SETRUBYATTR, NULL );
    std::set<sal_uInt16> aDelArr;
    aDelArr.insert( RES_TXTATR_CJK_RUBY );

    sal_uInt16 nListEntry = 0;

    const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
                *__pStartCrsr = _pStartCrsr;
    sal_Bool bCheckEmpty = &rPam != _pStartCrsr;
    do {
        const SwPosition* pStt = _pStartCrsr->Start(),
                        * pEnd = pStt == _pStartCrsr->GetPoint()
                                                ? _pStartCrsr->GetMark()
                                                : _pStartCrsr->GetPoint();
        if( !bCheckEmpty || ( pStt != pEnd && *pStt != *pEnd ))
        {

            SwPaM aPam( *pStt );
            do {
                SwRubyListEntry aCheckEntry;
                if( pEnd != pStt )
                {
                    aPam.SetMark();
                    *aPam.GetMark() = *pEnd;
                }
                if( _SelectNextRubyChars( aPam, aCheckEntry, nMode ))
                {
                    const SwRubyListEntry* pEntry = &rList[ nListEntry++ ];
                    if( aCheckEntry.GetRubyAttr() != pEntry->GetRubyAttr() )
                    {
                        // set/reset the attribut
                        if( pEntry->GetRubyAttr().GetText().Len() )
                        {
                            InsertPoolItem( aPam, pEntry->GetRubyAttr(), 0 );
                        }
                        else
                        {
                            ResetAttrs( aPam, true, aDelArr );
                        }
                    }

                    if( aCheckEntry.GetText() != pEntry->GetText() &&
                        pEntry->GetText().Len() )
                    {
                        // text is changed, so replace the original
                        ReplaceRange( aPam, pEntry->GetText(), false );
                    }
                    aPam.DeleteMark();
                }
                else
                {
                     if( *aPam.GetPoint() < *pEnd )
                     {
                        // goto next paragraph
                        aPam.DeleteMark();
                        aPam.Move( fnMoveForward, fnGoNode );
                     }
                     else
                    {
                        const SwRubyListEntry* pEntry = &rList[ nListEntry++ ];

                        // set/reset the attribut
                        if( pEntry->GetRubyAttr().GetText().Len() &&
                            pEntry->GetText().Len() )
                        {
                            InsertString( aPam, pEntry->GetText() );
                            aPam.SetMark();
                            aPam.GetMark()->nContent -= pEntry->GetText().Len();
                            InsertPoolItem( aPam, pEntry->GetRubyAttr(),
                                    nsSetAttrMode::SETATTR_DONTEXPAND );
                        }
                        else
                            break;
                        aPam.DeleteMark();
                    }
                }
            } while( nListEntry < rList.size() && *aPam.GetPoint() < *pEnd );
        }
    } while( 30 > rList.size() &&
        (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

    GetIDocumentUndoRedo().EndUndo( UNDO_SETRUBYATTR, NULL );

    return nListEntry;
}

sal_Bool SwDoc::_SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rEntry, sal_uInt16 )
{
    // Point must be the startposition, Mark is optional the end position
    SwPosition* pPos = rPam.GetPoint();
       const SwTxtNode* pTNd = pPos->nNode.GetNode().GetTxtNode();
    const String* pTxt = &pTNd->GetTxt();
    xub_StrLen nStart = pPos->nContent.GetIndex(), nEnd = pTxt->Len();

    sal_Bool bHasMark = rPam.HasMark();
    if( bHasMark )
    {
        // in the same node?
        if( rPam.GetMark()->nNode == pPos->nNode )
        {
            // then use that end
            xub_StrLen nTEnd = rPam.GetMark()->nContent.GetIndex();
            if( nTEnd < nEnd )
                nEnd = nTEnd;
        }
        rPam.DeleteMark();
    }

    // ----- search the start
    // --- look where a ruby attribut starts
    sal_uInt16 nHtIdx = USHRT_MAX;
    const SwpHints* pHts = pTNd->GetpSwpHints();
    const SwTxtAttr* pAttr = 0;
    if( pHts )
    {
        const SwTxtAttr* pHt;
        for( nHtIdx = 0; nHtIdx < pHts->Count(); ++nHtIdx )
            if( RES_TXTATR_CJK_RUBY == ( pHt = (*pHts)[ nHtIdx ])->Which() &&
                *pHt->GetAnyEnd() > nStart )
            {
                if( *pHt->GetStart() < nEnd )
                {
                    pAttr = pHt;
                    if( !bHasMark && nStart > *pAttr->GetStart() )
                    {
                        nStart = *pAttr->GetStart();
                        pPos->nContent = nStart;
                    }
                }
                break;
            }
    }

    if( !bHasMark && nStart && ( !pAttr || nStart != *pAttr->GetStart()) )
    {
        // skip to the word begin!
        long nWordStt = pBreakIt->GetBreakIter()->getWordBoundary(
                            *pTxt, nStart,
                            pBreakIt->GetLocale( pTNd->GetLang( nStart )),
                            WordType::ANYWORD_IGNOREWHITESPACES,
                            sal_True ).startPos;
        if( nWordStt < nStart && -1 != nWordStt )
        {
            nStart = (xub_StrLen)nWordStt;
            pPos->nContent = nStart;
        }
    }

    sal_Bool bAlphaNum = sal_False;
    long nWordEnd = nEnd;
    CharClass& rCC = GetAppCharClass();
    while(  nStart < nEnd )
    {
        if( pAttr && nStart == *pAttr->GetStart() )
        {
            pPos->nContent = nStart;
            if( !rPam.HasMark() )
            {
                rPam.SetMark();
                pPos->nContent = *pAttr->GetAnyEnd();
                if( pPos->nContent.GetIndex() > nEnd )
                    pPos->nContent = nEnd;
                rEntry.SetRubyAttr( pAttr->GetRuby() );
            }
            break;
        }

        sal_Int32 nChType = rCC.getType( *pTxt, nStart );
        sal_Bool bIgnoreChar = sal_False, bIsAlphaNum = sal_False, bChkNxtWrd = sal_False;
        switch( nChType )
        {
        case UnicodeType::UPPERCASE_LETTER:
        case UnicodeType::LOWERCASE_LETTER:
        case UnicodeType::TITLECASE_LETTER:
        case UnicodeType::DECIMAL_DIGIT_NUMBER:
                bChkNxtWrd = bIsAlphaNum = sal_True;
                break;

        case UnicodeType::SPACE_SEPARATOR:
        case UnicodeType::CONTROL:
/*??*/  case UnicodeType::PRIVATE_USE:
        case UnicodeType::START_PUNCTUATION:
        case UnicodeType::END_PUNCTUATION:
            bIgnoreChar = sal_True;
            break;


        case UnicodeType::OTHER_LETTER:
            bChkNxtWrd = sal_True;

        default:
                bIsAlphaNum = sal_False;
                break;
        }

        if( rPam.HasMark() )
        {
            if( bIgnoreChar || bIsAlphaNum != bAlphaNum || nStart >= nWordEnd )
                break;
        }
        else if( !bIgnoreChar )
        {
            rPam.SetMark();
            bAlphaNum = bIsAlphaNum;
            if( bChkNxtWrd && pBreakIt->GetBreakIter().is() )
            {
                // search the end of this word
                nWordEnd = pBreakIt->GetBreakIter()->getWordBoundary(
                            *pTxt, nStart,
                            pBreakIt->GetLocale( pTNd->GetLang( nStart )),
                            WordType::ANYWORD_IGNOREWHITESPACES,
                            sal_True ).endPos;
                if( 0 > nWordEnd || nWordEnd > nEnd || nWordEnd == nStart )
                    nWordEnd = nEnd;
            }
        }
        pTNd->GoNext( &pPos->nContent, CRSR_SKIP_CHARS );
        nStart = pPos->nContent.GetIndex();
    }

    nStart = rPam.GetMark()->nContent.GetIndex();
    rEntry.SetText( pTxt->Copy( nStart,
                           rPam.GetPoint()->nContent.GetIndex() - nStart ));
    return rPam.HasMark();
}

SwRubyListEntry::~SwRubyListEntry()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
