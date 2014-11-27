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
package com.sun.star.wizards.ui.event;

import java.util.ArrayList;
import java.util.List;

public class Task
{

    private int successful = 0;
    private int failed = 0;
    private int max = 0;
    private final String taskName;
    private final List<TaskListener> listeners = new ArrayList<TaskListener>();
    private String subtaskName;

    public Task(String taskName_, String subtaskName_, int max_)
    {
        taskName = taskName_;
        subtaskName = subtaskName_;
        max = max_;
    }

    public int getMax()
    {
        return max;
    }

    public void setMax(int max_)
    {
        max = max_;
        fireTaskStatusChanged();
    }

    public int getStatus()
    {
        return successful + failed;
    }

    protected void fireTaskStatusChanged()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_STATUS_CHANGED);

        for (int i = 0; i < listeners.size(); i++)
        {
            listeners.get(i).taskStatusChanged(te);
        }
    }

    protected void fireSubtaskNameChanged()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.SUBTASK_NAME_CHANGED);

        for (int i = 0; i < listeners.size(); i++)
        {
            listeners.get(i).subtaskNameChanged(te);
        }
    }

    public String getSubtaskName()
    {
        return subtaskName;
    }

    public String getTaskName()
    {
        return taskName;
    }

    public void setSubtaskName(String string)
    {
        subtaskName = string;
        fireSubtaskNameChanged();
    }

    public int getFailed()
    {
        return failed;
    }

    public int getSuccessfull()
    {
        return successful;
    }
}
