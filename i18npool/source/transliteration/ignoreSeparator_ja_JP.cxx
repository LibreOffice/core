/*************************************************************************
 *
 *  $RCSfile: ignoreSeparator_ja_JP.cxx,v $
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

#include <oneToOneMapping.hxx>
#define TRANSLITERATION_Separator_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreSeparatorTable[] = {
  MAKE_PAIR( 0x0021, 0x002C ),  // EXCLAMATION MARK
  MAKE_PAIR( 0x0023, 0x002C ),  // NUMBER SIGN
  MAKE_PAIR( 0x0024, 0x002C ),  // DOLLAR SIGN
  MAKE_PAIR( 0x0025, 0x002C ),  // PERCENT SIGN
  MAKE_PAIR( 0x0026, 0x002C ),  // AMPERSAND
  MAKE_PAIR( 0x002A, 0x002C ),  // ASTERISK
  MAKE_PAIR( 0x002B, 0x002C ),  // PLUS SIGN
  MAKE_PAIR( 0x002C, 0x002C ),  // COMMA
  MAKE_PAIR( 0x002D, 0x002C ),  // HYPHEN-MINUS
  MAKE_PAIR( 0x002E, 0x002C ),  // FULL STOP
  MAKE_PAIR( 0x002F, 0x002C ),  // SOLIDUS
  MAKE_PAIR( 0x003A, 0x002C ),  // COLON
  MAKE_PAIR( 0x003B, 0x002C ),  // SEMICOLON
  MAKE_PAIR( 0x003C, 0x002C ),  // LESS-THAN SIGN
  MAKE_PAIR( 0x003D, 0x002C ),  // EQUALS SIGN
  MAKE_PAIR( 0x003E, 0x002C ),  // GREATER-THAN SIGN
  MAKE_PAIR( 0x005C, 0x002C ),  // REVERSE SOLIDUS
  MAKE_PAIR( 0x005F, 0x002C ),  // LOW LINE
  MAKE_PAIR( 0x007B, 0x002C ),  // LEFT CURLY BRACKET
  MAKE_PAIR( 0x007C, 0x002C ),  // VERTICAL LINE
  MAKE_PAIR( 0x007D, 0x002C ),  // RIGHT CURLY BRACKET
  MAKE_PAIR( 0x007E, 0x002C ),  // TILDE
  MAKE_PAIR( 0x00A5, 0x002C ),  // YEN SIGN
  MAKE_PAIR( 0xFF01, 0x002C ),  // FULLWIDTH EXCLAMATION MARK
  MAKE_PAIR( 0xFF03, 0x002C ),  // FULLWIDTH NUMBER SIGN
  MAKE_PAIR( 0xFF04, 0x002C ),  // FULLWIDTH DOLLAR SIGN
  MAKE_PAIR( 0xFF05, 0x002C ),  // FULLWIDTH PERCENT SIGN
  MAKE_PAIR( 0xFF06, 0x002C ),  // FULLWIDTH AMPERSAND
  MAKE_PAIR( 0xFF0A, 0x002C ),  // FULLWIDTH ASTERISK
  MAKE_PAIR( 0xFF0B, 0x002C ),  // FULLWIDTH PLUS SIGN
  MAKE_PAIR( 0xFF0C, 0x002C ),  // FULLWIDTH COMMA
  MAKE_PAIR( 0xFF0D, 0x002C ),  // FULLWIDTH HYPHEN-MINUS
  MAKE_PAIR( 0xFF0E, 0x002C ),  // FULLWIDTH FULL STOP
  MAKE_PAIR( 0xFF0F, 0x002C ),  // FULLWIDTH SOLIDUS
  MAKE_PAIR( 0xFF1A, 0x002C ),  // FULLWIDTH COLON
  MAKE_PAIR( 0xFF1B, 0x002C ),  // FULLWIDTH SEMICOLON
  MAKE_PAIR( 0xFF1C, 0x002C ),  // FULLWIDTH LESS-THAN SIGN
  MAKE_PAIR( 0xFF1D, 0x002C ),  // FULLWIDTH EQUALS SIGN
  MAKE_PAIR( 0xFF1E, 0x002C ),  // FULLWIDTH GREATER-THAN SIGN
  MAKE_PAIR( 0xFF3C, 0x002C ),  // FULLWIDTH REVERSE SOLIDUS
  MAKE_PAIR( 0xFF3F, 0x002C ),  // FULLWIDTH LOW LINE
  MAKE_PAIR( 0xFF5B, 0x002C ),  // FULLWIDTH LEFT CURLY BRACKET
  MAKE_PAIR( 0xFF5C, 0x002C ),  // FULLWIDTH VERTICAL LINE
  MAKE_PAIR( 0xFF5D, 0x002C ),  // FULLWIDTH RIGHT CURLY BRACKET
  MAKE_PAIR( 0xFF5E, 0x002C ),  // FULLWIDTH TILDE
  MAKE_PAIR( 0xFFE5, 0x002C ),  // FULLWIDTH YEN SIGN
};

OUString SAL_CALL
ignoreSeparator_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
  oneToOneMapping table(ignoreSeparatorTable, sizeof(ignoreSeparatorTable));
  return transliteration_Ignore::transliterate( inStr, startPos, nCount, offset, table );
}

} } } }

