/*************************************************************************
 *
 *  $RCSfile: ignoreSpace_ja_JP.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 16:52:29 $
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

#define TRANSLITERATION_Space_ja_JP
#include <transliteration_Ignore.hxx>

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


ignoreSpace_ja_JP::ignoreSpace_ja_JP()
{
        func = (TransFunc)0;
        table = new oneToOneMapping(ignoreSpace_ja_JP_mappingTable, sizeof(ignoreSpace_ja_JP_mappingTable));
        map = 0;
        transliterationName = "ignoreSpace_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSpace_ja_JP";
}

} } } }
