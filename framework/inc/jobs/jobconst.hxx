/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_JOBS_JOBCONST_HXX_
#define __FRAMEWORK_JOBS_JOBCONST_HXX_

#include <stdtypes.h>
#include <general.h>


#include <rtl/ustring.hxx>
#include <fwidllapi.h>


namespace framework{


//_______________________________________
/**
    @short  defines all constant values used within a job environment.

    @descr  The protocol between job executor/dispatch and jobs is fix and well defined.
            But why every implemented job should write such constant values directly into
            it's code. Typos can occure or code will be changed by new developers ...
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
