import traceback
import time
from com.sun.star.util import DateTime
from common.PropertyNames import PropertyNames
import unicodedata

class TextFieldHandler(object):
    '''
    Creates a new instance of TextFieldHandler
    @param xMSF
    @param xTextDocument
    '''

    def __init__(self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextFieldsSupplier = xTextDocument

    def refreshTextFields(self):
        xUp = self.xTextFieldsSupplier.getTextFields()
        xUp.refresh()

    def getUserFieldContent(self, xTextCursor):
        try:
            xTextRange = xTextCursor.getEnd()
            oTextField = Helper.getUnoPropertyValue(xTextRange, "TextField")
            if com.sun.star.uno.AnyConverter.isVoid(oTextField):
                return ""
            else:
                xMaster = oTextField.getTextFieldMaster()
                UserFieldContent = xMaster.getPropertyValue("Content")
                return UserFieldContent

        except Exception, exception:
            traceback.print_exc()

        return ""

    def insertUserField(self, xTextCursor, FieldName, FieldTitle):
        try:
            xField = self.xMSFDoc.createInstance("com.sun.star.text.TextField.User")

            if self.xTextFieldsSupplier.getTextFieldMasters().hasByName("com.sun.star.text.FieldMaster.User." + FieldName):
                oMaster = self.xTextFieldsSupplier.getTextFieldMasters().getByName( \
                    "com.sun.star.text.FieldMaster.User." + FieldName)
                oMaster.dispose()

            xPSet = createUserField(FieldName, FieldTitle)
            xField.attachTextFieldMaster(xPSet)
            xTextCursor.getText().insertTextContent(xTextCursor, xField, False)
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()

    def createUserField(self, FieldName, FieldTitle):
        xPSet = self.xMSFDoc.createInstance("com.sun.star.text.FieldMaster.User")
        xPSet.setPropertyValue(PropertyNames.PROPERTY_NAME, FieldName)
        xPSet.setPropertyValue("Content", FieldTitle)
        return xPSet

    def __getTextFieldsByProperty(self, _PropertyName, _aPropertyValue, _TypeName):
        try:
            xDependentVector = []
            if self.xTextFieldsSupplier.getTextFields().hasElements():
                xEnum = self.xTextFieldsSupplier.getTextFields().createEnumeration()
                while xEnum.hasMoreElements():
                    oTextField = xEnum.nextElement()
                    xPropertySet = oTextField.getTextFieldMaster()
                    if xPropertySet.getPropertySetInfo().hasPropertyByName(_PropertyName):
                        oValue = xPropertySet.getPropertyValue(_PropertyName)
                        if isinstance(oValue,unicode):
                            if _TypeName == "String":
                                sValue = unicodedata.normalize('NFKD', oValue).encode('ascii','ignore')
                                if sValue == _aPropertyValue:
                                    xDependentVector.append(oTextField)
                        #COMMENTED
                        '''elif AnyConverter.isShort(oValue):
                            if _TypeName.equals("Short"):
                                iShortParam = (_aPropertyValue).shortValue()
                                ishortValue = AnyConverter.toShort(oValue)
                                if ishortValue == iShortParam:
                                    xDependentVector.append(oTextField) '''

            if len(xDependentVector) > 0:
                return xDependentVector

        except Exception, e:
            #TODO Auto-generated catch block
            traceback.print_exc()

        return None

    def changeUserFieldContent(self, _FieldName, _FieldContent):
        try:
            xDependentTextFields = self.__getTextFieldsByProperty(PropertyNames.PROPERTY_NAME, _FieldName, "String")
            if xDependentTextFields != None:
                for i in xDependentTextFields:
                    i.getTextFieldMaster().setPropertyValue("Content", _FieldContent)
                self.refreshTextFields()

        except Exception, e:
            traceback.print_exc()

    def updateDocInfoFields(self):
        try:
            xEnum = self.xTextFieldsSupplier.getTextFields().createEnumeration()
            while xEnum.hasMoreElements():
                oTextField = xEnum.nextElement()
                if oTextField.supportsService("com.sun.star.text.TextField.ExtendedUser"):
                    oTextField.update()

                if oTextField.supportsService("com.sun.star.text.TextField.User"):
                    oTextField.update()

        except Exception, e:
            traceback.print_exc()

    def updateDateFields(self):
        try:
            xEnum = self.xTextFieldsSupplier.getTextFields().createEnumeration()
            now = time.localtime(time.time())
            dt = DateTime()
            dt.Day = time.strftime("%d", now)
            dt.Year = time.strftime("%Y", now)
            dt.Month = time.strftime("%m", now)
            dt.Month += 1
            while xEnum.hasMoreElements():
                oTextField = xEnum.nextElement()
                if oTextField.supportsService("com.sun.star.text.TextField.DateTime"):
                    oTextField.setPropertyValue("IsFixed", False)
                    oTextField.setPropertyValue("DateTimeValue", dt)

        except Exception, e:
            traceback.print_exc()

    def fixDateFields(self, _bSetFixed):
        try:
            xEnum = self.xTextFieldsSupplier.getTextFields().createEnumeration()
            while xEnum.hasMoreElements():
                oTextField = xEnum.nextElement()
                if oTextField.supportsService("com.sun.star.text.TextField.DateTime"):
                    oTextField.setPropertyValue("IsFixed", _bSetFixed)

        except Exception, e:
            traceback.print_exc()

    def removeUserFieldByContent(self, _FieldContent):
        try:
            xDependentTextFields = self.__getTextFieldsByProperty("Content", _FieldContent, "String")
            if xDependentTextFields != None:
                i = 0
                while i < xDependentTextFields.length:
                    xDependentTextFields[i].dispose()
                    i += 1

        except Exception, e:
            traceback.print_exc()

    def changeExtendedUserFieldContent(self, UserDataPart, _FieldContent):
        try:
            xDependentTextFields = self.__getTextFieldsByProperty("UserDataType", uno.Any("short",UserDataPart), "Short")
            if xDependentTextFields != None:
                i = 0
                while i < xDependentTextFields.length:
                    xDependentTextFields[i].getTextFieldMaster().setPropertyValue("Content", _FieldContent)
                    i += 1

            self.refreshTextFields()
        except Exception, e:
            traceback.print_exc()

