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
import traceback
from abc import abstractmethod

from threading import RLock
from .PeerConfig import PeerConfig
from .event.CommonListener import OMouseListenerProcAdapter
from ..common.PropertyNames import PropertyNames
from ..common.HelpIds import HelpIds
from ..common.IRenderer import IRenderer
from .event.ListDataListener import ListDataListener

from com.sun.star.awt import Size

def synchronized(lock):
    ''' Synchronization decorator. '''
    def wrap(f):
        def newFunction(*args, **kw):
            lock.acquire()
            try:
                return f(*args, **kw)
            finally:
                lock.release()
        return newFunction
    return wrap

class ImageList(ListDataListener):
    NO_BORDER = 0
    BACKGROUND_COLOR = 16777216
    HIDE_PAGE = 99
    TRANSPARENT = -1
    LINE_HEIGHT = 8
    IMAGE_PROPS = ("Border", "BackgroundColor",
        PropertyNames.PROPERTY_HEIGHT,
        PropertyNames.PROPERTY_HELPURL,
        PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y, "ScaleImage",
        PropertyNames.PROPERTY_STEP,
        PropertyNames.PROPERTY_TABINDEX, "Tabstop",
        PropertyNames.PROPERTY_WIDTH)
    MOVE_SELECTION = (PropertyNames.PROPERTY_POSITION_X,
        PropertyNames.PROPERTY_POSITION_Y,
        PropertyNames.PROPERTY_STEP)
    lock = RLock()
    listModel = None
    btnNext = None
    btnBack = None

    def __init__(self):
        self.benabled = True
        self.gap = Size(4, 4)
        self.cols = 4
        self.rows = 3
        self.imageSize = Size(20, 20)
        self.selectionGap = Size(2, 2)
        self.showButtons = True
        self.refreshOverNull = True
        self.imageTextLines = 1
        self.rowSelect = False
        self.scaleImages = True
        self.name = "il"
        self.selected = -1
        self.pageStart = 0
        self.helpURL = 0
        self.renderer = None
        self.counterRenderer = self.SimpleCounterRenderer()
        self.MOVE_SELECTION_VALS = list(range(3))
        self.itemListenerList = None

    def onMousePressed(self, event):
        self.focus(self.getImageIndexFor(self.selected))


    def create(self, dialog):
        self.oUnoDialog = dialog
        self.dialogModel = dialog.xDialogModel
        imageTextHeight = self.imageTextLines * ImageList.LINE_HEIGHT
        opeerConfig = PeerConfig(dialog)
        self.MOVE_SELECTION_VALS[2] = self.step
        imgContainer = dialog.insertImage(
            self.name + "lblContainer",
            ("BackgroundColor", "Border",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_WIDTH),
            (ImageList.BACKGROUND_COLOR, 1,
                (self.imageSize.Height + self.gap.Height) \
                    * self.rows + self.gap.Height + imageTextHeight + 1,
                self.pos.Width, self.pos.Height, self.step,
                (self.imageSize.Width + self.gap.Width) \
                    * self.cols + self.gap.Width))
        opeerConfig.setPeerProperties(imgContainer,"MouseTransparent", True)

        if self.rowSelect:
            selectionWidth = (self.imageSize.Width + self.gap.Width) \
                * self.cols - self.gap.Width + (self.selectionGap.Width * 2)
        else:
            selectionWidth = self.imageSize.Width + (self.selectionGap.Width * 2)

        self.grbxSelectedImage = dialog.insertLabel(
            self.name + "_grbxSelected",
            ("BackgroundColor", "Border",
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP, "Tabstop",
                PropertyNames.PROPERTY_WIDTH),
            (ImageList.TRANSPARENT, 1,
                self.imageSize.Height + (self.selectionGap.Height * 2),
                0, #posx
                0, #posy
                self.step, True, selectionWidth))
        xWindow = self.grbxSelectedImage
        xWindow.addMouseListener(OMouseListenerProcAdapter(self.onMousePressed))
        pNames1 = (PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_TABINDEX, "Tabstop",
                PropertyNames.PROPERTY_WIDTH)
        self.lblImageText = dialog.insertLabel(
            self.name + "_imageText", pNames1,
            (imageTextHeight, "", self.pos.Width + 1,
                self.pos.Height + (self.imageSize.Height + self.gap.Height) \
                    * self.rows + self.gap.Height, self.step, 0, False,
                self.cols * (self.imageSize.Width + self.gap.Width) \
                    + self.gap.Width - 2))
        if self.showButtons:
            ImageList.btnBack = dialog.insertButton(
                self.name + "_btnBack", "prevPage", pNames1,
                (14, HelpIds.getHelpIdString((self.helpURL + 1)),
                self.pos.Width, self.pos.Height + \
                    (self.imageSize.Height + self.gap.Height) * \
                    self.rows + self.gap.Height + imageTextHeight + 1,
                self.step, self.tabIndex + 1, True, 14), self)
            ImageList.btnNext = dialog.insertButton(
                self.name + "_btnNext", "nextPage", pNames1,
                (14, HelpIds.getHelpIdString((self.helpURL + 1)),
                self.pos.Width + (self.imageSize.Width + self.gap.Width) * \
                    self.cols + self.gap.Width - 14 + 1,
                self.pos.Height + (self.imageSize.Height + self.gap.Height) \
                    * self.rows + self.gap.Height + imageTextHeight + 1,
                self.step, self.tabIndex + 2, True, 14), self)
            self.lblCounter = dialog.insertLabel(
                self.name + "_lblCounter", pNames1,
                (ImageList.LINE_HEIGHT, "", self.pos.Width + 14 + 1,int(
                    self.pos.Height + (self.imageSize.Height + self.gap.Height) \
                        * self.rows + self.gap.Height + imageTextHeight + \
                        ((14 - ImageList.LINE_HEIGHT) / 2)),
                    self.step, 0, False, self.cols * \
                        (self.imageSize.Width + self.gap.Width) + \
                        self.gap.Width - 2 * 14 - 1))
            self.lblCounter.Model.Align = 1
            ImageList.btnBack.Model.Label = "<"
            ImageList.btnNext.Model.Label = ">"

        self.m_aImages = [None] * (self.rows * self.cols)

        for r in range(self.rows):
            for c in range(self.cols):
                self.m_aImages[r * self.cols + c] = self.createImage(dialog, r, c)

        self.refreshImages()
        self.listModel.addListDataListener(self)

    #used for optimization

    def createImage(self, dialog, _row, _col):
        imageName = self.name + "_image" + str(_row * self.cols + _col)
        image = dialog.insertImage(
            imageName, ImageList.IMAGE_PROPS,
            (ImageList.NO_BORDER,
             ImageList.BACKGROUND_COLOR,
             self.imageSize.Height,
             HelpIds.getHelpIdString(self.helpURL + 1),
             self.getImagePosX(_col), self.getImagePosY(_row),
             self.scaleImages,
             self.step,
             self.tabIndex,
             False,
             self.imageSize.Width))
        #COMMENTED
        image.addMouseListener(OMouseListenerProcAdapter(self.mousePressed))
        image.addKeyListener(None)
        return image

    def getImagePosX(self, col):
        return self.pos.Width + col * \
            (self.imageSize.Width + self.gap.Width) + self.gap.Width

    def getImagePosY(self, row):
        return self.pos.Height + row * \
            (self.imageSize.Height + self.gap.Height) + self.gap.Height

    def refreshImages(self):
        if self.showButtons:
            self.refreshCounterText()

        self.hideSelection()
        if self.refreshOverNull:
            i = 0
            for i in self.m_aImages:
                i.Visible =  False

        focusable = True
        for index, item in enumerate(self.m_aImages):
            oResources = self.renderer.getImageUrls(self.getObjectFor(index))
            if oResources is not None:
                if len(oResources) == 1:
                    item.Model.ImageURL = oResources[0]
                elif len(oResources) == 2:
                    self.oUnoDialog.getPeerConfiguration().setImageUrl(
                        item.Model, oResources[0], oResources[1])

                item.Model.Tabstop = bool(focusable)
                if self.refreshOverNull:
                    item.Visible =  True

                focusable = False

        self.refreshSelection()

    def refreshCounterText(self):
        self.lblCounter.Model.Label = self.counterRenderer.render(
            self.Counter (self.pageStart + 1, self.pageEnd(),
                          self.listModel.getSize()))

    def pageEnd(self):
        i = self.pageStart + self.cols * self.rows
        if i > self.listModel.getSize() - 1:
            return self.listModel.getSize()
        else:
            return i

    def refreshSelection(self):
        if self.selected < self.pageStart or \
                self.selected >= (self.pageStart + self.rows * self.cols):
            self.hideSelection()
        else:
            self.moveSelection(self.getImageIndexFor(self.selected))

    def hideSelection(self):
        self.grbxSelectedImage.Model.Step = ImageList.HIDE_PAGE
        self.grbxSelectedImage.Visible = False

    '''
    Utility field holding list of ItemListeners.
    '''

    def moveSelection(self, image):
        self.grbxSelectedImage.Visible = False
        row = int(image / self.cols)
        if self.rowSelect:
            col = 0
        else:
            col = image - (row * self.cols)

        self.MOVE_SELECTION_VALS[0] = \
            int(self.getImagePosX(col) - self.selectionGap.Width)
        self.MOVE_SELECTION_VALS[1] = \
            int(self.getImagePosY(row) - self.selectionGap.Height)
        uno.invoke(self.grbxSelectedImage.Model, "setPropertyValues",
                   ((ImageList.MOVE_SELECTION),
                    (tuple(self.MOVE_SELECTION_VALS))))
        if (self.dialogModel.Step == self.step):
            self.grbxSelectedImage.Visible = True
            #now focus...

        for index,item in enumerate(self.m_aImages):
            if index != image:
                self.defocus(index)
            else:
                self.m_aImages[image].Model.Tabstop = True

    '''
    @param i
    @return the Object in the list model corresponding to the given image index
    '''

    def getObjectFor(self, i):
        ii = self.getIndexFor(i)
        if self.listModel.getSize() <= ii:
            return None
        else:
            return self.listModel.getElementAt(ii)

    '''
    @param i
    @return the index in the listModel for the given image index.
    '''

    def getIndexFor(self, i):
        return self.pageStart + i

    def getImageIndexFor(self, i):
        return i - self.pageStart

    def intervalAdded(self, event):
        if event.getIndex0() <= self.selected:
            if event.getIndex1() <= self.selected:
                self.selected += event.getIndex1() - event.getIndex0() + 1

        if event.getIndex0() < self.pageStart or \
                event.getIndex1() < (self.pageStart + self.rows + self.cols):
            self.refreshImages()

    '''
    Registers ItemListener to receive events.
    @param listener The listener to register.
    '''

    @synchronized(lock)
    def addItemListener(self, listener):
        if self.itemListenerList is None:
            self.itemListenerList = []
        self.itemListenerList.append(listener)

    '''
    Removes ItemListener from the list of listeners.
    @param listener The listener to remove.
    '''

    @synchronized(lock)
    def removeItemListener(self, listener):
        if self.itemListenerList is not None:
            self.itemListenerList.remove(listener)

    '''
    Notifies all registered listeners about the event.
    @param event The event to be fired
    '''

    def fireItemSelected(self):
        if self.itemListenerList is None:
            return
        auxlist = list(self.itemListenerList)
        for i in auxlist:
            i.itemStateChanged(None)

    def getSelected(self):
        return self.selected

    def setSelected(self, _object):
        if (isinstance (_object, int)):
            self.setSelected1(_object)
        elif (_object is None or _object is ""):
            self.setSelected1(-1)
        else:
            i = 0
            while (i < self.listModel.getSize()):
                item = self.listModel.getElementAt(i)
                if item == _object:
                    self.setSelected1(i)
                    return
                i += 1

    def setSelected1(self, index):
        if self.rowSelect and (index >= 0):
            index = int(index / self.cols) * self.cols

        if self.selected == index:
            return

        self.selected = index
        self.refreshImageText()
        self.refreshSelection()
        self.fireItemSelected()


    def refreshImageText(self):
        if self.selected >= 0:
            try:
                item = self.listModel.getElementAt(self.selected)
            except IndexError:
                item = None
                traceback.print_exc()
        else:
            item = None

        self.lblImageText.Model.Label = " " + self.renderer.render(item)

    def nextPage(self):
        if self.pageStart < self.listModel.getSize() - self.rows * self.cols:
            self.setPageStart(self.pageStart + self.rows * self.cols)

    def prevPage(self):
        if self.pageStart == 0:
            return

        i = self.pageStart - self.rows * self.cols
        if i < 0:
            i = 0

        self.setPageStart(i)

    def setPageStart(self, i):
        if (i == self.pageStart):
            return
        self.pageStart = i
        self.enableButtons()
        self.refreshImages()

    def enableButtons(self):
        self.enable(
            ImageList.btnNext,
            bool(self.pageStart + self.rows * self.cols < self.listModel.getSize()))
        self.enable(ImageList.btnBack, bool(self.pageStart > 0))

    def enable(self, control, enable):
        control.Model.Enabled = enable

    def setBorder(self, control, border):
        uno.invoke(control.Model, "setPropertyValues",(("Border"), (enable)))

    def getImageFromEvent(self, event):
        image = (event).Source
        controlName = image.Model.Name
        return int(controlName[6 + len(self.name):])

    def mousePressed(self, event):
        image = self.getImageFromEvent(event)
        index = self.getIndexFor(image)
        if index < self.listModel.getSize():
            self.focus(image)
            self.setSelected(index)

    def getSelectedObjects(self):
        return[self.listModel.getElementAt(self.selected)]

    class IImageRenderer(IRenderer):

         # @param listItem
         # @return two resource ids for an image referenced in the imaglist resourcefile of the
         # wizards project; The second one of them is designed to be used for High Contrast Mode.
        @abstractmethod
        def getImageUrls(self, listItem):
            pass

    class SimpleCounterRenderer(IRenderer):

        def render(self, counter):
            return \
                "" + (counter).start + ".." + (counter).end + "/" + (counter).max

    class Counter(object):

        def __init__(self, start_, end_, max_):
            self.start = start_
            self.end = end_
            self.max = max_

    def getSelectedObject(self):
        if self.selected >= 0:
            try:
                return self.listModel.getElementAt(self.selected)
            except IndexError:
                traceback.print_exc()
        return None

    def showSelected(self):
        oldPageStart = self.pageStart
        if self.selected != -1:
            self.pageStart = \
                int((self.selected / len(self.m_aImages)) * len(self.m_aImages))

        if oldPageStart != self.pageStart:
            self.enableButtons()
            self.refreshImages()

    def keyPressed(self, ke):
        image = self.getImageFromEvent(ke)
        r = image / self.cols
        c = image - (r * self.cols)
        d = self.getKeyMove(ke, r, c)
        newImage = image + d
        if newImage == image:
            return

        if self.isFocusable(newImage):
            self.changeFocus(image, newImage)

    def isFocusable(self, image):
        return (image >= 0) and \
            (self.getIndexFor(image) < self.listModel.getSize())

    def changeFocus(self, oldFocusImage, newFocusImage):
        self.focus(newFocusImage)
        self.defocus(oldFocusImage)

    def select(self, ke):
        self.setSelected(self.getIndexFor(self.getImageFromEvent(ke)))

    def focus(self, image):
        self.m_aImages[image].Model.Tabstop = True
        xWindow = self.m_aImages[image]
        xWindow.setFocus()

    def defocus(self, image):
        self.m_aImages[image].Model.Tabstop = False

    '''jump to the given item (display the screen
    that contains the given item).
    @param i
    '''

    def display(self, i):
        isAux = (self.cols * self.rows)
        ps = (self.listModel.getSize() / isAux) * isAux
        self.setPageStart(ps)

    def setenabled(self, b):
        i = 0
        while i < len(self.m_aImages):
            UnoDialog2.setEnabled(self.m_aImages[i], b)
            i += 1
        UnoDialog2.setEnabled(self.grbxSelectedImage, b)
        UnoDialog2.setEnabled(self.lblImageText, b)
        if self.showButtons:
            UnoDialog2.setEnabled(ImageList.btnBack, b)
            UnoDialog2.setEnabled(ImageList.btnNext, b)
            UnoDialog2.setEnabled(self.lblCounter, b)

        self.benabled = b
