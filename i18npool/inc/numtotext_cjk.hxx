/*************************************************************************
 *
 *  $RCSfile: numtotext_cjk.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:44:05 $
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
#ifndef _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_
#define _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_NUMTOTEXT_CJK( name ) \
class NumToText##name : public transliteration_Numeric \
{ \
public: \
        NumToText##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( Date_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( CircledNumber )
TRANSLITERATION_NUMTOTEXT_CJK ( TianGan_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( DiZi_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulSyllable_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledSyllable_ko )
#endif
#undef TRANSLITERATION_NUMTOTEXT

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOTEXT_HXX_
