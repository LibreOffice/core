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
#ifndef _GUESS_HXX
#define _GUESS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

#include "txttypes.hxx"
#include "breakit.hxx"
#include "porrst.hxx"	// SwHangingPortion
namespace binfilter {

class SwTxtSizeInfo;
class SwTxtFormatInfo;

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;

/*************************************************************************
 *						class SwTxtGuess
 *************************************************************************/

class SwTxtGuess
{
    uno::Reference< XHyphenatedWord >  xHyphWord;
    SwHangingPortion *pHanging; // for hanging punctuation
    xub_StrLen nCutPos;			// this character doesn't fit
    xub_StrLen nBreakStart;     // start index of word containing line break
    xub_StrLen nBreakPos;		// start index of break position
    xub_StrLen nFieldDiff;      // absolut positions can be wrong if we
                                // a field in the text has been expanded
    KSHORT nBreakWidth;			// width of the broken portion
public:
    inline SwTxtGuess(): pHanging( NULL ), nCutPos(0), nBreakStart(0),
                        nBreakPos(0), nFieldDiff(0), nBreakWidth(0)
        { }
    ~SwTxtGuess() {	delete pHanging; }

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
    inline uno::Reference< XHyphenatedWord > HyphWord() const
        { return xHyphWord; }
};

} //namespace binfilter
#endif
