/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jobconst.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 15:01:12 $
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
