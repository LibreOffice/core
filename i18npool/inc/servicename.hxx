/*************************************************************************
 *
 *  $RCSfile: servicename.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
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
