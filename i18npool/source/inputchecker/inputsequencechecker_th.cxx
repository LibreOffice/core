/*************************************************************************
 *
 *  $RCSfile: inputsequencechecker_th.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 12:54:35 $
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
#include <inputsequencechecker_th.hxx>
#include <wtt.h>

using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

InputSequenceChecker_th::InputSequenceChecker_th()
{
    serviceName = "com.sun.star.i18n.InputSequenceChecker_th";
}

InputSequenceChecker_th::~InputSequenceChecker_th()
{
}

/* Table for Thai Cell Manipulation */
sal_Char _TAC_celltype_inputcheck[17][17] = {
/* Cn */ /*  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   B,   C,   D,   E,   F       */
/* Cn-1 00 */   'X', 'A', 'A', 'A', 'A', 'A', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 10 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 20 */   'X', 'A', 'A', 'A', 'A', 'S', 'A', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C',
     /* 30 */   'X', 'S', 'A', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 40 */   'X', 'S', 'A', 'S', 'A', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 50 */   'X', 'A', 'A', 'A', 'A', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 60 */   'X', 'A', 'A', 'A', 'S', 'A', 'S', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 70 */   'X', 'A', 'A', 'A', 'A', 'S', 'A', 'R', 'R', 'R', 'C', 'C', 'R', 'R', 'R', 'R', 'R',
     /* 80 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'R', 'R', 'R', 'R', 'R',
     /* 90 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* A0 */   'X', 'A', 'A', 'A', 'A', 'A', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* B0 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* C0 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* D0 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
     /* E0 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'C', 'R', 'R', 'R', 'R', 'R',
     /* F0 */   'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'R', 'R', 'R', 'R', 'R',
        'X', 'A', 'A', 'A', 'S', 'S', 'A', 'R', 'R', 'R', 'C', 'R', 'C', 'R', 'R', 'R', 'R'
};

sal_Bool _TAC_Composible[3][5] = {
        /*  'A',    'C',        'S',        'R',        'X'   */
/* Mode 0 */    {sal_True,  sal_True,   sal_True,   sal_True,   sal_True}, // PASSTHROUGH = 0
/* Mode 1 */    {sal_True,  sal_True,   sal_True,   sal_False,      sal_True}, // BASIC = 1
/* Mode 2 */    {sal_True,  sal_True,   sal_False,      sal_False,      sal_True}  // STRICT = 2
};

sal_Bool SAL_CALL
InputSequenceChecker_th::checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
    sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException)
{
    sal_Int16  composible_class;
    sal_Unicode currentChar = Text[nStartPos];
    switch (_TAC_celltype_inputcheck[getCharType(currentChar)][getCharType(inputChar)]) {
        case 'A': composible_class = 0; break;
        case 'C': composible_class = 1; break;
        case 'S': composible_class = 2; break;
        case 'R': composible_class = 3; break;
        case 'X': composible_class = 4; break;
        default:  composible_class = 0;
    }
    return (_TAC_Composible[inputCheckMode][composible_class]);
}

} } } }
