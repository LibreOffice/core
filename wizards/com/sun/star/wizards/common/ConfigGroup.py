from ConfigNode import *
import traceback

class ConfigGroup(ConfigNode):

    def writeConfiguration(self, configurationView, param):
        for i in dir(self):
            if i.startswith(param):
                try:
                    self.writeField(i, configurationView, param)
                except Exception, ex:
                    print "Error writing field:" + i
                    traceback.print_exc()

    def writeField(self, field, configView, prefix):
        propertyName = field.getName().substring(prefix.length())
        fieldType = field.getType()
        if ConfigNode.isAssignableFrom(fieldType):
            childView = Configuration.addConfigNode(configView, propertyName)
            child = field.get(this)
            child.writeConfiguration(childView, prefix)
        elif fieldType.isPrimitive():
            Configuration.set(convertValue(field), propertyName, configView)
        elif isinstance(fieldType,str):
            Configuration.set(field.get(this), propertyName, configView)

    '''
    convert the primitive type value of the
    given Field object to the corresponding
    Java Object value.
    @param field
    @return the value of the field as a Object.
    @throws IllegalAccessException
    '''

    def convertValue(self, field):
        if field.getType().equals(Boolean.TYPE):
            return field.getBoolean(this)

        if field.getType().equals(Integer.TYPE):
            return field.getInt(this)

        if field.getType().equals(Short.TYPE):
            return field.getShort(this)

        if field.getType().equals(Float.TYPE):
            return field.getFloat(this)

        if (field.getType().equals(Double.TYPE)):
            return field.getDouble(this)
        return None
        #and good luck with it :-) ...

    def readConfiguration(self, configurationView, param):
        for i in dir(self):
            if i.startswith(param):
                try:
                    self.readField( i, configurationView, param)
                except Exception, ex:
                    print "Error reading field: " + i
                    traceback.print_exc()

    def readField(self, field, configView, prefix):
        propertyName = field[len(prefix):]
        child = getattr(self, field)
        fieldType = type(child)
        if type(ConfigNode) == fieldType:
            child.setRoot(self.root)
            child.readConfiguration(Configuration.getNode(propertyName, configView), prefix)
            field.set(this, Configuration.getString(propertyName, configView))

    def setRoot(self, newRoot):
        self.root = newRoot
