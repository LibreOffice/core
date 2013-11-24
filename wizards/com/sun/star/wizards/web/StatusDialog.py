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
import uno
import traceback

from ..ui.UnoDialog2 import UnoDialog2
from ..common.PropertyNames import PropertyNames

# Note on the argument resource:
# This should be a String array containing the followin strings, in the
# following order:
# dialog title, cancel, close, counter prefix, counter midfix, counter postfix
class StatusDialog(UnoDialog2):

    STANDARD_WIDTH = 240
    lblTaskName = None
    lblCounter = None
    progressBar = None
    btnCancel = None
    finished = False
    enableBreak = False
    closeOnFinish = True
    renderer = None
    finishedMethod = None

    def __init__(self, xmsf, width, taskName, displayCount, resources, hid):
        super(StatusDialog, self).__init__(xmsf)

        self.res = resources
        if not (len(self.res) == 6):
            # display a close button?
            raise ValueError("The resources argument should contain 6 Strings, see doc on constructor.")

        b = not (self.enableBreak or self.closeOnFinish)

        uno.invoke(self.xDialogModel, "setPropertyValues", (
                ("Closeable",
                 PropertyNames.PROPERTY_HEIGHT,
                 PropertyNames.PROPERTY_HELPURL, "Moveable",
                 PropertyNames.PROPERTY_NAME,
                 PropertyNames.PROPERTY_POSITION_X,
                 PropertyNames.PROPERTY_POSITION_Y,
                 PropertyNames.PROPERTY_STEP, "Title",
                 PropertyNames.PROPERTY_WIDTH),
                (False, 6 + 25 + (27 if b else 7), hid, True, "StatusDialog", 102, 52, 0,
                 self.res[0], width)))

        tabstop = 1

        self.lblTaskName = self.insertLabel("lblTask",
                (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (8, taskName, 6, 6, tabstop, int(width * 2 / 3)))
        tabstop += 1
        self.lblCounter = self.insertLabel("lblCounter",
                (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (8, "", int(width * 2 / 3), 6, tabstop, int(width / 3) - 4))
        tabstop += 1
        self.progressBar = self.insertProgressBar("progress",
                (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                (10, 6, 16, tabstop, width - 12))
        tabstop += 1

        if b:
            self.btnCancel = self.insertButton("btnCancel", "performCancel", self,
                    (PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH),
                    (14, self.res[1], width / 2 - 20, 6 + 25 + 7, tabstop, 40))
        tabstop += 1

    def initProgressBar(self, t):
        self.progressBar.setRange(0, t.getMax())
        self.progressBar.setValue(0)

    def setStatus(self, status):
        if self.finished:
            return
        self.progressBar.setValue(status)
        #self.xReschedule.reschedule()

    def setLabel(self, s):
        self.lblTaskName.Text = s
        #self.xReschedule.reschedule()

    # change the max property of the status bar
    # @param max
    def setMax(self, maximum):
        if self.finished:
            return
        self.progressBar.Model.ProgressValueMax = maximum

    # initialize the status bar according
    # to the given event.
    def taskStarted(self, te):
        self.finished = False
        self.initProgressBar(te.getTask())

    #closes the dialog.
    def taskFinished(self, te):
        self.finished = True
        if self.closeOnFinish:
            self.parent.xUnoDialog.setEnable(True)
            try:
                self.xUnoDialog.setVisible(False)
                self.xUnoDialog.dispose()
            except Exception:
                traceback.print_exc()
        else:
            self.btnCancel.Model.Label = self.res[2]

    # changes the status display
    def taskStatusChanged(self, te):
        self.setMax(te.getTask().getMax())
        self.setStatus(te.getTask().getStatus())

    # changes the displayed text.
    # A renderer is used to render
    #the task's subtask name to a resource string.
    def subtaskNameChanged(self, te):
        if self.renderer is not None:
            self.setLabel(self.renderer.render(te.getTask().getSubtaskName()))

    # displays the status dialog
    # @param parent_ the parent dialog
    # @param task what to do
    def execute(self, parent_, task, title):
        try:
            self.parent = parent_
            self.xDialogModel.Title = title
            try:
                # TODO change this to another execute dialog method.
                task.addTaskListener(self)
                self.setMax(10)
                self.setStatus(0)
                self.setLabel(task.getSubtaskName())
                self.parent.xUnoDialog.setEnable(False)
                self.setVisible(self.parent)
                if self.finishedMethod is not None:
                    self.finishedMethod.invoke()
            except Exception:
                traceback.print_exc()
        except Exception:
                traceback.print_exc()

    # not supported !
    def performCancel(self):
        #TODO - implement a thread thing here...
        self.xUnoDialog.setVisible(False)

    # @return the subTask renderer object
    def getRenderer(self):
        return self.renderer

    # @param renderer
    def setRenderer(self, renderer):
        self.renderer = renderer

    #sets a method to be invoced when the
    def setFinishedMethod(self, mi):
        self.finishedMethod = mi
