/*
 * Created on 04.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.web.status;

import java.util.EventObject;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class TaskEvent extends EventObject {

    public static final int TASK_STARTED = 1;
    public static final int TASK_FINISHED = 2;
    public static final int TASK_STATUS_CHANGED = 3;
    public static final int SUBTASK_NAME_CHANGED = 4;

    private int type;

    /**
     * general constructor-
     * @param source
     * @param type_
     * @param max_
     * @param success_
     * @param failed_
     */
    public TaskEvent(Task source, int type_) {
        super(source);
        type = type_;
    }

    public Task getTask() {
        return (Task)getSource();
    }

}
