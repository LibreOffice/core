/*************************************************************************
 *
 *  $RCSfile: transliteration_Ignore.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:10:22 $
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

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

inline sal_Int32 Min( sal_Int32 a, sal_Int32 b ) { return a > b ? b : a; }

sal_Bool SAL_CALL
transliteration_Ignore::equals(const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) throw(RuntimeException)
{
        Sequence< sal_Int32 > offset1;
        Sequence< sal_Int32 > offset2;

        // The method folding is defined in a sub class.
        OUString s1 = this->folding( str1, pos1, nCount1, offset1);
        OUString s2 = this->folding( str2, pos2, nCount2, offset2);

        const sal_Unicode * p1 = s1.getStr();
        const sal_Unicode * p2 = s2.getStr();
        sal_Int32 length = Min(s1.getLength(), s2.getLength());

        for (sal_Int32 nmatch = 0; nmatch < length; nmatch++)
            if (*p1++ != *p2++)
                break;

        if (nmatch > 0) {
            nMatch1 = offset1[ nmatch - 1 ] + 1; // Subtract 1 from nmatch because the index starts from zero.
            nMatch2 = offset2[ nmatch - 1 ] + 1; // And then, add 1 to position because it means the number of character matched.
        }
        else {
            nMatch1 = 0;  // No character was matched.
            nMatch2 = 0;
        }

        return (nmatch == s1.getLength()) && (nmatch == s2.getLength());
}


Sequence< OUString > SAL_CALL
transliteration_Ignore::transliterateRange( const OUString& str1, const OUString& str2 ) throw(RuntimeException)
{
        if (str1.getLength() < 1 || str2.getLength() < 1)
            throw RuntimeException();

        Sequence< OUString > r(2);
        r[0] = str1.copy(0, 1);
        r[1] = str2.copy(0, 1);
        return r;
}


sal_Int16 SAL_CALL
transliteration_Ignore::getType() throw(RuntimeException)
{
        // The type is also defined in com/sun/star/util/TransliterationType.hdl
        return TransliterationType::IGNORE;
}


OUString SAL_CALL
transliteration_Ignore::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset  ) throw(RuntimeException)
{
        // The method folding is defined in a sub class.
        return this->folding( inStr, startPos, nCount, offset);
}

Sequence< OUString > SAL_CALL
transliteration_Ignore::transliterateRange( const OUString& str1, const OUString& str2,
        XTransliteration& t1, XTransliteration& t2 ) throw(RuntimeException)
{
        if (str1.getLength() < 1 || str2.getLength() < 1)
            throw RuntimeException();

        Sequence< sal_Int32 > offset;
        OUString s11 = t1.transliterate( str1, 0, 1, offset );
        OUString s12 = t1.transliterate( str2, 0, 1, offset );
        OUString s21 = t2.transliterate( str1, 0, 1, offset );
        OUString s22 = t2.transliterate( str2, 0, 1, offset );

        if ( (s11 == s21) && (s12 == s22) ) {
            Sequence< OUString > r(2);
            r[0] = s11;
            r[1] = s12;
            return r;
        }

        Sequence< OUString > r(4);
        r[0] = s11;
        r[1] = s12;
        r[2] = s21;
        r[3] = s22;
        return r;
}

OUString SAL_CALL
transliteration_Ignore::folding( const OUString& inStr, sal_Int32 startPos,
    sal_Int32 nCount, Sequence< sal_Int32 >& offset)
    throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 0 now.
    rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    // Allocate nCount length to offset argument.
    sal_Int32 *p, position;
    if (useOffset) {
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    if (map) {
        sal_Unicode previousChar = *src ++;
        sal_Unicode currentChar;

        // Translation
        while (-- nCount > 0) {
            currentChar = *src ++;

            Mapping *m;
            for (m = map; m->replaceChar; m++) {
                if (previousChar == m->previousChar &&  currentChar == m->currentChar ) {
                    if (useOffset) {
                        position++;
                        *p++ = position++;
                    }
                    *dst++ = m->replaceChar;\
                    previousChar = *src++;
                    nCount--;
                    break;
                }
            }

            if (! m->replaceChar) {
                if (useOffset)
                    *p ++ = position ++;
                *dst ++ = previousChar;
                previousChar = currentChar;
            }
        }

        if (nCount == 0) {
            if (useOffset)
                *p = position;
            *dst ++ = previousChar;
        }
        newStr->length = sal_Int32(dst - newStr->buffer);
        if (useOffset)
            offset.realloc(newStr->length);
    } else {
        // Translation
        while (nCount -- > 0) {
            sal_Unicode c = *src++;
            *dst ++ = func ? func( c) : (*table)[ c ];
            if (useOffset)
                *p ++ = position ++;
        }
    }
    *dst = (sal_Unicode) 0;

    return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

sal_Unicode SAL_CALL
transliteration_Ignore::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
{
    return func ? func( inChar) : table ? (*table)[ inChar ] : inChar;
}

} } } }
