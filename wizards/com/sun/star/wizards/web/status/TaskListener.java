/*
 * Created on 04.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.status;

import java.util.EventListener;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public interface TaskListener extends EventListener {

    public void taskStarted(TaskEvent te);

    public void taskFinished(TaskEvent te);

    /**
     * is called when the status of the task has advanced.
     * @param te
     */
    public void taskStatusChanged(TaskEvent te);

    public void subtaskNameChanged(TaskEvent te);

}
