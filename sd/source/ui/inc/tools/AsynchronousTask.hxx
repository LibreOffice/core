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

#pragma once

namespace sd::tools
{
/** Interface for the asynchronous execution of a task.  This interface
    allows a controller to run the task either timer based with a fixed
    amount of time between the steps or thread based one step right after
    the other.
*/
class AsynchronousTask
{
public:
    /** Run the next step of the task.  After HasNextStep() returns false
        this method should ignore further calls.
    */
    virtual void RunNextStep() = 0;

    /** Return <TRUE/> when there is at least one more step to execute.
        When the task has been executed completely then <FALSE/> is
        returned.
    */
    virtual bool HasNextStep() = 0;

protected:
    ~AsynchronousTask() {}
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
