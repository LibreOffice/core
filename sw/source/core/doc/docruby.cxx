/*************************************************************************
 *
 *  $RCSfile: docruby.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 13:51:29 $
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


#pragma hdrstop

#include <string.h>         // fuer strchr()

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HDL
#include <com/sun/star/i18n/UnicodeType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>       // Strukturen zum Sichern beim Move/Delete
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _RUBYLIST_HXX
#include <rubylist.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

SV_IMPL_PTRARR( SwRubyList, SwRubyListEntryPtr )

using namespace ::com::sun::star::i18n;


/*
 * Members in the list:
 *   - String - the orig text
 *   - SwFmtRuby - the ruby attribut
 *
 *
 */
USHORT SwDoc::FillRubyList( const SwPaM& rPam, SwRubyList& rList,
                            USHORT nMode )
{
    const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
                *__pStartCrsr = _pStartCrsr;
    BOOL bCheckEmpty = &rPam != _pStartCrsr;
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
                    rList.Insert( pNew, rList.Count() );
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
            } while( 30 > rList.Count() && *aPam.GetPoint() < *pEnd );
        }
    } while( 30 > rList.Count() &&
        (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

    return rList.Count();
}

USHORT SwDoc::SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
                            USHORT nMode )
{
    StartUndo( UNDO_SETRUBYATTR );
    SvUShortsSort aDelArr;
    aDelArr.Insert( RES_TXTATR_CJK_RUBY );

    USHORT nListEntry = 0;

    const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
                *__pStartCrsr = _pStartCrsr;
    BOOL bCheckEmpty = &rPam != _pStartCrsr;
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
                    const SwRubyListEntry* pEntry = rList[ nListEntry++ ];
                    if( aCheckEntry.GetRubyAttr() != pEntry->GetRubyAttr() )
                    {
                        // set/reset the attribut
                        if( pEntry->GetRubyAttr().GetText().Len() )
                            Insert( aPam, pEntry->GetRubyAttr() );
                        else
                            ResetAttr( aPam, TRUE, &aDelArr );
                    }

                    if( aCheckEntry.GetText() != pEntry->GetText() &&
                        pEntry->GetText().Len() )
                    {
                        // text is changed, so replace the original
                        Replace( aPam, pEntry->GetText(), FALSE );
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
                        const SwRubyListEntry* pEntry = rList[ nListEntry++ ];

                        // set/reset the attribut
                        if( pEntry->GetRubyAttr().GetText().Len() &&
                            pEntry->GetText().Len() )
                        {
                            Insert( aPam, pEntry->GetText() );
                            aPam.SetMark();
                            aPam.GetMark()->nContent -= pEntry->GetText().Len();
                            Insert( aPam, pEntry->GetRubyAttr(), SETATTR_DONTEXPAND );
                        }
                        else
                            break;
                        aPam.DeleteMark();
                    }
                }
            } while( nListEntry < rList.Count() && *aPam.GetPoint() < *pEnd );
        }
    } while( 30 > rList.Count() &&
        (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

    EndUndo( UNDO_SETRUBYATTR );

    return nListEntry;
}

BOOL SwDoc::_SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rEntry,
                                    USHORT nMode )
{
    // Point must be the startposition, Mark is optional the end position
    SwPosition* pPos = rPam.GetPoint();
       const SwTxtNode* pTNd = pPos->nNode.GetNode().GetTxtNode();
    const String* pTxt = &pTNd->GetTxt();
    xub_StrLen nStart = pPos->nContent.GetIndex(), nEnd = pTxt->Len();

    BOOL bHasMark = rPam.HasMark();
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
    USHORT nHtIdx = USHRT_MAX;
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
        long nWordStt = pBreakIt->xBreak->getWordBoundary(
                            *pTxt, nStart,
                            pBreakIt->GetLocale( pTNd->GetLang( nStart )),
                            WordType::ANYWORD_IGNOREWHITESPACES,
                            TRUE ).startPos;
        if( nWordStt < nStart && -1 != nWordStt )
        {
            nStart = (xub_StrLen)nWordStt;
            pPos->nContent = nStart;
        }
    }

    BOOL bAlphaNum = FALSE;
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
        BOOL bIgnoreChar = FALSE, bIsAlphaNum = FALSE, bChkNxtWrd = FALSE;
        switch( nChType )
        {
        case UnicodeType::UPPERCASE_LETTER:
        case UnicodeType::LOWERCASE_LETTER:
        case UnicodeType::TITLECASE_LETTER:
        case UnicodeType::DECIMAL_DIGIT_NUMBER:
                bChkNxtWrd = bIsAlphaNum = TRUE;
                break;

        case UnicodeType::SPACE_SEPARATOR:
        case UnicodeType::CONTROL:
/*??*/  case UnicodeType::PRIVATE_USE:
        case UnicodeType::START_PUNCTUATION:
        case UnicodeType::END_PUNCTUATION:
            bIgnoreChar = TRUE;
            break;


        case UnicodeType::OTHER_LETTER:
            bChkNxtWrd = TRUE;
            // no break!
//      case UnicodeType::UNASSIGNED:
//      case UnicodeType::MODIFIER_LETTER:
//      case UnicodeType::NON_SPACING_MARK:
//      case UnicodeType::ENCLOSING_MARK:
//      case UnicodeType::COMBINING_SPACING_MARK:
//      case UnicodeType::LETTER_NUMBER:
//      case UnicodeType::OTHER_NUMBER:
//      case UnicodeType::LINE_SEPARATOR:
//      case UnicodeType::PARAGRAPH_SEPARATOR:
//      case UnicodeType::FORMAT:
//      case UnicodeType::SURROGATE:
//      case UnicodeType::DASH_PUNCTUATION:
//      case UnicodeType::CONNECTOR_PUNCTUATION:
///*?? */case UnicodeType::OTHER_PUNCTUATION:
//--> char '!' is to ignore!
//      case UnicodeType::MATH_SYMBOL:
//      case UnicodeType::CURRENCY_SYMBOL:
//      case UnicodeType::MODIFIER_SYMBOL:
//      case UnicodeType::OTHER_SYMBOL:
//      case UnicodeType::INITIAL_PUNCTUATION:
//      case UnicodeType::FINAL_PUNCTUATION:
        default:
                bIsAlphaNum = FALSE;
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
            if( bChkNxtWrd && pBreakIt->xBreak.is() )
            {
                // search the end of this word
                nWordEnd = pBreakIt->xBreak->getWordBoundary(
                            *pTxt, nStart,
                            pBreakIt->GetLocale( pTNd->GetLang( nStart )),
                            WordType::ANYWORD_IGNOREWHITESPACES,
                            TRUE ).endPos;
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
