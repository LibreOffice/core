/*
 ************************************************************************
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
