/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ignoreMiddleDot_ja_JP.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:28:56 $
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

#define TRANSLITERATION_MiddleDot_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreMiddleDot_ja_JP_translator (const sal_Unicode c)
{
    switch (c) {
        case 0x30FB: // KATAKANA MIDDLE DOT
        case 0xFF65: // HALFWIDTH KATAKANA MIDDLE DOT
        // no break;
        return 0xffff; // Skip this character
    }
    return c;
}

ignoreMiddleDot_ja_JP::ignoreMiddleDot_ja_JP()
{
        func = ignoreMiddleDot_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreMiddleDot_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreMiddleDot_ja_JP";
}

} } } }
