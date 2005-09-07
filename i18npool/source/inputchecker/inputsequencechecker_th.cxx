/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputsequencechecker_th.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:17:48 $
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
