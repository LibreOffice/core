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
package com.sun.star.wizards.web.status;

import java.util.EventObject;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
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
     * @param max_
     * @param success_
     * @param failed_
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
