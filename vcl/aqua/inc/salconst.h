/*************************************************************************
 *
 *  $RCSfile: salconst.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-02-14 19:39:47 $
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

#ifndef _SV_SALCONST_H
#define _SV_SALCONST_H

// -------------------
// - Constants -
// -------------------

const unsigned short kByteMask = 0xFF;

const unsigned short kOneByte  =  8;
const unsigned short kTwoBytes = 16;

const unsigned short kOneBit     =  1;
const unsigned short kFiveBits   =  5;
const unsigned short kEightBits  =  8;
const unsigned short kTenBits    = 10;
const unsigned short kElevenBits = 11;

const unsigned short kBlackAndWhite  =  1;
const unsigned short kFourBitColor   =  4;
const unsigned short kEightBitColor  =  8;
const unsigned short kThousandsColor = 16;
const unsigned short kTrueColor      = 32;

const unsigned long k16BitRedColorMask   = 0x00007c00;
const unsigned long k16BitGreenColorMask = 0x000003e0;
const unsigned long k16BitBlueColorMask  = 0x0000001f;

const unsigned long k32BitRedColorMask   = 0x00ff0000;
const unsigned long k32BitGreenColorMask = 0x0000ff00;
const unsigned long k32BitBlueColorMask  = 0x000000ff;

const unsigned short kPixMapCmpSizeOneBit    =  1;
const unsigned short kPixMapCmpSizeFourBits  =  4;
const unsigned short kPixMapCmpSizeFiveBits  =  5;
const unsigned short kPixMapCmpSizeEightBits =  8;

const long kPixMapHRes = 72;
const long kPixMapVRes = 72;

#endif // _SV_SALCONST_H
