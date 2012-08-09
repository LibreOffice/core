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
from threading import RLock
from wizards.ui.PeerConfig import PeerConfig
from wizards.common.PropertyNames import *
from wizards.common.HelpIds import HelpIds
from wizards.common.Helper import Helper

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

class ImageList(object):
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
    listModel = []
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
        self.counterRenderer = self.SimpleCounterRenderer()
        self.MOVE_SELECTION_VALS = range(3)

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
                self.pos.Width,self.pos.Height, self.step,
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
                0, #height
                0, #posx
                0, #posy
                self.step, True, selectionWidth))
        xWindow = self.grbxSelectedImage
        xWindow.addMouseListener(None)
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
                (ImageList.LINE_HEIGHT, "", self.pos.Width + 14 + 1,
                    self.pos.Height + (self.imageSize.Height + self.gap.Height) \
                        * self.rows + self.gap.Height + imageTextHeight + \
                        ((14 - ImageList.LINE_HEIGHT) / 2),
                    self.step, 0, False, self.cols * \
                        (self.imageSize.Width + self.gap.Width) + \
                        self.gap.Width - 2 * 14 - 1))
            Helper.setUnoPropertyValue(self.lblCounter.Model, "Align", 1)
            Helper.setUnoPropertyValue(ImageList.btnBack.Model,
                PropertyNames.PROPERTY_LABEL, "<")
            Helper.setUnoPropertyValue(ImageList.btnNext.Model,
                PropertyNames.PROPERTY_LABEL, ">")

        self.m_aImages = [None] * (self.rows * self.cols)

        for r in xrange(self.rows):
            for c in xrange(self.cols):
                self.m_aImages[r * self.cols + c] = self.createImage(dialog, r, c)

        self.refreshImages()
        #COMMENTED
        #ImageList.listModel.addListDataListener(None)

    #used for optimization

    def createImage(self, dialog, _row, _col):
        imageName = self.name + "_image" + str(_row * self.cols + _col)
        image = dialog.insertImage(
            imageName, ImageList.IMAGE_PROPS,
            (ImageList.NO_BORDER, ImageList.BACKGROUND_COLOR,
                self.imageSize.Height,
                HelpIds.getHelpIdString(self.helpURL + 1),
                self.getImagePosX(_col), self.getImagePosY(_row),
                self.scaleImages, self.step, self.tabIndex,
                False, self.imageSize.Width))
        #COMMENTED
        image.addMouseListener(None)
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
            #COMMENTED
            oResources = None #self.renderer.getImageUrls(self.getObjectFor(index))
            if oResources is not None:
                if len(oResources) == 1:
                    Helper.setUnoPropertyValue(item.Model,
                PropertyNames.PROPERTY_IMAGEURL, oResources[0])
                elif len(oResources) == 2:
                    self.oUnoDialog.getPeerConfiguration().setImageUrl(
                        item.Model, oResources[0], oResources[1])

                Helper.setUnoPropertyValue(
                    item.Model, "Tabstop", bool(focusable))
                if self.refreshOverNull:
                    item.Visible =  True

                focusable = False

        self.refreshSelection()

    def refreshCounterText(self):
        Helper.setUnoPropertyValue(
                self.lblCounter.Model, PropertyNames.PROPERTY_LABEL,
                self.counterRenderer.render(
                    self.Counter (self.pageStart + 1,
                        self.pageEnd, len(ImageList.listModel))))

    def pageEnd(self):
        i = self.pageStart + self.cols * self.rows
        if i > ImageList.listModel.getSize() - 1:
            return ImageList.listModel.getSize()
        else:
            return i

    def refreshSelection(self):
        if self.selected < self.pageStart or \
                self.selected >= (self.pageStart + self.rows * self.cols):
            self.hideSelection()
        else:
            self.moveSelection(self.getImageIndexFor(self.selected))

    def hideSelection(self):
        Helper.setUnoPropertyValue(self.grbxSelectedImage.Model,
                PropertyNames.PROPERTY_STEP, ImageList.HIDE_PAGE)
        self.grbxSelectedImage.Visible = False

    '''
    Utility field holding list of ItemListeners.
    '''

    def moveSelection(self, image):
        self.grbxSelectedImage.Visible = False
        row = image / self.cols
        if self.rowSelect:
            col = 0
        else:
            col = image - (row * self.cols)

        self.MOVE_SELECTION_VALS[0] = \
            (self.getImagePosX(col) - self.selectionGap.Width)
        self.MOVE_SELECTION_VALS[1] = \
            (self.getImagePosY(row) - self.selectionGap.Height)
        Helper.setUnoPropertyValues(
            self.grbxSelectedImage.Model, ImageList.MOVE_SELECTION,
            self.MOVE_SELECTION_VALS)
        if (Helper.getUnoPropertyValue(self.dialogModel,
                PropertyNames.PROPERTY_STEP)) == self.step:
            self.grbxSelectedImage.Visible = True
            #now focus...

        for index,item in enumerate(self.m_aImages):
            if index != image:
                self.defocus(index)
            else:
                Helper.setUnoPropertyValue(
                    self.m_aImages[image].Model, "Tabstop", True)

    '''
    @param i
    @return the Object in the list model corresponding to the given image index
    '''

    def getObjectFor(self, i):
        ii = self.getIndexFor(i)
        if ImageList.listModel.getSize() <= ii:
            return None
        else:
            return ImageList.listModel.getElementAt(ii)

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
                event.getIndex1() < (self.pageStart + getRows() + getCols()):
            refreshImages()

    '''
    Registers ItemListener to receive events.
    @param listener The listener to register.
    '''

    @synchronized(lock)
    def addItemListener(self, listener):
        if self.itemListenerList == None:
            self.itemListenerList = java.util.ArrayList.ArrayList()

        self.itemListenerList.append(listener)

    '''
    Removes ItemListener from the list of listeners.
    @param listener The listener to remove.
    '''

    @synchronized(lock)
    def removeItemListener(self, listener):
        if self.itemListenerList != None:
            self.itemListenerList.remove(listener)

    '''
    Notifies all registered listeners about the event.
    @param event The event to be fired
    '''

    def fireItemSelected(self):
        with ImageList.lock:
            if self.itemListenerList is None:
                return

            auxlist = self.itemListenerList.clone()

        for i in auxlist:
            i.itemStateChanged(None)

    def setSelected(self, _object):
        if not isinstance(_object, int):
            _object = -1
            if _object is not None:
                for index, item in enumerate(ImageList.listModel):
                    if item == _object:
                        _object = index
                        return

        if self.rowSelect and (_object >= 0):
            _object = (_object / self.cols) * self.cols

        if self.selected == _object:
            return

        self.selected = _object
        self.refreshImageText()
        self.refreshSelection()
        #COMMENTED
        #self.fireItemSelected()

    def refreshImageText(self):
        if self.selected >= 0:
            #COMMENTED
            item = None #ImageList.listModel.getElementAt(self.selected)
        else:
            item = None

        Helper.setUnoPropertyValue(
                self.lblImageText.Model, PropertyNames.PROPERTY_LABEL,
                " " + self.renderer.render(item))


    def nextPage(self):
        if self.pageStart < listModel().getSize() - self.rows * self.cols:
            setPageStart(self.pageStart + self.rows * self.cols)

    def prevPage(self):
        if self.pageStart == 0:
            return

        i = self.pageStart - self.rows * self.cols
        if i < 0:
            i = 0

        setPageStart(i)

    def enableButtons(self):
        self.enable(
            ImageList.btnNext,
            bool(self.pageStart + self.rows * self.cols < len(ImageList.listModel)))
        self.enable(ImageList.btnBack, bool(self.pageStart > 0))

    def enable(self, control, enable):
        Helper.setUnoPropertyValue(control.Model,
                PropertyNames.PROPERTY_ENABLED, enable)

    def setBorder(self, control, border):
        Helper.setUnoPropertyValue(control.Model, "Border", border)

    def getImageFromEvent(self, event):
        image = (event).Source
        controlName = Helper.getUnoPropertyValue(image.Model,
                PropertyNames.PROPERTY_NAME)
        return Integer.valueOf(controlName.substring(6 + self.name.length()))

    def mousePressed(self, event):
        image = getImageFromEvent(event)
        index = getIndexFor(image)
        if index < ImageList.listModel.getSize():
            focus(image)
            setSelected(index)

    def getSelectedObjects(self):
        return[listModel().getElementAt(self.selected)]

    class SimpleCounterRenderer:

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
            return listModel().getElementAt(self.selected)
        return None

    def showSelected(self):
        oldPageStart = self.pageStart
        if self.selected != -1:
            self.pageStart = \
                (self.selected / len(self.m_aImages)) * len(self.m_aImages)

        if oldPageStart != self.pageStart:
            #COMMENTED
            #self.enableButtons()
            self.refreshImages()

    def keyPressed(self, ke):
        image = getImageFromEvent(ke)
        r = image / getCols()
        c = image - (r * getCols())
        d = getKeyMove(ke, r, c)
        newImage = image + d
        if newImage == image:
            return

        if isFocusable(newImage):
            changeFocus(image, newImage)

    def isFocusable(self, image):
        return (image >= 0) and \
            (getIndexFor(image) < ImageList.listModel.getSize())

    def changeFocus(self, oldFocusImage, newFocusImage):
        focus(newFocusImage)
        defocus(oldFocusImage)

    def select(self, ke):
        setSelected(getIndexFor(getImageFromEvent(ke)))

    def focus(self, image):
        Helper.setUnoPropertyValue(
            self.m_aImages[image].Model, "Tabstop", True)
        xWindow = self.m_aImages[image]
        xWindow.setFocus()

    def defocus(self, image):
        Helper.setUnoPropertyValue(
            self.m_aImages[image].Model, "Tabstop", False)

    '''jump to the given item (display the screen
    that contains the given item).
    @param i
    '''

    def display(self, i):
        isAux = (getCols() * getRows())
        ps = (ImageList.listModel.getSize() / isAux) * isAux
        setPageStart(ps)

    def setenabled(self, b):
        i = 0
        while i < self.m_aImages.length:
            UnoDialog2.setEnabled(self.m_aImages[i], b)
            i += 1
        UnoDialog2.setEnabled(self.grbxSelectedImage, b)
        UnoDialog2.setEnabled(self.lblImageText, b)
        if self.showButtons:
            UnoDialog2.setEnabled(ImageList.btnBack, b)
            UnoDialog2.setEnabled(ImageList.btnNext, b)
            UnoDialog2.setEnabled(self.lblCounter, b)

        self.benabled = b
