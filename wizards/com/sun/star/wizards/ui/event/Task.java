/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Task.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:52:30 $
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
 ************************************************************************/package com.sun.star.wizards.ui.event;

import java.util.List;
import java.util.Vector;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class Task {
    private int successfull = 0;
    private int failed = 0;

    private int max = 0;
    private String taskName;
    private List listeners = new Vector();
    private String subtaskName;


    public Task(String taskName_ , String subtaskName_, int max_) {
        taskName = taskName_;
        subtaskName = subtaskName_;
        max = max_;
    }

    public void start() {
        fireTaskStarted();
    }

    public void fail() {
        fireTaskFailed();
    }

    public int getMax() {
        return max;
    }

    public void setMax(int max_) {
        max = max_;
        fireTaskStatusChanged();
    }

    public void advance(boolean success_) {
        if (success_) successfull++;
        else failed++;
        fireTaskStatusChanged();
        if (failed+successfull == max)
          fireTaskFinished();
    }

    public void advance(boolean success_, String nextSubtaskName) {
        advance(success_);
        setSubtaskName(nextSubtaskName);
    }



    public int getStatus() {
        return successfull+failed;
    }

    public void addTaskListener(TaskListener tl) {
        listeners.add(tl);
    }

    public void removeTaskListener(TaskListener tl) {
        listeners.remove(tl);
    }

    protected void fireTaskStatusChanged() {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_STATUS_CHANGED);

        for (int i = 0; i<listeners.size(); i++)
          ((TaskListener)listeners.get(i)).taskStatusChanged(te);
    }

    protected void fireTaskStarted() {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_STARTED);

        for (int i = 0; i<listeners.size(); i++)
          ((TaskListener)listeners.get(i)).taskStarted(te);
    }

    protected void fireTaskFailed() {
            TaskEvent te = new TaskEvent(this, TaskEvent.TASK_FAILED);

            for (int i = 0; i<listeners.size(); i++)
              ((TaskListener)listeners.get(i)).taskFinished(te);
        }

    protected void fireTaskFinished() {
        TaskEvent te = new TaskEvent(this, TaskEvent.TASK_FINISHED);

        for (int i = 0; i<listeners.size(); i++)
          ((TaskListener)listeners.get(i)).taskFinished(te);
    }

    protected void fireSubtaskNameChanged() {
        TaskEvent te = new TaskEvent(this, TaskEvent.SUBTASK_NAME_CHANGED);

        for (int i = 0; i<listeners.size(); i++)
          ((TaskListener)listeners.get(i)).subtaskNameChanged(te);
    }


    /**
     * @return
     */
    public String getSubtaskName() {
        return subtaskName;
    }

    /**
     * @return
     */
    public String getTaskName() {
        return taskName;
    }

    /**
     * @param string
     */
    public void setSubtaskName(String string) {
        subtaskName = string;
        fireSubtaskNameChanged();
    }

    /**
     * @return
     */
    public int getFailed() {
        return failed;
    }

    /**
     * @return
     */
    public int getSuccessfull() {
        return successfull;
    }

}
