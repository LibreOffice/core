/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SPLARGS_HXX
#define _SPLARGS_HXX

#include <i18npool/lang.h>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <limits.h>       // USHRT_MAX
#include <tools/string.hxx>

class SwTxtNode;
class SwIndex;
class SpellCheck;
class Font;
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

/*************************************************************************
 *                      struct SwArgsBase
 *************************************************************************/


struct SwArgsBase     // used for text conversion (Hangul/Hanja, ...)
{
    SwTxtNode  *pStartNode;
    SwIndex    *pStartIdx;
    SwTxtNode  *pEndNode;
    SwIndex    *pEndIdx;

    SwArgsBase(
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd )
        : pStartNode( pStart ), pStartIdx( &rStart ),
          pEndNode( pEnd ), pEndIdx( &rEnd )
        {}

    void SetStart(SwTxtNode* pStart, SwIndex& rStart )
    {
        pStartNode = pStart;    pStartIdx = &rStart ;
    }

    void SetEnd( SwTxtNode* pEnd, SwIndex& rEnd  )
    {
        pEndNode = pEnd;        pEndIdx = &rEnd ;
    }
};

/*************************************************************************
 *                      struct SwConversionArgs
 * used for text conversion (Hangul/Hanja, Simplified/Traditional Chinese, ...)
 *************************************************************************/

struct SwConversionArgs : SwArgsBase
{
    rtl::OUString   aConvText;          // convertible text found
    LanguageType    nConvSrcLang;       // (source) language to look for
    LanguageType    nConvTextLang;      // language of aConvText (if the latter one was found)

    // used for chinese translation
    LanguageType    nConvTargetLang;    // target language of text to be changed
    const Font     *pTargetFont;        // target font of text to be changed
    // explicitly enables or disables application of the above two
    sal_Bool        bAllowImplicitChangesForNotConvertibleText;

    SwConversionArgs( LanguageType nLang,
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd )
        : SwArgsBase( pStart, rStart, pEnd, rEnd ),
          nConvSrcLang( nLang ),
          nConvTextLang( LANGUAGE_NONE ),
          nConvTargetLang( LANGUAGE_NONE ),
          pTargetFont( NULL ),
          bAllowImplicitChangesForNotConvertibleText( sal_False )
        {}
};

/*************************************************************************
 *                      struct SwSpellArgs
 *************************************************************************/

struct SwSpellArgs : SwArgsBase
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpeller;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives > xSpellAlt;

    bool bIsGrammarCheck;

    SwSpellArgs(::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellChecker1 > &rxSplChk,
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd,
            bool bGrammar )
        :   SwArgsBase( pStart, rStart, pEnd, rEnd ),
            xSpeller( rxSplChk ),
            bIsGrammarCheck( bGrammar )
        {}
};

/*************************************************************************
 *                      class SwInterHyphInfo
 *************************************************************************/

// Parameter-Klasse fuer Hyphenate
// docedt.cxx:  SwDoc::Hyphenate()
// txtedt.cxx:  SwTxtNode::Hyphenate()
// txthyph.cxx: SwTxtFrm::Hyphenate()

class SwInterHyphInfo
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >    xHyphWord;
    const   Point aCrsrPos;
    sal_Bool    bAuto   : 1;
    sal_Bool    bNoLang : 1;
    sal_Bool    bCheck  : 1;
public:
    xub_StrLen nStart;
    xub_StrLen nLen;
    xub_StrLen nWordStart;
    xub_StrLen nWordLen;
    xub_StrLen nHyphPos;
    sal_uInt16 nMinTrail;

    inline SwInterHyphInfo( const Point &rCrsrPos,
                            const sal_uInt16 nStartPos = 0,
                            const sal_uInt16 nLength = USHRT_MAX )
         : aCrsrPos( rCrsrPos ),
           bAuto(sal_False), bNoLang(sal_False), bCheck(sal_False),
           nStart(nStartPos), nLen(nLength),
           nWordStart(0), nWordLen(0),
           nHyphPos(0), nMinTrail(0)
         { }
    inline xub_StrLen GetEnd() const
    { return STRING_LEN == nLen ? nLen : nStart + nLen; }
    inline const Point *GetCrsrPos() const
    { return aCrsrPos.X() || aCrsrPos.Y() ? &aCrsrPos : 0; }
    inline sal_Bool IsCheck() const { return bCheck; }
    inline void SetCheck( const sal_Bool bNew ) { bCheck = bNew; }
    inline void SetNoLang( const sal_Bool bNew ) { bNoLang = bNew; }

    inline void
            SetHyphWord(const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XHyphenatedWord >  &rxHW)
            { xHyphWord = rxHW; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >
            GetHyphWord() { return xHyphWord; }
};


#endif
