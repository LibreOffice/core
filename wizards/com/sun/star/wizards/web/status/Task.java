/*
 * Created on 17.10.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.status;

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
