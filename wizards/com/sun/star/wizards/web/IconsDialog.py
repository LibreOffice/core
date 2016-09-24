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

from .ImageListDialog import ImageListDialog
from .WWHID import HID_IS
from ..common.FileAccess import FileAccess
from ..common.ListModel import ListModel
from ..ui.ImageList import ImageList

from com.sun.star.awt import Size

'''
@author rpiterman
The dialog class for choosing an icon set.
This class simulates a model, though it does not functions really as one,
since it does not cast events.
It also implements the ImageList.ImageRenderer interface, to handle
its own objects.
'''

class IconsDialog(ImageListDialog, ImageList.IImageRenderer, ListModel):

    def __init__(self, xmsf, set_, resources):
        super(IconsDialog, self).__init__(xmsf, HID_IS,
            (resources.resIconsDialog,
                resources.resIconsDialogCaption,
                resources.resOK,
                resources.resCancel,
                resources.resHelp,
                resources.resDeselect,
                resources.resOther,
                resources.resCounter))
        self.htmlexpDirectory = FileAccess.getOfficePath2(
            xmsf, "Gallery", "share", "")
        self.icons = \
            ["firs", "prev", "next", "last", "nav", "text", "up", "down"]
        self.set = set_
        self.objects = range(self.set.getSize() * len(self.icons))

        self.il.listModel = self
        self.il.renderer = self
        self.il.rows = 4
        self.il.cols = 8
        self.il.ImageSize = Size (20, 20)
        self.il.showButtons = False
        self.il.rowSelect = True
        self.il.scaleImages = False
        self.showDeselectButton = True
        self.showOtherButton = False
        self.build()

    def getIconset(self):
        if self.getSelected() is None:
            return None
        else:
            selected = self.getSelected()
            value = int(selected / len(self.icons))
            return "iconset" + str(value)
            #return self.set.getKey(value)

    def setIconset(self, iconset):
        icon = self.set.getIndexOf(self.set.getElement(iconset)) * len(self.icons)
        aux = None
        if icon >=0:
            aux = self.objects[icon]
        self.setSelected(aux)

    def addListDataListener(self, listener):
        pass

    def removeListDataListener(self, listener):
        pass

    def getSize(self):
        return self.set.getSize() * len(self.icons)

    def getElementAt(self, arg0):
        return self.objects[arg0]

    def getImageUrls(self, listItem):
        i = listItem
        iset = self.getIconsetNum(i)
        icon = self.getIconNum(i)
        sRetUrls = list(range(2))
        sRetUrls[0] = self.htmlexpDirectory + "/htmlexpo/" \
            + self.getIconsetPref(iset) + self.icons[icon] + self.getIconsetPostfix(iset)
        sRetUrls[1] = sRetUrls[0]
        return sRetUrls

    def render(self, object):
        if object is None:
            return ""

        i = object
        iset = self.getIconsetNum(i)
        return self.getIconset1(iset).cp_Name

    def getIconsetNum(self, i):
        return int(i / len(self.icons))

    def getIconNum(self, i):
        return i % len(self.icons)

    def getIconsetPref(self, iconset):
        return self.getIconset1(iconset).cp_FNPrefix

    def getIconsetPostfix(self, iconset):
        return self.getIconset1(iconset).cp_FNPostfix

    def getIconset1(self, i):
        return self.set.getElementAt(i)
