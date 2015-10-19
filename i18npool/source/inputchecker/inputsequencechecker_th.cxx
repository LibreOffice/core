/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <inputsequencechecker_th.hxx>
#include <wtt.h>


namespace com { namespace sun { namespace star { namespace i18n {

InputSequenceChecker_th::InputSequenceChecker_th()
    : InputSequenceCheckerImpl("com.sun.star.i18n.InputSequenceChecker_th")
{
}

InputSequenceChecker_th::~InputSequenceChecker_th()
{
}

/* Table for Thai Cell Manipulation */
sal_Char _TAC_celltype_inputcheck[17][17] = {
/* Cn */ /*  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   B,   C,   D,   E,   F       */
/* Cn-1 00 */{  'X', 'A', 'A', 'A', 'A', 'A', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 10 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 20 */{  'X', 'A', 'A', 'A', 'A', 'S', 'A', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C' },
     /* 30 */{  'X', 'S', 'A', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 40 */{  'X', 'A', 'A', 'A', 'A', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 50 */{  'X', 'A', 'A', 'A', 'A', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 60 */{  'X', 'A', 'A', 'A', 'S', 'A', 'S', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 70 */{  'X', 'A', 'A', 'A', 'A', 'S', 'A', 'R', 'R', 'R', 'C', 'C', 'R', 'R', 'R', 'R', 'R' },
     /* 80 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* 90 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* A0 */{  'X', 'A', 'A', 'A', 'A', 'A', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* B0 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* C0 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* D0 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R' },
     /* E0 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'C', 'R', 'R', 'R', 'R', 'R' },
     /* F0 */{  'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'R', 'R', 'R', 'R', 'R' },
         { 'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'C', 'R', 'R', 'R', 'R' }
};

sal_Bool _TAC_Composible[3][5] = {
        /*  'A',    'C',        'S',        'R',        'X'   */
/* Mode 0 */    {sal_True,  sal_True,   sal_True,   sal_True,   sal_True}, // PASSTHROUGH = 0
/* Mode 1 */    {sal_True,  sal_True,   sal_True,   sal_False,      sal_True}, // BASIC = 1
/* Mode 2 */    {sal_True,  sal_True,   sal_False,      sal_False,      sal_True}  // STRICT = 2
};

static bool SAL_CALL check(sal_Unicode ch1, sal_Unicode ch2, sal_Int16 inputCheckMode)
{
    sal_Int16  composible_class;
    switch (_TAC_celltype_inputcheck[getCharType(ch1)][getCharType(ch2)]) {
        case 'A': composible_class = 0; break;
        case 'C': composible_class = 1; break;
        case 'S': composible_class = 2; break;
        case 'R': composible_class = 3; break;
        case 'X': composible_class = 4; break;
        default:  composible_class = 0;
    }
    return (_TAC_Composible[inputCheckMode][composible_class]);
}

sal_Bool SAL_CALL
InputSequenceChecker_th::checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
    sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(css::uno::RuntimeException, std::exception)
{
    return check(Text[nStartPos], inputChar, inputCheckMode);
}

sal_Int32 SAL_CALL
InputSequenceChecker_th::correctInputSequence(OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
  throw(css::uno::RuntimeException, std::exception)
{
/* 9 rules for input sequence correction, see issue i42661 for detail,

http://www.openoffice.org/issues/show_bug.cgi?id=42661

<abv> = <av1>|<av2>|<av3>|<bv1>|<bv2>
<abv1> = <av1>|<bv1>
<thanthakhat> = 0E4C (karan)

1. <cons> <abv>_x + <abv>_y => <cons> <abv>_y (replace)
2. <cons> <tone>_x + <tone>_y => <cons> <tone>_y (replace)
3. <cons> <abv> <tone>_x + <tone>_y => <cons> <abv> <tone>_y (replace)
4. <cons> <abv>_x <tone> + <abv>_y => <cons> <abv>_y <tone> (replace, reorder)
5. <cons> <tone> + <abv> => <cons> <abv> <tone> (reorder)
6. <cons> <fv1> + <tone> => <cons> <tone> <fv1> (reorder)
7. <cons> <tone>_x <fv1> + <tone>_y => <cons> <tone>_y <fv1> (replace, reorder)
8. <cons> <thanthakhat> + <abv1> => <cons> <abv1> <thanthakhat> (reorder)
9. <cons> <abv1>_x <thanthakhat> + <abv1>_y => <cons> <abv1>_y <thanthakhat>(reorder, replace)
*/
#define CT_ABV(t)  ( (t>=CT_AV1 && t<=CT_AV3) || t==CT_BV1 || t==CT_BV2)
#define CT_ABV1(t) (t==CT_AV1 || t==CT_BV1)

    if (check(Text[nStartPos], inputChar, inputCheckMode))
        Text = Text.replaceAt(++nStartPos, 0, OUString(inputChar));
    else if (nStartPos > 0 && getCharType(Text[nStartPos-1]) == CT_CONS) {
        sal_uInt16 t1=getCharType(Text[nStartPos]), t2=getCharType(inputChar);
        if ( (CT_ABV(t1) && CT_ABV(t2)) || // 1.
                (t1==CT_TONE && t2==CT_TONE) )// 2.
            Text = Text.replaceAt(nStartPos, 1, OUString(inputChar));
        else if ( (t1==CT_TONE && CT_ABV(t2)) ||  // 5.
                (t1==CT_FV1 && t2==CT_TONE) ||  // 6.
                (Text[nStartPos]==0x0E4C && CT_ABV1(t2)) ) // 8.
            Text = Text.replaceAt(nStartPos++, 0, OUString(inputChar));
        else
            nStartPos=Text.getLength();
    } else if (nStartPos > 1 && getCharType(Text[nStartPos-2]) == CT_CONS) {
        sal_uInt16 t1=getCharType(Text[nStartPos-1]), t2=getCharType(Text[nStartPos]), t3=getCharType(inputChar);
        if (CT_ABV(t1) && t2==CT_TONE && t3==CT_TONE) // 3.
            Text = Text.replaceAt(nStartPos, 1, OUString(inputChar));
        else if ( (CT_ABV(t1) && t2==CT_TONE && CT_ABV(t3)) || // 4.
                (t1==CT_TONE && t2==CT_FV1 && t3==CT_TONE) || // 7.
                (CT_ABV1(t1) && Text[nStartPos]==0x0E4C && CT_ABV1(t3)) ) // 9.
            Text = Text.replaceAt(nStartPos-1, 1, OUString(inputChar));
        else
            nStartPos=Text.getLength();
    } else
        nStartPos=Text.getLength();

    return nStartPos;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
