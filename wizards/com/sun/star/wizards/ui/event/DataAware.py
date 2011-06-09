from common.PropertyNames import *
from abc import ABCMeta, abstractmethod
import traceback

'''
@author rpiterman
DataAware objects are used to live-synchronize UI and DataModel/DataObject.
It is used as listener on UI events, to keep the DataObject up to date.
This class, as a base abstract class, sets a frame of functionality,
delegating the data Object get/set methods to a Value object,
and leaving the UI get/set methods abstract.
Note that event listenning is *not* a part of this model.
the updateData() or updateUI() methods should be porogramatically called.
in child classes, the updateData() will be binded to UI event calls.
<br><br>
This class holds references to a Data Object and a Value object.
The Value object "knows" how to get and set a value from the
Data Object.
'''

class DataAware(object):
    __metaclass__ = ABCMeta

    '''
    creates a DataAware object for the given data object and Value object.
    @param dataObject_
    @param value_
    '''

    def __init__(self, dataObject_, value_):
        self._dataObject = dataObject_
        self._value = value_

    '''
    Sets the given value to the data object.
    this method delegates the job to the
    Value object, but can be overwritten if
    another kind of Data is needed.
    @param newValue the new value to set to the DataObject.
    '''

    def setToData(self, newValue):
        self._value.set(newValue, self._dataObject)

    '''
    gets the current value from the data obejct.
    this method delegates the job to
    the value object.
    @return the current value of the data object.
    '''

    def getFromData(self):
        return self._value.get(self._dataObject)

    '''
    sets the given value to the UI control
    @param newValue the value to set to the ui control.
    '''
    @abstractmethod
    def setToUI (self,newValue):
        pass

    '''
    gets the current value from the UI control.
    @return the current value from the UI control.
    '''

    @abstractmethod
    def getFromUI (self):
        pass

    '''
    updates the UI control according to the
    current state of the data object.
    '''

    def updateUI(self):
        data = self.getFromData()
        ui = self.getFromUI()
        if data != ui:
            try:
                self.setToUI(data)
            except Exception, ex:
                traceback.print_exc()
                #TODO tell user...

    '''
    updates the DataObject according to
    the current state of the UI control.
    '''

    def updateData(self):
        data = self.getFromData()
        ui = self.getFromUI()
        if not equals(data, ui):
            setToData(ui)

    class Listener(object):
        @abstractmethod
        def eventPerformed (self, event):
            pass

    '''
    compares the two given objects.
    This method is null safe and returns true also if both are null...
    If both are arrays, treats them as array of short and compares them.
    @param a first object to compare
    @param b second object to compare.
    @return true if both are null or both are equal.
    '''

    def equals(self, a, b):
        if not a and not b :
            return True

        if not a or not b:
            return False

        if a.getClass().isArray():
            if b.getClass().isArray():
                return Arrays.equals(a, b)
            else:
                return False

        return a.equals(b)

    '''
    given a collection containing DataAware objects,
    calls updateUI() on each memebr of the collection.
    @param dataAwares a collection containing DataAware objects.
    '''
    @classmethod
    def updateUIs(self, dataAwares):
        for i in dataAwares:
            i.updateUI()

    '''
    Given a collection containing DataAware objects,
    sets the given DataObject to each DataAware object
    in the given collection
    @param dataAwares a collection of DataAware objects.
    @param dataObject new data object to set to the DataAware objects in the given collection.
    @param updateUI if true, calls updateUI() on each DataAware object.
    '''

    def setDataObject(self, dataObject, updateUI):
        if dataObject != None and not (type(self._value) is not type(dataObject)):
            raise ClassCastException ("can not cast new DataObject to original Class");
        self._dataObject = dataObject
        if updateUI:
            self.updateUI()

    '''
    Given a collection containing DataAware objects,
    sets the given DataObject to each DataAware object
    in the given collection
    @param dataAwares a collection of DataAware objects.
    @param dataObject new data object to set to the DataAware objects in the given collection.
    @param updateUI if true, calls updateUI() on each DataAware object.
    '''

    @classmethod
    def setDataObjects(self, dataAwares, dataObject, updateUI):
        for i in dataAwares:
            i.setDataObject(dataObject, updateUI);

    '''
    Value objects read and write a value from and
    to an object. Typically using reflection and JavaBeans properties
    or directly using memeber reflection API.
    DataAware delegates the handling of the DataObject
    to a Value object.
    2 implementations currently exist: PropertyValue,
    using JavaBeans properties reflection, and DataAwareFields classes
    which implement different memeber types.
    '''
    class Value (object):

        '''gets a value from the given object.
        @param target the object to get the value from.
        @return the value from the given object.
        '''
        @abstractmethod
        def Get (self, target):
            pass

        '''
        sets a value to the given object.
        @param value the value to set to the object.
        @param target the object to set the value to.
        '''
        @abstractmethod
        def Set (self, value, target):
            pass

        '''
        checks if this Value object can handle
        the given object type as a target.
        @param type the type of a target to check
        @return true if the given class is acceptible for
        the Value object. False if not.
        '''
        @abstractmethod
        def isAssifrom(self, Type):
            pass

    '''
    implementation of Value, handling JavaBeans properties through
    reflection.
    This Object gets and sets a value a specific
    (JavaBean-style) property on a given object.
    @author rp143992
    '''
    class PropertyValue(Value):

        __EMPTY_ARRAY = range(0)

        '''
        creates a PropertyValue for the property with
        the given name, of the given JavaBean object.
        @param propertyName the property to access. Must be a Cup letter (e.g. PropertyNames.PROPERTY_NAME for getName() and setName("..."). )
        @param propertyOwner the object which "own" or "contains" the property.
        '''

        def __init__(self, propertyName, propertyOwner):
            self.getMethod = createGetMethod(propertyName, propertyOwner)
            self.setMethod = createSetMethod(propertyName, propertyOwner, self.getMethod.getReturnType())

        '''
        called from the constructor, and creates a get method reflection object
        for the given property and object.
        @param propName the property name0
        @param obj the object which contains the property.
        @return the get method reflection object.
        '''

        def createGetMethod(self, propName, obj):
            m = None
            try:
                #try to get a "get" method.
                m = obj.getClass().getMethod("get" + propName, self.__class__.__EMPTY_ARRAY)
            except NoSuchMethodException, ex1:
                raise IllegalArgumentException ("get" + propName + "() method does not exist on " + obj.getClass().getName());

            return m

        def Get(self, target):
            try:
                return self.getMethod.invoke(target, self.__class__.__EMPTY_ARRAY)
            except IllegalAccessException, ex1:
                ex1.printStackTrace()
            except InvocationTargetException, ex2:
                ex2.printStackTrace()
            except NullPointerException, npe:
                if isinstance(self.getMethod.getReturnType(),str):
                    return ""

                if isinstance(self.getMethod.getReturnType(),int ):
                    return 0

                if isinstance(self.getMethod.getReturnType(),tuple):
                    return 0

                if isinstance(self.getMethod.getReturnType(),list ):
                    return []

            return None

        def createSetMethod(self, propName, obj, paramClass):
            m = None
            try:
                m = obj.getClass().getMethod("set" + propName, [paramClass])
            except NoSuchMethodException, ex1:
                raise IllegalArgumentException ("set" + propName + "(" + self.getMethod.getReturnType().getName() + ") method does not exist on " + obj.getClass().getName());

            return m

        def Set(self, value, target):
            try:
                self.setMethod.invoke(target, [value])
            except IllegalAccessException, ex1:
                ex1.printStackTrace()
            except InvocationTargetException, ex2:
                ex2.printStackTrace()

        def isAssignable(self, type):
            return self.getMethod.getDeclaringClass().isAssignableFrom(type) and self.setMethod.getDeclaringClass().isAssignableFrom(type)

