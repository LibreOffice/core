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
#ifndef INCLUDED_SW_INC_SPLARGS_HXX
#define INCLUDED_SW_INC_SPLARGS_HXX

#include <i18nlangtag/lang.h>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <limits.h>

#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

class SwTextNode;
class SwIndex;
namespace vcl { class Font; }

struct SwArgsBase     // used for text conversion (Hangul/Hanja, ...)
{
    SwTextNode  *pStartNode;
    SwIndex    *pStartIdx;
    SwTextNode  *pEndNode;
    SwIndex    *pEndIdx;

    SwArgsBase(
            SwTextNode* pStart, SwIndex& rStart,
            SwTextNode* pEnd, SwIndex& rEnd )
        : pStartNode( pStart ), pStartIdx( &rStart ),
          pEndNode( pEnd ), pEndIdx( &rEnd )
        {}

    void SetStart(SwTextNode* pStart, SwIndex& rStart )
    {
        pStartNode = pStart;    pStartIdx = &rStart ;
    }

    void SetEnd( SwTextNode* pEnd, SwIndex& rEnd  )
    {
        pEndNode = pEnd;        pEndIdx = &rEnd ;
    }
};

// used for text conversion (Hangul/Hanja, Simplified/Traditional Chinese, ...)
struct SwConversionArgs : SwArgsBase
{
    OUString   aConvText;          // convertible text found
    LanguageType    nConvSrcLang;       // (source) language to look for
    LanguageType    nConvTextLang;      // language of aConvText (if the latter one was found)

    // used for chinese translation
    LanguageType    nConvTargetLang;    // target language of text to be changed
    const vcl::Font *pTargetFont;        // target font of text to be changed
    // explicitly enables or disables application of the above two
    bool            bAllowImplicitChangesForNotConvertibleText;

    SwConversionArgs( LanguageType nLang,
            SwTextNode* pStart, SwIndex& rStart,
            SwTextNode* pEnd, SwIndex& rEnd )
        : SwArgsBase( pStart, rStart, pEnd, rEnd ),
          nConvSrcLang( nLang ),
          nConvTextLang( LANGUAGE_NONE ),
          nConvTargetLang( LANGUAGE_NONE ),
          pTargetFont( nullptr ),
          bAllowImplicitChangesForNotConvertibleText( false )
        {}
};

struct SwSpellArgs : SwArgsBase
{
    css::uno::Reference< css::linguistic2::XSpellChecker1 >     xSpeller;

    css::uno::Reference< css::linguistic2::XSpellAlternatives > xSpellAlt;

    bool bIsGrammarCheck;

    SwSpellArgs(css::uno::Reference<
            css::linguistic2::XSpellChecker1 > &rxSplChk,
            SwTextNode* pStart, SwIndex& rStart,
            SwTextNode* pEnd, SwIndex& rEnd,
            bool bGrammar )
        :   SwArgsBase( pStart, rStart, pEnd, rEnd ),
            xSpeller( rxSplChk ),
            bIsGrammarCheck( bGrammar )
        {}
};

// Parameter-class for Hyphenate.
// docedt.cxx:  SwDoc::Hyphenate()
// txtedt.cxx:  SwTextNode::Hyphenate()
// txthyph.cxx: SwTextFrm::Hyphenate()

class SwInterHyphInfo
{
    css::uno::Reference< css::linguistic2::XHyphenatedWord >    xHyphWord;
    const Point aCrsrPos;
    bool bNoLang : 1;
    bool bCheck  : 1;
public:
    sal_Int32 nStart;
    sal_Int32 nEnd;
    sal_Int32 nWordStart;
    sal_Int32 nWordLen;
    sal_Int32 nHyphPos;
    sal_uInt16 nMinTrail;

    SwInterHyphInfo( const Point &rCrsrPos, sal_Int32 nStartPos = 0,
        sal_Int32 nLength = SAL_MAX_INT32 )
        : aCrsrPos(rCrsrPos)
        , bNoLang(false)
        , bCheck(false)
        , nStart(nStartPos)
        , nEnd(nLength == SAL_MAX_INT32 ? SAL_MAX_INT32 : nStartPos + nLength)
        , nWordStart(0), nWordLen(0)
        , nHyphPos(0), nMinTrail(0)
    {
    }
    sal_Int32 GetEnd() const
    {
        return nEnd;
    }
    const Point *GetCrsrPos() const
    {
        return aCrsrPos.X() || aCrsrPos.Y() ? &aCrsrPos : nullptr;
    }
    bool IsCheck() const { return bCheck; }
    void SetCheck( const bool bNew ) { bCheck = bNew; }
    void SetNoLang( const bool bNew ) { bNoLang = bNew; }
    void SetHyphWord(const css::uno::Reference< css::linguistic2::XHyphenatedWord >  &rxHW)
    {
        xHyphWord = rxHW;
    }
    css::uno::Reference< css::linguistic2::XHyphenatedWord > GetHyphWord()
    {
        return xHyphWord;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
