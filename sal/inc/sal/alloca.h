/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alloca.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:46:27 $
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

#ifndef INCLUDED_SAL_ALLOCA_H
#define INCLUDED_SAL_ALLOCA_H

#if defined (SOLARIS) || defined (LINUX) || defined (IRIX)

#ifndef INCLUDED_ALLOCA_H
#include <alloca.h>
#define INCLUDED_ALLOCA_H
#endif

#elif defined (FREEBSD) || defined(NETBSD)

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

#elif defined (MACOSX)

#ifndef INCLUDED_SYS_TYPES_H
#include <sys/types.h>
#define INCLUDED_SYS_TYPES_H
#endif

#elif defined (WNT)

#ifndef INCLUDED_MALLOC_H
#include <malloc.h>
#define INCLUDED_MALLOC_H
#endif

#else

#error "unknown platform: please check for alloca"

#endif

#endif  /* INCLUDED_SAL_ALLOCA_H */

