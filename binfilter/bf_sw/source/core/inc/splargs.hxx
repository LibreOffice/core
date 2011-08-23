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
#ifndef _SPLARGS_HXX
#define _SPLARGS_HXX

#include <tools/solar.h>
#include <tools/gen.hxx>
#include <limits.h>       // USHRT_MAX
#include <tools/string.hxx>

class SwTxtNode;
class SwIndex;
class SpellCheck;

#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
namespace binfilter {

/*************************************************************************
 *                      struct SwConversionArgs
 *************************************************************************/


struct SwArgsBase     // used for text conversion (Hangul/Hanja, ...)
{
    SwTxtNode  *pStartNode;
    SwIndex    &rStartIdx;
    SwTxtNode  *pEndNode;
    SwIndex    &rEndIdx;

    SwArgsBase(
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd )
        : pStartNode( pStart ), rStartIdx( rStart ),
          pEndNode( pEnd ), rEndIdx( rEnd )
        {}
};

/*************************************************************************
 *                      struct SwConversionArgs
 * used for text conversion (Hangul/Hanja, ...)
 *************************************************************************/

struct SwConversionArgs : SwArgsBase
{
    ::rtl::OUString   aConvText;
    sal_Bool        bConvTextFound;     // convertible text portion found

    SwConversionArgs(
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd )
        : SwArgsBase( pStart, rStart, pEnd, rEnd ),
          bConvTextFound( sal_False )
        {}
};

/*************************************************************************
 *                      struct SwSpellArgs
 *************************************************************************/

struct SwSpellArgs : SwArgsBase
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >  	xSpeller;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >	xSpellAlt;

    SwSpellArgs(::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellChecker1 > &rxSplChk,
            SwTxtNode* pStart, SwIndex& rStart,
            SwTxtNode* pEnd, SwIndex& rEnd )
        :   SwArgsBase( pStart, rStart, pEnd, rEnd ),
            xSpeller( rxSplChk )
        {}
};

/*************************************************************************
 *						class SwInterHyphInfo
 *************************************************************************/

// Parameter-Klasse fuer Hyphenate
// docedt.cxx:	SwDoc::Hyphenate()
// txtedt.cxx:	SwTxtNode::Hyphenate()
// txthyph.cxx: SwTxtFrm::Hyphenate()

class SwInterHyphInfo
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord > 	xHyphWord;
    const	Point aCrsrPos;
    sal_Bool	bAuto	: 1;
    sal_Bool	bNoLang	: 1;
    sal_Bool	bCheck 	: 1;
public:
    xub_StrLen nStart;
    xub_StrLen nLen;
    xub_StrLen nWordStart;
    xub_StrLen nWordLen;
    xub_StrLen nHyphPos;
    sal_uInt16 nMinTrail;

    inline SwInterHyphInfo( const Point &rCrsrPos,
                            const sal_uInt16 nStart = 0,
                            const sal_uInt16 nLen = USHRT_MAX )
         : aCrsrPos( rCrsrPos ),
           bAuto(sal_False), bNoLang(sal_False), bCheck(sal_False),
           nStart(nStart), nLen(nLen),
           nWordStart(0), nWordLen(0),
           nMinTrail(0), nHyphPos(0)
         { }
    inline xub_StrLen GetStart() const { return nStart; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline xub_StrLen GetEnd() const
    { return STRING_LEN == nLen ? nLen : nStart + nLen; }
    inline const Point *GetCrsrPos() const
    { return aCrsrPos.X() || aCrsrPos.Y() ? &aCrsrPos : 0; }
    inline sal_Bool IsAuto() const { return bAuto; }
    inline void SetAuto( const sal_Bool bNew ) { bAuto = bNew; }
    inline sal_Bool IsCheck() const { return bCheck; }
    inline void SetCheck( const sal_Bool bNew ) { bCheck = bNew; }
    inline sal_Bool IsNoLang() const { return bNoLang; }
    inline void SetNoLang( const sal_Bool bNew ) { bNoLang = bNew; }

    inline void
            SetHyphWord(const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XHyphenatedWord >  &rxHW)
            { xHyphWord = rxHW; }
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >
            GetHyphWord() { return xHyphWord; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
