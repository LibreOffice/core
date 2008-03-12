/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: platform_xp.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:35:12 $
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

#ifndef _PLATFORM_XP_H_
#define _PLATFORM_XP_H_

#pragma once

// Change these values to use different versions
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE
#undef _WTL_NO_CSTRING

#define WINVER          0x0500
#define _WIN32_WINNT    0x0500
#define _WIN32_IE       0x0600
#define _WTL_NO_CSTRING

#if defined _MSC_VER
    #pragma warning(push, 1)
#endif

#include <windows.h>

#if defined _MSC_VER
    #pragma warning(pop)
#endif

#endif

