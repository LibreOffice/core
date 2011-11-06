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


/*
 * Created on 04.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web;

import com.sun.star.wizards.common.PropertyNames;
import java.io.PrintStream;

import com.sun.star.wizards.ui.event.TaskEvent;
import com.sun.star.wizards.ui.event.TaskListener;

/**
 * used for debugging.
 * @author rpiterman
 */
public class LogTaskListener implements TaskListener, ErrorHandler
{

    private PrintStream out;

    public LogTaskListener(PrintStream os)
    {
        out = os;
    }

    public LogTaskListener()
    {
        this(System.out);
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskStarted(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskStarted(TaskEvent te)
    {
        out.println("TASK " + te.getTask().getTaskName() + " STARTED.");

    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskFinished(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskFinished(TaskEvent te)
    {
        out.println("TASK " + te.getTask().getTaskName() + " FINISHED: " + te.getTask().getSuccessfull() + "/" + te.getTask().getMax() + "Succeeded.");

    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskStatusChanged(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskStatusChanged(TaskEvent te)
    {
        out.println("TASK " + te.getTask().getTaskName() + " status : " + te.getTask().getSuccessfull() + "(+" + te.getTask().getFailed() + ")/" + te.getTask().getMax());
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#subtaskNameChanged(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void subtaskNameChanged(TaskEvent te)
    {
        out.println("SUBTASK Name:" + te.getTask().getSubtaskName());
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.ErrorReporter#error(java.lang.Exception, java.lang.Object, java.lang.String)
     */
    public boolean error(Exception ex, Object arg, int ix, int i)
    {
        System.out.println(PropertyNames.EMPTY_STRING + arg + "//" + ix + "//Exception: " + ex.getLocalizedMessage());
        ex.printStackTrace();
        return true;
    }
}
