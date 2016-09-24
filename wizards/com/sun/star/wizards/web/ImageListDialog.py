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
import uno

from ..ui.UnoDialog2 import UnoDialog2
from ..ui.ImageList import ImageList
from ..common.HelpIds import HelpIds
from ..common.PropertyNames import PropertyNames
from ..common.IRenderer import IRenderer

from com.sun.star.awt import FontDescriptor
from com.sun.star.awt.PushButtonType import OK, CANCEL, HELP, STANDARD
from com.sun.star.awt import Size

'''
This class is a general implementation for a
dialog which displays a choice of images.
The model and the renderer are
still abstract in this class.
To use the class one should extend it,
in the constructor then set the imageList
properties (member name il) like image size, grid size,
model renderer aso, and then call "build".
This class uses a counter renderer which
gets an expression which contains %START %END %TOTAL and replaces
them with the actual numbers.
<BR/>
two buttons: "other" and "none" which are there
to enable choosing images that are not in the list and
"no image" respectivley, are optional, with default to True,
so dialogs which do not need those, should set the corresponding
members showDeselectButton and/or showOtherButton to false.
<br/>
the constructor should receive, among others, an Array of String resources
see constructor documentation for details.

@author rpiterman
'''

class ImageListDialog(UnoDialog2):
    RES_TITLE = 0
    RES_LABEL = 1
    RES_OK = 2
    RES_CANCEL = 3
    RES_HELP = 4
    RES_DESELECT = 5
    RES_OTHER = 6
    RES_COUNTER = 7
    START = "%START"
    END = "%END"
    TOTAL = "%TOTAL"

    '''
    @param xmsf
    @param resources_ a string array with the following strings :
    dialog title, label text, ok, cancel, help, deselect, other.
    <br/> if "deselect" and "other" are not displayed,
    the array can also be shorter. but if "other" is displayed
    and "deselect" not, both must be there :-(
    '''

    def __init__(self, xmsf, hid_, resources_):
        super(ImageListDialog, self).__init__(xmsf)
        self.hid = hid_
        self.cutFilename = 0
        self.showDeselectButton = True
        self.showOtherButton = True
        self.fontDescriptor1 = FontDescriptor()
        self.resources = resources_
        self.il = ImageList()
        self.il.counterRenderer = self.ARenderer(
            self.resources[ImageListDialog.RES_COUNTER])

    '''
    adds the controls to the dialog, depending on
    the size of the image list.
    This method should be called by subclasses after setting
    the il ImageList member properties
    '''

    def build(self):
        #set dialog properties...
        ilWidth = (self.il.imageSize.Width + self.il.gap.Width) \
            * self.il.cols + self.il.gap.Width
        ilHeight = (self.il.imageSize.Height + self.il.gap.Height) \
            * self.il.rows + self.il.gap.Height
        dialogWidth = 6 + ilWidth + 6 + 50 + 6
        dialogHeight = 3 + 16 + 3 + (ilHeight + 8 + 14) + 6
        uno.invoke(self.xDialogModel, "setPropertyValues",(
            ("Closeable",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                "Moveable",
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                "Title",
                PropertyNames.PROPERTY_WIDTH),
            (True, dialogHeight, HelpIds.getHelpIdString(self.hid), True,
                "imgDialog", 59, 24, 1,
                self.resources[ImageListDialog.RES_TITLE],dialogWidth)))
        #Set member- FontDescriptors...
        self.fontDescriptor1.Weight = 150
        PROPNAMES = ("DefaultButton",
            PropertyNames.PROPERTY_HEIGHT,
            PropertyNames.PROPERTY_HELPURL,
            PropertyNames.PROPERTY_LABEL,
            PropertyNames.PROPERTY_NAME,
            PropertyNames.PROPERTY_POSITION_X,
            PropertyNames.PROPERTY_POSITION_Y,
            "PushButtonType",
            PropertyNames.PROPERTY_TABINDEX,
            PropertyNames.PROPERTY_WIDTH)
        iButtonsX = 6 + ilWidth + 6
        self.btnOK = self.insertButton(
            "btnOK", None, PROPNAMES,
            (True, 14, HelpIds.getHelpIdString(self.hid + 3),
                self.resources[ImageListDialog.RES_OK], "btnOK",
                iButtonsX, 22, uno.Any("short", OK), 7, 50), self)
        self.btnCancel = self.insertButton(
            "btnCancel", None, PROPNAMES,
            (False, 14, HelpIds.getHelpIdString(self.hid + 4),
                self.resources[ImageListDialog.RES_CANCEL], "btnCancel",
                iButtonsX, 41, uno.Any("short", CANCEL), 8, 50), self)
        self.btnHelp = self.insertButton(
            "btnHelp", None, PROPNAMES,
            (False, 14, "", self.resources[ImageListDialog.RES_HELP],
                "CommandButton3", iButtonsX, 71,
                uno.Any("short", HELP), 9, 50), self)
        if self.showOtherButton:
            aux = 0
            if self.showDeselectButton:
                aux = 19
            otherY = 22 + ilHeight - 14 - aux
            self.btnOther = self.insertButton(
                "btnOther", "other", PROPNAMES,
                (False, 14, HelpIds.getHelpIdString(self.hid + 1),
                    self.resources[ImageListDialog.RES_OTHER], "btnOther",
                    iButtonsX, otherY,
                    uno.Any("short", STANDARD), 5, 50), self)

        if self.showDeselectButton:
            deselectY = 22 + ilHeight - 14
            self.btnDeselect = self.insertButton(
                "btnNoImage", "deselect", PROPNAMES,
                (False, 14, HelpIds.getHelpIdString(self.hid + 2),
                    self.resources[ImageListDialog.RES_DESELECT], "btnNoImage",
                    iButtonsX, deselectY,
                    uno.Any("short", STANDARD), 4, 50), self)

        self.il.step = 1
        self.il.pos = Size(6, 22)
        self.il.helpURL = self.hid + 5
        self.il.tabIndex = 1
        self.il.create(self)
        self.lblTitle = self.insertLabel("lblTitle",
            ("FontDescriptor",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_LABEL,
                PropertyNames.PROPERTY_NAME,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX,
                PropertyNames.PROPERTY_WIDTH),
            (self.fontDescriptor1, 8,
                self.resources[ImageListDialog.RES_LABEL],
                "lblTitle", 6, 6, 1, 4, 216))

    '''
    is called when the user clicks "none"
    '''

    def deselect(self):
        self.il.setSelected(-1)

    '''
    @return the currently elected object.
    '''

    def getSelected(self):
        return self.il.getSelectedObject()

    '''
    sets the currently selected object.
    @param obj the object (out of the model) to be selected.
    '''

    def setSelected(self, obj):
        self.il.setSelected(obj)
        self.il.showSelected()

    '''
    The counter renderer, which uses a template.
    The template replaces the Strings "%START", "%END" and
    "%TOTAL" with the respective values.
    @author rpiterman
    '''
    class ARenderer(IRenderer):

        '''
        @param aTemplate is a template for this renderer.
        The strings %START, %END ,%TOTAL will be replaced
        with the actual values.
        '''

        def __init__(self, aTemplate):
            self.template = aTemplate

        def render(self, counter):
            s = self.template.replace(ImageListDialog.START, "" + \
                str(counter.start))
            s = s.replace(ImageListDialog.END, "" + str(counter.end))
            s = s.replace(ImageListDialog.TOTAL, "" + str(counter.max))
            return s
