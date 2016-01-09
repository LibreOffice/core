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
#
from abc import abstractmethod

from .ErrorHandler import ErrorHandler
from ..common.SystemDialog import SystemDialog

from com.sun.star.awt.VclWindowPeerAttribute import OK, DEF_OK, OK_CANCEL, DEF_CANCEL
from com.sun.star.awt.VclWindowPeerAttribute import YES_NO, DEF_NO, DEF_YES

#import com.sun.star.awt.VclWindowPeerAttribute


# An abstract implementation of ErrorHandler, which
# uses a renderer method geMessageFor(Exception, Object, int, int)
# (in this class still abstract...)
# to render the errors, and displays
# error messages.
class AbstractErrorHandler(ErrorHandler):

    xmsf = None
    peer = None

    def __init__(self, xmsf, peer_):
        self.xmsf = xmsf
        self.peer = peer_

    # Implementation of ErrorHandler:
    # shows a message box with the rendered error.
    # @param arg identifies the error. This object is passed to the render method
    # which returns the right error message.
    # @return true/false for continue/abort.
    def error(self, ex, arg, ix, errorType):
        if (errorType == ErrorHandler.ERROR_FATAL):
                return not self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_PROCESS_FATAL):
                return not self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_NORMAL_ABORT):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_NORMAL_IGNORE):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_QUESTION_CANCEL):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_QUESTION_OK):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_QUESTION_NO):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_QUESTION_YES):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_WARNING):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        elif (errorType == ErrorHandler.ERROR_MESSAGE):
                return self.showMessage(self.getMessageFor(ex, arg, ix, errorType), errorType)
        raise ValueError("unknown error type")

    # @deprecated
    # @param message
    # @param errorType
    # @return true if the ok/yes button is clicked, false otherwise.
    def showMessage(self, message, errorType):
        return self.showMessage1(self.xmsf, self.peer, message, errorType)

    # display a message
    # @deprecated
    # @param xmsf
    # @param message the message to display
    # @param errorType an int constant from the ErrorHandler interface.
    # @return
    @classmethod
    def showMessage1(self, xmsf, peer, message, errorType):
        serviceName = self.getServiceNameFor(errorType)
        attribute = self.getAttributeFor(errorType)
        b = SystemDialog.showMessageBox(xmsf, serviceName, attribute, message, peer)
        return b == self.getTrueFor(errorType)

    @classmethod
    def showMessage2(self, xmsf, peer, message, dialogType, buttons, defaultButton, returnTrueOn):
        b = SystemDialog.showMessageBox(xmsf, dialogType, defaultButton + buttons, message, peer, )
        return b == returnTrueOn


    # normally ok(1) is the value for true.
    # but a question dialog may use yes. so i use this method
    # for each error type to get its type of "true" value.
    # @param errorType
    # @return
    @classmethod
    def getTrueFor(self, errorType):
        if (errorType in (ErrorHandler.ERROR_FATAL, ErrorHandler.ERROR_PROCESS_FATAL,
                          ErrorHandler.ERROR_NORMAL_ABORT, ErrorHandler.ERROR_NORMAL_IGNORE,
                          ErrorHandler.ERROR_QUESTION_CANCEL, ErrorHandler.ERROR_QUESTION_OK)):
                return 1
        elif (errorType in (ErrorHandler.ERROR_QUESTION_NO, ErrorHandler.ERROR_QUESTION_YES)):
                return 2
        elif (errorType in (ErrorHandler.ERROR_WARNING, ErrorHandler.ERROR_MESSAGE)):
                return 1
        raise ValueError("unknown error type")


    # @param errorType
    # @return the Uno attributes for each error type.
    @classmethod
    def getAttributeFor(self, errorType):
        if (errorType == ErrorHandler.ERROR_FATAL):
                return OK
        elif (errorType == ErrorHandler.ERROR_PROCESS_FATAL):
                return OK
        elif (errorType == ErrorHandler.ERROR_NORMAL_ABORT):
                return OK_CANCEL + DEF_CANCEL
        elif (errorType == ErrorHandler.ERROR_NORMAL_IGNORE):
                return OK_CANCEL + DEF_OK
        elif (errorType == ErrorHandler.ERROR_QUESTION_CANCEL):
                return OK_CANCEL + DEF_CANCEL
        elif (errorType == ErrorHandler.ERROR_QUESTION_OK):
                return OK_CANCEL + DEF_OK
        elif (errorType == ErrorHandler.ERROR_QUESTION_NO):
                return YES_NO + DEF_NO
        elif (errorType == ErrorHandler.ERROR_QUESTION_YES):
                return YES_NO + DEF_YES
        elif (errorType == ErrorHandler.ERROR_WARNING):
                return OK
        elif (errorType == ErrorHandler.ERROR_MESSAGE):
                return OK
        raise ValueError("unknown error type")

    # @deprecated
    # @param errorType
    # @return the uno service name for each error type
    @classmethod
    def getServiceNameFor(self, errorType):
        if (errorType == ErrorHandler.ERROR_FATAL):
                return "errorbox"
        elif (errorType == ErrorHandler.ERROR_PROCESS_FATAL):
                return "errorbox"
        elif (errorType == ErrorHandler.ERROR_NORMAL_ABORT):
                return "errorbox"
        elif (errorType == ErrorHandler.ERROR_NORMAL_IGNORE):
                return "warningbox"
        elif (errorType == ErrorHandler.ERROR_QUESTION_CANCEL):
                return "querybox"
        elif (errorType == ErrorHandler.ERROR_QUESTION_OK):
                return "querybox"
        elif (errorType == ErrorHandler.ERROR_QUESTION_NO):
                return "querybox"
        elif (errorType == ErrorHandler.ERROR_QUESTION_YES):
                return "querybox"
        elif (errorType == ErrorHandler.ERROR_WARNING):
                return "warningbox"
        elif (errorType == ErrorHandler.ERROR_MESSAGE):
                return "infobox"
        raise ValueError("unknown error type")

    # renders the error
    # @param ex the exception
    # @param arg a free argument
    # @param ix a free argument
    # @param type the error type (from the int constants
    # in ErrorHandler interface)
    # @return a Strings which will be displayed in the message box,
    # and which describes the error, and the needed action from the user.
    @abstractmethod
    def getMessageFor(self, ex, arg, ix, errorType):
        pass



