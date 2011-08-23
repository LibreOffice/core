/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef UT_HXX
#define UT_HXX
# include <sal/types.h>

namespace OpenStormBento
{

#define UtMin(x,y) ((x) < (y) ? (x) : (y))
#define UtMax(x,y) ((x) > (y) ? (x) : (y))

#define UtHugeMemcpy memcpy

#define UtErr_OK 0
#define UtErr_Fail 1

#define UT_TRUE	sal_True
#define UT_FALSE sal_False

#define UtByte sal_uInt8
#define UtDWord sal_uInt32
#define UtWord sal_uInt16
#define UtBool sal_Int32

#define UtDefClassP(Name) class Name; typedef Name * p##Name; \
  typedef const Name * pConst##Name

inline UtWord UtGetIntelWord(UtByte * pData)
{ return * (UtWord *) pData; }

inline UtDWord UtGetIntelDWord(UtByte * pData)
{ return * (UtDWord *) pData; }

inline void UtPutIntelWord(UtByte * pData, UtWord Val)
{ * (UtWord *) pData = Val; }

inline void UtPutIntelDWord(UtByte * pData, UtDWord Val)
{ * (UtDWord *) pData = Val; }

inline UtByte UtGetIntelByte(UtByte * pData)
{ return * pData; }

inline void UtPutIntelByte(UtByte * pData, UtByte Val)

{ * pData = Val; }

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
