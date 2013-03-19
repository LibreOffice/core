#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .

# used for debugging.
class LogTaskListener(TaskListener, ErrorHandler):

    out

    def __init__(os):
        out = os

    def __init__():
        self.__init__(System.out)

    # @see com.sun.star.wizards.web.status.TaskListener#taskStarted(com.sun.star.wizards.web.status.TaskEvent)
    def taskStarted(te):
        out.println("TASK " + te.getTask().getTaskName() + " STARTED.")

    #@see com.sun.star.wizards.web.status.TaskListener#taskFinished(com.sun.star.wizards.web.status.TaskEvent)
    def taskFinished(te):
        out.println("TASK " + te.getTask().getTaskName() + " FINISHED: " + te.getTask().getSuccessfull() + "/" + te.getTask().getMax() + "Succeeded.")

    #@see com.sun.star.wizards.web.status.TaskListener#taskStatusChanged(com.sun.star.wizards.web.status.TaskEvent)
    def taskStatusChanged(te)
        out.println("TASK " + te.getTask().getTaskName() + " status : " + te.getTask().getSuccessfull() + "(+" + te.getTask().getFailed() + ")/" + te.getTask().getMax())

    #@see com.sun.star.wizards.web.status.TaskListener#subtaskNameChanged(com.sun.star.wizards.web.status.TaskEvent)
    def subtaskNameChanged(te):
        out.println("SUBTASK Name:" + te.getTask().getSubtaskName())

    # @see com.sun.star.wizards.web.status.ErrorReporter#error(java.lang.Exception, java.lang.Object, java.lang.String)
    def error(ex, arg, ix, i):
        print (PropertyNames.EMPTY_STRING + arg + "//" + ix + "//Exception: " + ex.getLocalizedMessage())
        ex.printStackTrace()
        return True;
