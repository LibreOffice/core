import uno
import traceback
from common.PropertyNames import PropertyNames
from com.sun.star.awt import Rectangle
from common.Helper import Helper
from PeerConfig import PeerConfig
from com.sun.star.awt import Rectangle
from com.sun.star.awt.PosSize import POS

class UnoDialog(object):

    def __init__(self, xMSF, PropertyNames, PropertyValues):
        try:
            self.xMSF = xMSF
            self.ControlList = {}
            self.xDialogModel = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialogModel")
            self.xDialogModel.setPropertyValues(PropertyNames, PropertyValues)
            self.xUnoDialog = xMSF.createInstance(
                "com.sun.star.awt.UnoControlDialog")
            self.xUnoDialog.setModel(self.xDialogModel)
            self.BisHighContrastModeActivated = None
            self.m_oPeerConfig = None
            self.xWindowPeer = None
        except UnoException, e:
            traceback.print_exc()

    def getControlKey(self, EventObject, ControlList):
        xControlModel = EventObject.getModel()
        try:
            sName = xControlModel.getPropertyValue(
                PropertyNames.PROPERTY_NAME)
            iKey = ControlList.get(sName).intValue()
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            iKey = 2000

        return iKey

    def createPeerConfiguration(self):
        self.m_oPeerConfig = PeerConfig(self)

    def getPeerConfiguration(self):
        if self.m_oPeerConfig == None:
            self.createPeerConfiguration()
        return self.m_oPeerConfig

    def setControlProperty(self, ControlName, PropertyName, PropertyValue):
        try:
            if PropertyValue is not None:
                if self.xDialogModel.hasByName(ControlName) == False:
                    return
                xPSet = self.xDialogModel.getByName(ControlName)
                if isinstance(PropertyValue,bool):
                    xPSet.setPropertyValue(PropertyName, PropertyValue)
                else:
                    methodname = "[]string"
                    if not isinstance(PropertyValue,tuple):
                        if isinstance(PropertyValue,list):
                            methodname = "[]short"
                            PropertyValue = tuple(PropertyValue)
                        else:
                            PropertyValue = (PropertyValue,)

                    uno.invoke(xPSet, "setPropertyValue", (PropertyName,
                        uno.Any( methodname, PropertyValue)))

        except Exception, exception:
            traceback.print_exc()

    def transform( self, struct , propName, value ):
        myinv = self.inv.createInstanceWithArguments( (struct,) )
        access = self.insp.inspect( myinv )
        method = access.getMethod( "setValue" , -1 )
        uno.invoke( method, "invoke", ( myinv, ( propName , value ) ))
        method = access.getMethod( "getMaterial" , -1 )
        ret,dummy = method.invoke(myinv,() )
        return ret

    def getResource(self):
        return self.m_oResource

    def setControlProperties(
            self, ControlName, PropertyNames, PropertyValues):
        self.setControlProperty(ControlName, PropertyNames, PropertyValues)

    def getControlProperty(self, ControlName, PropertyName):
        try:
            xPSet = self.xDialogModel().getByName(ControlName)
            oPropValuezxPSet.getPropertyValue(PropertyName)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None


    def printControlProperties(self, ControlName):
        try:
            xControlModel = self.xDialogModel().getByName(ControlName)
            allProps = xControlModel.PropertySetInfo.Properties
            i = 0
            while i < allProps.length:
                sName = allProps[i].Name
                i += 1
        except UnoException, exception:
            traceback.print_exc()

    def getMAPConversionFactor(self, ControlName):
        xControl2 = self.xUnoDialog.getControl(ControlName)
        aSize = xControl2.Size
        dblMAPWidth = Helper.getUnoPropertyValue(xControl2.Model,
            int(PropertyNames.PROPERTY_WIDTH))
        return (aSize.Width / dblMAPWidth)

    def getpreferredLabelSize(self, LabelName, sLabel):
        xControl2 = self.xUnoDialog.getControl(LabelName)
        OldText = xControl2.Text
        xControl2.setText(sLabel)
        aSize = xControl2.PreferredSize
        xControl2.setText(OldText)
        return aSize

    def removeSelectedItems(self, xListBox):
        SelList = xListBox.SelectedItemsPos
        Sellen = SelList.length
        i = Sellen - 1
        while i >= 0:
            xListBox.removeItems(SelList[i], 1)
            i -= 1

    def getListBoxItemCount(self, _xListBox):
        # This function may look ugly, but this is the only way to check
        # the count of values in the model,which is always right.
        # the control is only a view and could be right or not.
        fieldnames = Helper.getUnoPropertyValue(getModel(_xListBox),
            "StringItemList")
        return fieldnames.length

    def getSelectedItemPos(self, _xListBox):
        ipos = Helper.getUnoPropertyValue(getModel(_xListBox),
            "SelectedItems")
        return ipos[0]

    def isListBoxSelected(self, _xListBox):
        ipos = Helper.getUnoPropertyValue(getModel(_xListBox),
            "SelectedItems")
        return ipos.length > 0

    def addSingleItemtoListbox(self, xListBox, ListItem, iSelIndex):
        xListBox.addItem(ListItem, xListBox.getItemCount())
        if iSelIndex != -1:
            xListBox.selectItemPos(iSelIndex, True)

    def insertLabel(self, sName, sPropNames, oPropValues):
        try:
            oFixedText = self.insertControlModel(
                "com.sun.star.awt.UnoControlFixedTextModel",
                sName, sPropNames, oPropValues)
            oFixedText.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
            oLabel = self.xUnoDialog.getControl(sName)
            return oLabel
        except Exception, ex:
            traceback.print_exc()
            return None

    def insertButton(
            self, sName, iControlKey, xActionListener, sProperties, sValues):
        oButtonModel = self.insertControlModel(
            "com.sun.star.awt.UnoControlButtonModel",
            sName, sProperties, sValues)
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
        xButton = self.xUnoDialog.getControl(sName)
        if xActionListener != None:
            xButton.addActionListener(
                ActionListenerProcAdapter(xActionListener))

        ControlKey = iControlKey
        if self.ControlList != None:
            self.ControlList.put(sName, ControlKey)

        return xButton

    def insertCheckBox(
            self, sName, iControlKey, xItemListener, sProperties, sValues):
        oButtonModel = self.insertControlModel(
            "com.sun.star.awt.UnoControlCheckBoxModel",
            sName, sProperties, sValues)
        oButtonModel.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
        xCheckBox = self.xUnoDialog.getControl(sName)
        if xItemListener != None:
            xCheckBox.addItemListener(
                ItemListenerProcAdapter(xItemListener))

        ControlKey = iControlKey
        if self.ControlList != None:
            self.ControlList.put(sName, ControlKey)

    def insertNumericField(
            self, sName, iControlKey, xTextListener, sProperties, sValues):
        oNumericFieldModel = self.insertControlModel(
            "com.sun.star.awt.UnoControlNumericFieldModel",
            sName, sProperties, sValues)
        oNumericFieldModel.setPropertyValue(
            PropertyNames.PROPERTY_NAME, sName)
        xNumericField = self.xUnoDialog.getControl(sName)
        if xTextListener != None:
            xNumericField.addTextListener(
                TextListenerProcAdapter(xTextListener))

        ControlKey = iControlKey
        if self.ControlList != None:
            self.ControlList.put(sName, ControlKey)

    def insertScrollBar(
        self, sName, iControlKey, xAdjustmentListener, sProperties, sValues):
        try:
            oScrollModel = self.insertControlModel(
                "com.sun.star.awt.UnoControlScrollBarModel",
                sName, sProperties, sValues)
            oScrollModel.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
            xScrollBar = self.xUnoDialog.getControl(sName)
            if xAdjustmentListener != None:
                xScrollBar.addAdjustmentListener(xAdjustmentListener)

            ControlKey = iControlKey
            if self.ControlList != None:
                self.ControlList.put(sName, ControlKey)

            return xScrollBar
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None

    def insertTextField(
        self, sName, iControlKey, xTextListener, sProperties, sValues):
        xTextBox = insertEditField(
            "com.sun.star.awt.UnoControlEditModel", sName, iControlKey,
            xTextListener, sProperties, sValues)
        return xTextBox

    def insertFormattedField(
        self, sName, iControlKey, xTextListener, sProperties, sValues):
        xTextBox = insertEditField(
            "com.sun.star.awt.UnoControlFormattedFieldModel", sName,
            iControlKey, xTextListener, sProperties, sValues)
        return xTextBox

    def insertEditField(
        self, ServiceName, sName, iControlKey,
        xTextListener, sProperties, sValues):

        try:
            xTextModel = self.insertControlModel(
                ServiceName, sName, sProperties, sValues)
            xTextModel.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
            xTextBox = self.xUnoDialog.getControl(sName)
            if xTextListener != None:
                xTextBox.addTextListener(TextListenerProcAdapter(xTextListener))

            ControlKey = iControlKey
            self.ControlList.put(sName, ControlKey)
            return xTextBox
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None

    def insertListBox(
        self, sName, iControlKey, xActionListener,
        xItemListener, sProperties, sValues):
        xListBoxModel = self.insertControlModel(
            "com.sun.star.awt.UnoControlListBoxModel",
            sName, sProperties, sValues)
        xListBoxModel.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
        xListBox = self.xUnoDialog.getControl(sName)
        if xItemListener != None:
            xListBox.addItemListener(ItemListenerProcAdapter(xItemListener))

        if xActionListener != None:
            xListBox.addActionListener(
                ActionListenerProcAdapter(xActionListener))

        ControlKey = iControlKey
        self.ControlList.put(sName, ControlKey)
        return xListBox

    def insertComboBox(
        self, sName, iControlKey, xActionListener, xTextListener,
        xItemListener, sProperties, sValues):
        xComboBoxModel = self.insertControlModel(
            "com.sun.star.awt.UnoControlComboBoxModel",
            sName, sProperties, sValues)
        xComboBoxModel.setPropertyValue(PropertyNames.PROPERTY_NAME, sName)
        xComboBox = self.xUnoDialog.getControl(sName)
        if xItemListener != None:
            xComboBox.addItemListener(ItemListenerProcAdapter(xItemListener))

        if xTextListener != None:
            xComboBox.addTextListener(TextListenerProcAdapter(xTextListener))

        if xActionListener != None:
            xComboBox.addActionListener(
                ActionListenerProcAdapter(xActionListener))

        ControlKey = iControlKey
        self.ControlList.put(sName, ControlKey)
        return xComboBox

    def insertRadioButton(
            self, sName, iControlKey, xItemListener, sProperties, sValues):
        try:
            xRadioButton = insertRadioButton(
                sName, iControlKey, sProperties, sValues)
            if xItemListener != None:
                xRadioButton.addItemListener(
                    ItemListenerProcAdapter(xItemListener))

            return xRadioButton
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None

    def insertRadioButton(
            self, sName, iControlKey, xActionListener, sProperties, sValues):
        try:
            xButton = insertRadioButton(
                sName, iControlKey, sProperties, sValues)
            if xActionListener != None:
                xButton.addActionListener(
                    ActionListenerProcAdapter(xActionListener))

            return xButton
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None

    def insertRadioButton(self, sName, iControlKey, sProperties, sValues):
        xRadioButton = insertRadioButton(sName, sProperties, sValues)
        ControlKey = iControlKey
        self.ControlList.put(sName, ControlKey)
        return xRadioButton

    def insertRadioButton(self, sName, sProperties, sValues):
        try:
            oRadioButtonModel = self.insertControlModel(
                "com.sun.star.awt.UnoControlRadioButtonModel",
                sName, sProperties, sValues)
            oRadioButtonModel.setPropertyValue(
                PropertyNames.PROPERTY_NAME, sName)
            xRadioButton = self.xUnoDialog.getControl(sName)
            return xRadioButton
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            return None
    '''
    The problem with setting the visibility of controls
    is that changing the current step of a dialog will automatically
    make all controls visible. The PropertyNames.PROPERTY_STEP property
    always wins against the property "visible". Therfor a control meant
    to be invisible is placed on a step far far away.
    @param the name of the control
    @param iStep  change the step if you want to make the control invisible
    '''

    def setControlVisible(self, controlname, iStep):
        try:
            iCurStep = int(getControlProperty(
                controlname, PropertyNames.PROPERTY_STEP))
            setControlProperty(
                controlname, PropertyNames.PROPERTY_STEP, iStep)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    '''
    The problem with setting the visibility of controls is that
    changing the current step of a dialog will automatically make
    all controls visible. The PropertyNames.PROPERTY_STEP property
    always wins against the property "visible".
    Therfor a control meant to be invisible is placed on a step far far away.
    Afterwards the step property of the dialog has to be set with
    "repaintDialogStep". As the performance of that method is very bad it
    should be used only once for all controls
    @param controlname the name of the control
    @param bIsVisible sets the control visible or invisible
    '''

    def setControlVisible(self, controlname, bIsVisible):
        try:
            iCurControlStep = int(getControlProperty(
                controlname, PropertyNames.PROPERTY_STEP))
            iCurDialogStep = int(Helper.getUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP))
            if bIsVisible:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP, iCurDialogStep)
            else:
                setControlProperty(
                    controlname, PropertyNames.PROPERTY_STEP,
                    UIConsts.INVISIBLESTEP)

        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    # repaints the currentDialogStep


    def repaintDialogStep(self):
        try:
            ncurstep = int(Helper.getUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP))
            Helper.setUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP, 99)
            Helper.setUnoPropertyValue(
                self.xDialogModel, PropertyNames.PROPERTY_STEP, ncurstep)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    def insertControlModel(self, ServiceName, sName, sProperties, sValues):
        try:
            xControlModel = self.xDialogModel.createInstance(ServiceName)
            Helper.setUnoPropertyValues(xControlModel, sProperties, sValues)
            self.xDialogModel.insertByName(sName, xControlModel)
            return xControlModel
        except Exception, exception:
            traceback.print_exc()
            return None

    def setFocus(self, ControlName):
        oFocusControl = self.xUnoDialog.getControl(ControlName)
        oFocusControl.setFocus()

    def combineListboxList(self, sFirstEntry, MainList):
        try:
            FirstList = [sFirstEntry]
            ResultList = [MainList.length + 1]
            System.arraycopy(FirstList, 0, ResultList, 0, 1)
            System.arraycopy(MainList, 0, ResultList, 1, len(MainList))
            return ResultList
        except java.lang.Exception, jexception:
            traceback.print_exc()
            return None

    def selectListBoxItem(self, xListBox, iFieldsSelIndex):
        if iFieldsSelIndex > -1:
            FieldCount = xListBox.getItemCount()
            if FieldCount > 0:
                if iFieldsSelIndex < FieldCount:
                    xListBox.selectItemPos(iFieldsSelIndex, True)
                else:
                    xListBox.selectItemPos((short)(iFieldsSelIndex - 1), True)

    # deselects a Listbox. MultipleMode is not supported

    def deselectListBox(self, _xBasisListBox):
        oListBoxModel = getModel(_xBasisListBox)
        sList = Helper.getUnoPropertyValue(oListBoxModel, "StringItemList")
        Helper.setUnoPropertyValue(oListBoxModel, "StringItemList", [[],[]])
        Helper.setUnoPropertyValue(oListBoxModel, "StringItemList", sList)

    def calculateDialogPosition(self, FramePosSize):
        # Todo:check if it would be useful or possible to create a dialog peer
        # that can be used for the messageboxes to
        # maintain modality when they pop up.
        CurPosSize = self.xUnoDialog.getPosSize()
        WindowHeight = FramePosSize.Height
        WindowWidth = FramePosSize.Width
        DialogWidth = CurPosSize.Width
        DialogHeight = CurPosSize.Height
        iXPos = ((WindowWidth / 2) - (DialogWidth / 2))
        iYPos = ((WindowHeight / 2) - (DialogHeight / 2))
        self.xUnoDialog.setPosSize(
            iXPos, iYPos, DialogWidth, DialogHeight, POS)

    '''
     @param FramePosSize
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialog(self, FramePosSize):
        if self.xUnoDialog.getPeer() == None:
            raise AttributeError(
                "Please create a peer, using your own frame");

        self.calculateDialogPosition(FramePosSize)

        if self.xWindowPeer == None:
            self.createWindowPeer()

        self.BisHighContrastModeActivated = self.isHighContrastModeActivated()
        return self.xUnoDialog.execute()

    def setVisible(self, parent):
        self.calculateDialogPosition(parent.xWindow.getPosSize())
        if self.xWindowPeer == None:
            self.createWindowPeer()

        self.xUnoDialog.setVisible(True)

    '''
    @param parent
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialogFromParent(self, parent):
        return self.executeDialog(parent.xWindow.PosSize)

    '''
    @param XComponent
    @return 0 for cancel, 1 for ok
    @throws com.sun.star.uno.Exception
    '''

    def executeDialogFromComponent(self, xComponent):
        if xComponent is not None:
            w = xComponent.ComponentWindow
            if w is not None:
                return self.executeDialog(w.PosSize)

        return self.executeDialog( Rectangle (0, 0, 640, 400))

    def setAutoMnemonic(self, ControlName, bValue):
        self.xUnoDialog = self.xUnoDialog.getControl(ControlName)
        xVclWindowPedsfer = self.xUnoDialog.getPeer()
        self.xContainerWindow.setProperty("AutoMnemonics", bValue)

    def modifyFontWeight(self, ControlName, FontWeight):
        oFontDesc = FontDescriptor.FontDescriptor()
        oFontDesc.Weight = FontWeight
        setControlProperty(ControlName, "FontDescriptor", oFontDesc)

    '''
    create a peer for this
    dialog, using the given
    peer as a parent.
    @param parentPeer
    @return
    @throws java.lang.Exception
    '''

    def createWindowPeer(self, parentPeer=None):
        self.xUnoDialog.setVisible(False)
        xToolkit = self.xMSF.createInstance("com.sun.star.awt.Toolkit")
        if parentPeer == None:
            parentPeer = xToolkit.getDesktopWindow()

        self.xUnoDialog.createPeer(xToolkit, parentPeer)
        self.xWindowPeer = self.xUnoDialog.getPeer()
        return self.xUnoDialog.getPeer()

    # deletes the first entry when this is equal to "DelEntryName"
    # returns true when a new item is selected

    def deletefirstListboxEntry(self, ListBoxName, DelEntryName):
        xListBox = self.xUnoDialog.getControl(ListBoxName)
        FirstItem = xListBox.getItem(0)
        if FirstItem.equals(DelEntryName):
            SelPos = xListBox.getSelectedItemPos()
            xListBox.removeItems(0, 1)
            if SelPos > 0:
                setControlProperty(ListBoxName, "SelectedItems", [SelPos])
                xListBox.selectItemPos((short)(SelPos - 1), True)

    def setPeerProperty(self, ControlName, PropertyName, PropertyValue):
        xControl = self.xUnoDialog.getControl(ControlName)
        xVclWindowPeer = self.xControl.getPeer()
        self.xContainerWindow.setProperty(PropertyName, PropertyValue)

    @classmethod
    def getModel(self, control):
        return control.getModel()

    @classmethod
    def setEnabled(self, control, enabled):
        setEnabled(control, enabled)

    @classmethod
    def setEnabled(self, control, enabled):
        Helper.setUnoPropertyValue(
            getModel(control), PropertyNames.PROPERTY_ENABLED, enabled)

    '''
    @author bc93774
    @param oControlModel the model of a control
    @return the LabelType according to UIConsts.CONTROLTYPE
    '''

    @classmethod
    def getControlModelType(self, oControlModel):
        if oControlModel.supportsService(
                "com.sun.star.awt.UnoControlFixedTextModel"):
            return UIConsts.CONTROLTYPE.FIXEDTEXT
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlButtonModel"):
            return UIConsts.CONTROLTYPE.BUTTON
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlCurrencyFieldModel"):
            return UIConsts.CONTROLTYPE.CURRENCYFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlDateFieldModel"):
            return UIConsts.CONTROLTYPE.DATEFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlFixedLineModel"):
            return UIConsts.CONTROLTYPE.FIXEDLINE
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlFormattedFieldModel"):
            return UIConsts.CONTROLTYPE.FORMATTEDFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlRoadmapModel"):
            return UIConsts.CONTROLTYPE.ROADMAP
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlNumericFieldModel"):
            return UIConsts.CONTROLTYPE.NUMERICFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlPatternFieldModel"):
            return UIConsts.CONTROLTYPE.PATTERNFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlHyperTextModel"):
            return UIConsts.CONTROLTYPE.HYPERTEXT
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlProgressBarModel"):
            return UIConsts.CONTROLTYPE.PROGRESSBAR
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlTimeFieldModel"):
            return UIConsts.CONTROLTYPE.TIMEFIELD
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlImageControlModel"):
            return UIConsts.CONTROLTYPE.IMAGECONTROL
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlRadioButtonModel"):
            return UIConsts.CONTROLTYPE.RADIOBUTTON
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlCheckBoxModel"):
            return UIConsts.CONTROLTYPE.CHECKBOX
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlEditModel"):
            return UIConsts.CONTROLTYPE.EDITCONTROL
        elif oControlModel.supportsService(
                "com.sun.star.awt.UnoControlComboBoxModel"):
            return UIConsts.CONTROLTYPE.COMBOBOX
        else:
            if (oControlModel.supportsService(
                "com.sun.star.awt.UnoControlListBoxModel")):
                return UIConsts.CONTROLTYPE.LISTBOX
            else:
                return UIConsts.CONTROLTYPE.UNKNOWN

    '''
    @author bc93774
    @param oControlModel
    @return the name of the property that contains the value of a controlmodel
    '''

    @classmethod
    def getDisplayProperty(self, oControlModel):
        itype = getControlModelType(oControlModel)
        return getDisplayProperty(itype)

    '''
    @param itype The type of the control conforming to UIConst.ControlType
    @return the name of the property that contains the value of a controlmodel
    '''

    '''
    @classmethod
    def getDisplayProperty(self, itype):
        # String propertyname = "";
        tmp_switch_var1 = itype
        if 1:
            pass
        else:
            return ""
    '''

    def addResourceHandler(self, _Unit, _Module):
        self.m_oResource = Resource(self.xMSF, _Unit, _Module)

    def setInitialTabindex(self, _istep):
        return (short)(_istep * 100)

    def isHighContrastModeActivated(self):
        if self.xContainerWindow != None:
            if self.BisHighContrastModeActivated == None:
                try:
                    nUIColor = int(self.xContainerWindow.getProperty(
                        "DisplayBackgroundColor"))
                except IllegalArgumentException, e:
                    traceback.print_exc()
                    return False

                #TODO: The following methods could be wrapped
                # in an own class implementation
                nRed = self.getRedColorShare(nUIColor)
                nGreen = self.getGreenColorShare(nUIColor)
                nBlue = self.getBlueColorShare(nUIColor)
                nLuminance = ((nBlue * 28 + nGreen * 151 + nRed * 77) / 256)
                bisactivated = (nLuminance <= 25)
                self.BisHighContrastModeActivated = bisactivated
                return bisactivated
            else:
                return self.BisHighContrastModeActivated.booleanValue()

        else:
            return False

    def getRedColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = (int)(nRedModulo / 256)
        nGreenModulo = (nRedModulo % 256)
        nBlue = nGreenModulo
        return nRed

    def getGreenColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = (int)(nRedModulo / 256)
        return nGreen

    def getBlueColorShare(self, _nColor):
        nRed = _nColor / 65536
        nRedModulo = _nColor % 65536
        nGreen = (int)(nRedModulo / 256)
        nGreenModulo = (nRedModulo % 256)
        nBlue = nGreenModulo
        return nBlue

    def getWizardImageUrl(self, _nResId, _nHCResId):
        if isHighContrastModeActivated():
            return "private:resource/wzi/image/" + _nHCResId
        else:
            return "private:resource/wzi/image/" + _nResId

    def getImageUrl(self, _surl, _shcurl):
        if isHighContrastModeActivated():
            return _shcurl
        else:
            return _surl

    def getListBoxLineCount(self):
        return 20
