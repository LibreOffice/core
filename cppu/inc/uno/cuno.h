/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuno.h,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UNO_CUNO_H_
#define _UNO_CUNO_H_

#include <sal/types.h>

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

