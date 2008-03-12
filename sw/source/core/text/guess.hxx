/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guess.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:24:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _GUESS_HXX
#define _GUESS_HXX
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

#include "txttypes.hxx"
#include "breakit.hxx"
#include "porrst.hxx"   // SwHangingPortion

class SwTxtFormatInfo;

/*************************************************************************
 *                      class SwTxtGuess
 *************************************************************************/

class SwTxtGuess
{
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord >  xHyphWord;
    SwHangingPortion *pHanging; // for hanging punctuation
    xub_StrLen nCutPos;         // this character doesn't fit
    xub_StrLen nBreakStart;     // start index of word containing line break
    xub_StrLen nBreakPos;       // start index of break position
    xub_StrLen nFieldDiff;      // absolut positions can be wrong if we
                                // a field in the text has been expanded
    KSHORT nBreakWidth;         // width of the broken portion
public:
    inline SwTxtGuess(): pHanging( NULL ), nCutPos(0), nBreakStart(0),
                        nBreakPos(0), nFieldDiff(0), nBreakWidth(0)
        { }
    ~SwTxtGuess() { delete pHanging; }

    // true, if current portion still fits to current line
    sal_Bool Guess( const SwTxtPortion& rPor, SwTxtFormatInfo &rInf,
                    const KSHORT nHeight );
    sal_Bool AlternativeSpelling( const SwTxtFormatInfo &rInf, const xub_StrLen nPos );

    inline SwHangingPortion* GetHangingPortion() const { return pHanging; }
    inline void ClearHangingPortion() { pHanging = NULL; }
    inline KSHORT BreakWidth() const { return nBreakWidth; }
    inline xub_StrLen CutPos() const { return nCutPos; }
    inline xub_StrLen BreakStart() const { return nBreakStart; }
    inline xub_StrLen BreakPos() const {return nBreakPos; }
    inline xub_StrLen FieldDiff() const {return nFieldDiff; }
    inline ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > HyphWord() const
        { return xHyphWord; }
};

#endif
