/*************************************************************************
 *
 *  $RCSfile: namesort.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:14:08 $
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


#include <precomp.h>
#include <namesort.hxx>


// NOT FULLY DEFINED SERVICES



namespace
{

int C_cAryNameOrdering1[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       71, 72, 73, 74,  75, 76, 77, 78,      79, 80,255,255, 255,255,255,255,

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

int C_cAryNameOrdering2[256] =
    {   0,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, //  0 ..
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255,
      255,255,255,255, 255,255,255,255,     255,255,255,255, 255,255,255,255, // 32 ..
       71, 72, 73, 74,  75, 76, 77, 78,      79, 80,255,255, 255,255,255,255,

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
}


namespace ary
{





CompareCeNames::CompareCeNames()
    :   aOrdering1(C_cAryNameOrdering1),
        aOrdering2(C_cAryNameOrdering2)
{
}




}   // namespace ary



