import traceback
from DataAware import *
import uno
'''
This class is a factory for Value objects for different types of
memebers.
Other than some Value implementations classes this class contains static
type conversion methods and factory methods.

@see com.sun.star.wizards.ui.event.DataAware.Value
'''

class DataAwareFields(object):
    TRUE = "true"
    FALSE = "false"
    '''
    returns a Value Object which sets and gets values
    and converting them to other types, according to the "value" argument.

    @param owner
    @param fieldname
    @param value
    @return
    @throws NoSuchFieldException
    '''

    @classmethod
    def getFieldValueFor(self, owner, fieldname, value):
        try:
            f = getattr(owner, fieldname)
            if isinstance(f,bool):
                pass
                return self.__BooleanFieldValue(fieldname, value)
            elif isinstance(f,str):
                pass
                return self.__ConvertedStringValue(fieldname, value)
            elif isinstance(f,int):
                pass
                return self.__IntFieldValue(fieldname, value)
            elif isinstance(f,float):
                pass
                return self.__IntFieldValue(fieldname, value)
            else:
                pass
                return self.__IntFieldValue(fieldname, value)

        except AttributeError, ex:
            traceback.print_exc()
            return None

    '''__ConvertedStringValue
    an abstract implementation of DataAware.Value to access
    object memebers (fields) usign reflection.
    '''
    class __FieldValue(DataAware.Value):
        __metaclass__ = ABCMeta

        def __init__(self, field_):
            self.field = field_

        def isAssignable(self, type_):
            return self.field.getDeclaringClass().isAssignableFrom(type_)

    class __BooleanFieldValue(__FieldValue):

        def __init__(self, f, convertTo_):
            super(type(self),self).__init__(f)
            self.convertTo = convertTo_

        def get(self, target):
            try:
                b = getattr(target, self.field)

                if isinstance(self.convertTo,bool):
                    if b:
                        return True
                    else:
                        return False
                elif isinstance(self.convertTo,int):
                    return int(b)
                elif isinstance(self.convertTo,str):
                    return str(b)
                elif self.convertTo.type == uno.Any("short",0).type:
                    return uno.Any("short",b)
                else:
                    raise AttributeError(
                        "Cannot convert boolean value to given type (" + \
                        str(type(self.convertTo)) + ").")

            except Exception, ex:
                traceback.print_exc()
                return None

        def set(self, value, target):
            try:
                self.field.setBoolean(target, toBoolean(value))
            except Exception, ex:
                traceback.print_exc()

    class __IntFieldValue(__FieldValue):

        def __init__(self, f, convertTo_):
            super(type(self),self).__init__(f)
            self.convertTo = convertTo_

        def get(self, target):
            try:
                i = getattr(target, self.field)
                if isinstance(self.convertTo,bool):
                    if i:
                        return True
                    else:
                        return False
                elif isinstance(self.convertTo, int):
                    return int(i)
                elif isinstance(self.convertTo,str):
                    return str(i)
                elif self.convertTo.type == uno.Any("short",0).type:
                    return uno.Any("[]short",(i,))
                else:
                    raise AttributeError(
                        "Cannot convert int value to given type (" + \
                        str(type(self.convertTo)) + ").");

            except Exception, ex:
                traceback.print_exc()
                #traceback.print_exc__ConvertedStringValue()
                return None

        def set(self, value, target):
            try:
                self.field.setInt(target, toDouble(value))
            except Exception, ex:
                traceback.print_exc()

    class __ConvertedStringValue(__FieldValue):

        def __init__(self, f, convertTo_):
            super(type(self),self).__init__(f)
            self.convertTo = convertTo_

        def get(self, target):
            try:
                s = getattr(target, self.field)
                if isinstance(self.convertTo,bool):
                    if s != None and not s == "" and s == "true":
                        return True
                    else:
                        return False
                elif isinstance(self.convertTo,str):
                    if s == None or s == "":
                        pass
                    else:
                        return s
                else:
                    raise AttributeError(
                        "Cannot convert int value to given type (" + \
                        str(type(self.convertTo)) + ")." )

            except Exception, ex:
                traceback.print_exc()
                return None

        def set(self, value, target):
            try:
                string_aux = ""
                #if value is not None or not isinstance(value,uno.Any()):
                #    string_aux = str(value)

                self.field.set(target, string_aux)
            except Exception, ex:
                traceback.print_exc()
