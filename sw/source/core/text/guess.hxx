/*************************************************************************
 *
 *  $RCSfile: guess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 12:09:37 $
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
    xub_StrLen nLeftPos;        // untere Kante: Idx
    xub_StrLen nRightPos;       // obere  Kante: Idx
    KSHORT nLeftWidth;          // untere Kante: Width
    KSHORT nRightWidth;         // obere  Kante: Width
    KSHORT nHeight;             // die GetTxtSize()-Hoehe.
    xub_StrLen GetWordEnd( const SwTxtFormatInfo &rInf,
                        const xub_StrLen nPos, const sal_Bool bFwd = sal_True ) const;
public:
    inline SwTxtGuess(): nLeftPos(0), nLeftWidth(0), nRightPos(0),
                         nRightWidth(0), nHeight(0)

        { }

    // liefert zuerueck, ob es noch passte
    sal_Bool Guess( const SwTxtFormatInfo &rInf, const KSHORT nHeight );

    inline xub_StrLen LeftPos() const { return nLeftPos; }
    inline KSHORT LeftWidth() const { return nLeftWidth; }
    inline xub_StrLen RightPos() const { return nRightPos; }
    inline KSHORT RightWidth() const { return nRightWidth; }
    inline KSHORT Height() const { return nHeight; }
    inline uno::Reference< XHyphenatedWord > HyphWord() const
        { return xHyphWord; }

    inline xub_StrLen GetPrevEnd( const SwTxtFormatInfo &rInf,
                                  const xub_StrLen nPos ) const
        { return GetWordEnd( rInf, nPos, sal_False ); }
    inline xub_StrLen GetNextEnd( const SwTxtFormatInfo &rInf,
                                  const xub_StrLen nPos ) const
        { return GetWordEnd( rInf, nPos, sal_True ); }
    static sal_Bool IsWordEnd( const SwTxtSizeInfo &rInf, const xub_StrLen nPos );
};


#endif
