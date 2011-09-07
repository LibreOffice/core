from ConfigNode import *
from Configuration import Configuration
import traceback
import inspect

class ConfigGroup(ConfigNode):

    def writeConfiguration(self, configurationView, param):
        for name,data in inspect.getmembers(self):
            if name.startswith(param):
                self.writeField( name, configurationView, param)

    def writeField(self, field, configView, prefix):
        propertyName = field[len(prefix):]
        child = getattr(self, field)
        if isinstance(child, ConfigNode):
            child.writeConfiguration(configView.getByName(propertyName),
                prefix)
        else:
            setattr(configView,propertyName,getattr(self,field))

    def readConfiguration(self, configurationView, param):
        for name,data in inspect.getmembers(self):
            if name.startswith(param):
                self.readField( name, configurationView, param)

    def readField(self, field, configView, prefix):
        propertyName = field[len(prefix):]
        child = getattr(self, field)
        if isinstance(child, ConfigNode):
            child.root = self.root
            child.readConfiguration(configView.getByName(propertyName),
                prefix)
        else:
            setattr(self,field,configView.getByName(propertyName))
