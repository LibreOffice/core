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
from wizards.common.Desktop import Desktop
from wizards.common.PropertyNames import PropertyNames
from wizards.common.HelpIds import HelpIds
from wizards.common.Helper import Helper
from wizards.ui.UnoDialog import UnoDialog

from com.sun.star.awt.ScrollBarOrientation import HORIZONTAL, VERTICAL

class ControlScroller(object):

    SORELFIRSTPOSY = 3
    iScrollBarWidth = 10
    scrollfields = []
    CurUnoDialog = None
    iStep = None
    curHelpIndex = None

    # TODO add parameters for tabindices and helpindex
    def __init__(self, _CurUnoDialog, _xMSF, _iStep, _iCompPosX, _iCompPosY,
            _iCompWidth, _nblockincrement, _nlinedistance, _firsthelpindex):
        self.xMSF = _xMSF
        ControlScroller.nblockincrement = _nblockincrement
        ControlScroller.CurUnoDialog = _CurUnoDialog
        ControlScroller.iStep = _iStep
        ControlScroller.curHelpIndex = _firsthelpindex
        self.curtabindex = ControlScroller.iStep * 100
        self.linedistance = _nlinedistance
        self.iCompPosX = _iCompPosX
        self.iCompPosY = _iCompPosY
        self.iCompWidth = _iCompWidth
        self.iCompHeight = 2 * ControlScroller.SORELFIRSTPOSY + \
                ControlScroller.nblockincrement * self.linedistance
        self.iStartPosY = self.iCompPosY + ControlScroller.SORELFIRSTPOSY
        ScrollHeight = self.iCompHeight - 2
        self.nlineincrement = 1
        self.sincSuffix = Desktop.getIncrementSuffix(
            ControlScroller.CurUnoDialog.xDialogModel, "imgBackground")
        self.oImgControl = ControlScroller.CurUnoDialog.insertControlModel(
            "com.sun.star.awt.UnoControlImageControlModel",
            "imgBackground" + self.sincSuffix,
            ("Border", PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_WIDTH),
            (1, self.iCompHeight, self.iCompPosX,
                self.iCompPosY, ControlScroller.iStep, self.iCompWidth))
        self.oImgControl = ControlScroller.CurUnoDialog.xUnoDialog.getControl(
            "imgBackground" + self.sincSuffix)
        self.setComponentMouseTransparent()
        ControlScroller.xScrollBar = ControlScroller.CurUnoDialog.insertScrollBar(
            "TitleScrollBar" + self.sincSuffix,
            ("Border", PropertyNames.PROPERTY_ENABLED,
                PropertyNames.PROPERTY_HEIGHT,
                PropertyNames.PROPERTY_HELPURL, "Orientation",
                PropertyNames.PROPERTY_POSITION_X,
                PropertyNames.PROPERTY_POSITION_Y,
                PropertyNames.PROPERTY_STEP,
                PropertyNames.PROPERTY_WIDTH),
            (0, True, ScrollHeight,
                HelpIds.getHelpIdString(ControlScroller.curHelpIndex),
                VERTICAL, self.iCompPosX + self.iCompWidth - \
                    ControlScroller.iScrollBarWidth - 1,
                self.iCompPosY + 1, ControlScroller.iStep,
                ControlScroller.iScrollBarWidth), 0, self)
        ControlScroller.nscrollvalue = 0
        ControlScroller.ControlGroupVector = []
        ypos = self.iStartPosY + ControlScroller.SORELFIRSTPOSY
        for i in xrange(ControlScroller.nblockincrement):
            self.insertControlGroup(i, ypos)
            ypos += self.linedistance

    def setComponentMouseTransparent(self):
        ControlScroller.CurUnoDialog.getPeerConfiguration().\
            setPeerProperties(self.oImgControl, "MouseTransparent", True)

    def setScrollBarOrientationHorizontal(self):
        Helper.setUnoPropertyValue(
            ControlScroller.xScrollBar, "Orientation", HORIZONTAL)

    @classmethod
    def fillupControls(self, binitialize):
        for i in xrange(ControlScroller.nblockincrement):
            if i < self.ncurfieldcount:
                self.fillupControl(i)

        if binitialize:
            ControlScroller.CurUnoDialog.repaintDialogStep()

    @classmethod
    def fillupControl(self, guiRow):
        nameProps = ControlScroller.scrollfields[guiRow]
        valueProps = ControlScroller.scrollfields[guiRow + ControlScroller.nscrollvalue]
        for index, item in enumerate(nameProps):
            if ControlScroller.CurUnoDialog.xDialogModel.hasByName(item.Name):
                self.setControlData(item.Name, valueProps[index].Value)
            else:
                raise AttributeError("No such control !")

    @classmethod
    def setScrollValue(self, _nscrollvalue, _ntotfieldcount=None):
        if _ntotfieldcount is not None:
            self.setTotalFieldCount(_ntotfieldcount)
        if _nscrollvalue >= 0:
            Helper.setUnoPropertyValue(
                ControlScroller.xScrollBar.Model, "ScrollValue", _nscrollvalue)
            self.scrollControls()

    @classmethod
    def setCurFieldCount(self):
        if self.ntotfieldcount > ControlScroller.nblockincrement:
            self.ncurfieldcount = ControlScroller.nblockincrement
        else:
            self.ncurfieldcount = self.ntotfieldcount

    @classmethod
    def setTotalFieldCount(self, _ntotfieldcount):
        self.ntotfieldcount = _ntotfieldcount
        self.setCurFieldCount()
        if self.ntotfieldcount > ControlScroller.nblockincrement:
            Helper.setUnoPropertyValues(
                ControlScroller.xScrollBar.Model,
                (PropertyNames.PROPERTY_ENABLED, "ScrollValueMax"),
                (True, self.ntotfieldcount - ControlScroller.nblockincrement))
        else:
            Helper.setUnoPropertyValue(ControlScroller.xScrollBar.Model,
                PropertyNames.PROPERTY_ENABLED, False)

    def toggleComponent(self, _bdoenable):
        bdoenable = _bdoenable and \
            (self.ntotfieldcount > ControlScroller.nblockincrement)
        ControlScroller.CurUnoDialog.setControlProperty(
                "TitleScrollBar" + self.sincSuffix,
                PropertyNames.PROPERTY_ENABLED, bdoenable)

    def toggleControls(self, _bdoenable):
        n = 0
        while n < ControlScroller.scrollfields.size():
            curproperties = ControlScroller.scrollfields.elementAt(n)
            m = 0
            while m < curproperties.length:
                curproperty = curproperties[m]
                ControlScroller.CurUnoDialog.setControlProperty(
                curproperty.Name, PropertyNames.PROPERTY_ENABLED, _bdoenable)
                m += 1
            n += 1

    @classmethod
    def scrollControls(self):
        try:
            ControlScroller.nscrollvalue = int(Helper.getUnoPropertyValue(
                    ControlScroller.xScrollBar.Model, "ScrollValue"))
            if ControlScroller.nscrollvalue + ControlScroller.nblockincrement \
                    >= self.ntotfieldcount:
                ControlScroller.nscrollvalue = \
                    self.ntotfieldcount - ControlScroller.nblockincrement
            self.fillupControls(False)
        except Exception:
            traceback.print_exc()

    '''
    updates the corresponding data to
    the control in guiRow and column
    @param guiRow 0 based row index
    @param column 0 based column index
    @return the propertyValue object corresponding to
    this control.
    '''

    @classmethod
    def fieldInfo(self, guiRow, column):
        if guiRow + ControlScroller.nscrollvalue < len(ControlScroller.scrollfields):
            valueProp = (ControlScroller.scrollfields[guiRow + ControlScroller.nscrollvalue])[column]
            nameProp = (ControlScroller.scrollfields[guiRow])[column]
            if ControlScroller.CurUnoDialog.xDialogModel.hasByName(nameProp.Name):
                valueProp.Value = self.getControlData(nameProp.Name)
            else:
                valueProp.Value = nameProp.Value
            return valueProp
        else:
            return None

    @classmethod
    def unregisterControlGroup(self, _index):
        del ControlScroller.scrollfields[_index]

    @classmethod
    def registerControlGroup(self, _currowproperties, _i):
        if _i == 0:
            del ControlScroller.scrollfields[:]

        if _i >= len(ControlScroller.scrollfields):
            ControlScroller.scrollfields.append(_currowproperties)
        else:
            ControlScroller.scrollfields.insert(_currowproperties, _i)

    @classmethod
    def setControlData(self, controlname, newvalue):
        oControlModel = ControlScroller.CurUnoDialog.xUnoDialog.getControl(
            controlname).Model
        propertyname = UnoDialog.getDisplayProperty(oControlModel)
        if propertyname != "":
            ControlScroller.CurUnoDialog.setControlProperty(
                controlname, propertyname, newvalue)

    @classmethod
    def getControlData(self, controlname):
        oControlModel = ControlScroller.CurUnoDialog.xUnoDialog.getControl(
            controlname).Model
        propertyname = UnoDialog.getDisplayProperty(oControlModel)
        if propertyname != "":
            return ControlScroller.CurUnoDialog.getControlProperty(
                controlname, propertyname)
        else:
            return None

    def getScrollFieldValues(self):
        retproperties = [[ControlScroller.scrollfields.size()],[]]
        try:
            i = 0
            while i < ControlScroller.scrollfields.size():
                curproperties = ControlScroller.scrollfields.elementAt(i)
                retproperties[i] = curproperties
                i += 1
            return retproperties
        except java.lang.Exception, ex:
            ex.printStackTrace(System.out)
            return None
