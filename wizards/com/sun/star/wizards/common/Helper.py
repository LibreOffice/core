import uno
import calendar
import traceback
from datetime import date as DateTime
from com.sun.star.uno import Exception as UnoException
from com.sun.star.uno import RuntimeException
from NumberFormatter import NumberFormatter

class Helper(object):

    def convertUnoDatetoInteger(self, DateValue):
        oCal = java.util.Calendar.getInstance()
        oCal.set(DateValue.Year, DateValue.Month, DateValue.Day)
        dTime = oCal.getTime()
        lTime = dTime.getTime()
        lDate = lTime / (3600 * 24000)
        return lDate

    @classmethod
    def setUnoPropertyValue(self, xPSet, PropertyName, PropertyValue):
        try:
            if xPSet.getPropertySetInfo().hasPropertyByName(PropertyName):
                uno.invoke(xPSet,"setPropertyValue",
                    (PropertyName,PropertyValue))
            else:
                selementnames = xPSet.getPropertySetInfo().getProperties()
                raise ValueError("No Such Property: '" + PropertyName + "'")

        except UnoException, exception:
            traceback.print_exc()

    @classmethod
    def getUnoObjectbyName(self, xName, ElementName):
        try:
            if xName.hasByName(ElementName) == True:
                return xName.getByName(ElementName)
            else:
                raise RuntimeException();

        except UnoException, exception:
            traceback.print_exc()
            return None

    @classmethod
    def getPropertyValue(self, CurPropertyValue, PropertyName):
        MaxCount = len(CurPropertyValue)
        i = 0
        while i < MaxCount:
            if CurPropertyValue[i] is not None:
                if CurPropertyValue[i].Name.equals(PropertyName):
                    return CurPropertyValue[i].Value

            i += 1
        raise RuntimeException()

    @classmethod
    def getPropertyValuefromAny(self, CurPropertyValue, PropertyName):
        try:
            if CurPropertyValue is not None:
                MaxCount = len(CurPropertyValue)
                i = 0
                while i < MaxCount:
                    if CurPropertyValue[i] is not None:
                        aValue = CurPropertyValue[i]
                        if aValue is not None and aValue.Name == PropertyName:
                            return aValue.Value

                    i += 1
            return None
        except UnoException, exception:
            traceback.print_exc()
            return None

    @classmethod
    def getUnoPropertyValue(self, xPSet, PropertyName):
        try:
            if xPSet is not None:
                oObject = xPSet.getPropertyValue(PropertyName)
                if oObject is not None:
                    return oObject
            return None

        except UnoException, exception:
            traceback.print_exc()
            return None

    @classmethod
    def getUnoArrayPropertyValue(self, xPSet, PropertyName):
        try:
            if xPSet is not None:
                oObject = xPSet.getPropertyValue(PropertyName)
                if isinstance(oObject,list):
                    return getArrayValue(oObject)

        except UnoException, exception:
            traceback.print_exc()

        return None

    @classmethod
    def getUnoStructValue(self, xPSet, PropertyName):
        try:
            if xPSet is not None:
                if xPSet.getPropertySetInfo().hasPropertyByName(PropertyName):
                    oObject = xPSet.getPropertyValue(PropertyName)
                    return oObject

            return None
        except UnoException, exception:
            traceback.print_exc()
            return None

    @classmethod
    def setUnoPropertyValues(self, xMultiPSetLst, PropertyNames,
        PropertyValues):
        try:
            if xMultiPSetLst is not None:
                uno.invoke(xMultiPSetLst, "setPropertyValues",
                    (PropertyNames, PropertyValues))
            else:
                i = 0
                while i < len(PropertyNames):
                    self.setUnoPropertyValue(xMultiPSetLst, PropertyNames[i],
                        PropertyValues[i])
                    i += 1

        except Exception, e:
            traceback.print_exc()

    '''
    checks if the value of an object that represents an array is null.
    check beforehand if the Object is really an array with
    "AnyConverter.IsArray(oObject)
    @param oValue the paramter that has to represent an object
    @return a null reference if the array is empty
    '''

    @classmethod
    def getArrayValue(self, oValue):
        try:
            #VetoableChangeSupport Object
            oPropList = list(oValue)
            nlen = len(oPropList)
            if nlen == 0:
                return None
            else:
                return oPropList

        except UnoException, exception:
            traceback.print_exc()
            return None

    def getComponentContext(_xMSF):
        #Get the path to the extension and
        #try to add the path to the class loader
        aHelper = PropertySetHelper(_xMSF);
        aDefaultContext = aHelper.getPropertyValueAsObject("DefaultContext")
        return aDefaultContext;

    def getMacroExpander(_xMSF):
        xComponentContext = self.getComponentContext(_xMSF);
        aSingleton = xComponentContext.getValueByName(
            "/singletons/com.sun.star.util.theMacroExpander")
        return aSingleton;

    class DateUtils(object):

        def __init__(self, xmsf, document):
            defaults = document.createInstance("com.sun.star.text.Defaults")
            l = Helper.getUnoStructValue(defaults, "CharLocale")
            self.formatSupplier = document
            formatSettings = self.formatSupplier.getNumberFormatSettings()
            date = Helper.getUnoPropertyValue(formatSettings, "NullDate")
            self.calendar = DateTime(date.Year, date.Month, date.Day)
            self.formatter = NumberFormatter.createNumberFormatter(xmsf,
                self.formatSupplier)

        '''
        @param format a constant of the enumeration NumberFormatIndex
        @return
        '''

        def getFormat(self, format):
            return NumberFormatter.getNumberFormatterKey(
                self.formatSupplier, format)

        '''
        @param date a VCL date in form of 20041231
        @return a document relative date
        '''

        def format(self, formatIndex, date):
            difference =  date - self.calendar
            return self.formatter.convertNumberToString(formatIndex,
                difference.days)
