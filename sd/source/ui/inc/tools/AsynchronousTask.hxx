/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SD_ASYNCHRONOUS_TASK_HXX
#define SD_ASYNCHRONOUS_TASK_HXX

namespace sd { namespace tools {

/** Interface for the asynchronous execution of a task.  This interface
    allows an controller to run the task either timer based with a fixed
    amount of time between the steps or thread based one step right after
    the other.
*/
class AsynchronousTask
{
public:
    /** Run the next step of the task.  After HasNextStep() returns false
        this method should ignore further calls.
    */
    virtual void RunNextStep (void) = 0;

    /** Return <TRUE/> when there is at least one more step to execute.
        When the task has been executed completely then <FALSE/> is
        returned.
    */
    virtual bool HasNextStep (void) = 0;
};

} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
