/*************************************************************************
 *
 *  $RCSfile: wtt.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 17:57:44 $
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
#ifndef _I18N_WTT_H_
#define _I18N_WTT_H_

namespace com { namespace sun { namespace star { namespace i18n {

/*
 * Thai character type definition.
 */

#define CT_CTRL     0       // Control character
#define CT_NON      1       // Non-composible
#define CT_CONS     2       // Consonant
#define CT_LV       3       // Leading vowel
#define CT_FV1      4       // Following vowel
#define CT_FV2      5       // Following vowel
#define CT_FV3      6
#define CT_BV1      7       // Below vowel
#define CT_BV2      8
#define CT_BD       9       // Below diacritic
#define CT_TONE     10      // Tone
#define CT_AD1      11      // Above diacritic
#define CT_AD2      12
#define CT_AD3      13
#define CT_AV1      14      // Above vowel
#define CT_AV2      15
#define CT_AV3      16

#define MAX_CT      17

static const sal_uInt16 thaiCT[128] = {     // Thai character type
    CT_NON, CT_CONS, CT_CONS, CT_CONS, CT_CONS,CT_CONS, CT_CONS, CT_CONS,       //0E00
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,     //0E10
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_FV3, CT_CONS, CT_FV3, CT_CONS,       //0E20
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_NON,
    CT_FV1, CT_AV2, CT_FV1, CT_FV1, CT_AV1, CT_AV3, CT_AV2, CT_AV3,         //0E30
    CT_BV1, CT_BV2, CT_BD, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON,
    CT_LV, CT_LV, CT_LV, CT_LV, CT_LV, CT_FV2, CT_NON, CT_AD2,          //0E40
    CT_TONE, CT_TONE, CT_TONE, CT_TONE, CT_AD1, CT_AD1, CT_AD3, CT_NON,
    CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON,         //0E50
    CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_CTRL };

#define getCharType(x) ((x >= 0x0E00 && x < 0x0E60) ? thaiCT[x - 0x0E00] : CT_NON)

} } } }

#endif // _I18N_WTT_H_
