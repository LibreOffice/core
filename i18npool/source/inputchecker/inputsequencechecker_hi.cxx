/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputsequencechecker_hi.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:17:29 $
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

#include <inputsequencechecker_hi.hxx>

using namespace rtl;

namespace com {
namespace sun {
namespace star {
namespace i18n {

InputSequenceChecker_hi::InputSequenceChecker_hi()
{
    serviceName = "com.sun.star.i18n.InputSequenceChecker_hi";
}

InputSequenceChecker_hi::~InputSequenceChecker_hi()
{
}
/* Non-Defined Class type */
#define __ND        0

/*
 * Devanagari character type definitions
 */
#define __UP  1  // ChandraBindu & Anuswar
#define __NP  2  // Visarg
#define __IV    3  // Independant Vowels
#define __CN    4  // Consonants except _CK & _RC
#define __CK    5  // Consonants that can be followed by Nukta
#define __RC    6  // Ra
#define __NM    7  // Matra
#define __RM    8  // Ra + HAL
#define __IM    9  // Choti I Matra
#define __HL    10 // HAL
#define __NK    11 // Nukta
#define __VD    12 // Vedic
#define __HD    13 // Hindu Numerals

/*
 * Devanagari character type table
 */
static const sal_uInt16 devaCT[128] = {
/*         0,    1,    2,    3,    4,    5,    6,    7,
           8,    9,    A,    B,    C,    D,    E,    F, */
/* 0900 */ __ND, __UP, __UP, __NP, __ND, __IV, __IV, __IV,
           __IV, __IV, __IV, __IV, __IV, __IV, __IV, __IV,
/* 0910 */ __IV, __IV, __IV, __IV, __IV, __CK, __CK, __CK,
           __CN, __CN, __CN, __CN, __CK, __CN, __CN, __CN,
/* 0920 */ __CN, __CK, __CK, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __CN, __CK, __CN, __CN, __CN, __CN,
/* 0930 */ __RC, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __ND, __ND, __NK, __VD, __NM, __IM,
/* 0940 */ __RM, __NM, __NM, __NM, __NM, __RM, __RM, __RM,
           __RM, __RM, __RM, __RM, __RM, __HL, __ND, __ND,
/* 0950 */ __ND, __VD, __VD, __VD, __VD, __ND, __ND, __ND,
           __CN, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
/* 0960 */ __IV, __IV, __NM, __NM, __ND, __ND, __HD, __HD,
           __HD, __HD, __HD, __HD, __HD, __HD, __HD, __HD,
/* 0970 */ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
           __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
};

/*
 * Devanagari character composition table
 */
static const sal_uInt16 dev_cell_check[14][14] = {
  /*      ND, UP, NP, IV, CN, CK, RC, NM, RM, IM, HL, NK, VD, HD, */
  /* 0  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* ND */
  /* 1  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* UP */
  /* 2  */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* NP */
  /* 3  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* IV */
  /* 4  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* CN */
  /* 5  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0, /* CK */
  /* 6  */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* RC */
  /* 7  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* NM */
  /* 8  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* RM */
  /* 9  */ 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* IM */
  /* 10 */ 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0, /* HL */
  /* 11 */ 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, /* NK */
  /* 12 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* VD */
  /* 13 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* HD */
};

sal_Bool _DEV_Composible[2][2] = {
/* Mode 0 */    {sal_True, sal_True }, // PASSTHROUGH = 0
/* Mode 1 */    {sal_False, sal_True}  // STRICT = 1
};

#define getCharType(x) \
    ((x >= 0x0900 && x < 0x097f) ? devaCT[x - 0x0900] : __ND)

sal_Bool SAL_CALL
InputSequenceChecker_hi::checkInputSequence(const OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
  throw(com::sun::star::uno::RuntimeException)
{
    sal_Unicode currentChar = Text[nStartPos];
  sal_uInt16  ch1 = getCharType(inputChar);
    sal_uInt16  ch2 = getCharType(currentChar);

    return (_DEV_Composible[inputCheckMode][dev_cell_check[ch2][ch1]]);
}

} } } }
