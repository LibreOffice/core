/*************************************************************************
 *
 *  $RCSfile: numtochar.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 16:53:49 $
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

#define TRANSLITERATION_ALL
#include <numtochar.hxx>
#include <com/sun/star/i18n/NativeNumberMode.hpp>

namespace com { namespace sun { namespace star { namespace i18n {


#define TRANSLITERATION_NUMTOCHAR( name, number ) \
NumToChar##name::NumToChar##name() \
{ \
        nNativeNumberMode = number; \
        tableSize = 0; \
        transliterationName = "NumToChar"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.NumToChar"#name; \
}

using namespace com::sun::star::i18n::NativeNumberMode;

TRANSLITERATION_NUMTOCHAR( Halfwidth, NATNUM0 )
TRANSLITERATION_NUMTOCHAR( Fullwidth, NATNUM3 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_CN, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_TW, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_CN, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_TW, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( KanjiShort_ja_JP, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( KanjiTraditional_ja_JP, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Lower_ko, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_ko, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Hangul_ko, NATNUM9 )
TRANSLITERATION_NUMTOCHAR( Indic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( EastIndic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Indic_hi, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( _th, NATNUM1 )
#undef TRANSLITERATION_NUMTOCHAR

} } } }
