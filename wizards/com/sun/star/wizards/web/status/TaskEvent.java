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
