/*************************************************************************
 *
 *  $RCSfile: eqnolefilehdr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 11:10:19 $
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

#ifndef __EQNOLEFILEHDR_HXX__
#define __EQNOLEFILEHDR_HXX__

#include <sal/types.h>

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

class SvStorageStream;
class SotStorage;

#define EQNOLEFILEHDR_SIZE 28

class EQNOLEFILEHDR
{
public:
    EQNOLEFILEHDR() {}
    EQNOLEFILEHDR(sal_uInt32 nLenMTEF) : nCBHdr(0x1c),nVersion(0x20000),
        nCf(0xc1c6),nCBObject(nLenMTEF),nReserved1(0),nReserved2(0x0014F690),
        nReserved3(0x0014EBB4), nReserved4(0) {}

    sal_uInt16   nCBHdr;     // length of header, sizeof(EQNOLEFILEHDR) = 28
    sal_uInt32   nVersion;   // hiword = 2, loword = 0
    sal_uInt16   nCf;        // clipboard format ("MathType EF")
    sal_uInt32   nCBObject;  // length of MTEF data following this header
    sal_uInt32   nReserved1; // not used
    sal_uInt32   nReserved2; // not used
    sal_uInt32   nReserved3; // not used
    sal_uInt32   nReserved4; // not used

    void Read(SvStorageStream *pS);
    void Write(SvStorageStream *pS);
};

sal_Bool GetMathTypeVersion( SotStorage* pStor, sal_uInt8 &nVersion );

#endif

