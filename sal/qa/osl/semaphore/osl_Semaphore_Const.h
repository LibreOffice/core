/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: osl_Semaphore_Const.h,v $
 * $Revision: 1.4 $
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_SEMAPHORE_CONST_H_
#define _OSL_SEMAPHORE_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#include <sal/types.h>
#include <rtl/ustring.hxx>

#ifndef _OSL_THREAD_HXX_
#include <osl/thread.hxx>
#endif
#include <osl/mutex.hxx>
#include <osl/pipe.hxx>

#ifndef _OSL_SEMAPHOR_HXX_
#include <osl/semaphor.hxx>
#endif
#include <osl/time.h>

#include <testshl/simpleheader.hxx>

#ifdef UNX
#include <unistd.h>
#endif

//------------------------------------------------------------------------
// most return value -1 denote a fail of operation.
//------------------------------------------------------------------------
#define OSL_PIPE_FAIL   -1

#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// semaphre names
//------------------------------------------------------------------------
OSLTEST_DECLARE_USTRING( TestSem,  "testsem" );

const char pTestString[17] = "Sun Microsystems";


#endif /* _OSL_SEMAPHORE_CONST_H_ */
