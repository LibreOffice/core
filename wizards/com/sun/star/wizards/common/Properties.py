from com.sun.star.beans import PropertyValue

'''
Simplifies handling Arrays of PropertyValue.
To make a use of this class, instantiate it, and call
the put(propName,propValue) method.
caution: propName should always be a String.
When finished, call the getProperties() method to get an array of the set properties.
@author  rp
'''

class Properties(dict):

    @classmethod
    def getPropertyValue(self, props, propName):
        for i in props:
            if propName == i.Name:
                return i.Value

        raise AttributeError ("Property '" + propName + "' not found.")

    @classmethod
    def hasPropertyValue(self, props, propName):
        for i in props:
            if propName == i.Name:
                return True
        return False

    @classmethod
    def getProperties(self, _map):
        pv = []
        for k,v in _map.items():
            pv.append(self.createProperty(k, v))
        return pv

    @classmethod
    def createProperty(self, name, value, handle=None):
        pv = PropertyValue()
        pv.Name = name
        pv.Value = value
        if handle is not None:
            pv.Handle = handle
        return pv
