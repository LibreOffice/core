/*************************************************************************
 *
 *  $RCSfile: guess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-15 13:40:55 $
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
#include "porrst.hxx"   // SwHangingPortion

class SwTxtSizeInfo;
class SwTxtFormatInfo;

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;

/*************************************************************************
 *                      class SwTxtGuess
 *************************************************************************/

class SwTxtGuess
{
    uno::Reference< XHyphenatedWord >  xHyphWord;
    SwHangingPortion *pHanging; // for hanging punctuation
    xub_StrLen nCutPos;         // this character doesn't fit
    xub_StrLen nBreakStart;     // start index of word containing line break
    xub_StrLen nBreakPos;       // start index of break position
    KSHORT nBreakWidth;         // width of the broken portion
    KSHORT nHeight;             // GetTxtSize()-Height
public:
    inline SwTxtGuess(): pHanging( NULL ), nCutPos(0), nBreakStart(0),
                        nBreakPos(0), nBreakWidth(0), nHeight(0)
        { }
    ~SwTxtGuess() { delete pHanging; }

    // true, if current portion still fits to current line
    sal_Bool Guess( const SwTxtFormatInfo &rInf, const KSHORT nHeight );
    sal_Bool AlternativeSpelling( const SwTxtFormatInfo &rInf, const xub_StrLen nPos );

    inline SwHangingPortion* GetHangingPortion() const { return pHanging; }
    inline void ClearHangingPortion() { pHanging = NULL; }
    inline KSHORT BreakWidth() const { return nBreakWidth; }
    inline xub_StrLen CutPos() const { return nCutPos; }
    inline xub_StrLen BreakStart() const { return nBreakStart; }
    inline xub_StrLen BreakPos() const {return nBreakPos; }
    inline KSHORT Height() const { return nHeight; }
    inline uno::Reference< XHyphenatedWord > HyphWord() const
        { return xHyphWord; }
    static xub_StrLen GetWordStart( const SwTxtFormatInfo &rInf,
                    const xub_StrLen nPos );
};

#endif
