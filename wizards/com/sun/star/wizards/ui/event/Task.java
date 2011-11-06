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


package com.sun.star.wizards.ui.event;

import java.util.List;
import java.util.Vector;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class Task
{

    private int successfull = 0;
    private int failed = 0;
    private int max = 0;
    private String taskName;
    private List listeners = new Vector();
    private String subtaskName;

    public Task(String taskName_, String subtaskName_, int max_)
    {
        taskName = taskName_;
        subtaskName = subtaskName_;
        max = max_;
    }

    public void start()
    {
        fireTaskStarted();
    }

    public void fail()
    {
        fireTaskFailed();
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

    public void advance(boolean success_)
    {
        if (success_)
        {
            successfull++;
        }
        else
        {
            failed++;
        }
        fireTaskStatusChanged();
        if (failed + successfull == max)
        {
            fireTaskFinished();
        }
    }

    public void advance(boolean success_, String nextSubtaskName)
    {
        advance(success_);
        setSubtaskName(nextSubtaskName);
    }

    public int getStatus()
    {
        return successfull + failed;
    }

    public void addTaskListener(TaskListener tl)
    {
        listeners.add(tl);
    }

    public void removeTaskListener(TaskListener tl)
    {
        listeners.remove(tl);
    }

    protected void fireTaskStatusChanged()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_STATUS_CHANGED);

        for (int i = 0; i < listeners.size(); i++)
        {
            ((TaskListener) listeners.get(i)).taskStatusChanged(te);
        }
    }

    protected void fireTaskStarted()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_STARTED);

        for (int i = 0; i < listeners.size(); i++)
        {
            ((TaskListener) listeners.get(i)).taskStarted(te);
        }
    }

    protected void fireTaskFailed()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_FAILED);

        for (int i = 0; i < listeners.size(); i++)
        {
            ((TaskListener) listeners.get(i)).taskFinished(te);
        }
    }

    protected void fireTaskFinished()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_FINISHED);

        for (int i = 0; i < listeners.size(); i++)
        {
            ((TaskListener) listeners.get(i)).taskFinished(te);
        }
    }

    protected void fireSubtaskNameChanged()
    {
        TaskEvent te = new TaskEvent(this, TaskEvent.SUBTASK_NAME_CHANGED);

        for (int i = 0; i < listeners.size(); i++)
        {
            ((TaskListener) listeners.get(i)).subtaskNameChanged(te);
        }
    }

    /**
     * @return
     */
    public String getSubtaskName()
    {
        return subtaskName;
    }

    /**
     * @return
     */
    public String getTaskName()
    {
        return taskName;
    }

    /**
     * @param string
     */
    public void setSubtaskName(String string)
    {
        subtaskName = string;
        fireSubtaskNameChanged();
    }

    /**
     * @return
     */
    public int getFailed()
    {
        return failed;
    }

    /**
     * @return
     */
    public int getSuccessfull()
    {
        return successfull;
    }
}
