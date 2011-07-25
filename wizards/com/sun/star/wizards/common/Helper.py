import uno
import traceback
from datetime import date as DateTime
from com.sun.star.uno import RuntimeException
from NumberFormatter import NumberFormatter

class Helper(object):

    @classmethod
    def setUnoPropertyValue(self, xPSet, PropertyName, PropertyValue):
        try:
            if PropertyValue is not None:
                setattr(xPSet, PropertyName, PropertyValue)

        except AttributeError:
            raise AttributeError, "No Such Property: '%s'" % PropertyName
        except Exception:
            traceback.print_exc()

    @classmethod
    def getUnoObjectbyName(self, xName, ElementName):
        try:
            if xName.hasByName(ElementName):
                return xName.getByName(ElementName)
            else:
                raise RuntimeException()

        except Exception, exception:
            traceback.print_exc()
            return None

    @classmethod
    def getPropertyValue(self, CurPropertyValue, PropertyName):
        for i in  CurPropertyValue:
            if i is not None:
                if i.Name == PropertyName:
                    return i.Value

        raise RuntimeException()

    @classmethod
    def getUnoPropertyValue(self, xPSet, PropertyName):
        try:
            if xPSet is not None:
                oObject = xPSet.getPropertyValue(PropertyName)
                return oObject
            return None

        except Exception, exception:
            traceback.print_exc()
            return None

    @classmethod
    def getUnoArrayPropertyValue(self, xPSet, PropertyName):
        try:
            if xPSet is not None:
                oObject = xPSet.getPropertyValue(PropertyName)
                if isinstance(oObject,list):
                    return getArrayValue(oObject)

        except Exception, exception:
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
        except Exception, exception:
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
                for index, workwith in enumerate(PropertyNames):
                    self.setUnoPropertyValue(
                        xMultiPSetLst, PropertyNames[index], workwith)

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

        except Exception, exception:
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
            defaults = xmsf.createInstance("com.sun.star.text.Defaults")
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
