/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ignoreTraditionalKana_ja_JP.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:31:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_TraditionalKana_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreTraditionalKana_ja_JP_translator (const sal_Unicode c)
{

    switch (c) {
        case 0x3090:     // HIRAGANA LETTER WI
            return 0x3044; // HIRAGANA LETTER I

        case 0x3091:     // HIRAGANA LETTER WE
            return 0x3048; // HIRAGANA LETTER E

        case 0x30F0:     // KATAKANA LETTER WI
            return 0x30A4; // KATAKANA LETTER I

        case 0x30F1:     // KATAKANA LETTER WE
            return 0x30A8; // KATAKANA LETTER E
    }
    return c;
}

ignoreTraditionalKana_ja_JP::ignoreTraditionalKana_ja_JP()
{
        func = ignoreTraditionalKana_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreTraditionalKana_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreTraditionalKana_ja_JP";
}

} } } }
