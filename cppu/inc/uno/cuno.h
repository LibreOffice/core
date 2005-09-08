/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cuno.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:38:31 $
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
#ifndef _UNO_CUNO_H_
#define _UNO_CUNO_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#define CUNO_ERROR_NONE                 0
#define CUNO_ERROR_CALL_FAILED          (1 << 31)
#define CUNO_ERROR_EXCEPTION            (1 | CUNO_ERROR_CALL_FAILED)

/** macro to call on a C interface

    @param interface_pointer interface pointer
*/
#define CUNO_CALL( interface_pointer ) (*interface_pointer)
/** macro to test if an exception was signalled.

    @param return_code return code of call
*/
#define CUNO_EXCEPTION_OCCURED( return_code ) (0 != ((return_code) & CUNO_ERROR_EXCEPTION))

typedef sal_Int32 cuno_ErrorCode;

#endif

