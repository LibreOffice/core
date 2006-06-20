/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnose.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:05:07 $
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


#ifndef _VOS_DIAGNOSE_H_
#define _VOS_DIAGNOSE_H_


#ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
#endif


/*
    Diagnostic support
*/

#define VOS_THIS_FILE       __FILE__

#define VOS_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define VOS_TRACE           _OSL_TRACE
#define VOS_ASSERT(c)       _OSL_ASSERT(c, VOS_THIS_FILE, __LINE__)
#define VOS_VERIFY(c)       OSL_VERIFY(c)
#define VOS_ENSHURE(c, m)   _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)
#define VOS_ENSURE(c, m)    _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)

#define VOS_PRECOND(c, m)   VOS_ENSHURE(c, m)
#define VOS_POSTCOND(c, m)  VOS_ENSHURE(c, m)

#endif /* _VOS_DIAGNOSE_H_ */


