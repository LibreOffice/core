/*
 * Created on 04.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web;

import java.io.PrintStream;

import com.sun.star.wizards.ui.event.*;

/**
 * used for debugging.
 * @author rpiterman
 */
public class LogTaskListener implements TaskListener, ErrorHandler {

    private PrintStream out;

    public LogTaskListener(PrintStream os) {
        out = os;
    }

    public LogTaskListener() {
        this(System.out);
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskStarted(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskStarted(TaskEvent te) {
        out.println("TASK " + te.getTask().getTaskName()
         + " STARTED.");

    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskFinished(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskFinished(TaskEvent te) {
        out.println("TASK " + te.getTask().getTaskName()
         + " FINISHED: " + te.getTask().getSuccessfull() + "/" + te.getTask().getMax() + "Succeeded.");

    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#taskStatusChanged(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void taskStatusChanged(TaskEvent te) {
        out.println("TASK " + te.getTask().getTaskName()
         + " status : " + te.getTask().getSuccessfull() + "(+" + te.getTask().getFailed() + ")/" + te.getTask().getMax());
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.TaskListener#subtaskNameChanged(com.sun.star.wizards.web.status.TaskEvent)
     */
    public void subtaskNameChanged(TaskEvent te) {
        out.println("SUBTASK Name:" + te.getTask().getSubtaskName());
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.status.ErrorReporter#error(java.lang.Exception, java.lang.Object, java.lang.String)
     */
    public boolean error(Exception ex, Object arg, int ix, int i) {
        System.out.println("" + arg + "//" + ix + "//Exception: " + ex.getLocalizedMessage());
        ex.printStackTrace();
        return true;
    }



}
