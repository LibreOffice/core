/*************************************************************************
 *
 *  $RCSfile: ignoreSeZe_ja_JP.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 10:54:48 $
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
#include <stl/utility>

#define TRANSLITERATION_SeZe_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreSeZe_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
  // Create a string buffer which can hold nCount + 1 characters.
  // The reference count is 0 now.
  rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h
  sal_Unicode * dst = newStr->buffer;
  const sal_Unicode * src = inStr.getStr() + startPos;

  // Allocate nCount length to offset argument.
  offset.realloc( nCount );
  sal_Int32 *p = offset.getArray();
  sal_Int32 position = startPos;

  //
  sal_Unicode previousChar = *src ++;
  sal_Unicode currentChar;

  // Translation
  while (-- nCount > 0) {
    currentChar = *src ++;

    // SI + E --> SE
    if (previousChar == 0x30B7 &&  // KATAKANA LETTER SI
    currentChar  == 0x30A7 ) { // KATAKANA LETTER SMALL E
      position ++;
      *p ++ = position;
      position ++;
      *dst ++ = 0x30BB;            // KATAKANA LETTER SE
      previousChar = *src ++;
      nCount --;
      continue;
    }

    // SI + E --> SE
    if (previousChar == 0x3057 &&  // HIRAGANA LETTER SI
    currentChar  == 0x3047 ) { // HIRAGANA LETTER SMALL E
      position ++;
      *p ++ = position;
      position ++;
      *dst ++ = 0x305B;            // HIRAGANA LETTER SE
      previousChar = *src ++;
      nCount --;
      continue;
    }

    // ZI + E --> ZE
    if (previousChar == 0x30B8 &&  // KATAKANA LETTER ZI
    currentChar  == 0x30A7 ) { // KATAKANA LETTER SMALL E
      position ++;
      *p ++ = position;
      position ++;
      *dst ++ = 0x30BC;            // KATAKANA LETTER ZE
      previousChar = *src ++;
      nCount --;
      continue;
    }

    // ZI + E --> ZE
    if (previousChar == 0x3058 &&  // HIRAGANA LETTER ZI
    currentChar  == 0x3047 ) { // HIRAGANA LETTER SMALL E
      position ++;
      *p ++ = position;
      position ++;
      *dst ++ = 0x305C;            // HIRAGANA LETTER ZE
      previousChar = *src ++;
      nCount --;
      continue;
    }

    *p ++ = position;
    position ++;
    *dst ++ = previousChar;
    previousChar = currentChar;
  }

  if (nCount == 0) {
    *p = position;
    *dst ++ = previousChar;
  }

  *dst = (sal_Unicode) 0;

  newStr->length = sal_Int32(dst - newStr->buffer);
  offset.realloc(newStr->length);
  return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}


} } } }
