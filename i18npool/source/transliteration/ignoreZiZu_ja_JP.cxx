/*************************************************************************
 *
 *  $RCSfile: ignoreZiZu_ja_JP.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:23:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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

#define TRANSLITERATION_ZiZu_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreZiZu_ja_JP_translator (const sal_Unicode c)
{

  switch (c) {
  case 0x30C2:     // KATAKANA LETTER DI
    return 0x30B8; // KATAKANA LETTER ZI

  case 0x3062:     // HIRAGANA LETTER DI
    return 0x3058; // HIRAGANA LETTER ZI

  case 0x30C5:     // KATAKANA LETTER DU
    return 0x30BA; // KATAKANA LETTER ZU

  case 0x3065:     // HIRAGANA LETTER DU
    return 0x305A; // HIRAGANA LETTER ZU
  }
  return c;
}

OUString SAL_CALL
ignoreZiZu_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
  return transliteration_Ignore::transliterate( inStr, startPos, nCount, offset, ignoreZiZu_ja_JP_translator );
}

} } } }
