from ConfigNode import *
from Configuration import Configuration
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
        propertyName = field[len(prefix):]
        field = getattr(self,field)

        if isinstance(field, ConfigNode):
            pass
            #print configView
            #childView = Configuration.addConfigNode(configView, propertyName)
            #self.writeConfiguration(childView, prefix)
        else:
            #print type(field)
            Configuration.Set(self.convertValue(field), propertyName,
                configView)

    '''
    convert the primitive type value of the
    given Field object to the corresponding
    Java Object value.
    @param field
    @return the value of the field as a Object.
    @throws IllegalAccessException
    '''

    def convertValue(self, field):
        if isinstance(field,bool):
            return bool(field)
        elif isinstance(field,int):
            return int(field)

    def readConfiguration(self, configurationView, param):
        for i in dir(self):
            if i.startswith(param):
                try:
                    self.readField( i, configurationView, param)
                except Exception, ex:
                    print "Error reading field: " + i
                    traceback.print_exc()

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
            child.readConfiguration(Configuration.getNode(propertyName, configView),
                prefix)
            field.set(this, Configuration.getString(propertyName, configView))

    def setRoot(self, newRoot):
        self.root = newRoot
