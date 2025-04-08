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
#include <tools/gen.hxx>

#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <functional>
#include <utility>

struct SwPosition;
class SwTextFrame;
class SwTextNode;
namespace vcl { class Font; }
namespace com::sun::star::linguistic2 { class XHyphenatedWord; }

struct SwArgsBase     // used for text conversion (Hangul/Hanja, ...)
{
    SwPosition *pStartPos; // these both point to SwTextNode
    SwPosition *pEndPos;

    SwArgsBase(SwPosition& rStart, SwPosition& rEnd )
        : pStartPos( &rStart ), pEndPos( &rEnd )
        {}

    void SetStart(SwPosition& rStart )
    {
        pStartPos = &rStart;
    }

    void SetEnd( SwPosition& rEnd  )
    {
        pEndPos = &rEnd ;
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
            SwPosition& rStart,
            SwPosition& rEnd )
        : SwArgsBase( rStart, rEnd ),
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

    SwSpellArgs(css::uno::Reference< css::linguistic2::XSpellChecker1 > xSplChk,
            SwPosition& rStart,
            SwPosition& rEnd,
            bool bGrammar )
        :   SwArgsBase( rStart, rEnd ),
            xSpeller(std::move( xSplChk )),
            bIsGrammarCheck( bGrammar )
        {}
};

// Parameter-class for Hyphenate.
// docedt.cxx:  SwDoc::Hyphenate()
// txtedt.cxx:  SwTextNode::Hyphenate()
// txthyph.cxx: SwTextFrame::Hyphenate()

class SwInterHyphInfo
{
    /// output: hyphenated word
    css::uno::Reference<css::linguistic2::XHyphenatedWord> m_xHyphWord;
    /// input: cursor point to locate the frame
    const Point m_aCursorPos;
public:
    /// input: requested range to hyphenate
    sal_Int32 m_nStart;
    sal_Int32 m_nEnd;
    /// output: found word
    sal_Int32 m_nWordStart;
    sal_Int32 m_nWordLen;

    SwInterHyphInfo( const Point &rCursorPos )
        : m_aCursorPos(rCursorPos)
        , m_nStart(0)
        , m_nEnd(SAL_MAX_INT32)
        , m_nWordStart(0), m_nWordLen(0)
    {
    }
    const Point *GetCursorPos() const
    {
        return m_aCursorPos.X() || m_aCursorPos.Y() ? &m_aCursorPos : nullptr;
    }
    void SetHyphWord(const css::uno::Reference< css::linguistic2::XHyphenatedWord >  &rxHW)
    {
        m_xHyphWord = rxHW;
    }
    const css::uno::Reference< css::linguistic2::XHyphenatedWord >& GetHyphWord() const
    {
        return m_xHyphWord;
    }
};


namespace sw {

typedef std::function<SwTextFrame*()> Creator;

SwTextFrame *
SwHyphIterCacheLastTextFrame(SwTextNode const *, const Creator& rCreator);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
