/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include <officecfg/Office/Writer.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/charclass.hxx>
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <splargs.hxx>
#include <viewopt.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>
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
#include <redline.hxx>
#include <docary.hxx>
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
#include <utility>

#include <unotextrange.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;



extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;

/*
 * This has basically the same function as SwScriptInfo::MaskHiddenRanges,
 * only for deleted redlines
 */

static sal_uInt16
lcl_MaskRedlines( const SwTxtNode& rNode, OUStringBuffer& rText,
                         sal_Int32 nStt, sal_Int32 nEnd,
                         const sal_Unicode cChar )
{
    sal_uInt16 nNumOfMaskedRedlines = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    sal_uInt16 nAct = rDoc.GetRedlinePos( rNode, USHRT_MAX );

    for ( ; nAct < rDoc.GetRedlineTbl().size(); nAct++ )
    {
        const SwRangeRedline* pRed = rDoc.GetRedlineTbl()[ nAct ];

        if ( pRed->Start()->nNode > rNode.GetIndex() )
            break;

        if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
        {
            sal_Int32 nRedlineEnd;
            sal_Int32 nRedlineStart;

            pRed->CalcStartEnd( rNode.GetIndex(), nRedlineStart, nRedlineEnd );

            if ( nRedlineEnd < nStt || nRedlineStart > nEnd )
                continue;

            while ( nRedlineStart < nRedlineEnd && nRedlineStart < nEnd )
            {
                if ( nRedlineStart >= nStt && nRedlineStart < nEnd )
                {
                    rText[nRedlineStart] = cChar;
                    ++nNumOfMaskedRedlines;
                }
                ++nRedlineStart;
            }
        }
    }

    return nNumOfMaskedRedlines;
}

/**
 * Used for spell checking. Deleted redlines and hidden characters are masked
 */
static sal_uInt16
lcl_MaskRedlinesAndHiddenText( const SwTxtNode& rNode, OUStringBuffer& rText,
                                      sal_Int32 nStt, sal_Int32 nEnd,
                                      const sal_Unicode cChar = CH_TXTATR_INWORD,
                                      bool bCheckShowHiddenChar = true )
{
    sal_uInt16 nRedlinesMasked = 0;
    sal_uInt16 nHiddenCharsMasked = 0;

    const SwDoc& rDoc = *rNode.GetDoc();
    const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( rDoc.GetRedlineMode() );

    
    
    if ( bShowChg )
    {
        nRedlinesMasked = lcl_MaskRedlines( rNode, rText, nStt, nEnd, cChar );
    }

    const bool bHideHidden = !SW_MOD()->GetViewOption(rDoc.get(IDocumentSettingAccess::HTML_MODE))->IsShowHiddenChar();

    
    
    if ( !bCheckShowHiddenChar || bHideHidden )
    {
        nHiddenCharsMasked =
            SwScriptInfo::MaskHiddenRanges( rNode, rText, nStt, nEnd, cChar );
    }

    return nRedlinesMasked + nHiddenCharsMasked;
}

/**
 * Used for spell checking. Calculates a rectangle for repaint.
 */
static SwRect lcl_CalculateRepaintRect( SwTxtFrm& rTxtFrm, sal_Int32 nChgStart, sal_Int32 nChgEnd )
{
    SwRect aRect;

    SwTxtNode *pNode = rTxtFrm.GetTxtNode();

    SwNodeIndex aNdIdx( *pNode );
    SwPosition aPos( aNdIdx, SwIndex( pNode, nChgEnd ) );
    SwCrsrMoveState aTmpState( MV_NONE );
    aTmpState.b2Lines = sal_True;
    rTxtFrm.GetCharRect( aRect, aPos, &aTmpState );
    
    Sw2LinesPos* pEnd2Pos = aTmpState.p2Lines;

    const SwTxtFrm *pEndFrm = &rTxtFrm;

    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    if ( pEnd2Pos )
    {
        
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

    
    
    
    const SwTxtFrm* pStartFrm = &rTxtFrm;
    while( pStartFrm->HasFollow() &&
           nChgStart >= pStartFrm->GetFollow()->GetOfst() )
        pStartFrm = pStartFrm->GetFollow();
    pEndFrm = pStartFrm;
    while( pEndFrm->HasFollow() &&
           nChgEnd >= pEndFrm->GetFollow()->GetOfst() )
        pEndFrm = pEndFrm->GetFollow();

    
    Sw2LinesPos* pSt2Pos = aTmpState.p2Lines;
    if ( pSt2Pos )
    {
        
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

    bool bSameFrame = true;

    if( rTxtFrm.HasFollow() )
    {
        if( pEndFrm != pStartFrm )
        {
            bSameFrame = false;
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
            while( true )
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

/**
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
        if ( SFX_ITEM_SET == rSet2.GetItemState( nWhichId, false ) )
        {
            pNewSet = rSet2.Clone( sal_True );
            pNewSet->ClearItem( nWhichId );
        }
    }
    else if ( pSet1->Count() )
    {
        SfxItemIter aIter( *pSet1 );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            if ( SFX_ITEM_SET == rSet2.GetItemState( pItem->Which(), false ) )
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

/** Delete all attributes
 *
 * 5 cases:
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
 *
 * @param rIdx starting position
 * @param nLen length of the deletion
 * @param nthat ???
 * @param pSet ???
 * @param bInclRefToxMark ???
 */

void SwTxtNode::RstTxtAttr(
    const SwIndex &rIdx,
    const sal_Int32 nLen,
    const sal_uInt16 nWhich,
    const SfxItemSet* pSet,
    const bool bInclRefToxMark )
{
    if ( !GetpSwpHints() )
        return;

    sal_Int32 nStt = rIdx.GetIndex();
    sal_Int32 nEnd = nStt + nLen;
    {
        
        const SwTxtInputFld* pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtAttrAt( nStt, RES_TXTATR_INPUTFIELD, PARENT ));
        if ( pTxtInputFld == NULL )
        {
            pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtAttrAt(nEnd, RES_TXTATR_INPUTFIELD, PARENT ));
        }
        if ( pTxtInputFld != NULL )
        {
            if ( nStt > *(pTxtInputFld->GetStart()) )
            {
                nStt = *(pTxtInputFld->GetStart());
            }
            if ( nEnd < *(pTxtInputFld->End()) )
            {
                nEnd = *(pTxtInputFld->End());
            }
        }
    }

    bool bChanged = false;

    
    sal_Int32 nMin = m_Text.getLength();
    sal_Int32 nMax = nStt;
    const bool bNoLen = nMin == 0;

    
    
    
    

    
    sal_uInt16 i = 0;
    sal_Int32 nAttrStart;
    SwTxtAttr *pHt = NULL;
    while ( (i < m_pSwpHints->Count())
            && ( ( ( nAttrStart = *(*m_pSwpHints)[i]->GetStart()) < nEnd )
                 || nLen==0 ) )
    {
        pHt = m_pSwpHints->GetTextHint(i);

        
        
        sal_Int32* const pAttrEnd = pHt->GetEnd();
        const bool bKeepAttrWithoutEnd =
            pAttrEnd == NULL
            && ( !bInclRefToxMark
                 || ( RES_TXTATR_REFMARK != pHt->Which()
                      && RES_TXTATR_TOXMARK != pHt->Which()
                      && RES_TXTATR_META != pHt->Which()
                      && RES_TXTATR_METAFIELD != pHt->Which() ) );
        if ( bKeepAttrWithoutEnd )
        {

            i++;
            continue;
        }
        
        if ( pHt->HasContent() )
        {
            ++i;
            continue;
        }

        
        bool bSkipAttr = false;
        boost::shared_ptr<SfxItemSet> pStyleHandle;

        
        if ( pSet )
        {
            bSkipAttr = SFX_ITEM_SET != pSet->GetItemState( pHt->Which(), false );
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                
                
                
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), pSet, 0, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( nWhich )
        {
            
            bSkipAttr = nWhich != pHt->Which();
            if ( bSkipAttr && RES_TXTATR_AUTOFMT == pHt->Which() )
            {
                bSkipAttr = !lcl_HaveCommonAttributes( getIDocumentStyleAccess(), 0, nWhich, *static_cast<const SwFmtAutoFmt&>(pHt->GetAttr()).GetStyleHandle(), pStyleHandle );
            }
        }
        else if ( !bInclRefToxMark )
        {
            
            
            
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

        if (nStt <= nAttrStart)     
        {
            const sal_Int32 nAttrEnd = pAttrEnd != NULL
                                        ? *pAttrEnd
                                        : nAttrStart;
            if (nEnd > nAttrStart
                || (nEnd == nAttrEnd && nEnd == nAttrStart)) 
            {
                if ( nMin > nAttrStart )
                    nMin = nAttrStart;
                if ( nMax < nAttrEnd )
                    nMax = nAttrEnd;
                
                bChanged = bChanged || nEnd > nAttrStart || bNoLen;
                if (nAttrEnd <= nEnd)   
                {
                    m_pSwpHints->DeleteAtPos(i);
                    DestroyAttr( pHt );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nAttrEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    
                    if ( !m_pSwpHints )
                        break;

                    
                    
                    
                    
                    i = 0;
                    continue;
                }
                else    
                {
                    m_pSwpHints->NoteInHistory( pHt );
                    
                    *pHt->GetStart() = nEnd;
                    m_pSwpHints->NoteInHistory( pHt, true );

                    if ( pStyleHandle.get() && nAttrStart < nEnd )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                                *pStyleHandle, nAttrStart, nEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    
                    
                    assert(pHt == m_pSwpHints->GetTextHint(i));

                    bChanged = true;
                }
            }
        }
        else if (pAttrEnd != 0)         
        {
            if (*pAttrEnd > nStt)       
            {
                if (*pAttrEnd < nEnd)   
                {
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = true;

                    const sal_Int32 nAttrEnd = *pAttrEnd;

                    m_pSwpHints->NoteInHistory( pHt );
                    
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, true );

                    if ( pStyleHandle.get() )
                    {
                        SwTxtAttr* pNew = MakeTxtAttr( *GetDoc(),
                            *pStyleHandle, nStt, nAttrEnd );
                        InsertHint( pNew, nsSetAttrMode::SETATTR_NOHINTADJUST );
                    }

                    
                    
                    assert(pHt == m_pSwpHints->GetTextHint(i));
                }
                else if (nLen)  
                {
                    
                    
                    if ( nMin > nAttrStart )
                        nMin = nAttrStart;
                    if ( nMax < *pAttrEnd )
                        nMax = *pAttrEnd;
                    bChanged = true;
                    const sal_Int32 nTmpEnd = *pAttrEnd;
                    m_pSwpHints->NoteInHistory( pHt );
                    
                    *pAttrEnd = nStt;
                    m_pSwpHints->NoteInHistory( pHt, true );

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
                    }

                    
                    
                    assert(pHt == m_pSwpHints->GetTextHint(i));
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
            m_pSwpHints->MergePortions(*this);
        }
        
        SwUpdateAttr aHint( nMin, nMax, 0 );
        NotifyClients( 0, &aHint );
        SwFmtChg aNew( GetFmtColl() );
        NotifyClients( 0, &aNew );
    }
}

sal_Int32 clipIndexBounds(const OUString &rStr, sal_Int32 nPos)
{
    if (nPos < 0)
        return 0;
    if (nPos > rStr.getLength())
        return rStr.getLength();
    return nPos;
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
OUString SwTxtNode::GetCurWord( sal_Int32 nPos ) const
{
    assert(nPos <= m_Text.getLength()); 

    if (m_Text.isEmpty())
        return m_Text;

    Boundary aBndry;
    const uno::Reference< XBreakIterator > &rxBreak = g_pBreakIt->GetBreakIter();
    if (rxBreak.is())
    {
        sal_Int16 nWordType = WordType::DICTIONARY_WORD;
        lang::Locale aLocale( g_pBreakIt->GetLocale( GetLang( nPos ) ) );
#if OSL_DEBUG_LEVEL > 1
        sal_Bool bBegin = rxBreak->isBeginWord( m_Text, nPos, aLocale, nWordType );
        sal_Bool bEnd   = rxBreak->isEndWord  ( m_Text, nPos, aLocale, nWordType );
        (void)bBegin;
        (void)bEnd;
#endif
        aBndry =
            rxBreak->getWordBoundary( m_Text, nPos, aLocale, nWordType, sal_True );

        
        if (aBndry.startPos == aBndry.endPos)
        {
            aBndry = rxBreak->previousWord( m_Text, nPos, aLocale, nWordType );
        }
    }

    
    
    if (aBndry.endPos != aBndry.startPos && IsSymbol( aBndry.startPos ))
        aBndry.endPos = aBndry.startPos;

    
    aBndry.startPos = clipIndexBounds(m_Text, aBndry.startPos);
    aBndry.endPos = clipIndexBounds(m_Text, aBndry.endPos);

    return m_Text.copy(aBndry.startPos,
                       aBndry.endPos - aBndry.startPos);
}

SwScanner::SwScanner( const SwTxtNode& rNd, const OUString& rTxt,
    const LanguageType* pLang, const ModelToViewHelper& rConvMap,
    sal_uInt16 nType, sal_Int32 nStart, sal_Int32 nEnde, sal_Bool bClp )
    : rNode( rNd )
    , aPreDashReplacementText(rTxt)
    , pLanguage( pLang )
    , rConversionMap( rConvMap )
    , nLen( 0 )
    , nOverriddenDashCount( 0 )
    , nWordType( nType )
    , bClip( bClp )
{
    OSL_ENSURE( !aPreDashReplacementText.isEmpty(), "SwScanner: EmptyString" );
    nStartPos = nBegin = nStart;
    nEndPos = nEnde;

    
    
    //
    
    
    if (nWordType == i18n::WordType::WORD_COUNT)
    {
        OUString sDashes = officecfg::Office::Writer::WordCount::AdditionalSeparators::get();
        OUStringBuffer aBuf(aPreDashReplacementText);
        for (sal_Int32 i = nStartPos; i < nEndPos; ++i)
        {
            sal_Unicode cChar = aBuf[i];
            if (sDashes.indexOf(cChar) != -1)
            {
                aBuf[i] = ' ';
                ++nOverriddenDashCount;
            }
        }
        aText = aBuf.makeStringAndClear();
    }
    else
        aText = aPreDashReplacementText;

    assert(aPreDashReplacementText.getLength() == aText.getLength());

    if ( pLanguage )
    {
        aCurrLang = *pLanguage;
    }
    else
    {
        ModelToViewHelper::ModelPosition aModelBeginPos = rConversionMap.ConvertToModelPosition( nBegin );
        aCurrLang = rNd.GetLang( aModelBeginPos.mnPos );
    }
}

namespace
{
    
    sal_Int32 forceEachAsianCodePointToWord(const OUString &rText, sal_Int32 nBegin, sal_Int32 nLen)
    {
        if (nLen > 1)
        {
            const uno::Reference< XBreakIterator > &rxBreak = g_pBreakIt->GetBreakIter();

            sal_uInt16 nCurrScript = rxBreak->getScriptType( rText, nBegin );

            sal_Int32 indexUtf16 = nBegin;
            rText.iterateCodePoints(&indexUtf16, 1);

            
            if (nCurrScript == i18n::ScriptType::ASIAN)
            {
                nLen = indexUtf16 - nBegin;
                return nLen;
            }

            
            
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
    LanguageTag aOldLanguageTag = rCC.getLanguageTag();

    while ( true )
    {
        
        while ( nBegin < aText.getLength() )
        {
            if ( !u_isspace( aText[nBegin] ) )
            {
                if ( !pLanguage )
                {
                    const sal_uInt16 nNextScriptType = g_pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
                    ModelToViewHelper::ModelPosition aModelBeginPos = rConversionMap.ConvertToModelPosition( nBegin );
                    aCurrLang = rNode.GetLang( aModelBeginPos.mnPos, 1, nNextScriptType );
                }

                if ( nWordType != i18n::WordType::WORD_COUNT )
                {
                    rCC.setLanguageTag( LanguageTag( g_pBreakIt->GetLocale( aCurrLang )) );
                    if ( rCC.isLetterNumeric(OUString(aText[nBegin])) )
                        break;
                }
                else
                    break;
            }
            ++nBegin;
        }

        if ( nBegin >= aText.getLength() || nBegin >= nEndPos )
            return sal_False;

        
        aBound = g_pBreakIt->GetBreakIter()->getWordBoundary( aText, nBegin,
                g_pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );
        OSL_ENSURE( aBound.endPos >= aBound.startPos, "broken aBound result" );

        
        
        
        
        if (nWordType == i18n::WordType::WORD_COUNT && aBound.startPos < nBegin)
            aBound.startPos = nBegin;

        
        if(aBound.endPos == aBound.startPos)
            return sal_False;

        
        if(aBound.endPos == nBegin)
            ++nBegin;
        else
            break;
    } 

    rCC.setLanguageTag( aOldLanguageTag );

    
    if ( nWordType == i18n::WordType::WORD_COUNT )
    {
        nBegin = std::max(aBound.startPos, nBegin);
        nLen   = 0;
        if (aBound.endPos > nBegin)
            nLen = aBound.endPos - nBegin;
    }
    else
    {
        
        if ( aBound.startPos <= nBegin )
        {
            OSL_ENSURE( aBound.endPos >= nBegin, "Unexpected aBound result" );

            
            const sal_uInt16 nCurrScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, nBegin );
            OUString aTmpWord = aText.copy( nBegin, aBound.endPos - nBegin );
            const sal_Int32 nScriptEnd = nBegin +
                g_pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = std::min( aBound.endPos, nScriptEnd );

            
            sal_Int32 nScriptBegin = 0;
            if ( aBound.startPos < nBegin )
            {
                
                aTmpWord = aText.copy( aBound.startPos,
                                       nBegin - aBound.startPos + 1 );
                nScriptBegin = aBound.startPos +
                    g_pBreakIt->GetBreakIter()->beginOfScript( aTmpWord, nBegin - aBound.startPos,
                                                    nCurrScript );
            }

            nBegin = std::max( aBound.startPos, nScriptBegin );
            nLen = nEnd - nBegin;
        }
        else
        {
            const sal_uInt16 nCurrScript = g_pBreakIt->GetBreakIter()->getScriptType( aText, aBound.startPos );
            OUString aTmpWord = aText.copy( aBound.startPos,
                                             aBound.endPos - aBound.startPos );
            const sal_Int32 nScriptEnd = aBound.startPos +
                g_pBreakIt->GetBreakIter()->endOfScript( aTmpWord, 0, nCurrScript );
            const sal_Int32 nEnd = std::min( aBound.endPos, nScriptEnd );
            nBegin = aBound.startPos;
            nLen = nEnd - nBegin;
        }
    }

    
    if ( bClip )
    {
        aBound.startPos = std::max( aBound.startPos, nStartPos );
        aBound.endPos = std::min( aBound.endPos, nEndPos );
        nBegin = aBound.startPos;
        nLen = aBound.endPos - nBegin;
    }

    if( ! nLen )
        return sal_False;

    if ( nWordType == i18n::WordType::WORD_COUNT )
        nLen = forceEachAsianCodePointToWord(aText, nBegin, nLen);

    aWord = aPreDashReplacementText.copy( nBegin, nLen );

    return sal_True;
}

sal_uInt16 SwTxtNode::Spell(SwSpellArgs* pArgs)
{
    
    

    
    const OUString aOldTxt( m_Text );
    OUStringBuffer buf(m_Text);
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText(*this, buf, 0, m_Text.getLength()) > 0;
    if (bRestoreString)
    {   
        m_Text = buf.makeStringAndClear();
    }

    sal_Int32 nBegin = ( pArgs->pStartNode != this )
        ? 0
        : pArgs->pStartIdx->GetIndex();

    sal_Int32 nEnd = ( pArgs->pEndNode != this )
            ? m_Text.getLength()
            : pArgs->pEndIdx->GetIndex();

    pArgs->xSpellAlt = NULL;

    
    //
    
    
    
    
    
    
    
    
    //
    
    if ( ( IsWrongDirty() || GetWrong() ) && m_Text.getLength() )
    {
        if (nBegin > m_Text.getLength())
        {
            nBegin = m_Text.getLength();
        }
        if (nEnd > m_Text.getLength())
        {
            nEnd = m_Text.getLength();
        }
        //
        if(!IsWrongDirty())
        {
            const sal_Int32 nTemp = GetWrong()->NextWrong( nBegin );
            if(nTemp > nEnd)
            {
                
                if ( bRestoreString )
                {
                    m_Text = aOldTxt;
                }
                return 0;
            }
            if(nTemp > nBegin)
                nBegin = nTemp;

        }

        
        
        SwScanner aScanner( *this, m_Text, 0, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD,
                            nBegin, nEnd );
        while( !pArgs->xSpellAlt.is() && aScanner.NextWord() )
        {
            const OUString& rWord = aScanner.GetWord();

            
            
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            if( rWord.getLength() > 0 && LANGUAGE_NONE != eActLang )
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
                        
                        
                        
                        
                        
                        const sal_Unicode* pChar = rWord.getStr();
                        sal_Int32 nLeft = 0;
                        while (pChar && *pChar++ == CH_TXTATR_INWORD)
                            ++nLeft;
                        pChar = rWord.getLength() ? rWord.getStr() + rWord.getLength() - 1 : 0;
                        sal_Int32 nRight = 0;
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
        aRanges[2] = aRanges[3] = 0;    

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
    
    
    
    

}

sal_uInt16 SwTxtNode::Convert( SwConversionArgs &rArgs )
{
    
    
    
    const sal_Int32 nTextBegin = ( rArgs.pStartNode == this )
        ? ::std::min(rArgs.pStartIdx->GetIndex(), m_Text.getLength())
        : 0;

    const sal_Int32 nTextEnd = ( rArgs.pEndNode == this )
        ?  ::std::min(rArgs.pEndIdx->GetIndex(), m_Text.getLength())
        :  m_Text.getLength();

    rArgs.aConvText = OUString();

    
    const OUString aOldTxt( m_Text );
    OUStringBuffer buf(m_Text);
    const bool bRestoreString =
        lcl_MaskRedlinesAndHiddenText(*this, buf, 0, m_Text.getLength()) > 0;
    if (bRestoreString)
    {   
        m_Text = buf.makeStringAndClear();
    }

    bool    bFound  = false;
    sal_Int32  nBegin  = nTextBegin;
    sal_Int32  nLen = 0;
    LanguageType nLangFound = LANGUAGE_NONE;
    if (m_Text.isEmpty())
    {
        if (rArgs.bAllowImplicitChangesForNotConvertibleText)
        {
            
            
            SwPaM aCurPaM( *this, 0 );

            SetLanguageAndFont( aCurPaM,
                    rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE,
                    rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }
    }
    else
    {
        SwLanguageIterator aIter( *this, nBegin );

        
        
        
        typedef std::pair<sal_Int32, sal_Int32> ImplicitChangesRange;
        std::vector<ImplicitChangesRange> aImplicitChanges;

        
        do {
            nLangFound = aIter.GetLanguage();
            bool bLangOk =  (nLangFound == rArgs.nConvSrcLang) ||
                                (editeng::HangulHanjaConversion::IsChinese( nLangFound ) &&
                                 editeng::HangulHanjaConversion::IsChinese( rArgs.nConvSrcLang ));

            sal_Int32 nChPos = aIter.GetChgPos();
            
            
            assert(nChPos != -1);
            if (nChPos == -1 || nChPos == COMPLETE_STRING)
            {
                nChPos = m_Text.getLength();
            }

            nLen = nChPos - nBegin;
            bFound = bLangOk && nLen > 0;
            if (!bFound)
            {
                
                
                SwPaM aCurPaM( *this, nBegin );
                aCurPaM.SetMark();
                aCurPaM.GetPoint()->nContent = nBegin + nLen;

                
                SwEditShell *pEditShell = GetDoc()->GetEditShell();
                pEditShell->Push();             
                pEditShell->SetSelection( aCurPaM );
                bool bIsAsianScript = (SCRIPTTYPE_ASIAN == pEditShell->GetScriptType());
                pEditShell->Pop( sal_False );   

                if (!bIsAsianScript && rArgs.bAllowImplicitChangesForNotConvertibleText)
                {
                    
                    aImplicitChanges.push_back(ImplicitChangesRange(nBegin, nBegin+nLen));
                }
                nBegin = nChPos;    
            }
        } while (!bFound && aIter.Next());  /* loop while nothing was found and still sth is left to be searched */

        
        for (size_t i = 0; i < aImplicitChanges.size(); ++i)
        {
            SwPaM aPaM( *this, aImplicitChanges[i].first );
            aPaM.SetMark();
            aPaM.GetPoint()->nContent = aImplicitChanges[i].second;
            SetLanguageAndFont( aPaM, rArgs.nConvTargetLang, RES_CHRATR_CJK_LANGUAGE, rArgs.pTargetFont, RES_CHRATR_CJK_FONT );
        }

    }

    
    if (nBegin < nTextBegin)
        nBegin = nTextBegin;
    if (nBegin + nLen > nTextEnd)
        nLen = nTextEnd - nBegin;
    bool bInSelection = nBegin < nTextEnd;

    if (bFound && bInSelection)     
    {
        OSL_ENSURE( !m_Text.isEmpty(), "convertible text portion missing!" );
        rArgs.aConvText     = m_Text.copy(nBegin, nLen);
        rArgs.nConvTextLang = nLangFound;

        
        rArgs.pStartNode = this;
        rArgs.pStartIdx->Assign(this, nBegin + nLen );
        
        rArgs.pEndNode = this;
        rArgs.pEndIdx->Assign(this, nBegin );
    }

    
    if ( bRestoreString )
    {
        m_Text = aOldTxt;
    }

    return rArgs.aConvText.isEmpty() ? 0 : 1;
}



SwRect SwTxtFrm::_AutoSpell( const SwCntntNode* pActNode, const SwViewOption& rViewOpt, sal_Int32 nActPos )
{
    SwRect aRect;
#if OSL_DEBUG_LEVEL > 1
    static bool bStop = false;
    if ( bStop )
        return aRect;
#endif
    
    
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = COMPLETE_STRING;

    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    
    const OUString aOldTxt( pNode->GetTxt() );
    OUStringBuffer buf(pNode->m_Text);
    const bool bRestoreString = lcl_MaskRedlinesAndHiddenText( *pNode, buf,
                0, pNode->GetTxt().getLength()) > 0;
    if (bRestoreString)
    {   
        pNode->m_Text = buf.makeStringAndClear();
    }

    
    const bool bRedlineChg = (pNode->GetTxt().getStr() != aOldTxt.getStr());

    sal_Int32 nBegin = 0;
    sal_Int32 nEnd = pNode->GetTxt().getLength();
    sal_Int32 nInsertPos = 0;
    sal_Int32 nChgStart = COMPLETE_STRING;
    sal_Int32 nChgEnd = 0;
    sal_Int32 nInvStart = COMPLETE_STRING;
    sal_Int32 nInvEnd = 0;

    const bool bAddAutoCmpl = pNode->IsAutoCompleteWordDirty() &&
                                  rViewOpt.IsAutoCompleteWords();

    if( pNode->GetWrong() )
    {
        nBegin = pNode->GetWrong()->GetBeginInv();
        if( COMPLETE_STRING != nBegin )
        {
            nEnd = std::max(pNode->GetWrong()->GetEndInv(), pNode->GetTxt().getLength());
        }

        
        if ( COMPLETE_STRING != nBegin )
        {
            if ( nBegin )
                --nBegin;

            LanguageType eActLang = pNode->GetLang( nBegin );
            Boundary aBound =
                g_pBreakIt->GetBreakIter()->getWordBoundary( pNode->GetTxt(), nBegin,
                    g_pBreakIt->GetLocale( eActLang ),
                    WordType::DICTIONARY_WORD, sal_True );
            nBegin = aBound.startPos;
        }

        
        nInsertPos = pNode->GetWrong()->GetWrongPos( nBegin );

        
        if( nInsertPos < pNode->GetWrong()->Count() &&
            nBegin == pNode->GetWrong()->Pos( nInsertPos ) +
                      pNode->GetWrong()->Len( nInsertPos ) )
                nInsertPos++;
    }

    bool bFresh = nBegin < nEnd;

    if( nBegin < nEnd )
    {
        
        
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        uno::Reference< XSpellChecker1 > xSpell( ::GetSpellChecker() );
        SwDoc* pDoc = pNode->GetDoc();

        SwScanner aScanner( *pNode, pNode->GetTxt(), 0, ModelToViewHelper(),
                            WordType::DICTIONARY_WORD, nBegin, nEnd);

        while( aScanner.NextWord() )
        {
            const OUString& rWord = aScanner.GetWord();
            nBegin = aScanner.GetBegin();
            sal_Int32 nLen = aScanner.GetLen();

            
            
            LanguageType eActLang = aScanner.GetCurrentLanguage();

            sal_Bool bSpell = xSpell.is() ? xSpell->hasLanguage( eActLang ) : sal_False;
            if( bSpell && !rWord.isEmpty() )
            {
                
                OSL_ENSURE(!bSpell || xSpell.is(), "NULL pointer");

                if( !xSpell->isValid( rWord, eActLang, Sequence< PropertyValue >() ) )
                {
                    sal_Int32 nSmartTagStt = nBegin;
                    sal_Int32 nDummy = 1;
                    if ( !pNode->GetSmartTags() || !pNode->GetSmartTags()->InWrongWord( nSmartTagStt, nDummy ) )
                    {
                        if( !pNode->GetWrong() )
                        {
                            pNode->SetWrong( new SwWrongList( WRONGLIST_SPELL ) );
                            pNode->GetWrong()->SetInvalid( 0, nEnd );
                        }
                        if( pNode->GetWrong()->Fresh( nChgStart, nChgEnd,
                            nBegin, nLen, nInsertPos, nActPos ) )
                            pNode->GetWrong()->Insert( OUString(), 0, nBegin, nLen, nInsertPos++ );
                        else
                        {
                            nInvStart = nBegin;
                            nInvEnd = nBegin + nLen;
                        }
                    }
                }
                else if( bAddAutoCmpl && rACW.GetMinWordLen() <= rWord.getLength() )
                {
                    if ( bRedlineChg )
                    {
                        OUString rNewWord( rWord );
                        rACW.InsertWord( rNewWord, *pDoc );
                    }
                    else
                        rACW.InsertWord( rWord, *pDoc );
                }
            }
        }
    }

    
    
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
        
        //
        if( nChgStart < nChgEnd )
        {
            aRect = lcl_CalculateRepaintRect( *this, nChgStart, nChgEnd );
        }

        pNode->GetWrong()->SetInvalid( nInvStart, nInvEnd );
        pNode->SetWrongDirty( COMPLETE_STRING != pNode->GetWrong()->GetBeginInv() );
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
    recognized words are stored into a list that is used later for drawing
    the underline.

    @param pActNode ???
    @param nActPos ???
    @return SwRect Repaint area
*/
SwRect SwTxtFrm::SmartTagScan( SwCntntNode* /*pActNode*/, sal_Int32 /*nActPos*/ )
{
    SwRect aRet;
    SwTxtNode *pNode = GetTxtNode();
    const OUString& rText = pNode->GetTxt();

    
    SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();

    SwWrongList* pSmartTagList = pNode->GetSmartTags();

    sal_Int32 nBegin = 0;
    sal_Int32 nEnd = rText.getLength();

    if ( pSmartTagList )
    {
        if ( pSmartTagList->GetBeginInv() != COMPLETE_STRING )
        {
            nBegin = pSmartTagList->GetBeginInv();
            nEnd = std::min( pSmartTagList->GetEndInv(), rText.getLength() );

            if ( nBegin < nEnd )
            {
                const LanguageType aCurrLang = pNode->GetLang( nBegin );
                const com::sun::star::lang::Locale aCurrLocale = g_pBreakIt->GetLocale( aCurrLang );
                nBegin = g_pBreakIt->GetBreakIter()->beginOfSentence( rText, nBegin, aCurrLocale );
                nEnd = g_pBreakIt->GetBreakIter()->endOfSentence(rText, nEnd, aCurrLocale);
                if (nEnd > rText.getLength() || nEnd < 0)
                    nEnd = rText.getLength();
            }
        }
    }

    const sal_uInt16 nNumberOfEntries = pSmartTagList ? pSmartTagList->Count() : 0;
    sal_uInt16 nNumberOfRemovedEntries = 0;
    sal_uInt16 nNumberOfInsertedEntries = 0;

    
    if ( 0 != nNumberOfEntries )
    {
        sal_Int32 nChgStart = COMPLETE_STRING;
        sal_Int32 nChgEnd = 0;
        const sal_uInt16 nCurrentIndex = pSmartTagList->GetWrongPos( nBegin );
        pSmartTagList->Fresh( nChgStart, nChgEnd, nBegin, nEnd - nBegin, nCurrentIndex, COMPLETE_STRING );
        nNumberOfRemovedEntries = nNumberOfEntries - pSmartTagList->Count();
    }

    if ( nBegin < nEnd )
    {
        
        const ModelToViewHelper aConversionMap(*pNode);
        OUString aExpandText = aConversionMap.getViewText();

        
        com::sun::star::uno::Reference< com::sun::star::text::XTextMarkup > xTextMarkup =
             new SwXTextMarkup( *pNode, aConversionMap );

        com::sun::star::uno::Reference< ::com::sun::star::frame::XController > xController = pNode->GetDoc()->GetDocShell()->GetController();

        SwPosition start(*pNode, nBegin);
        SwPosition end  (*pNode, nEnd);
        Reference< ::com::sun::star::text::XTextRange > xRange = SwXTextRange::CreateXTextRange(*pNode->GetDoc(), start, &end);

        rSmartTagMgr.RecognizeTextRange(xRange, xTextMarkup, xController);


        sal_Int32 nLangBegin = nBegin;
        sal_Int32 nLangEnd = nEnd;

        
        SwLanguageIterator aIter( *pNode, nLangBegin );

        do
        {
            const LanguageType nLang = aIter.GetLanguage();
            const com::sun::star::lang::Locale aLocale = g_pBreakIt->GetLocale( nLang );
            nLangEnd = std::min<sal_Int32>( nEnd, aIter.GetChgPos() );

            const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nLangBegin );
            const sal_Int32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nLangEnd );

            rSmartTagMgr.RecognizeString(aExpandText, xTextMarkup, xController, aLocale, nExpandBegin, nExpandEnd - nExpandBegin );

            nLangBegin = nLangEnd;
        }
        while ( aIter.Next() && nLangEnd < nEnd );

        pSmartTagList = pNode->GetSmartTags();

        const sal_uInt16 nNumberOfEntriesAfterRecognize = pSmartTagList ? pSmartTagList->Count() : 0;
        nNumberOfInsertedEntries = nNumberOfEntriesAfterRecognize - ( nNumberOfEntries - nNumberOfRemovedEntries );
    }

    if( pSmartTagList )
    {
        
        pSmartTagList->SetInvalid( COMPLETE_STRING, 0 );
        pNode->SetSmartTagDirty( COMPLETE_STRING != pSmartTagList->GetBeginInv() );

        if( !pSmartTagList->Count() && !pNode->IsSmartTagDirty() )
            pNode->SetSmartTags( NULL );

        
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


void SwTxtFrm::CollectAutoCmplWrds( SwCntntNode* pActNode, sal_Int32 nActPos )
{
    SwTxtNode *pNode = GetTxtNode();
    if( pNode != pActNode || !nActPos )
        nActPos = COMPLETE_STRING;

    SwDoc* pDoc = pNode->GetDoc();
    SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

    sal_Int32  nBegin = 0;
    sal_Int32  nEnd = pNode->GetTxt().getLength();
    sal_Int32  nLen;
    bool bACWDirty = false, bAnyWrd = false;

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
                const OUString& rWord = aScanner.GetWord();

                if( nActPos < nBegin || ( nBegin + nLen ) < nActPos )
                {
                    if( rACW.GetMinWordLen() <= rWord.getLength() )
                        rACW.InsertWord( rWord, *pDoc );
                    bAnyWrd = true;
                }
                else
                    bACWDirty = true;
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
        pNode->SetAutoCompleteWordDirty( false );
}

/** Findet den TxtFrm und sucht dessen CalcHyph */
bool SwTxtNode::Hyphenate( SwInterHyphInfo &rHyphInf )
{
    
    if ( LANGUAGE_NONE == sal_uInt16( GetSwAttrSet().GetLanguage().GetLanguage() )
         && USHRT_MAX == GetLang(0, m_Text.getLength()))
    {
        if( !rHyphInf.IsCheck() )
            rHyphInf.SetNoLang( true );
        return false;
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
        
        
        
        OSL_ENSURE( pFrm, "!SwTxtNode::Hyphenate: can't find any frame" );
        return false;
    }

    while( pFrm )
    {
        if( pFrm->Hyphenate( rHyphInf ) )
        {
            
            
            
            pFrm->SetCompletePaint();
            return true;
        }
        pFrm = (SwTxtFrm*)(pFrm->GetFollow());
        if( pFrm )
        {
            rHyphInf.nEnd = rHyphInf.nEnd - (pFrm->GetOfst() - rHyphInf.nStart);
            rHyphInf.nStart = pFrm->GetOfst();
        }
    }
    return false;
}

namespace
{
    struct swTransliterationChgData
    {
        sal_Int32               nStart;
        sal_Int32               nLen;
        OUString                sChanged;
        Sequence< sal_Int32 >   aOffsets;
    };
}


void SwTxtNode::TransliterateText(
    utl::TransliterationWrapper& rTrans,
    sal_Int32 nStt, sal_Int32 nEnd,
    SwUndoTransliterate* pUndo )
{
    if (nStt < nEnd && g_pBreakIt->GetBreakIter().is())
    {
        
        
        
        
        
        const sal_Int16 nWordType = WordType::ANYWORD_IGNOREWHITESPACES;

        
        
        
        
        
        
        std::vector< swTransliterationChgData >   aChanges;
        swTransliterationChgData                  aChgData;

        if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::TITLE_CASE)
        {
            

            Boundary aSttBndry;
            Boundary aEndBndry;
            aSttBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                        GetTxt(), nStt,
                        g_pBreakIt->GetLocale( GetLang( nStt ) ),
                        nWordType,
                        sal_True /*prefer forward direction*/);
            aEndBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                        GetTxt(), nEnd,
                        g_pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType,
                        sal_False /*prefer backward direction*/);

            
            if (aSttBndry.endPos <= nStt)
            {
                aSttBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), aSttBndry.endPos,
                        g_pBreakIt->GetLocale( GetLang( aSttBndry.endPos ) ),
                        nWordType);
            }
            
            if (aEndBndry.startPos >= nEnd)
            {
                aEndBndry = g_pBreakIt->GetBreakIter()->previousWord(
                        GetTxt(), aEndBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aEndBndry.startPos ) ),
                        nWordType);
            }

            Boundary aCurWordBndry( aSttBndry );
            while (aCurWordBndry.startPos <= aEndBndry.startPos)
            {
                nStt = aCurWordBndry.startPos;
                nEnd = aCurWordBndry.endPos;
                const sal_Int32 nLen = nEnd - nStt;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(
                            GetTxt(), GetLang(nStt), nStt, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
                {
                    aChgData.nStart     = nStt;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                aCurWordBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nEnd,
                        g_pBreakIt->GetLocale( GetLang( nEnd ) ),
                        nWordType);
            }
        }
        else if (rTrans.getType() == (sal_uInt32)TransliterationModulesExtra::SENTENCE_CASE)
        {
            

            sal_Int32 nLastStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                    GetTxt(), nEnd,
                    g_pBreakIt->GetLocale( GetLang( nEnd ) ) );
            sal_Int32 nLastEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                    GetTxt(), nLastStart,
                    g_pBreakIt->GetLocale( GetLang( nLastStart ) ) );

            
            sal_Int32 nCurrentStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                    GetTxt(), nStt,
                    g_pBreakIt->GetLocale( GetLang( nStt ) ) );
            sal_Int32 nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                    GetTxt(), nCurrentStart,
                    g_pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );

            
            if (nCurrentEnd <= nStt)
            {
                
                
                
                
                i18n::Boundary aBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nCurrentEnd,
                        g_pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        i18n::WordType::DICTIONARY_WORD);

                
                nCurrentStart = g_pBreakIt->GetBreakIter()->beginOfSentence(
                        GetTxt(), aBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), nCurrentStart,
                        g_pBreakIt->GetLocale( GetLang( nCurrentStart) ) );
            }
            
            if (nLastStart >= nEnd)
            {
                
                
                
                
                i18n::Boundary aBndry = g_pBreakIt->GetBreakIter()->previousWord(
                        GetTxt(), nLastStart,
                        g_pBreakIt->GetLocale( GetLang( nLastStart) ),
                        i18n::WordType::DICTIONARY_WORD);
                nLastEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), aBndry.startPos,
                        g_pBreakIt->GetLocale( GetLang( aBndry.startPos) ) );
                if (nCurrentEnd > nLastEnd)
                    nCurrentEnd = nLastEnd;
            }

            while (nCurrentStart < nLastEnd)
            {
                sal_Int32 nLen = nCurrentEnd - nCurrentStart;
                OSL_ENSURE( nLen > 0, "invalid word length of 0" );

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(GetTxt(),
                    GetLang(nCurrentStart), nCurrentStart, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
                {
                    aChgData.nStart     = nCurrentStart;
                    aChgData.nLen       = nLen;
                    aChgData.sChanged   = sChgd;
                    aChgData.aOffsets   = aOffsets;
                    aChanges.push_back( aChgData );
                }

                Boundary aFirstWordBndry;
                aFirstWordBndry = g_pBreakIt->GetBreakIter()->nextWord(
                        GetTxt(), nCurrentEnd,
                        g_pBreakIt->GetLocale( GetLang( nCurrentEnd ) ),
                        nWordType);
                nCurrentStart = aFirstWordBndry.startPos;
                nCurrentEnd = g_pBreakIt->GetBreakIter()->endOfSentence(
                        GetTxt(), nCurrentStart,
                        g_pBreakIt->GetLocale( GetLang( nCurrentStart ) ) );
            }
        }
        else
        {
            

            SwLanguageIterator* pIter;
            if( rTrans.needLanguageForTheMode() )
                pIter = new SwLanguageIterator( *this, nStt );
            else
                pIter = 0;

            sal_Int32 nEndPos = 0;
            sal_uInt16 nLang = LANGUAGE_NONE;
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
                const sal_Int32 nLen = nEndPos - nStt;

                Sequence <sal_Int32> aOffsets;
                OUString const sChgd( rTrans.transliterate(
                            m_Text, nLang, nStt, nLen, &aOffsets) );

                assert(nStt < m_Text.getLength());
                if (0 != rtl_ustr_shortenedCompare_WithLength(
                            m_Text.getStr() + nStt, m_Text.getLength() - nStt,
                            sChgd.getStr(), sChgd.getLength(), nLen))
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
            
            
            size_t nSum(0);
            for (size_t i = 0; i < aChanges.size(); ++i)
            {   
                
                swTransliterationChgData & rData =
                    aChanges[ aChanges.size() - 1 - i ];
                nSum += rData.sChanged.getLength() - rData.nLen;
                if (nSum > static_cast<size_t>(GetSpaceLeft()))
                {
                    SAL_WARN("sw.core", "SwTxtNode::ReplaceTextOnly: "
                            "node text with insertion > node capacity.");
                    return;
                }
                if (pUndo)
                    pUndo->AddChanges( *this, rData.nStart, rData.nLen, rData.aOffsets );
                ReplaceTextOnly( rData.nStart, rData.nLen, rData.sChanged, rData.aOffsets );
            }
        }
    }
}

void SwTxtNode::ReplaceTextOnly( sal_Int32 nPos, sal_Int32 nLen,
                                const OUString & rText,
                                const Sequence<sal_Int32>& rOffsets )
{
    assert(rText.getLength() - nLen <= GetSpaceLeft());

    m_Text = m_Text.replaceAt(nPos, nLen, rText);

    sal_Int32 nTLen = rText.getLength();
    const sal_Int32* pOffsets = rOffsets.getConstArray();
    
    sal_Int32 nMyOff = nPos;
    for( sal_Int32 nI = 0; nI < nTLen; ++nI )
    {
        const sal_Int32 nOff = pOffsets[ nI ];
        if( nOff < nMyOff )
        {
            
            sal_Int32 nCnt = 1;
            while( nI + nCnt < nTLen && nOff == pOffsets[ nI + nCnt ] )
                ++nCnt;

            Update( SwIndex( this, nMyOff ), nCnt, false );
            nMyOff = nOff;
            
            nI += nCnt - 1;
        }
        else if( nOff > nMyOff )
        {
            
            Update( SwIndex( this, nMyOff+1 ), nOff - nMyOff, true );
            nMyOff = nOff;
        }
        ++nMyOff;
    }
    if( nMyOff < nLen )
        
        Update( SwIndex( this, nMyOff ), nLen - nMyOff, true );

    
    SwDelTxt aDelHint( nPos, nTLen );
    NotifyClients( 0, &aDelHint );

    SwInsTxt aHint( nPos, nTLen );
    NotifyClients( 0, &aHint );
}



bool SwTxtNode::CountWords( SwDocStat& rStat,
                            sal_Int32 nStt, sal_Int32 nEnd ) const
{
    if( nStt > nEnd )
    {   
        return false;
    }
    if (IsInRedlines())
    {   
        return false;
    }
    bool bCountAll = ( (0 == nStt) && (GetTxt().getLength() == nEnd) );
    ++rStat.nAllPara; 
    if ( IsHidden() )
    {   
        return false;
    }
    
    bool bCountNumbering = nStt == 0;
    bool bHasBullet = false, bHasNumbering = false;
    OUString sNumString;
    if (bCountNumbering)
    {
        sNumString = GetNumString();
        bHasNumbering = !sNumString.isEmpty();
        if (!bHasNumbering)
            bHasBullet = HasBullet();
        bCountNumbering = bHasNumbering || bHasBullet;
    }

    if( nStt == nEnd && !bCountNumbering)
    {   
        return false;
    }

    
    ++rStat.nPara;

    
    if ( bCountAll && !IsWordCountDirty() )
    {
        
        rStat.nWord += GetParaNumberOfWords();
        rStat.nAsianWord += GetParaNumberOfAsianWords();
        rStat.nChar += GetParaNumberOfChars();
        rStat.nCharExcludingSpaces += GetParaNumberOfCharsExcludingSpaces();
        return false;
    }

    
    const ModelToViewHelper aConversionMap(*this, EXPANDFIELDS | HIDEINVISIBLE | HIDEREDLINED);
    OUString aExpandText = aConversionMap.getViewText();

    
    const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nStt );
    const sal_Int32 nExpandEnd   = aConversionMap.ConvertToViewPosition( nEnd );

    if (aExpandText.isEmpty() && !bCountNumbering)
    {
        return false;
    }

    
    
    
    
    sal_uInt32 nTmpWords = 0;        
    sal_uInt32 nTmpAsianWords = 0;   
    sal_uInt32 nTmpChars = 0;        
    sal_uInt32 nTmpCharsExcludingSpaces = 0;  

    
    if (!aExpandText.isEmpty())
    {
        if (g_pBreakIt->GetBreakIter().is())
        {
            
            SwScanner aScanner( *this, aExpandText, 0, aConversionMap, i18n::WordType::WORD_COUNT,
                                nExpandBegin, nExpandEnd, true );

            
            const OUString aBreakWord( CH_TXTATR_BREAKWORD );

            while ( aScanner.NextWord() )
            {
                if( !aExpandText.match(aBreakWord, aScanner.GetBegin() ))
                {
                    ++nTmpWords;
                    const OUString &rWord = aScanner.GetWord();
                    if (g_pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                        ++nTmpAsianWords;
                    nTmpCharsExcludingSpaces += g_pBreakIt->getGraphemeCount(rWord);
                }
            }

            nTmpCharsExcludingSpaces += aScanner.getOverriddenDashCount();
        }

        nTmpChars = g_pBreakIt->getGraphemeCount(aExpandText, nExpandBegin, nExpandEnd);
    }

    
    

    
    
    if (bHasNumbering) 
    {
        LanguageType aLanguage = GetLang( 0 );

        SwScanner aScanner( *this, sNumString, &aLanguage, ModelToViewHelper(),
                            i18n::WordType::WORD_COUNT, 0, sNumString.getLength(), true );

        while ( aScanner.NextWord() )
        {
            ++nTmpWords;
            const OUString &rWord = aScanner.GetWord();
            if (g_pBreakIt->GetBreakIter()->getScriptType(rWord, 0) == i18n::ScriptType::ASIAN)
                ++nTmpAsianWords;
            nTmpCharsExcludingSpaces += g_pBreakIt->getGraphemeCount(rWord);
        }

        nTmpCharsExcludingSpaces += aScanner.getOverriddenDashCount();
        nTmpChars += g_pBreakIt->getGraphemeCount(sNumString);
    }
    else if ( bHasBullet )
    {
        ++nTmpWords;
        ++nTmpChars;
        ++nTmpCharsExcludingSpaces;
    }

    
    if ( bCountAll )
    {
        SetParaNumberOfWords( nTmpWords );
        SetParaNumberOfAsianWords( nTmpAsianWords );
        SetParaNumberOfChars( nTmpChars );
        SetParaNumberOfCharsExcludingSpaces( nTmpCharsExcludingSpaces );
        SetWordCountDirty( false );
    }
    
    rStat.nWord += nTmpWords;
    rStat.nAsianWord += nTmpAsianWords;
    rStat.nChar += nTmpChars;
    rStat.nCharExcludingSpaces += nTmpCharsExcludingSpaces;

    return true;
}



struct SwParaIdleData_Impl
{
    SwWrongList* pWrong;                
    SwGrammarMarkUp* pGrammarCheck;     
    SwWrongList* pSmartTags;
    sal_uLong nNumberOfWords;
    sal_uLong nNumberOfAsianWords;
    sal_uLong nNumberOfChars;
    sal_uLong nNumberOfCharsExcludingSpaces;
    bool bWordCountDirty;
    bool bWrongDirty;                   
    bool bGrammarCheckDirty;
    bool bSmartTagDirty;
    bool bAutoComplDirty;               

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
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bWordCountDirty;
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
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bWrongDirty;
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
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bGrammarCheckDirty;
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
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bSmartTagDirty;
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
    return m_pParaIdleData_Impl && m_pParaIdleData_Impl->bAutoComplDirty;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
