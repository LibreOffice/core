import uno
import locale
import traceback
from com.sun.star.uno import Exception as UnoException
from com.sun.star.uno import RuntimeException

#TEMPORAL
import inspect

class Helper(object):

    DAY_IN_MILLIS = (24 * 60 * 60 * 1000)

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
                #xPSet.setPropertyValue(PropertyName, PropertyValue)
                uno.invoke(xPSet,"setPropertyValue", (PropertyName,PropertyValue))
            else:
                selementnames = xPSet.getPropertySetInfo().getProperties()
                raise ValueError("No Such Property: '" + PropertyName + "'");

        except UnoException, exception:
            print type(PropertyValue)
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
                        if aValue is not None and aValue.Name.equals(PropertyName):
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
                if xPSet.getPropertySetInfo().hasPropertyByName(PropertyName) == True:
                    oObject = xPSet.getPropertyValue(PropertyName)
                    return oObject

            return None
        except UnoException, exception:
            traceback.print_exc()
            return None

    @classmethod
    def setUnoPropertyValues(self, xMultiPSetLst, PropertyNames, PropertyValues):
        try:
            if xMultiPSetLst is not None:
                uno.invoke(xMultiPSetLst, "setPropertyValues", (PropertyNames, PropertyValues))
            else:
                i = 0
                while i < len(PropertyNames):
                    self.setUnoPropertyValue(xMultiPSetLst, PropertyNames[i], PropertyValues[i])
                    i += 1

        except Exception, e:
            curframe = inspect.currentframe()
            calframe = inspect.getouterframes(curframe, 2)
            #print "caller name:", calframe[1][3]
            traceback.print_exc()

    '''
    checks if the value of an object that represents an array is null.
    check beforehand if the Object is really an array with "AnyConverter.IsArray(oObject)
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
        # Get the path to the extension and try to add the path to the class loader
        aHelper = PropertySetHelper(_xMSF);
        aDefaultContext = aHelper.getPropertyValueAsObject("DefaultContext");
        return aDefaultContext;

    def getMacroExpander(_xMSF):
        xComponentContext = self.getComponentContext(_xMSF);
        aSingleton = xComponentContext.getValueByName("/singletons/com.sun.star.util.theMacroExpander");
        return aSingleton;

    class DateUtils(object):

        @classmethod
        def DateUtils_XMultiServiceFactory_Object(self, xmsf, document):
            tmp = DateUtils()
            tmp.DateUtils_body_XMultiServiceFactory_Object(xmsf, document)
            return tmp

        def DateUtils_body_XMultiServiceFactory_Object(self, xmsf, docMSF):
            defaults = docMSF.createInstance("com.sun.star.text.Defaults")
            l = Helper.getUnoStructValue(defaults, "CharLocale")
            jl = locale.setlocale(l.Language, l.Country, l.Variant)
            self.calendar = Calendar.getInstance(jl)
            self.formatSupplier = document
            formatSettings = self.formatSupplier.getNumberFormatSettings()
            date = Helper.getUnoPropertyValue(formatSettings, "NullDate")
            self.calendar.set(date.Year, date.Month - 1, date.Day)
            self.docNullTime = getTimeInMillis()
            self.formatter = NumberFormatter.createNumberFormatter(xmsf, self.formatSupplier)

        '''
        @param format a constant of the enumeration NumberFormatIndex
        @return
        '''

        def getFormat(self, format):
            return NumberFormatter.getNumberFormatterKey(self.formatSupplier, format)

        def getFormatter(self):
            return self.formatter

        def getTimeInMillis(self):
            dDate = self.calendar.getTime()
            return dDate.getTime()

        '''
        @param date a VCL date in form of 20041231
        @return a document relative date
        '''

        def getDocumentDateAsDouble(self, date):
            self.calendar.clear()
            self.calendar.set(date / 10000, (date % 10000) / 100 - 1, date % 100)
            date1 = getTimeInMillis()
            '''
            docNullTime and date1 are in millis, but
            I need a day...
            '''

            daysDiff = (date1 - self.docNullTime) / DAY_IN_MILLIS + 1
            return daysDiff

        def getDocumentDateAsDouble(self, date):
            return getDocumentDateAsDouble(date.Year * 10000 + date.Month * 100 + date.Day)

        def getDocumentDateAsDouble(self, javaTimeInMillis):
            self.calendar.clear()
            JavaTools.setTimeInMillis(self.calendar, javaTimeInMillis)
            date1 = getTimeInMillis()

            '''
            docNullTime and date1 are in millis, but
            I need a day...
            '''

            daysDiff = (date1 - self.docNullTime) / DAY_IN_MILLIS + 1
            return daysDiff

        def format(self, formatIndex, date):
            return self.formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(date))

        def format(self, formatIndex, date):
            return self.formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(date))

        def format(self, formatIndex, javaTimeInMillis):
            return self.formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(javaTimeInMillis))
