/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namesort.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:13:38 $
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

#include <precomp.h>
#include <ary/namesort.hxx>


// NOT FULLY DEFINED SERVICES



namespace
{


int C_cAutodocNameOrdering1[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       70, 71, 72, 73,  74, 75, 76, 77,      78, 79,255,255, 255,255,255,255,

      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 64 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255, 63,
      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 96 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //128 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //160 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255
    };

int C_cAutodocNameOrdering2[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       70, 71, 72, 73,  74, 75, 76, 77,      78, 79,255,255, 255,255,255,255,

      255, 11, 13, 15,  17, 19, 21, 23,      25, 27, 29, 31,  33, 35, 37, 39, // 64 ..
       41, 43, 45, 47,  49, 51, 53, 55,      57, 59, 61,255, 255,255,255, 63,
      255, 12, 14, 16,  18, 20, 22, 24,      26, 28, 30, 32,  34, 36, 38, 40, // 96 ..
       42, 44, 46, 48,  50, 52, 54, 56,      58, 60, 62,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //128 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //160 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,

      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255
    };


}   // namespace anonymous


namespace ary
{


const csv::CharOrder_Table
LesserName::aOrdering1_(C_cAutodocNameOrdering1);

const csv::CharOrder_Table
LesserName::aOrdering2_(C_cAutodocNameOrdering2);



}   // namespace ary
