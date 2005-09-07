/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servicename.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:55:56 $
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
/*
Service Name
  com.sun.star.i18n.Transliteration     ... dispatching UNO object and generic methods
  com.sun.star.i18n.Transliteration.u2l ... upper to lower
  com.sun.star.i18n.Transliteration.l2u ... lower to upper
  com.sun.star.i18n.Transliteration.caseignore ... case ignore
  com.sun.star.i18n.Transliteration.l10n ... other

  We have to adopt different service name for internal UNO objects.
  Case realted functionality vary depending on Locale.


Implementation Name
  For geneic service: com.sun.star.i18n.Transliteration,
         com.sun.star.i18n.Transliteration

  com.sun.star.i18n.Transliteration.u2l
  com.sun.star.i18n.Transliteration.l2u
  com.sun.star.i18n.Transliteration.caseignore
  For these there services above,

         com.sun.star.i18n.Transliteration.ja_JP
                                            ^^^^^
                        Locale name

  For com.sun.star.i18n.l10n.Transliteration service,
         com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH,
         com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH,
         com.sun.star.i18n.Transliteration.IGNORE_WIDTH,
         com.sun.star.i18n.Transliteration.KATAKANA_HIRAGANA,
         com.sun.star.i18n.Transliteration.HIRAGANA_KATAKANA,
         com.sun.star.i18n.Transliteration.IGNORE_KANA,

          ... These objects above can be used in every Locale context.

*/


#ifndef _I18N_SERVICENAME_HXX_
#define _I18N_SERVICENAME_HXX_



#define TRLT_SERVICELNAME "com.sun.star.i18n.Transliteration"
#define TRLT_SERVICELNAME_PREFIX TRLT_SERVICELNAME "."
#define TRLT_SERVICELNAME_U2L   TRLT_SERVICELNAME_PREFIX "u2l"
#define TRLT_SERVICELNAME_L2U   TRLT_SERVICELNAME_PREFIX "l2u"
#define TRLT_SERVICELNAME_CASEIGNORE TRLT_SERVICELNAME_PREFIX "caseignore"
#define TRLT_SERVICELNAME_L10N  TRLT_SERVICELNAME_PREFIX "l10n"

#define TRLT_IMPLNAME "com.sun.star.i18n.Transliteration"
#define TRLT_IMPLNAME_PREFIX_LEN    34
#define TRLT_IMPLNAME_PREFIX "com.sun.star.i18n.Transliteration."
//                1...5...901...5...901...5...901..4  --> 34

#define TRLT_IMPLNAME_L10N  TRLT_IMPLNAME_PREFIX "l10n"
#define TRLT_IMPLNAME_FH  TRLT_IMPLNAME_PREFIX "FULLWIDTH_HALFWIDTH" // for test
#define TRLT_IMPLNAME_Ja_JP TRLT_IMPLNAME_PREFIX "ja_JP"

#endif
