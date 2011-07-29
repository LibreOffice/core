import uno
import traceback
from common.Desktop import Desktop
from common.PropertyNames import PropertyNames
from common.HelpIds import HelpIds
from common.Helper import Helper
from ui.UnoDialog import UnoDialog

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
        self.nblockincrement = _nblockincrement
        ControlScroller.CurUnoDialog = _CurUnoDialog
        ControlScroller.iStep = _iStep
        ControlScroller.curHelpIndex = _firsthelpindex
        self.curtabindex = ControlScroller.iStep * 100
        self.linedistance = _nlinedistance
        self.iCompPosX = _iCompPosX
        self.iCompPosY = _iCompPosY
        self.iCompWidth = _iCompWidth
        self.iCompHeight = 2 * ControlScroller.SORELFIRSTPOSY + \
                self.nblockincrement * self.linedistance
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
        self.xScrollBar = ControlScroller.CurUnoDialog.insertScrollBar(
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
        self.nscrollvalue = 0
        self.ControlGroupVector = []
        ypos = self.iStartPosY + ControlScroller.SORELFIRSTPOSY
        for i in xrange(self.nblockincrement):
            self.insertControlGroup(i, ypos)
            ypos += self.linedistance

    def setComponentMouseTransparent(self):
        ControlScroller.CurUnoDialog.getPeerConfiguration().\
            setPeerProperties(self.oImgControl, "MouseTransparent", True)

    def setScrollBarOrientationHorizontal(self):
        Helper.setUnoPropertyValue(self.xScrollBar, "Orientation",HORIZONTAL)

    '''
    @author bc93774
    @param _ntotfieldcount: The number of fields that are
    to be administered by the ControlScroller
    '''

    def initialize(self, _ntotfieldcount):
        try:
            self.ntotfieldcount = _ntotfieldcount
            self.setCurFieldCount()
            self.nscrollvalue = 0
            Helper.setUnoPropertyValue(
                self.xScrollBar.Model, "ScrollValue", self.nscrollvalue)
            if self.ntotfieldcount > self.nblockincrement:
                Helper.setUnoPropertyValues(
                    self.xScrollBar.Model, (PropertyNames.PROPERTY_ENABLED,
                        "BlockIncrement", "LineIncrement",
                        "ScrollValue", "ScrollValueMax"),
                    (True, self.nblockincrement, self.nlineincrement,
                        self.nscrollvalue,
                        self.ntotfieldcount - self.nblockincrement))
            else:
                Helper.setUnoPropertyValues(
                    self.xScrollBar.Model,
                    (PropertyNames.PROPERTY_ENABLED, "ScrollValue"),
                    (False, self.nscrollvalue))

            self.fillupControls(True)
        except Exception:
            traceback.print_exc()

    def fillupControls(self, binitialize):
        for i in xrange(0, self.nblockincrement):
            if i < self.ncurfieldcount:
                self.fillupControl(i)

        if binitialize:
            ControlScroller.CurUnoDialog.repaintDialogStep()

    @classmethod
    def fillupControl(self, guiRow):
        nameProps = ControlScroller.scrollfields[guiRow]
        valueProps = ControlScroller.scrollfields[guiRow + self.nscrollvalue]
        for i in nameProps:
            if ControlScroller.CurUnoDialog.xDialogModel.hasByName(i.Name):
                self.setControlData(i.Name, i.Value)
            else:
                raise AttributeError("No such control !")

    def setScrollValue(self, _nscrollvalue, _ntotfieldcount=None):
        if _ntotfieldcount is not None:
            setTotalFieldCount(_ntotfieldcount)
        if _nscrollvalue >= 0:
            Helper.setUnoPropertyValue(
                self.xScrollBar.Model, "ScrollValue", _nscrollvalue)
            scrollControls()

    def setCurFieldCount(self):
        if self.ntotfieldcount > self.nblockincrement:
            self.ncurfieldcount = self.nblockincrement
        else:
            self.ncurfieldcount = self.ntotfieldcount

    def setTotalFieldCount(self, _ntotfieldcount):
        self.ntotfieldcount = _ntotfieldcount
        self.setCurFieldCount()
        if self.ntotfieldcount > self.nblockincrement:
            Helper.setUnoPropertyValues(
                self.xScrollBar.Model,
                (PropertyNames.PROPERTY_ENABLED, "ScrollValueMax"),
                (True, self.ntotfieldcount - self.nblockincrement))
        else:
            Helper.setUnoPropertyValue(self.xScrollBar.Model,
                PropertyNames.PROPERTY_ENABLED, False)

    def toggleComponent(self, _bdoenable):
        bdoenable = _bdoenable and \
            (self.ntotfieldcount > self.nblockincrement)
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

    def setLineIncrementation(self, _nlineincrement):
        self.nlineincrement = _nlineincrement
        Helper.setUnoPropertyValue(
            self.xScrollBar.Model, "LineIncrement", self.nlineincrement)

    def getLineIncrementation(self):
        return self.nlineincrement

    def setBlockIncrementation(self, _nblockincrement):
        self.nblockincrement = _nblockincrement
        Helper.setUnoPropertyValues(
            self.xScrollBar.Model,
            (PropertyNames.PROPERTY_ENABLED, "BlockIncrement",
                "ScrollValueMax"),
            (self.ntotfieldcount > self.nblockincrement, self.nblockincrement,
                self.ntotfieldcount - self.nblockincrement))

    def scrollControls(self):
        try:
            scrollRowsInfo()
            self.nscrollvalue = int(Helper.getUnoPropertyValue(
                    self.xScrollBar.Model, "ScrollValue"))
            if self.nscrollvalue + self.nblockincrement >= self.ntotfieldcount:
                self.nscrollvalue = self.ntotfieldcount - self.nblockincrement

            fillupControls(False)
        except java.lang.Exception, ex:
            ex.printStackTrace()

    def scrollRowsInfo(self):
        if ControlScroller.scrollfields.size() > 0:
            cols = len(ControlScroller.scrollfields[0])
        else:
            cols = 0

        a = 0
        while a < self.ncurfieldcount:
            n = 0
            while n < cols:
                fieldInfo(a, n)
                n += 1
            a += 1

    '''
    updates the corresponding data to
    the control in guiRow and column
    @param guiRow 0 based row index
    @param column 0 based column index
    @return the propertyValue object corresponding to
    this control.
    '''

    def fieldInfo(self, guiRow, column):
        if guiRow + self.nscrollvalue < ControlScroller.scrollfields.size():
            pv = fieldInfo(
                (ControlScroller.scrollfields.elementAt(
                    guiRow + self.nscrollvalue))[column],
                (ControlScroller.scrollfields.elementAt(guiRow))[column])
            return pv
        else:
            return None

    def fieldInfo(self, valueProp, nameProp):
        if ControlScroller.CurUnoDialog.xDialogModel.hasByName(nameProp.Name):
            valueProp.Value = getControlData(nameProp.Name)
        else:
            valueProp.Value = nameProp.Value

        return valueProp

    def unregisterControlGroup(self, _index):
        ControlScroller.scrollfields.remove(_index)

    def registerControlGroup(self, _currowproperties, _i):
        if _i == 0:
            del ControlScroller.scrollfields[:]

        if _i >= len(ControlScroller.scrollfields):
            ControlScroller.scrollfields.append(_currowproperties)
        else:
            ControlScroller.scrollfields.insert(_currowproperties, _i)

    def getControlGroupInfo(self, _i):
        return ControlScroller.scrollfields.index(_i)

    @classmethod
    def setControlData(self, controlname, newvalue):
        oControlModel = ControlScroller.CurUnoDialog.xUnoDialog.getControl(
            controlname).Model
        propertyname = UnoDialog.getDisplayProperty(oControlModel)
        if propertyname != "":
            ControlScroller.CurUnoDialog.setControlProperty(
                controlname, propertyname, newvalue)

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
        scrollRowsInfo()
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
