/*************************************************************************
 *
 *  $RCSfile: transliteration_Numeric.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:10:32 $
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

#include <transliteration_Numeric.hxx>
#include <nativenumbersupplier.hxx>
#include <defaultnumberingprovider.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_Numeric::getType() throw(RuntimeException)
{
        return TransliterationType::NUMERIC;
}

OUString SAL_CALL
transliteration_Numeric::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}

sal_Bool SAL_CALL
transliteration_Numeric::equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}

Sequence< OUString > SAL_CALL
transliteration_Numeric::transliterateRange( const OUString& str1, const OUString& str2 )
        throw(RuntimeException)
{
        throw (new RuntimeException());
}


#define isNumber(c) ((c) >= 0x30 && (c) <= 0x39)
#define NUMBER_ZERO 0x30

OUString SAL_CALL
transliteration_Numeric::transliterateBullet( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
        sal_Int32 number = -1, j = 0, endPos = startPos + nCount;

        if (endPos >  inStr.getLength())
            endPos = inStr.getLength();

        rtl_uString* pStr = x_rtl_uString_new_WithLength( nCount, 1 );  // our x_rtl_ustring.h
        sal_Unicode* out = pStr->buffer;

        if (useOffset)
            offset.realloc(nCount);

        for (sal_Int32 i = startPos; i < endPos; i++) {
            if (i < endPos && isNumber(inStr[i])) {
                if (number == -1) {
                    startPos = i;
                    number = (inStr[i] - NUMBER_ZERO);
                } else  {
                    number = number * 10 + (inStr[i] - NUMBER_ZERO);
                }
            } else {
                if (number == 0) {
                    if (useOffset)
                        offset[j] = startPos;
                    out[j++] = NUMBER_ZERO;
                } if (number > tableSize && !recycleSymbol) {
                    for (sal_Int32 k = startPos; k < i; k++) {
                        if (useOffset)
                            offset[j] = k;
                        out[j++] = inStr[k];
                    }
                } else if (number > 0) {
                    if (useOffset)
                        offset[j] = startPos;
                    out[j++] = table[--number % tableSize];
                } else if (i < endPos) {
                    if (useOffset)
                        offset[j] = i;
                    out[j++] = inStr[i];
                }
                number = -1;
            }
        }
        out[j] = 0;

        if (useOffset)
            offset.realloc(j);

        return OUString( pStr, SAL_NO_ACQUIRE );
}

OUString SAL_CALL
transliteration_Numeric::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
        if (tableSize)
            return transliterateBullet( inStr, startPos, nCount, offset);
        else
            return NativeNumberSupplier(useOffset).getNativeNumberString( inStr.copy(startPos, nCount), aLocale, nNativeNumberMode, offset );
}

sal_Unicode SAL_CALL
transliteration_Numeric::transliterateChar2Char( sal_Unicode inChar ) throw(RuntimeException, MultipleCharsOutputException)
{
        if (tableSize) {
            if (isNumber(inChar)) {
                sal_Int16 number = inChar - NUMBER_ZERO;
                if (number <= tableSize || recycleSymbol)
                    return table[--number % tableSize];
            }
            return inChar;
        }
        else
            return NativeNumberSupplier().getNativeNumberChar( inChar, aLocale, nNativeNumberMode );
}

} } } }
