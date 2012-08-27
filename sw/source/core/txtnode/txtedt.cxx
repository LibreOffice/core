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


#include <hintids.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemiter.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/hangulhanja.hxx>
#include <SwSmartTagMgr.hxx>
#include <linguistic/lngprops.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/charclass.hxx>
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <splargs.hxx>
#include <viewopt.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>      // GetDoc()
#include <docsh.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <txatbase.hxx>
#include <charatr.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <SwGrammarMarkUp.hxx>

#include <txttypes.hxx>
#include <breakit.hxx>
#include <crstate.hxx>
#include <UndoOverwrite.hxx>
#include <txatritr.hxx>
#include <redline.hxx>      // SwRedline
#include <docary.hxx>       // SwRedlineTbl
#include <scriptinfo.hxx>
#include <docstat.hxx>
#include <editsh.hxx>
#include <unotextmarkup.hxx>
#include <txtatr.hxx>
#include <fmtautofmt.hxx>
#include <istyleaccess.hxx>
#include <unicode/uchar.h>

#include <unomid.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

#include <vector>

using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;

/*
 * This has basically the same function as SwScriptInfo::MaskHiddenRanges,
 * only for deleted redlines
 */

sal_uInt16 lcl_MaskRedlines( const SwTxtNode& rNode, XubString& rText,
                         const xub_StrLen nStt, const xub_StrLen nEnd,
                         const xub_Unicode cChar )
{
    sal_uInt16 nNumOfMaskedRedlines = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    sal_uInt16 nAct = rDoc.GetRedlinePos( rNode, USHRT_MAX );

    for ( ; nAct < rDoc.GetRedlineTbl().size(); nAct++ )
    {
        const SwRedline* pRed = rDoc.GetRedlineTbl()[ nAct ];

        if ( pRed->Start()->nNode > rNode.GetIndex() )
            break;

        if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
        {
            xub_StrLen nRedlineEnd;
            xub_StrLen nRedlineStart;

            pRed->CalcStartEnd( rNode.GetIndex(), nRedlineStart, nRedlineEnd );

            if ( nRedlineEnd < nStt || nRedlineStart > nEnd )
                continue;

            while ( nRedlineStart < nRedlineEnd && nRedlineStart < nEnd )
            {
                if ( nRedlineStart >= nStt && nRedlineStart < nEnd )
                {
                    rText.SetChar( nRedlineStart, cChar );
                    ++nNumOfMaskedRedlines;
                }
                ++nRedlineStart;
            }
        }
    }

    return nNumOfMaskedRedlines;
}

/*
 * Used for spell checking. Deleted redlines and hidden characters are masked
 */

sal_uInt16 lcl_MaskRedlinesAndHiddenText( const SwTxtNode& rNode, XubString& rText,
                                      const xub_StrLen nStt, const xub_StrLen nEnd,
                                      const xub_Unicode cChar = CH_TXTATR_INWORD,
                                      bool bCheckShowHiddenChar = true )
{
    sal_uInt16 nRedlinesMasked = 0;
    sal_uInt16 nHiddenCharsMasked = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    const bool bShowChg = 0 != IDocumentRedlineAccess::IsShowChanges( rDoc.GetRedlineMode() );

    // If called from word count or from spell checking, deleted redlines
    // should be masked:
    if ( bShowChg )
    {
        nRedlinesMasked = lcl_MaskRedlines( rNode, rText, nStt, nEnd, cChar );
    }

    const bool bHideHidden = !SW_MOD()->GetViewOption(rDoc.get(IDocumentSettingAccess::HTML_MODE))->IsShowHiddenChar();

    // If called from word count, we want to mask the hidden ranges even
    // if they are visible:
    if ( !bCheckShowHiddenChar || bHideHidden )
    {
        nHiddenCharsMasked =
            SwScriptInfo::MaskHiddenRanges( rNode, rText, nStt, nEnd, cChar );
    }

    return nRedlinesMasked + nHiddenCharsMasked;
}

/*
 * Used for spell checking. Calculates a rectangle for repaint.
 */

static SwRect lcl_CalculateRepaintRect( SwTxtFrm& rTxtFrm, xub_StrLen nChgStart, xub_StrLen nChgEnd )
{
    SwRect aRect;

    SwTxtNode *pNode = rTxtFrm.GetTxtNode();

    SwNodeIndex aNdIdx( *pNode );
    SwPosition aPos( aNdIdx, SwIndex( pNode, nChgEnd ) );
    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.b2Lines = sal_True;
    rTxtFrm.GetCharRect( aRect, aPos, &aTmpState );
    // information about end of repaint area
    Sw2LinesPos* pEnd2Pos = aTmpState.p2Lines;

    const SwTxtFrm *pEndFrm = &rTxtFrm;

    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    if ( pEnd2Pos )
    {
        // we are inside a special portion, take left border
        SWRECTFN( pEndFrm )
        (aRect.*fnRect->fnSetTop)( (pEnd2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pEndFrm->IsRightToLeft() )
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetLeft)() );
        else
            (aRect.*fnRect->fnSetLeft)( (pEnd2Pos->aPortion.*fnRect->fnGetRight)() );
        (aRect.*fnRect->fnSetWidth)( 1 );
        (aRect.*fnRect->fnSetHeight)( (pEnd2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pEnd2Pos;
    }

    aTmpState.p2Lines = NULL;
    SwRect aTmp;
    aPos = SwPosition( aNdIdx, SwIndex( pNode, nChgStart ) );
    rTxtFrm.GetCharRect( aTmp, aPos, &aTmpState );

    // i63141: GetCharRect(..) could cause a formatting,
    // during the formatting SwTxtFrms could be joined, deleted, created...
    // => we have to reinit pStartFrm and pEndFrm after the formatting
    const SwTxtFrm* pStartFrm = &rTxtFrm;
    while( pStartFrm->HasFollow() &&
           nChgStart >= pStartFrm->GetFollow()->GetOfst() )
        pStartFrm = pStartFrm->GetFollow();
    pEndFrm = pStartFrm;
    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    // information about start of repaint area
    Sw2LinesPos* pSt2Pos = aTmpState.p2Lines;
    if ( pSt2Pos )
    {
        // we are inside a special portion, take right border
        SWRECTFN( pStartFrm )
        (aTmp.*fnRect->fnSetTop)( (pSt2Pos->aLine.*fnRect->fnGetTop)() );
        if ( pStartFrm->IsRightToLeft() )
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetRight)() );
        else
            (aTmp.*fnRect->fnSetLeft)( (pSt2Pos->aPortion.*fnRect->fnGetLeft)() );
        (aTmp.*fnRect->fnSetWidth)( 1 );
        (aTmp.*fnRect->fnSetHeight)( (pSt2Pos->aLine.*fnRect->fnGetHeight)() );
        delete pSt2Pos;
    }

    sal_Bool bSameFrame = sal_True;

    if( rTxtFrm.HasFollow() )
    {
        if( pEndFrm != pStartFrm )
        {
            bSameFrame = sal_False;
            SwRect aStFrm( pStartFrm->PaintArea() );
            {
                SWRECTFN( pStartFrm )
                (aTmp.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                (aTmp.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
                (aTmp.*fnRect->fnSetBottom)( (aStFrm.*fnRect->fnGetBottom)() );
            }
            aStFrm = pEndFrm->PaintArea();
            {
                SWRECTFN( pEndFrm )
                (aRect.*fnRect->fnSetTop)( (aStFrm.*fnRect->fnGetTop)() );
                (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
                (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
            }
            aRect.Union( aTmp );
            while( sal_True )
            {
                pStartFrm = pStartFrm->GetFollow();
                if( pStartFrm == pEndFrm )
                    break;
                aRect.Union( pStartFrm->PaintArea() );
            }
        }
    }
    if( bSameFrame )
    {
        SWRECTFN( pStartFrm )
        if( (aTmp.*fnRect->fnGetTop)() == (aRect.*fnRect->fnGetTop)() )
            (aRect.*fnRect->fnSetLeft)( (aTmp.*fnRect->fnGetLeft)() );
        else
        {
            SwRect aStFrm( pStartFrm->PaintArea() );
            (aRect.*fnRect->fnSetLeft)( (aStFrm.*fnRect->fnGetLeft)() );
            (aRect.*fnRect->fnSetRight)( (aStFrm.*fnRect->fnGetRight)() );
            (aRect.*fnRect->fnSetTop)( (aTmp.*fnRect->fnGetTop)() );
        }

        if( aTmp.Height() > aRect.Height() )
            aRect.Height( aTmp.Height() );
    }

    return aRect;
}

/*
 * Used for automatic styles. Used during RstAttr.
 */

static bool lcl_HaveCommonAttributes( IStyleAccess& rStyleAccess,
                                      const SfxItemSet* pSet1,
                                      sal_uInt16 nWhichId,
                                      const SfxItemSet& rSet2,
                                      boost::shared_ptr<SfxItemSet>& pStyleHandle )
{
    bool bRet = false;

    SfxItemSet* pNewSet = 0;

    if ( !pSet1 )
    {
        OSL_ENSURE( nWhichId, "lcl_HaveCommonAttributes not used correctly" );
        if ( SFX_ITEM_SET == rSet2.GetItemState( nWhichId, sal_False ) )
        {
            pNewSet = rSet2.Clone( sal_True );
            pNewSet->ClearItem( nWhichId );
        }
    }
    else if ( pSet1->Count() )
    {
        SfxItemIter aIter( *pSet1 );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( sal_True )
        {
            if ( SFX_ITEM_SET == rSet2.GetItemState( pItem->Which(), sal_False ) )
            {
                if ( !pNewSet )
                    pNewSet = rSet2.Clone( sal_True );
                pNewSet->ClearItem( pItem->Which() );
            }

            if( aIter.IsAtEnd() )
                break;

            pItem = aIter.NextItem();
        }
    }

    if ( pNewSet )
    {
        if ( pNewSet->Count() )
            pStyleHandle = rStyleAccess.getAutomaticStyle( *pNewSet, IStyleAccess::AUTO_STYLE_CHAR );
        delete pNewSet;
        bRet = true;
    }

    return bRet;
}

inline sal_Bool InRange(xub_StrLen nIdx, xub_StrLen nStart, xub_StrLen nEnd) {
    return ((nIdx >=nStart) && (nIdx <= nEnd));
}

/*
 * void SwTxtNode::RstAttr(const SwIndex &rIdx, sal_uInt16 nLen)
 *
 * Deletes all attributes, starting at position rIdx, for length nLen.
 */

/* 5 cases:
 * 1) The attribute is completely in the deletion range:
 *    -> delete it
 * 2) The end of the attribute is in the deletion range:
 *    -> delete it, then re-insert it with new end
 * 3) The start of the attribute is in the deletion range:
 *    -> delete it, then re-insert it with new start
 * 4) The attribute contains the deletion range:
 *       Split, i.e.,
 *    -> Delete, re-insert from old start to start of deletion range
 *    -> insert new attribute from end of deletion range to old end
 * 5) The attribute is outside the deletion range
 *    -> nothing to do
 */

void SwTxtNode::RstAttr(const SwIndex &rIdx, xub_StrLen nLen, sal_uInt16 nWhich,
                        const SfxItemSet* pSet, sal_Bool bInclRefToxMark )
{
    // Attribute?
    if ( !GetpSwpHints() )
        return;

    sal_uInt16 i = 0;
    xub_StrLen nStt = rIdx.GetIndex();
    xub_StrLen nEnd = nStt + nLen;
    xub_StrLen nAttrStart;
    SwTxtAttr *pHt;

    sal_Bool    bChanged = sal_False;

    // nMin and nMax initialized to maximum / minimum (inverse)
    xub_StrLen nMin = m_Text.Len();
    xub_StrLen nMax = nStt;

    const sal_Bool bNoLen = !nMin;

    // We have to remember the "new" attributes, which have
    // been introduced by splitting surrounding attributes (case 4).
    // They may not be forgotten inside the "Forget" function
    //std::vector< const SwTxtAttr* > aNewAttributes;

    // iterate over attribute array until start of attribute is behind
    // deletion range
    while ((i < m_pSwpHints->Count()) &&
        ((( nAttrStart = *(*m_pSwpHints)[i]->GetStart()) < nEnd ) || nLen==0) )
    {
        pHt = m_pSwpHints->GetTextHint(i);

        // attributes without end stay in!
        xub_StrLen * const pAttrEnd = pHt->GetEnd();
        if ( !pAttrEnd /*|| pHt->HasDummyChar()*/ ) // see bInclRefToxMark
        {
            i++;
            continue;
        }

        // Default behavior is to process all attributes:
        bool bSkipAttr = false;
        boost::shared_ptr<SfxItemSet> pStyleHandle;

        // 1. case: We want to reset only the attributes listed in pSet:
        if ( pSet )
        {
            bSkipAttr = SFX_ITEM_SET != pSet->GetItemState( pHt->Which(), sal_False );
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                // if the current attribute is an autostyle, we have to check if the autostyle
                // and pSet have any attributes in common. If so, pStyleHandle will contain
                // a handle to AutoStyle / pSet:
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), pSet, 0, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( nWhich )
        {
            // 2. case: We want to reset only the attributes with WhichId nWhich:
            bSkipAttr = nWhich != pHt->Which();
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), 0, nWhich, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( !bInclRefToxMark )
        {
            // 3. case: Reset all attributes except from ref/toxmarks:
            // skip hints with CH_TXTATR here
            // (deleting those is ONLY allowed for UNDO!)
            bSkipAttr = RES_TXTATR_REFMARK   == pHt->Which()
                     || RES_TXTATR_TOXMARK   == pHt->Which()
                     || RES_TXTATR_META      == pHt->Which()
                     || RES_TXTATR_METAFIELD == pHt->Which();
        }

        if ( bSkipAttr )
        {
            i++;
            continue;
        }

        if( nStt <= nAttrStart )          // Faelle: 1,3,5
        {
            if( nEnd > nAttrStart
                || ( nEnd == *pAttrEnd && nEnd==nAttrStart ) )
            {
                // Faelle: 1,3
                if ( nMin > nAttrStart )
                    nMin = nAttrStart;
                if ( nMax < *pAttrEnd )
                    nMax = *pAttrEnd;
                // Falls wir nur ein nichtaufgespanntes Attribut entfernen,
                // tun wir mal so, als ob sich nichts geaendert hat.
                bChanged = bChanged || nEnd > nAttrStart || bNoLen;
                if( *pAttrEnd <= nEnd )     // Fall: 1
                {
                    const xub_StrLen nAttrEnd = *pAttrEnd;

                    m_pSwpHints->DeleteAtPos(i);
                    DestroyAttr( pHt );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nAttrEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    // if the last attribute is a Field, the HintsArray is
                    // deleted!
                    if ( !m_pSwpHints )
                        break;

                    //JP 26.11.96:
                    // beim DeleteAtPos wird ein Resort ausgefuehrt!!
                    // darum muessen wir wieder bei 0 anfangen!!!
                    // ueber den Fall 3 koennen Attribute nach hinten
                    // verschoben worden sein; damit stimmt jetzt das i
                    // nicht mehr!!!
                    i = 0;

                    continue;
                }
                else                        // Fall: 3
                {
                    m_pSwpHints->NoteInHistory( pHt );
                    *pHt->GetStart() = nEnd;
                    m_pSwpHints->NoteInHistory( pHt, sal_True );

                    if ( pStyleHandle.get() && nAttrStart < nEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    bChanged = sal_True;
                }
            }
        }
        else                                // Faelle: 2,4,5
            if( *pAttrEnd > nStt )     // Faelle: 2,4
            {
                if( *pAttrEnd < nEnd )      // Fall: 2
                {
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = sal_True;

                    const xub_StrLen nAttrEnd = *pAttrEnd;

                    m_pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, sal_True );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nStt, nAttrEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }
                }
                else if( nLen )             // Fall: 4
                {       // bei Lange 0 werden beide Hints vom Insert(Ht)
                        // wieder zu einem zusammengezogen !!!!
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = sal_True;
                    xub_StrLen nTmpEnd = *pAttrEnd;
                    m_pSwpHints->NoteInHistory( pHt );
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, sal_True );

                    if ( pStyleHandle.get() && nStt < nEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nStt, nEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    if( nEnd < nTmpEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                pHt->GetAttr(), nEnd, nTmpEnd );
                        if ( pNew )
                        {
                            SwTxtCharFmt* pCharFmt = dynamic_cast<SwTxtCharFmt*>(pHt);
                            if ( pCharFmt )
                                static_cast<SwTxtCharFmt*>(pNew)->SetSortNumber( pCharFmt->GetSortNumber() );

                            InsertHint( pNew,
                                nsSetAttrMode::SETATTR_NOHINTADJUST );
                        }

                        // jetzt kein i+1, weil das eingefuegte Attribut
                        // ein anderes auf die Position geschoben hat !
                        continue;
                    }
                }
            }
        ++i;
    }

    TryDeleteSwpHints();
    if (bChanged)
    {
        if ( HasHints() )
        {
            m_pSwpHints->Resort();
        }
        //TxtFrm's reagieren auf aHint, andere auf aNew
        SwUpdateAttr aHint( nMin, nMax, 0 );
        NotifyClients( 0, &aHint );
        SwFmtChg aNew( GetFmtColl() );
        NotifyClients( 0, &aNew );
    }
}

/*************************************************************************
 *                SwTxtNode::GetCurWord()
 *
 * Aktuelles Wort zurueckliefern:
 * Wir suchen immer von links nach rechts, es wird also das Wort
 * vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
 * Absatzes, dann wird das erste Wort zurueckgeliefert.
 * Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
 * einen leeren String.
 *************************************************************************/

XubString SwTxtNode::GetCurWord( xub_StrLen nPos ) const
{
    OSL_ENSURE( nPos <= m_Text.Len(), "SwTxtNode::GetCurWord: invalid index." );

    if (!m_Text.Len())
        return m_Text;

    Boundary aBndry;
    const uno::Reference< XBreakIterator > &rxBreak = pBreakIt->GetBreakIter();
    if (rxBreak.is())
    {
        sal_Int16 nWordType = WordType::DICTIONARY_WORD;
        lang::Locale aLocale( pBreakIt->GetLocale( GetLang( nPos ) ) );
#if OSL_DEBUG_LEVEL > 1
        sal_Bool bBegin = rxBreak->isBeginWord( m_Text, nPos, aLocale, nWordType );
        sal_Bool bEnd   = rxBreak->isEndWord  ( m_Text, nPos, aLocale, nWordType );
        (void)bBegin;
        (void)bEnd;
#endif
        aBndry =
            rxBreak->getWordBoundary( m_Text, nPos, aLocale, nWordType, sal_True );

        // if no word was found use previous word (if any)
        if (aBndry.startPos == aBndry.endPos)
        {
            aBndry = rxBreak->previousWord( m_Text, nPos, aLocale, nWordType );
        }
    }

    // check if word was found and if it uses a symbol font, if so
    // enforce returning an empty string
    if (aBndry.endPos != aBndry.startPos && IsSymbol( (xub_StrLen)aBndry.startPos ))
        aBndry.endPos = aBndry.startPos;

    return m_Text.Copy( static_cast<xub_StrLen>(aBndry.startPos),
                       static_cast<xub_StrLen>(aBndry.endPos - aBndry.startPos) );
}

SwScanner::SwScanner( const SwTxtNode& rNd, const rtl::OUString& rTxt,
    const LanguageType* pLang, const ModelToViewHelper& rConvMap,
    sal_uInt16 nType, sal_Int32 nStart, sal_Int32 nEnde, sal_Bool bClp )
    : rNode( rNd ), aText( rTxt), pLanguage( pLang ), rConversionMap( rConvMap ), nLen( 0 ), nWordType( nType ), bClip( bClp )
{
    OSL_ENSURE( !aText.isEmpty(), "SwScanner: EmptyString" );
    nStartPos = nBegin = nStart;
    nEndPos = nEnde;

    if ( pLanguage )
    {
        aCurrLang = *pLanguage;
    }
    else
    {
        ModelToViewHelper::ModelPosition aModelBeginPos = rConversionMap.ConvertToModelPosition( nBegin );
        const sal_Int32 nModelBeginPos = aModelBeginPos.mnPos;
        aCurrLang = rNd.GetLang( nModelBeginPos );
    }
}

namespace
{
    //fdo#45271 for Asian words count characters instead of words
    sal_Int32 forceEachAsianCodePointToWord(const rtl::OUString &rText, sal_Int32 nBegin, sal_Int32 nLen)
    {
        if (nLen > 1)
        {
            const uno::Reference< XBreakIterator > &rxBreak = pBreakIt->GetBreakIter();

            sal_uInt16 nCurrScript = rxBreak->getScriptType( rText, nBegin );

            sal_Int32 indexUtf16 = nBegin;
            rText.iterateCodePoints(&indexUtf16, 1);

            //First character is Asian, consider it a word :-(
            if (nCurrScript == i18n::ScriptType::ASIAN)
            {
                nLen = indexUtf16 - nBegin;
                return nLen;
            }

            //First character was not Asian, consider appearance of any Asian character
            //to be the end of the word
            while (indexUtf16 < nBegin + nLen)
            {
                nCurrScript = rxBreak->getScriptType( rText, indexUtf16 );
                if (nCurrScript == i18n::ScriptType::ASIAN)
                {
                    nLen = indexUtf16 - nBegin;
                    return nLen;
                }
                rText.iterateCodePoints(&indexUtf16, 1);
            }
        }
        return nLen;
    }
}

sal_Bool SwScanner::NextWord()
{
    nBegin = nBegin + nLen;
    Boundary aBound;

    CharClass& rCC = GetAppCharClass();
    lang::Locale aOldLocale = rCC.getLocale();

    while ( true )
    {
        // skip non-letter characters:
        while ( nBegin < aText.getLength() )
        {
            if ( !u_isspace( aText[nBegin] ) )
            {
                if ( !pLanguage )
                {
                    const sal_uInt16 nNextScriptType = pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
                    ModelToViewHelper::ModelPosition aModelBeginPos = rConversionMap.ConvertToModelPosition( nBegin );
                    const sal_Int32 nBeginModelPos = aModelBeginPos.mnPos;
                    aCurrLang = rNode.GetLang( nBeginModelPos, 1, nNextScriptType );
                }

                if ( nWordType != i18n::WordType::WORD_COUNT )
                {
                    rCC.setLocale( pBreakIt->GetLocale( aCurrLang ) );
                    if ( rCC.isLetterNumeric(rtl::OUString(aText[nBegin])) )
                        break;
                }
                else
                    break;
            }
            ++nBegin;
        }

        if ( nBegin >= aText.getLength() || nBegin >= nEndPos )
            return sal_False;

        // get the word boundaries
        aBound = pBreakIt->GetBreakIter()->getWordBoundary( aText, nBegin,
                pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );
        OSL_ENSURE( aBound.endPos >= aBound.startPos, "broken aBound result" );

        // we don't want to include preceeding text
        if (aBound.startPos < nBegin)
            aBound.startPos = nBegin;

        //no word boundaries could be found
        if(aBound.endPos == aBound.startPos)
            return sal_False;

        //if a word before is found it has to be searched for the next
        if(aBound.endPos == nBegin)
            ++nBegin;
        else
            break;
    } // end while( true )

    rCC.setLocale( aOldLocale );

    // #i89042, as discussed with HDU: don't evaluate script changes for word count. Use whole word.
    if ( nWordType == i18n::WordType::WORD_COUNT )
    {
        nBegin = Max(aBound.startPos, nBegin);
        nLen   = 0;
        if (aBound.endPos > nBegin)
            nLen = aBound.endPos - nBegin;
    }
    else
    {
        // we have to differenciate between these cases:
        if ( aBound.startPos <= nBegin )
        {
            OSL_ENSURE( aBound.endPos >= nBegin, "Unexpected aBound result" );

            // restrict boundaries to script boundaries and nEndPos
            const sal_uInt16 nCurrScript = pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
            rtl::OUString aTmpWord = aText.copy( nBegin, aBound.endPos - nBegin );
            const sal_Int32 nScriptEnd = nBegin +
                pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );

            // restrict word start to last script change position
            sal_Int32 nScriptBegin = 0;
            if ( aBound.startPos < nBegin )
            {
                // search from nBegin backwards until the next script change
                aTmpWord = aText.copy( aBound.startPos,
                                       nBegin - aBound.startPos + 1 );
                nScriptBegin = aBound.startPos +
                    pBreakIt->GetBreakIter()->beginOfScript( aTmpWord, nBegin - aBound.startPos,
                                                    nCurrScript );
            }

            nBegin = Max( aBound.startPos, nScriptBegin );
            nLen = nEnd - nBegin;
        }
        else
        {
            const sal_uInt16 nCurrScript = pBreakIt->GetBreakIter()->getScriptType( aText, aBound.startPos );
            rtl::OUString aTmpWord = aText.copy( aBound.startPos,
                                             aBound.endPos - aBound.startPos );
            const sal_Int32 nScriptEnd = aBound.startPos +
                pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
            nBegin = aBound.startPos;
            nLen = nEnd - nBegin;
        }
    }

    // optionally clip the result of getWordBoundaries:
    if ( bClip )
    {
        aBound.startPos = Max( aBound.startPos, nStartPos );
        aBound.endPos = Min( aBound.endPos, nEndPos );
        nBegin = aBound.startPos;
        nLen = aBound.endPos - nBegin;
    }

    if( ! nLen )
        return sal_False;

    if ( nWordType == i18n::WordType::WORD_COUNT )
        nLen = forceEachAsianCodePointToWord(aText, nBegin, nLen);

    aWord = aText.copy( nBegin, nLen );

    return sal_True;
}

sal_uInt16 SwTxtNode::Spell(SwSpellArgs* pArgs)
{
    // Die Aehnlichkeiten zu SwTxtFrm::_AutoSpell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!

    uno::Reference<beans::XPropertySet> xProp( GetLinguPropertySet() );

    xub_StrLen nBegin, nEnd;

    // modify string according to redline information and hidden text
    const XubString aOldTxt( m_Text );
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText( *this, m_Text, 0, m_Text.Len() ) > 0;

    if ( pArgs->pStartNode != this )
        nBegin = 0;
    else
        nBegin = pArgs->pStartIdx->GetIndex();

    nEnd = ( pArgs->pEndNode != this )
            ? m_Text.Len()
            : pArgs->pEndIdx->GetIndex();

    pArgs->xSpellAlt = NULL;

    // 4 cases:
    //
    // 1. IsWrongDirty = 0 and GetWrong = 0
    //      Everything is checked and correct
    // 2. IsWrongDirty = 0 and GetWrong = 1
    //      Everything is checked and errors are identified in the wrong list
    // 3. IsWrongDirty = 1 and GetWrong = 0
    //      Nothing has been checked
    // 4. IsWrongDirty = 1 and GetWrong = 1
    //      Text has been checked but there is an invalid range in the wrong list
    //
    // Nothing has to be done for case 1.
    if ( ( IsWrongDirty() || GetWrong() ) && m_Text.Len() )
    {
        if ( nBegin > m_Text.Len() )
        {
            nBegin = m_Text.Len();
        }
        if ( nEnd > m_Text.Len() )
        {
            nEnd = m_Text.Len();
        }
        //
        if(!IsWrongDirty())
        {
            xub_StrLen nTemp = GetWrong()->NextWrong( nBegin );
            if(nTemp > nEnd)
            {
                // reset original text
                if ( bRestoreString )
                {
                    m_Text = aOldTxt;
                }
                return 0;
            }
            if(nTemp > nBegin)
                nBegin = nTemp;

        }

        // In case 2. we pass the wrong list to the scanned, because only
        // the words in the wrong list have to be checked
        SwScanner aScanner( *this, m_Text, 0, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord() )
        {
            const XubString& rWord = aScanner.GetWord();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            if( rWord.Len() > 0 && LANGUAGE_NONE != eActLang )
            {
                if (pArgs->xSpeller.is())
                {
                    SvxSpellWrapper::CheckSpellLang( pArgs->xSpeller, eActLang );
                    pArgs->xSpellAlt = pArgs->xSpeller->spell( rWord, eActLang,
                                            Sequence< PropertyValue >() );
                }
                if( (pArgs->xSpellAlt).is() )
                {
                    if( IsSymbol( aScanner.GetBegin() ) )
                    {
                        pArgs->xSpellAlt = NULL;
                    }
                    else
                    {
                        // make sure the selection build later from the data
                        // below does not include "in word" character to the
                        // left and right in order to preserve those. Therefore
                        // count those "in words" in order to modify the
                        // selection accordingly.
                        const sal_Unicode* pChar = rWord.GetBuffer();
                        xub_StrLen nLeft = 0;
                        while (pChar && *pChar++ == CH_TXTATR_INWORD)
                            ++nLeft;
                        pChar = rWord.Len() ? rWord.GetBuffer() + rWord.Len() - 1 : 0;
                        xub_StrLen nRight = 0;
                        while (pChar && *pChar-- == CH_TXTATR_INWORD)
                            ++nRight;

                        pArgs->pStartNode = this;
                        pArgs->pEndNode = this;
                        pArgs->pStartIdx->Assign(this, aScanner.GetEnd() - nRight );
                        pArgs->pEndIdx->Assign(this, aScanner.GetBegin() + nLeft );
                    }
                }
            }
        }
    }

    // reset original text
    if ( bRestoreString )
    {
        m_Text = aOldTxt;
    }

    return pArgs->xSpellAlt.is() ? 1 : 0;
}

void SwTxtNode::SetLanguageAndFont( const SwPaM &rPaM,
    LanguageType nLang, sal_uInt16 nLangWhichId,
    const Font *pFont,  sal_uInt16 nFontWhichId )
{
    sal_uInt16 aRanges[] = {
            nLangWhichId, nLangWhichId,
            nFontWhichId, nFontWhichId,
            0, 0, 0 };
    if (!pFont)
        aRanges[2] = aRanges[3] = 0;    // clear entries with font WhichId

    SwEditShell *pEditShell = GetDoc()->GetEditShell();
    SfxItemSet aSet( pEditShell->GetAttrPool(), aRanges );
    aSet.Put( SvxLanguageItem( nLang, nLangWhichId ) );

    OSL_ENSURE( pFont, "target font missing?" );
    if (pFont)
    {
        SvxFontItem aFontItem = (SvxFontItem&) aSet.Get( nFontWhichId );
        aFontItem.SetFamilyName(   pFont->GetName());
        aFontItem.SetFamily(       pFont->GetFamily());
        aFontItem.SetStyleName(    pFont->GetStyleName());
        aFontItem.SetPitch(        pFont->GetPitch());
        aFontItem.SetCharSet( pFont->GetCharSet() );
        aSet.Put( aFontItem );
    }

    GetDoc()->InsertItemSet( rPaM, aSet, 0 );
    // SetAttr( aSet );    <- Does not set language attribute of empty paragraphs correctly,
    //                     <- because since there is no selection the flag to garbage
    //                     <- collect all attributes is set, and therefore attributes spanned
    //                     <- over empty selection are removed.

}

sal_uInt16 SwTxtNode::Convert( SwConversionArgs &rArgs )
{
    // get range of text within node to be converted
    // (either all the text or the the text within the selection
    // when the conversion was started)
    xub_StrLen nTextBegin, nTextEnd;
    //
    if ( rArgs.pStartNode != this )
    {
        nTextBegin = 0;
    }
    else
        nTextBegin = rArgs.pStartIdx->GetIndex();
    if (nTextBegin > m_Text.Len())
    {
        nTextBegin = m_Text.Len();
    }

    nTextEnd = ( rArgs.pEndNode != this )
        ?  m_Text.Len()
        :  ::std::min( rArgs.pEndIdx->GetIndex(), m_Text.Len() );

    rArgs.aConvText = rtl::OUString();

    // modify string according to redline information and hidden text
    const XubString aOldTxt( m_Text );
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText( *this, m_Text, 0, m_Text.Len() ) > 0;

    sal_Bool    bFound  = sal_False;
    xub_StrLen  nBegin  = nTextBegin;
    xub_StrLen  nLen = 0;
    LanguageType nLangFound = LANGUAGE_NONE;
    if (!m_Text.Len())
    {
        if (rArgs.bAllowImplicitChangesForNotConvertibleText)
        {
            // create SwPaM with mark & point spanning empty paragraph
            //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
            SwPaM aCurPaM( *this, 0 );

            SetLanguageAndFont( aCurPaM,
                    rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                    rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }
    }
    else
    {
        SwLanguageIterator aIter( *this, nBegin );

        // find non zero length text portion of appropriate language
        do {
            nLangFound = aIter.GetLanguage();
            sal_Bool bLangOk =  (nLangFound == rArgs.nConvSrcLang) ||
                                (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 editeng::HangulHanjaConversion::IsChinese( rArgs.nConvSrcLang ));

            xub_StrLen nChPos = aIter.GetChgPos();
            // the position at the end of the paragraph returns -1
            // which becomes 65535 when converted to xub_StrLen,
            // and thus must be cut to the end of the actual string.
            if (nChPos == (xub_StrLen) -1)
            {
                nChPos = m_Text.Len();
            }

            nLen = nChPos - nBegin;
            bFound = bLangOk && nLen > 0;
            if (!bFound)
            {
                // create SwPaM with mark & point spanning the attributed text
                //SwPaM aCurPaM( *this, *this, nBegin, nBegin + nLen ); <-- wrong c-tor, does sth different
                SwPaM aCurPaM( *this, nBegin );
                aCurPaM.SetMark();
                aCurPaM.GetPoint()->nContent = nBegin + nLen;

                // check script type of selected text
                SwEditShell *pEditShell = GetDoc()->GetEditShell();
                pEditShell->Push();             // save current cursor on stack
                pEditShell->SetSelection( aCurPaM );
                sal_Bool bIsAsianScript = (SCRIPTTYPE_ASIAN == pEditShell->GetScriptType());
                pEditShell->Pop( sal_False );   // restore cursor from stack

                if (!bIsAsianScript && rArgs.bAllowImplicitChangesForNotConvertibleText)
                {
                    SetLanguageAndFont( aCurPaM,
                            rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                            rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
                }
                nBegin = nChPos;    // start of next language portion
            }
        } while (!bFound && aIter.Next());  /* loop while nothing was found and still sth is left to be searched */
    }

    // keep resulting text within selection / range of text to be converted
    if (nBegin < nTextBegin)
        nBegin = nTextBegin;
    if (nBegin + nLen > nTextEnd)
        nLen = nTextEnd - nBegin;
    sal_Bool bInSelection = nBegin < nTextEnd;

    if (bFound && bInSelection)     // convertible text found within selection/range?
    {
        const XubString aTxtPortion = m_Text.Copy( nBegin, nLen );
        OSL_ENSURE( m_Text.Len() > 0, "convertible text portion missing!" );
        rArgs.aConvText     = m_Text.Copy( nBegin, nLen );
        rArgs.nConvTextLang = nLangFound;

        // position where to start looking in next iteration (after current ends)
        rArgs.pStartNode = this;
        rArgs.pStartIdx->Assign(this, nBegin + nLen );
        // end position (when we have travelled over the whole document)
        rArgs.pEndNode = this;
        rArgs.pEndIdx->Assign(this, nBegin );
    }

    // restore original text
    if ( bRestoreString )
    {
        m_Text = aOldTxt;
    }

    return rArgs.aConvText.isEmpty() ? 0 : 1;
}

// Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
// ACHTUNG: Ev. Bugs in beiden Routinen fixen!
SwRect SwTxtFrm::_AutoSpell( const SwCntntNode* pActNode, const SwViewOption& rViewOpt, xub_StrLen nActPos )
{
    SwRect aRect;
#if OSL_DEBUG_LEVEL > 1
    static sal_Bool bStop = sal_False;
    if ( bStop )
        return aRect;
#endif
    // Die Aehnlichkeiten zu SwTxtNode::Spell sind beabsichtigt ...
    // ACHTUNG: Ev. Bugs in beiden Routinen fixen!
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    // modify string according to redline information and hidden text
    const XubString aOldTxt( pNode->GetTxt() );
    const bool bRestoreString =
            lcl_MaskRedlinesAndHiddenText( *pNode, pNode->m_Text,
                0, pNode->GetTxt().Len() )     > 0;

    // a change of data indicates that at least one word has been modified
    const sal_Bool bRedlineChg =
        ( pNode->GetTxt().GetBuffer() != aOldTxt.GetBuffer() );

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->GetTxt().Len();
    xub_StrLen nInsertPos = 0;
    xub_StrLen nChgStart = STRING_LEN;
    xub_StrLen nChgEnd = 0;
    xub_StrLen nInvStart = STRING_LEN;
    xub_StrLen nInvEnd = 0;

    const sal_Bool bAddAutoCmpl = pNode->IsAutoCompleteWordDirty() &&
                                  rViewOpt.IsAutoCompleteWords();

    if( pNode->GetWrong() )
    {
        nBegin = pNode->GetWrong()->GetBeginInv();
        if( STRING_LEN != nBegin )
        {
            nEnd = pNode->GetWrong()->GetEndInv();
            if ( nEnd > pNode->GetTxt().Len() )
            {
                nEnd = pNode->GetTxt().Len();
            }
        }

        // get word around nBegin, we start at nBegin - 1
        if ( STRING_LEN != nBegin )
        {
            if ( nBegin )
                --nBegin;

            LanguageType eActLang = pNode->GetLang( nBegin );
            Boundary aBound =
                pBreakIt->GetBreakIter()->getWordBoundary( pNode->GetTxt(), nBegin,
                    pBreakIt->GetLocale( eActLang ),
                    WordType::DICTIONARY_WORD, sal_True );
            nBegin = xub_StrLen(aBound.startPos);
        }

        // get the position in the wrong list
        nInsertPos = pNode->GetWrong()->GetWrongPos( nBegin );

        // sometimes we have to skip one entry
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
                nInsertPos++;
    }

    sal_Bool bFresh = nBegin < nEnd;

    if( nBegin < nEnd )
    {
        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        uno::Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );
        SwDoc* pDoc = pNode->GetDoc();

        SwScanner aScanner( *pNode, pNode->GetTxt(), 0, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD, nBegin, nEnd);

        while( aScanner.NextWord() )
        {
            const XubString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            xub_StrLen nLen = aScanner.GetLen();

            // get next language for next word, consider language attributes
            // within the word
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            sal_Bool bSpell = sal_True;
            bSpell = xSpell.is() ? xSpell->hasLanguage( eActLang ) : sal_False;
            if( bSpell && rWord.Len() > 0 )
            {
                // check for: bAlter => xHyphWord.is()
                OSL_ENSURE(!bSpell || xSpell.is(), "NULL pointer");

                if( !xSpell->isValid( rWord, eActLang, Sequence< PropertyValue >() ) )
                {
                    xub_StrLen nSmartTagStt = nBegin;
                    xub_StrLen nDummy = 1;
                    if ( !pNode->GetSmartTags() || !pNode->GetSmartTags()->InWrongWord( nSmartTagStt, nDummy ) )
                    {
                        if( !pNode->GetWrong() )
                        {
                            pNode->SetWrong( new SwWrongList( WRONGLIST_SPELL ) );
                            pNode->GetWrong()->SetInvalid( 0, nEnd );
                        }
                        if( pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                            nBegin, nLen, nInsertPos, nActPos ) )
                            pNode->GetWrong()->Insert( rtl::OUString(), 0, nBegin, nLen, nInsertPos++ );
                        else
                        {
                            nInvStart = nBegin;
                            nInvEnd = nBegin + nLen;
                        }
                    }
                }
                else if( bAddAutoCmpl && rACW.GetMinWordLen() <= rWord.Len() )
                {
                    if ( bRedlineChg )
                    {
                        XubString rNewWord( rWord );
                        rACW.InsertWord( rNewWord, *pDoc );
                    }
                    else
                        rACW.InsertWord( rWord, *pDoc );
                }
            }
        }
    }

    // reset original text
    // i63141 before calling GetCharRect(..) with formatting!
    if ( bRestoreString )
    {
        pNode->m_Text = aOldTxt;
    }
    if( pNode->GetWrong() )
    {
        if( bFresh )
            pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                                      nEnd, 0, nInsertPos, nActPos );

        //
        // Calculate repaint area:
        //
        if( nChgStart < nChgEnd )
        {
            aRect = lcl_CalculateRepaintRect( *this, nChgStart, nChgEnd );
        }

        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty( STRING_LEN != pNode->GetWrong()->GetBeginInv() );
        if( !pNode->GetWrong()->Count() && ! pNode->IsWrongDirty() )
            pNode->SetWrong( NULL );
    }
    else
        pNode->SetWrongDirty( false );

    if( bAddAutoCmpl )
        pNode->SetAutoCompleteWordDirty( false );

    return aRect;
}

/** Function: SmartTagScan

    Function scans words in current text and checks them in the
    smarttag libraries. If the check returns true to bounds of the
    recognized words are stored into a list which is used later for drawing
    the underline.

    @param SwCntntNode* pActNode

    @param xub_StrLen nActPos

    @return SwRect: Repaint area
*/
SwRect SwTxtFrm::SmartTagScan( SwCntntNode* /*pActNode*/, xub_StrLen /*nActPos*/ )
{
    SwRect aRet;
    SwTxtNode *pNode = GetTxtNode();
    const rtl::OUString& rText = pNode->GetTxt();

    // Iterate over language portions
    SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();

    SwWrongList* pSmartTagList = pNode->GetSmartTags();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = static_cast< xub_StrLen >(rText.getLength());

    if ( pSmartTagList )
    {
        if ( pSmartTagList->GetBeginInv() != STRING_LEN )
        {
            nBegin = pSmartTagList->GetBeginInv();
            nEnd = Min( pSmartTagList->GetEndInv(), (xub_StrLen)rText.getLength() );

            if ( nBegin < nEnd )
            {
                const LanguageType aCurrLang = pNode->GetLang( nBegin );
                const com::sun::star::lang::Locale aCurrLocale = pBreakIt->GetLocale( aCurrLang );
                nBegin = static_cast< xub_StrLen >(pBreakIt->GetBreakIter()->beginOfSentence( rText, nBegin, aCurrLocale ));
                nEnd = static_cast< xub_StrLen >(Min( rText.getLength(), pBreakIt->GetBreakIter()->endOfSentence( rText, nEnd, aCurrLocale ) ));
            }
        }
    }

    const sal_uInt16 nNumberOfEntries = pSmartTagList ? pSmartTagList->Count() : 0;
    sal_uInt16 nNumberOfRemovedEntries = 0;
    sal_uInt16 nNumberOfInsertedEntries = 0;

    // clear smart tag list between nBegin and nEnd:
    if ( 0 != nNumberOfEntries )
    {
        xub_StrLen nChgStart = STRING_LEN;
        xub_StrLen nChgEnd = 0;
        const sal_uInt16 nCurrentIndex = pSmartTagList->GetWrongPos( nBegin );
        pSmartTagList->Fresh( nChgStart, nChgEnd, nBegin, nEnd - nBegin, nCurrentIndex, STRING_LEN );
        nNumberOfRemovedEntries = nNumberOfEntries - pSmartTagList->Count();
    }

    if ( nBegin < nEnd )
    {
        // Expand the string:
        const ModelToViewHelper aConversionMap(*pNode);
        rtl::OUString aExpandText = aConversionMap.getViewText();

        // Ownership ov ConversionMap is passed to SwXTextMarkup object!
        com::sun::star::uno::Reference< com::sun::star::text::XTextMarkup > xTextMarkup =
             new SwXTextMarkup( *pNode, aConversionMap );

        com::sun::star::uno::Reference< ::com::sun::star::frame::XController > xController = pNode->GetDoc()->GetDocShell()->GetController();

        xub_StrLen nLangBegin = nBegin;
        xub_StrLen nLangEnd = nEnd;

        // smart tag recognization has to be done for each language portion:
        SwLanguageIterator aIter( *pNode, nLangBegin );

        do
        {
            const LanguageType nLang = aIter.GetLanguage();
            const com::sun::star::lang::Locale aLocale = pBreakIt->GetLocale( nLang );
            nLangEnd = Min( nEnd, aIter.GetChgPos() );

            const sal_uInt32 nExpandBegin = aConversionMap.ConvertToViewPosition( nLangBegin );
            const sal_uInt32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nLangEnd );

            rSmartTagMgr.Recognize( aExpandText, xTextMarkup, xController, aLocale, nExpandBegin, nExpandEnd - nExpandBegin );

            nLangBegin = nLangEnd;
        }
        while ( aIter.Next() && nLangEnd < nEnd );

        pSmartTagList = pNode->GetSmartTags();

        const sal_uInt16 nNumberOfEntriesAfterRecognize = pSmartTagList ? pSmartTagList->Count() : 0;
        nNumberOfInsertedEntries = nNumberOfEntriesAfterRecognize - ( nNumberOfEntries - nNumberOfRemovedEntries );
    }

    if( pSmartTagList )
    {
        //
        // Update WrongList stuff
        //
        pSmartTagList->SetInvalid( STRING_LEN, 0 );
        pNode->SetSmartTagDirty( STRING_LEN != pSmartTagList->GetBeginInv() );

        if( !pSmartTagList->Count() && !pNode->IsSmartTagDirty() )
            pNode->SetSmartTags( NULL );

        //
        // Calculate repaint area:
        //
#if OSL_DEBUG_LEVEL > 1
        const sal_uInt16 nNumberOfEntriesAfterRecognize2 = pSmartTagList->Count();
        (void) nNumberOfEntriesAfterRecognize2;
#endif
        if ( nBegin < nEnd && ( 0 != nNumberOfRemovedEntries ||
                                0 != nNumberOfInsertedEntries ) )
        {
            aRet = lcl_CalculateRepaintRect( *this, nBegin, nEnd );
        }
    }
    else
        pNode->SetSmartTagDirty( false );

    return aRet;
}

// Wird vom CollectAutoCmplWords gerufen
void SwTxtFrm::CollectAutoCmplWrds( SwCntntNode* pActNode, xub_StrLen nActPos )
{
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = STRING_LEN;

    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    xub_StrLen nBegin = 0;
    xub_StrLen nEnd = pNode->GetTxt().Len();
    xub_StrLen nLen;
    sal_Bool bACWDirty = sal_False, bAnyWrd = sal_False;

    if( nBegin < nEnd )
    {
        sal_uInt16 nCnt = 200;
        SwScanner aScanner( *pNode, pNode->GetTxt(), 0, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD, nBegin, nEnd );
        while( aScanner.NextWord() )
        {
            nBegin = aScanner.GetBegin();
            nLen = aScanner.GetLen();
            if( rACW.GetMinWordLen() <= nLen )
            {
                const XubString& rWord = aScanner.GetWord();

                if( nActPos < nBegin || ( nBegin + nLen ) < nActPos )
                {
                    if( rACW.GetMinWordLen() <= rWord.Len() )
                        rACW.InsertWord( rWord, *pDoc );
                    bAnyWrd = sal_True;
                }
                else
                    bACWDirty = sal_True;
            }
            if( !--nCnt )
            {
                if ( Application::AnyInput( VCL_INPUT_ANY ) )
                    return;
                nCnt = 100;
            }
        }
    }

    if( bAnyWrd && !bACWDirty )
        pNode->SetAutoCompleteWordDirty( sal_False );
}

/*************************************************************************
 *                      SwTxtNode::Hyphenate
 *************************************************************************/
// Findet den TxtFrm und sucht dessen CalcHyph

sal_Bool SwTxtNode::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    // Abkuerzung: am Absatz ist keine Sprache eingestellt:
    if ( LANGUAGE_NONE == sal_uInt16( GetSwAttrSet().GetLanguage().GetLanguage() )
         && USHRT_MAX == GetLang( 0, m_Text.Len() ) )
    {
        if( !rHyphInf.IsCheck() )
            rHyphInf.SetNoLang( sal_True );
        return sal_False;
    }

    if( pLinguNode != this )
    {
        pLinguNode = this;
        pLinguFrm = (SwTxtFrm*)getLayoutFrm( GetDoc()->GetCurrentLayout(), (Point*)(rHyphInf.GetCrsrPos()) );
    }
    SwTxtFrm *pFrm = pLinguFrm;
    if( pFrm )
        pFrm = &(pFrm->GetFrmAtOfst( rHyphInf.nStart ));
    else
    {
        // 4935: Seit der Trennung ueber Sonderbereiche sind Faelle
        // moeglich, in denen kein Frame zum Node vorliegt.
        // Also keinOSL_ENSURE
        OSL_ENSURE( pFrm, "!SwTxtNode::Hyphenate: can't find any frame" );
        return sal_False;
    }

    while( pFrm )
    {
        if( pFrm->Hyphenate( rHyphInf ) )
        {
            // Das Layout ist nicht robust gegen "Direktformatierung"
            // (7821, 7662, 7408); vgl. layact.cxx,
            // SwLayAction::_TurboAction(), if( !pCnt->IsValid() ...
            pFrm->SetCompletePaint();
            return sal_True;
        }
        pFrm = (SwTxtFrm*)(pFrm->GetFollow());
        if( pFrm )
        {
            rHyphInf.nLen = rHyphInf.nLen - (pFrm->GetOfst() - rHyphInf.nStart);
            rHyphInf.nStart = pFrm->GetOfst();
        }
    }
    return sal_False;
}

namespace
{
    struct swTransliterationChgData
    {
        xub_StrLen              nStart;
        xub_StrLen              nLen;
        String                  sChanged;
        Sequence< sal_Int32 >   aOffsets;
    };
}

// change text to Upper/Lower/Hiragana/Katagana/...
void SwTxtNode::TransliterateText(
    utl::TransliterationWrapper& rTrans,
    xub_StrLen nStt, xub_StrLen nEnd,
    SwUndoTransliterate* pUndo )
{
    if (nStt < nEnd && pBreakIt->GetBreakIter().is())
    {
        // since we don't use Hiragana/Katakana or half-width/full-width transliterations here
        // it is fine to use ANYWORD_IGNOREWHITESPACES. (ANY_WORD btw is broken and will
        // occasionaly miss words in consecutive sentences). Also with ANYWORD_IGNOREWHITESPACES
        // text like 'just-in-time' will be converted to 'Just-In-Time' which seems to be the
        // proper thing to do.
        const sal_Int16 nWordType = WordType::ANYWORD_IGNOREWHITESPACES;

        //! In order to have less trouble with changing text size, e.g. because
        //! of ligatures or � (German small sz) being resolved, we need to process
        //! the text replacements from end to start.
        //! This way the offsets for the yet to be changed words will be
        //! left unchanged by the already replaced text.
        //! For this we temporarily save the changes to be done in this vector
        std::vector< swTransliterationChgData >   aChanges;
        swTransliterationChgData                  aChgData;

        if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::TITLE_CASE)
        {
            // for 'capitalize every word' we need to iterate over each word

            Boundary aSttBndry;
            Boundary aEndBndry;
            aSttBndry = pBreakIt->GetBreakIter()->getWordBoundary(
                        GetTxt(), nStt,
                        pBreakIt->GetLocale( GetLang( nStt ) ),
                        nWordType,
                        sal_True /*prefer forward direction*/);
            aEndBndry = pBreakIt->GetBreakIter()->getWordBoundary(
                        GetTxt(), nEnd,
                        pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType,
                        sal_False /*prefer backward direction*/);

            // prevent backtracking to the previous word if selection is at word boundary
            if (aSttBndry.endPos <= nStt)
            {
                aSttBndry = pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), aSttBndry.endPos,
                        pBreakIt->GetLocale( GetLang( aSttBndry.endPos ) ),
                        nWordType);
            }
            // prevent advancing to the next word if selection is at word boundary
            if (aEndBndry.startPos >= nEnd)
            {
                aEndBndry = pBreakIt->GetBreakIter()->previousWord(
                        GetTxt(), aEndBndry.startPos,
                        pBreakIt->GetLocale( GetLang( aEndBndry.startPos ) ),
                        nWordType);
            }

            Boundary aCurWordBndry( aSttBndry );
            while (aCurWordBndry.startPos <= aEndBndry.startPos)
            {
                nStt = (xub_StrLen)aCurWordBndry.startPos;
                nEnd = (xub_StrLen)aCurWordBndry.endPos;
                sal_Int32 nLen = nEnd - nStt;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                String sChgd( rTrans.transliterate( GetTxt(), GetLang( nStt ), nStt, nLen, &aOffsets ));

                if (!m_Text.Equals( sChgd, nStt, nLen ))
                {
                    aChgData.nStart     = nStt;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                aCurWordBndry = pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nEnd,
                        pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType);
            }
        }
        else if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::SENTENCE_CASE)
        {
            // for 'sentence case' we need to iterate sentence by sentence

            sal_Int32 nLastStart = pBreakIt->GetBreakIter()->beginOfSentence(
                    GetTxt(), nEnd,
                    pBreakIt->GetLocale( GetLang( nEnd ) ) );
            sal_Int32 nLastEnd = pBreakIt->GetBreakIter()->endOfSentence(
                    GetTxt(), nLastStart,
                    pBreakIt->GetLocale( GetLang( nLastStart ) ) );

            // extend nStt, nEnd to the current sentence boundaries
            sal_Int32 nCurrentStart = pBreakIt->GetBreakIter()->beginOfSentence(
                    GetTxt(), nStt,
                    pBreakIt->GetLocale( GetLang( nStt ) ) );
            sal_Int32 nCurrentEnd = pBreakIt->GetBreakIter()->endOfSentence(
                    GetTxt(), nCurrentStart,
                    pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );

            // prevent backtracking to the previous sentence if selection starts at end of a sentence
            if (nCurrentEnd <= nStt)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the next real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nCurrentEnd,
                        pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        i18n::WordType::DICTIONARY_WORD);

                // now get new current sentence boundaries
                nCurrentStart = pBreakIt->GetBreakIter()->beginOfSentence(
                        GetTxt(), aBndry.startPos,
                        pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                nCurrentEnd = pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), nCurrentStart,
                        pBreakIt->GetLocale( GetLang( nCurrentStart) ) );
            }
            // prevent advancing to the next sentence if selection ends at start of a sentence
            if (nLastStart >= nEnd)
            {
                // now nCurrentStart is probably located on a non-letter word. (unless we
                // are in Asian text with no spaces...)
                // Thus to get the real sentence start we should locate the previous real word,
                // that is one found by DICTIONARY_WORD
                i18n::Boundary aBndry = pBreakIt->GetBreakIter()->previousWord(
                        GetTxt(), nLastStart,
                        pBreakIt->GetLocale( GetLang( nLastStart) ),
                        i18n::WordType::DICTIONARY_WORD);
                nLastEnd = pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), aBndry.startPos,
                        pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                if (nCurrentEnd > nLastEnd)
                    nCurrentEnd = nLastEnd;
            }

            while (nCurrentStart < nLastEnd)
            {
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                String sChgd( rTrans.transliterate( GetTxt(),
                        GetLang( nCurrentStart ), nCurrentStart, nLen, &aOffsets ));

                if (!m_Text.Equals( sChgd, nStt, nLen ))
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                Boundary aFirstWordBndry;
                aFirstWordBndry = pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nCurrentEnd,
                        pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        nWordType);
                nCurrentStart = aFirstWordBndry.startPos;
                nCurrentEnd = pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), nCurrentStart,
                        pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );
            }
        }
        else
        {
            // here we may transliterate over complete language portions...

            SwLanguageIterator* pIter;
            if( rTrans.needLanguageForTheMode() )
                pIter = new SwLanguageIterator( *this, nStt );
            else
                pIter = 0;

            xub_StrLen nEndPos;
            sal_uInt16 nLang;
            do {
                if( pIter )
                {
                    nLang = pIter->GetLanguage();
                    nEndPos = pIter->GetChgPos();
                    if( nEndPos > nEnd )
                        nEndPos = nEnd;
                }
                else
                {
                    nLang = LANGUAGE_SYSTEM;
                    nEndPos = nEnd;
                }
                xub_StrLen nLen = nEndPos - nStt;

                Sequence <sal_Int32> aOffsets;
                String sChgd( rTrans.transliterate( m_Text, nLang, nStt, nLen, &aOffsets ));

                if (!m_Text.Equals( sChgd, nStt, nLen ))
                {
                    aChgData.nStart     = nStt;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                nStt = nEndPos;
            } while( nEndPos < nEnd && pIter && pIter->Next() );
            delete pIter;
        }

        if (!aChanges.empty())
        {
            // now apply the changes from end to start to leave the offsets of the
            // yet unchanged text parts remain the same.
            for (size_t i = 0; i < aChanges.size(); ++i)
            {
                swTransliterationChgData &rData = aChanges[ aChanges.size() - 1 - i ];
                if (pUndo)
                    pUndo->AddChanges( *this, rData.nStart, rData.nLen, rData.aOffsets );
                ReplaceTextOnly( rData.nStart, rData.nLen, rData.sChanged, rData.aOffsets );
            }
        }
    }
}

void SwTxtNode::ReplaceTextOnly( xub_StrLen nPos, xub_StrLen nLen,
                                const XubString& rText,
                                const Sequence<sal_Int32>& rOffsets )
{
    m_Text.Replace( nPos, nLen, rText );

    xub_StrLen nTLen = rText.Len();
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    // now look for no 1-1 mapping -> move the indizies!
    xub_StrLen nI, nMyOff;
    for( nI = 0, nMyOff = nPos; nI < nTLen; ++nI, ++nMyOff )
    {
        xub_StrLen nOff = (xub_StrLen)pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            // something is inserted
            xub_StrLen nCnt = 1;
            while( nI + nCnt < nTLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt, sal_False );
            nMyOff = nOff;
            //nMyOff -= nCnt;
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            // something is deleted
            Update( SwIndex( this, nMyOff+1 ), nOff - nMyOff, sal_True );
            nMyOff = nOff;
        }
    }
    if( nMyOff < nLen )
        // something is deleted at the end
        Update( SwIndex( this, nMyOff ), nLen - nMyOff, sal_True );

    // notify the layout!
    SwDelTxt aDelHint( nPos, nTLen );
    NotifyClients( 0, &aDelHint );

    SwInsTxt aHint( nPos, nTLen );
    NotifyClients( 0, &aHint );
}

void SwTxtNode::CountWords( SwDocStat& rStat,
                            xub_StrLen nStt, xub_StrLen nEnd ) const
{
    if( nStt > nEnd )
    {   // bad call
        return;
    }
    if (IsInRedlines())
    {   //not counting txtnodes used to hold deleted redline content
        return;
    }
    bool bCountAll = ( (0 == nStt) && (GetTxt().Len() == nEnd) );
    ++rStat.nAllPara; // #i93174#: count _all_ paragraphs
    if ( IsHidden() )
    {   // not counting hidden paras
        return;
    }
    // count words in numbering string if started at beginning of para:
    bool bCountNumbering = nStt == 0;
    bool bHasBullet = false, bHasNumbering = false;
    rtl::OUString sNumString;
    if (bCountNumbering)
    {
        sNumString = GetNumString();
        bHasNumbering = !sNumString.isEmpty();
        if (!bHasNumbering)
            bHasBullet = HasBullet();
        bCountNumbering = bHasNumbering || bHasBullet;
    }

    if( nStt == nEnd && !bCountNumbering)
    {   // unnumbered empty node or empty selection
        return;
    }

    // count of non-empty paras
    ++rStat.nPara;

    // Shortcut when counting whole paragraph and current count is clean
    if ( bCountAll && !IsWordCountDirty() )
    {
        // accumulate into DocStat record to return the values
        rStat.nWord += GetParaNumberOfWords();
        rStat.nAsianWord += GetParaNumberOfAsianWords();
        rStat.nChar += GetParaNumberOfChars();
        rStat.nCharExcludingSpaces += GetParaNumberOfCharsExcludingSpaces();
        return;
    }

    // ConversionMap to expand fields, remove invisible and redline deleted text for scanner
    const ModelToViewHelper aConversionMap(*this, EXPANDFIELDS | HIDEINVISIBLE | HIDEREDLINED);
    rtl::OUString aExpandText = aConversionMap.getViewText();

    // map start and end points onto the ConversionMap
    const sal_uInt32 nExpandBegin = aConversionMap.ConvertToViewPosition( nStt );
    const sal_uInt32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nEnd );

    if (aExpandText.isEmpty() && !bCountNumbering)
    {
        OSL_ENSURE(aExpandText.getLength() >= 0, "Node text expansion error: length < 0." );
        return;
    }

    //do the count
    // all counts exclude hidden paras and hidden+redlined within para
    // definition of space/white chars in SwScanner (and BreakIter!)
    // uses both u_isspace and BreakIter getWordBoundary in SwScanner
    sal_uInt32 nTmpWords = 0;        // count of all words
    sal_uInt32 nTmpAsianWords = 0;   //count of all Asian codepoints
    sal_uInt32 nTmpChars = 0;        // count of all chars
    sal_uInt32 nTmpCharsExcludingSpaces = 0;  // all non-white chars

    // count words in masked and expanded text:
    if (!aExpandText.isEmpty() && pBreakIt->GetBreakIter().is())
    {
        // zero is NULL for pLanguage -----------v               last param = true for clipping
        SwScanner aScanner( *this, aExpandText, 0, aConversionMap, i18n::WordType::WORD_COUNT,
                            nExpandBegin, nExpandEnd, true );

        // used to filter out scanner returning almost empty strings (len=1; unichar=0x0001)
        const rtl::OUString aBreakWord( CH_TXTATR_BREAKWORD );

        while ( aScanner.NextWord() )
        {
            //  1 is len(CH_TXTATR_BREAKWORD) : match returns length of match
            if( 1 != aExpandText.match(aBreakWord, aScanner.GetBegin() ))
            {
                ++nTmpWords;
                const rtl::OUString &rWord = aScanner.GetWord();
                if (pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                    ++nTmpAsianWords;
                nTmpCharsExcludingSpaces += pBreakIt->getGraphemeCount(rWord);
            }
        }
    }

    nTmpChars = pBreakIt->getGraphemeCount(aExpandText, nExpandBegin, nExpandEnd);

    // no nTmpCharsExcludingSpaces adjust needed neither for blanked out MaskedChars
    // nor for mid-word selection - set scanner bClip = true at creation

    // count outline number label - ? no expansion into map
    // always counts all of number-ish label
    if (bHasNumbering) // count words in numbering string
    {
        LanguageType aLanguage = GetLang( 0 );

        SwScanner aScanner( *this, sNumString, &aLanguage, ModelToViewHelper(),
                            i18n::WordType::WORD_COUNT, 0, sNumString.getLength(), true );

        while ( aScanner.NextWord() )
        {
            ++nTmpWords;
            const rtl::OUString &rWord = aScanner.GetWord();
            if (pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                ++nTmpAsianWords;
            nTmpCharsExcludingSpaces += pBreakIt->getGraphemeCount(rWord);
        }

        nTmpChars += pBreakIt->getGraphemeCount(sNumString);
    }
    else if ( bHasBullet )
    {
        ++nTmpWords;
        ++nTmpChars;
        ++nTmpCharsExcludingSpaces;
    }

    // If counting the whole para then update cached values and mark clean
    if ( bCountAll )
    {
        SetParaNumberOfWords( nTmpWords );
        SetParaNumberOfAsianWords( nTmpAsianWords );
        SetParaNumberOfChars( nTmpChars );
        SetParaNumberOfCharsExcludingSpaces( nTmpCharsExcludingSpaces );
        SetWordCountDirty( false );
    }
    // accumulate into DocStat record to return the values
    rStat.nWord += nTmpWords;
    rStat.nAsianWord += nTmpAsianWords;
    rStat.nChar += nTmpChars;
    rStat.nCharExcludingSpaces += nTmpCharsExcludingSpaces;
}

//
// Paragraph statistics start
//
struct SwParaIdleData_Impl
{
    SwWrongList* pWrong;                // for spell checking
    SwGrammarMarkUp* pGrammarCheck;     // for grammar checking /  proof reading
    SwWrongList* pSmartTags;
    sal_uLong nNumberOfWords;
    sal_uLong nNumberOfAsianWords;
    sal_uLong nNumberOfChars;
    sal_uLong nNumberOfCharsExcludingSpaces;
    bool bWordCountDirty;
    bool bWrongDirty;                   // Ist das Wrong-Feld auf invalid?
    bool bGrammarCheckDirty;
    bool bSmartTagDirty;
    bool bAutoComplDirty;               // die ACompl-Liste muss angepasst werden

    SwParaIdleData_Impl() :
        pWrong              ( 0 ),
        pGrammarCheck       ( 0 ),
        pSmartTags          ( 0 ),
        nNumberOfWords      ( 0 ),
        nNumberOfAsianWords ( 0 ),
        nNumberOfChars      ( 0 ),
        nNumberOfCharsExcludingSpaces ( 0 ),
        bWordCountDirty     ( true ),
        bWrongDirty         ( true ),
        bGrammarCheckDirty  ( true ),
        bSmartTagDirty      ( true ),
        bAutoComplDirty     ( true ) {};
};

void SwTxtNode::InitSwParaStatistics( bool bNew )
{
    if ( bNew )
    {
        m_pParaIdleData_Impl = new SwParaIdleData_Impl;
    }
    else if ( m_pParaIdleData_Impl )
    {
        delete m_pParaIdleData_Impl->pWrong;
        delete m_pParaIdleData_Impl->pGrammarCheck;
        delete m_pParaIdleData_Impl->pSmartTags;
        delete m_pParaIdleData_Impl;
        m_pParaIdleData_Impl = 0;
    }
}

void SwTxtNode::SetWrong( SwWrongList* pNew, bool bDelete )
{
    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pWrong;
        }
        m_pParaIdleData_Impl->pWrong = pNew;
    }
}

SwWrongList* SwTxtNode::GetWrong()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pWrong : 0;
}

// #i71360#
const SwWrongList* SwTxtNode::GetWrong() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pWrong : 0;
}

void SwTxtNode::SetGrammarCheck( SwGrammarMarkUp* pNew, bool bDelete )
{
    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pGrammarCheck;
        }
        m_pParaIdleData_Impl->pGrammarCheck = pNew;
    }
}

SwGrammarMarkUp* SwTxtNode::GetGrammarCheck()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pGrammarCheck : 0;
}

void SwTxtNode::SetSmartTags( SwWrongList* pNew, bool bDelete )
{
    OSL_ENSURE( !pNew || SwSmartTagMgr::Get().IsSmartTagsEnabled(),
            "Weird - we have a smart tag list without any recognizers?" );

    if ( m_pParaIdleData_Impl )
    {
        if ( bDelete )
        {
            delete m_pParaIdleData_Impl->pSmartTags;
        }
        m_pParaIdleData_Impl->pSmartTags = pNew;
    }
}

SwWrongList* SwTxtNode::GetSmartTags()
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->pSmartTags : 0;
}

void SwTxtNode::SetParaNumberOfWords( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfWords = nNew;
    }
}

sal_uLong SwTxtNode::GetParaNumberOfWords() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfWords : 0;
}

void SwTxtNode::SetParaNumberOfAsianWords( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfAsianWords = nNew;
    }
}

sal_uLong SwTxtNode::GetParaNumberOfAsianWords() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfAsianWords : 0;
}

void SwTxtNode::SetParaNumberOfChars( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfChars = nNew;
    }
}

sal_uLong SwTxtNode::GetParaNumberOfChars() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfChars : 0;
}

void SwTxtNode::SetWordCountDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bWordCountDirty = bNew;
    }
}

sal_uLong SwTxtNode::GetParaNumberOfCharsExcludingSpaces() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->nNumberOfCharsExcludingSpaces : 0;
}

void SwTxtNode::SetParaNumberOfCharsExcludingSpaces( sal_uLong nNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->nNumberOfCharsExcludingSpaces = nNew;
    }
}

bool SwTxtNode::IsWordCountDirty() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->bWordCountDirty : 0;
}
void SwTxtNode::SetWrongDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bWrongDirty = bNew;
    }
}
bool SwTxtNode::IsWrongDirty() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->bWrongDirty : 0;
}
void SwTxtNode::SetGrammarCheckDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bGrammarCheckDirty = bNew;
    }
}
bool SwTxtNode::IsGrammarCheckDirty() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->bGrammarCheckDirty : 0;
}
void SwTxtNode::SetSmartTagDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bSmartTagDirty = bNew;
    }
}
bool SwTxtNode::IsSmartTagDirty() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->bSmartTagDirty : 0;
}
void SwTxtNode::SetAutoCompleteWordDirty( bool bNew ) const
{
    if ( m_pParaIdleData_Impl )
    {
        m_pParaIdleData_Impl->bAutoComplDirty = bNew;
    }
}
bool SwTxtNode::IsAutoCompleteWordDirty() const
{
    return m_pParaIdleData_Impl ? m_pParaIdleData_Impl->bAutoComplDirty : 0;
}
//
// Paragraph statistics end
//

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
