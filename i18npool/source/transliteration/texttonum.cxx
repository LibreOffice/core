/*************************************************************************
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define TRANSLITERATION_ALL
#include <texttonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_TEXTTONUM( name ) \
TextToNum##name::TextToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        transliterationName = "TextToNum"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.TextToNum"#name; \
}

TRANSLITERATION_TEXTTONUM( Lower_zh_CN)
TRANSLITERATION_TEXTTONUM( Upper_zh_CN)
TRANSLITERATION_TEXTTONUM( Lower_zh_TW)
TRANSLITERATION_TEXTTONUM( Upper_zh_TW)
TRANSLITERATION_TEXTTONUM( FormalLower_ko)
TRANSLITERATION_TEXTTONUM( FormalUpper_ko)
TRANSLITERATION_TEXTTONUM( FormalHangul_ko)
TRANSLITERATION_TEXTTONUM( InformalLower_ko)
TRANSLITERATION_TEXTTONUM( InformalUpper_ko)
TRANSLITERATION_TEXTTONUM( InformalHangul_ko)
TRANSLITERATION_TEXTTONUM( KanjiLongTraditional_ja_JP)
TRANSLITERATION_TEXTTONUM( KanjiLongModern_ja_JP)
#undef TRANSLITERATION_TEXTTONUM

} } } }
