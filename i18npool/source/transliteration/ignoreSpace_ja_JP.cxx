/*************************************************************************
 *
 *  $RCSfile: ignoreSpace_ja_JP.cxx,v $
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

#include <oneToOneMapping.hxx>
#define TRANSLITERATION_Space_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreSpace_ja_JP_mappingTable[] = {
  MAKE_PAIR( 0x0020, 0xffff ),  // SPACE
  MAKE_PAIR( 0x00A0, 0xffff ),  // NO-BREAK SPACE
  MAKE_PAIR( 0x2002, 0xffff ),  // EN SPACE
  MAKE_PAIR( 0x2003, 0xffff ),  // EM SPACE
  MAKE_PAIR( 0x2004, 0xffff ),  // THREE-PER-EM SPACE
  MAKE_PAIR( 0x2005, 0xffff ),  // FOUR-PER-EM SPACE
  MAKE_PAIR( 0x2006, 0xffff ),  // SIX-PER-EM SPACE
  MAKE_PAIR( 0x2007, 0xffff ),  // FIGURE SPACE
  MAKE_PAIR( 0x2008, 0xffff ),  // PUNCTUATION SPACE
  MAKE_PAIR( 0x2009, 0xffff ),  // THIN SPACE
  MAKE_PAIR( 0x200A, 0xffff ),  // HAIR SPACE
  MAKE_PAIR( 0x200B, 0xffff ),  // ZERO WIDTH SPACE
  MAKE_PAIR( 0x202F, 0xffff ),  // NARROW NO-BREAK SPACE
  MAKE_PAIR( 0x3000, 0xffff ),  // IDEOGRAPHIC SPACE
  MAKE_PAIR( 0x303F, 0xffff )   // IDEOGRAPHIC HALF FILL SPACE
};


OUString SAL_CALL
ignoreSpace_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
  oneToOneMapping table(ignoreSpace_ja_JP_mappingTable, sizeof(ignoreSpace_ja_JP_mappingTable));
  return transliteration_Ignore::transliterate( inStr, startPos, nCount, offset, table );
}

} } } }
