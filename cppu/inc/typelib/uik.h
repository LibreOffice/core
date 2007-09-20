/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uik.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 14:43:14 $
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
#ifndef _TYPELIB_UIK_H_
#define _TYPELIB_UIK_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#if defined( SAL_W32)
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(push, 8)
#endif

/** Binary typelib uik struct.  Internally not used anymore.
*/
typedef struct _typelib_Uik
{
    sal_uInt32  m_Data1;
    sal_uInt16  m_Data2;
    sal_uInt16  m_Data3;
    sal_uInt32  m_Data4;
    sal_uInt32  m_Data5;
} typelib_Uik;

#if defined( SAL_W32) ||  defined(SAL_OS2)
#   pragma pack(pop)
#endif

#endif
