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

from com.sun.star.awt.VclWindowPeerAttribute import OK, DEF_OK, OK_CANCEL, DEF_CANCEL
from com.sun.star.awt.VclWindowPeerAttribute import YES_NO, DEF_NO

class ErrorHandler:

    MESSAGE_INFO = "infobox"
    MESSAGE_QUESTION = "querybox"
    MESSAGE_ERROR = "errorbox"
    MESSAGE_WARNING = "warningbox"
    BUTTONS_OK = OK
    BUTTONS_OK_CANCEL = OK_CANCEL
    BUTTONS_YES_NO = YES_NO
    RESULT_CANCEL = 0
    RESULT_OK = 1
    RESULT_YES = 2
    DEF_OK = DEF_OK
    DEF_NO = DEF_NO
    DEF_CANCEL = DEF_CANCEL

    #Error type for fatal errors which should abort application
    # execution. Should actually never be used :-)
    ERROR_FATAL = 0
    # An Error type for errors which should stop the current process.
    ERROR_PROCESS_FATAL = 1
    # An Error type for errors to which the user can choose, whether
    # to continue or to abort the current process.
    # default is abort.
    ERROR_NORMAL_ABORT = 2
    # An Error type for errors to which the user can choose, whether
    # to continue or to abort the current process.
    # default is continue.
    ERROR_NORMAL_IGNORE = 3
    # An error type for warnings which requires user interaction.
    # (a question :-) )
    # Default is abort (cancel).
    ERROR_QUESTION_CANCEL = 4
    # An error type for warnings which requires user interaction
    # (a question :-) )
    # Default is to continue (ok).
    ERROR_QUESTION_OK = 5
    # An error type for warnings which requires user interaction.
    # (a question :-) )
    # Default is abort (No).
    ERROR_QUESTION_NO = 6
    # An error type for warnings which requires user interaction
    # (a question :-) )
    # Default is to continue (Yes).
    ERROR_QUESTION_YES = 7
    # An error type which is just a warning...
    ERROR_WARNING = 8
    # An error type which just tells the user something
    # ( like "you look tired! you should take a bath! and so on)
    ERROR_MESSAGE = 9

    # @param ex the exception that accured
    # @param arg an object as help for recognizing the exception
    # @param ix an integer which helps for detailed recognizing of the exception
    # @param errorType one of the int constants defined by this Interface
    # @return true if the execution should continue, false if it should stop.
    def error(self, ex, arg, ix, errorType):
        pass

