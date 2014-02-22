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

#include <ctype.h>
#include <editeng/unolingu.hxx>
#include <tools/shl.hxx>
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <IDocumentSettingAccess.hxx>
#include <guess.hxx>
#include <inftxt.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/charclass.hxx>
#include <porfld.hxx>
#include <paratr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define CH_FULL_BLANK 0x3000

/*************************************************************************
 *                      SwTxtGuess::Guess
 *
 * provides information for line break calculation
 * returns true if no line break has to be performed
 * otherwise possible break or hyphenation position is determined
 *************************************************************************/

bool SwTxtGuess::Guess( const SwTxtPortion& rPor, SwTxtFormatInfo &rInf,
                            const KSHORT nPorHeight )
{
    nCutPos = rInf.GetIdx();

    
    if( !rInf.GetLen() || rInf.GetTxt().isEmpty() )
        return false;

    OSL_ENSURE( rInf.GetIdx() < rInf.GetTxt().getLength(),
            "+SwTxtGuess::Guess: invalid SwTxtFormatInfo" );

    OSL_ENSURE( nPorHeight, "+SwTxtGuess::Guess: no height" );

    sal_uInt16 nMaxSizeDiff;

    const SwScriptInfo& rSI =
            ((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();

    sal_uInt16 nMaxComp = ( SW_CJK == rInf.GetFont()->GetActual() ) &&
                        rSI.CountCompChg() &&
                        ! rInf.IsMulti() &&
                        ! rPor.InFldGrp() &&
                        ! rPor.IsDropPortion() ?
                        10000 :
                            0 ;

    SwTwips nLineWidth = rInf.Width() - rInf.X();
    sal_Int32 nMaxLen = rInf.GetTxt().getLength() - rInf.GetIdx();

    if ( rInf.GetLen() < nMaxLen )
        nMaxLen = rInf.GetLen();

    if( !nMaxLen )
        return false;

    KSHORT nItalic = 0;
    if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
    {
        bool bAddItalic = true;

        
        if ( rInf.SnapToGrid() )
        {
            GETGRID( rInf.GetTxtFrm()->FindPageFrm() )
            bAddItalic = !pGrid || GRID_LINES_CHARS != pGrid->GetGridType();
        }

        
        if ( 1 == rInf.GetLen() &&
             CH_BLANK == rInf.GetTxt()[ rInf.GetIdx() ] )
            bAddItalic = false;

        nItalic = bAddItalic ? nPorHeight / 12 : 0;

        nLineWidth -= nItalic;

        
        if ( nLineWidth < 0 ) nLineWidth = 0;
    }

    const sal_Int32 nLeftRightBorderSpace =
        (!rPor.GetJoinBorderWithNext() ? rInf.GetFont()->GetRightBorderSpace() : 0) +
        (!rPor.GetJoinBorderWithPrev() ? rInf.GetFont()->GetLeftBorderSpace() : 0);

    nLineWidth -= nLeftRightBorderSpace;

    const bool bUnbreakableNumberings = rInf.GetTxtFrm()->GetTxtNode()->
            getIDocumentSettingAccess()->get(IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS);

    
    if ( ( long ( nLineWidth ) * 2 > long ( nMaxLen ) * nPorHeight ) ||
         ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
    {
        
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen,
                         nMaxComp, nBreakWidth, nMaxSizeDiff );

        if ( ( nBreakWidth <= nLineWidth ) || ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
        {
            
            nCutPos = rInf.GetIdx() + nMaxLen;
            if( nItalic &&
                ( nCutPos >= rInf.GetTxt().getLength() ||
                  
                  
                  rInf.GetTxt()[ nCutPos ] == CH_BREAK ) )
                nBreakWidth = nBreakWidth + nItalic;

            
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

            nBreakWidth += nLeftRightBorderSpace;

            return true;
        }
    }

    bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
    sal_Int32 nHyphPos = 0;

    
    
    
    if( bHyph )
    {
        nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp, nHyphPos );

        if ( !nHyphPos && rInf.GetIdx() )
            nHyphPos = rInf.GetIdx() - 1;
    }
    else
    {
        nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp );

#if OSL_DEBUG_LEVEL > 1
        if ( COMPLETE_STRING != nCutPos )
        {
            sal_uInt16 nMinSize;
            rInf.GetTxtSize( &rSI, rInf.GetIdx(), nCutPos - rInf.GetIdx(),
                             nMaxComp, nMinSize, nMaxSizeDiff );
            OSL_ENSURE( nMinSize <= nLineWidth, "What a Guess!!!" );
        }
#endif
    }

    if( nCutPos > rInf.GetIdx() + nMaxLen )
    {
        
        nCutPos = nBreakPos = rInf.GetIdx() + nMaxLen - 1;
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen, nMaxComp,
                         nBreakWidth, nMaxSizeDiff );

        
        
        if ( nBreakWidth <= nLineWidth )
        {
            if( nItalic && ( nBreakPos + 1 ) >= rInf.GetTxt().getLength() )
                nBreakWidth = nBreakWidth + nItalic;

            
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

            nBreakWidth += nLeftRightBorderSpace;

            return true;
        }
    }

    
    
    if ( rPor.IsFtnPortion() )
    {
        nBreakPos = rInf.GetIdx();
        nCutPos = rInf.GetLen();
        return false;
    }

    sal_Int32 nPorLen = 0;
    
    sal_Unicode cCutChar = rInf.GetTxt()[ nCutPos ];
    if( CH_BLANK == cCutChar || CH_FULL_BLANK == cCutChar )
    {
        nBreakPos = nCutPos;
        sal_Int32 nX = nBreakPos;

        const SvxAdjust& rAdjust = rInf.GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetAdjust().GetAdjust();
        if ( rAdjust == SVX_ADJUST_LEFT )
        {
            
            
            while( nX && nBreakPos > rInf.GetTxt().getLength() &&
                   ( CH_BLANK == ( cCutChar = rInf.GetChar( --nX ) ) ||
                     CH_FULL_BLANK == cCutChar ) )
                --nBreakPos;
        }
        else 
        {
            while( nX && nBreakPos > rInf.GetLineStart() + 1 &&
                   ( CH_BLANK == ( cCutChar = rInf.GetChar( --nX ) ) ||
                     CH_FULL_BLANK == cCutChar ) )
                --nBreakPos;
        }

        if( nBreakPos > rInf.GetIdx() )
            nPorLen = nBreakPos - rInf.GetIdx();
        while( ++nCutPos < rInf.GetTxt().getLength() &&
               ( CH_BLANK == ( cCutChar = rInf.GetChar( nCutPos ) ) ||
                 CH_FULL_BLANK == cCutChar ) )
            ; 

        nBreakStart = nCutPos;
    }
    else if( g_pBreakIt->GetBreakIter().is() )
    {
        
        
        
        

        sal_Int32 nOldIdx = rInf.GetIdx();
        sal_Unicode cFldChr = 0;

#if OSL_DEBUG_LEVEL > 0
        OUString aDebugString;
#endif

        
        
        if ( rInf.GetLast() && rInf.GetLast()->InFldGrp() &&
             ! rInf.GetLast()->IsFtnPortion() &&
             rInf.GetIdx() > rInf.GetLineStart() &&
             CH_TXTATR_BREAKWORD ==
                ( cFldChr = rInf.GetTxt()[ rInf.GetIdx() - 1 ] ) )
        {
            SwFldPortion* pFld = (SwFldPortion*)rInf.GetLast();
            OUString aTxt;
            pFld->GetExpTxt( rInf, aTxt );

            if ( !aTxt.isEmpty() )
            {
                nFieldDiff = aTxt.getLength() - 1;
                nCutPos = nCutPos + nFieldDiff;
                nHyphPos = nHyphPos + nFieldDiff;

#if OSL_DEBUG_LEVEL > 0
                aDebugString = rInf.GetTxt();
#endif

                OUString& rOldTxt = const_cast<OUString&> (rInf.GetTxt());
                rOldTxt = rOldTxt.replaceAt( rInf.GetIdx() - 1, 1, aTxt );
                rInf.SetIdx( rInf.GetIdx() + nFieldDiff );
            }
            else
                cFldChr = 0;
        }

        LineBreakHyphenationOptions aHyphOpt;
        Reference< XHyphenator >  xHyph;
        if( bHyph )
        {
            xHyph = ::GetHyphenator();
            aHyphOpt = LineBreakHyphenationOptions( xHyph,
                                rInf.GetHyphValues(), nHyphPos );
        }

        
        
        
        
        
        LanguageType aLang = rInf.GetFont()->GetLanguage();

        
        
        
        if ( nCutPos && ! rPor.InFldGrp() )
        {
            const CharClass& rCC = GetAppCharClass();

            
            sal_Int32 nLangIndex = nCutPos;

            
            
            
            
            const sal_Int32 nDoNotStepOver = CH_TXTATR_BREAKWORD == cFldChr ?
                                              rInf.GetIdx() - nFieldDiff - 1:
                                              0;

            while ( nLangIndex > nDoNotStepOver &&
                    ! rCC.isLetterNumeric( rInf.GetTxt(), nLangIndex ) )
                --nLangIndex;

            
            
            if ( nLangIndex < rInf.GetIdx() )
            {
                sal_uInt16 nScript = g_pBreakIt->GetRealScriptOfText( rInf.GetTxt(),
                                                                nLangIndex );
                OSL_ENSURE( nScript, "Script is not between 1 and 4" );

                
                if ( nScript - 1 != rInf.GetFont()->GetActual() )
                    aLang = rInf.GetTxtFrm()->GetTxtNode()->GetLang(
                        CH_TXTATR_BREAKWORD == cFldChr ?
                        nDoNotStepOver :
                        nLangIndex, 0, nScript );
            }
        }

        const ForbiddenCharacters aForbidden(
                *rInf.GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->getForbiddenCharacters( aLang, true ) );

        const bool bAllowHanging = rInf.IsHanging() && ! rInf.IsMulti() &&
                                      ! rPor.InFldGrp();

        LineBreakUserOptions aUserOpt(
                aForbidden.beginLine, aForbidden.endLine,
                rInf.HasForbiddenChars(), bAllowHanging, false );

        
        
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        
        
        
        const lang::Locale aLocale = g_pBreakIt->GetLocale( aLang );

        
        
        LineBreakResults aResult = g_pBreakIt->GetBreakIter()->getLineBreak(
            rInf.GetTxt(), nCutPos, aLocale,
            rInf.GetLineStart(), aHyphOpt, aUserOpt );

        nBreakPos = aResult.breakIndex;

        
        
        
        
        if ( nBreakPos < rInf.GetLineStart() && rInf.IsFirstMulti() &&
             ! rInf.IsFtnInside() )
            nBreakPos = rInf.GetLineStart();

        nBreakStart = nBreakPos;

        bHyph = BreakType::HYPHENATION == aResult.breakType;

        if ( bHyph && nBreakPos != COMPLETE_STRING )
        {
            
            
            xHyphWord = aResult.rHyphenatedWord;
            nBreakPos += xHyphWord->getHyphenationPos() + 1;

#if OSL_DEBUG_LEVEL > 1
            
            const OUString aWord = xHyphWord->getWord();
            
            const OUString aHyphenatedWord = xHyphWord->getHyphenatedWord();
            
            const sal_uInt16 nHyphenationPos = xHyphWord->getHyphenationPos();
            (void)nHyphenationPos;
            
            const sal_uInt16 nHyphenPos = xHyphWord->getHyphenPos();
            (void)nHyphenPos;
#endif

            
            if ( ! rInf.IsInterHyph() && rInf.GetIdx() )
            {
                const long nSoftHyphPos =
                        xHyphWord->getWord().indexOf( CHAR_SOFTHYPHEN );

                if ( nSoftHyphPos >= 0 &&
                     nBreakStart + nSoftHyphPos <= nBreakPos &&
                     nBreakPos > rInf.GetLineStart() )
                    nBreakPos = rInf.GetIdx() - 1;
            }

            if( nBreakPos >= rInf.GetIdx() )
            {
                nPorLen = nBreakPos - rInf.GetIdx();
                if( '-' == rInf.GetTxt()[ nBreakPos - 1 ] )
                    xHyphWord = NULL;
            }
        }
        else if ( !bHyph && nBreakPos >= rInf.GetLineStart() )
        {
            OSL_ENSURE( nBreakPos != COMPLETE_STRING, "we should have found a break pos" );

            
            xHyphWord = NULL;

            
            
            if( nBreakPos > rInf.GetLineStart() && rInf.GetIdx() &&
                CHAR_SOFTHYPHEN == rInf.GetTxt()[ nBreakPos - 1 ] )
                nBreakPos = rInf.GetIdx() - 1;

            const SvxAdjust& rAdjust = rInf.GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetAdjust().GetAdjust();
            if( rAdjust != SVX_ADJUST_LEFT )
            {
                
                
                
                
                sal_Int32 nX = nBreakPos;
                while( nX > rInf.GetLineStart() &&
                       ( CH_TXTATR_BREAKWORD != cFldChr || nX > rInf.GetIdx() ) &&
                       ( CH_BLANK == rInf.GetChar( --nX ) ||
                         CH_FULL_BLANK == rInf.GetChar( nX ) ) )
                    nBreakPos = nX;
            }
            if( nBreakPos > rInf.GetIdx() )
                nPorLen = nBreakPos - rInf.GetIdx();
        }
        else
        {
            
            
            nBreakPos = COMPLETE_STRING;
            OSL_ENSURE( nCutPos >= rInf.GetIdx(), "Deep cut" );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        if( nBreakPos > nCutPos && nBreakPos != COMPLETE_STRING )
        {
            const sal_Int32 nHangingLen = nBreakPos - nCutPos;
            SwPosSize aTmpSize = rInf.GetTxtSize( &rSI, nCutPos,
                                                  nHangingLen, 0 );
            aTmpSize.Width(aTmpSize.Width() + nLeftRightBorderSpace);
            OSL_ENSURE( !pHanging, "A hanging portion is hanging around" );
            pHanging = new SwHangingPortion( aTmpSize );
            pHanging->SetLen( nHangingLen );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        
        
        
        
        
        if ( CH_TXTATR_BREAKWORD == cFldChr )
        {
            if ( nBreakPos < rInf.GetIdx() )
                nBreakPos = nOldIdx - 1;
            else if ( COMPLETE_STRING != nBreakPos )
            {
                OSL_ENSURE( nBreakPos >= nFieldDiff, "I've got field trouble!" );
                nBreakPos = nBreakPos - nFieldDiff;
            }

            OSL_ENSURE( nCutPos >= rInf.GetIdx() && nCutPos >= nFieldDiff,
                    "I've got field trouble, part2!" );
            nCutPos = nCutPos - nFieldDiff;

            OUString& rOldTxt = const_cast<OUString&> (rInf.GetTxt());
            OUString aReplacement( cFldChr );
            rOldTxt = rOldTxt.replaceAt( nOldIdx - 1, nFieldDiff + 1, aReplacement);
            rInf.SetIdx( nOldIdx );

#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( aDebugString == rInf.GetTxt(),
                    "Somebody, somebody, somebody put something in my string" );
#endif
        }
    }

    if( nPorLen )
    {
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nPorLen,
                         nMaxComp, nBreakWidth, nMaxSizeDiff );

        
        if ( nMaxSizeDiff )
            rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

        nBreakWidth += nItalic + nLeftRightBorderSpace;
    }
    else
        nBreakWidth = 0;

    if( pHanging )
        nBreakPos = nCutPos;

    return false;
}



bool SwTxtGuess::AlternativeSpelling( const SwTxtFormatInfo &rInf,
    const sal_Int32 nPos )
{
    
    Boundary aBound =
        g_pBreakIt->GetBreakIter()->getWordBoundary( rInf.GetTxt(), nPos,
        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
        WordType::DICTIONARY_WORD, true );
    nBreakStart = aBound.startPos;
    sal_Int32 nWordLen = aBound.endPos - nBreakStart;

    
    nCutPos = nPos;

    OUString aTxt( rInf.GetTxt().copy( nBreakStart, nWordLen ) );

    
    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    OSL_ENSURE( xHyph.is(), "Hyphenator is missing");
    
    xHyphWord = xHyph->queryAlternativeSpelling( aTxt,
                        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
                        nPos - nBreakStart, rInf.GetHyphValues() );
    return xHyphWord.is() && xHyphWord->isAlternativeSpelling();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
