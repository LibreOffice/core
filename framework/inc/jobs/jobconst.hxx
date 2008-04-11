/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jobconst.hxx,v $
 * $Revision: 1.3 $
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

#ifndef __FRAMEWORK_JOBS_JOBCONST_HXX_
#define __FRAMEWORK_JOBS_JOBCONST_HXX_

//_______________________________________
// my own includes

#include <stdtypes.h>
#include <general.h>

//_______________________________________
// interface includes

//_______________________________________
// other includes

#include <rtl/ustring.hxx>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  defines all constant values used within a job environment.

    @descr  The protocol between job executor/dispatch and jobs is fix and well defined.
            But why every implemented job should write such constant values directly into
            it's code. Typos can occure or code will be changed by new developers ...
            Shared set of constant values can help to improve the mentainance of this code.
 */
class JobConst
{
    public:

        static const ::rtl::OUString ANSWER_DEACTIVATE_JOB();
        static const ::rtl::OUString ANSWER_SAVE_ARGUMENTS();
        static const ::rtl::OUString ANSWER_SEND_DISPATCHRESULT();
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBCONST_HXX_
