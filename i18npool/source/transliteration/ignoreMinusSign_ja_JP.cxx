/*************************************************************************
 *
 *  $RCSfile: ignoreMinusSign_ja_JP.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:42:04 $
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

#define TRANSLITERATION_MinusSign_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreMinusSign_ja_JP_translator (const sal_Unicode c)
{
    switch (c) {
        case 0x2212: // MINUS SIGN
        case 0x002d: // HYPHEN-MINUS
        case 0x2010: // HYPHEN
        case 0x2011: // NON-BREAKING HYPHEN
        case 0x2012: // FIGURE DASH
        case 0x2013: // EN DASH
        case 0x2014: // EM DASH
        case 0x2015: // HORIZONTAL BAR
        case 0xff0d: // FULLWIDTH HYPHEN-MINUS
        case 0xff70: // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK
        return 0x30fc; // KATAKANA-HIRAGANA PROLONGED SOUND MARK
    }
    return c;
}

ignoreMinusSign_ja_JP::ignoreMinusSign_ja_JP()
{
        func = ignoreMinusSign_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreMinusSign_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreMinusSign_ja_JP";
}

} } } }
