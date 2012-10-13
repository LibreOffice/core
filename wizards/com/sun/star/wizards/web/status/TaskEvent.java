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

package com.sun.star.wizards.web.status;

import java.util.EventObject;

public class TaskEvent extends EventObject
{

    public static final int TASK_STARTED = 1;
    public static final int TASK_FINISHED = 2;
    public static final int TASK_STATUS_CHANGED = 3;
    public static final int SUBTASK_NAME_CHANGED = 4;
    private int type;

    /**
     * general constructor-
     * @param source
     * @param type_
     */
    public TaskEvent(Task source, int type_)
    {
        super(source);
        type = type_;
    }

    public Task getTask()
    {
        return (Task) getSource();
    }
}
