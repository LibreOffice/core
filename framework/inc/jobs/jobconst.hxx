/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <fwidllapi.h>

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
            it's code. Typos can occur or code will be changed by new developers ...
            Shared set of constant values can help to improve the mentainance of this code.
 */
class FWI_DLLPUBLIC JobConst
{
    public:
        static const ::rtl::OUString ANSWER_DEACTIVATE_JOB();
        static const ::rtl::OUString ANSWER_SAVE_ARGUMENTS();
        static const ::rtl::OUString ANSWER_SEND_DISPATCHRESULT();
};

} // namespace framework

#endif // __FRAMEWORK_JOBS_JOBCONST_HXX_
