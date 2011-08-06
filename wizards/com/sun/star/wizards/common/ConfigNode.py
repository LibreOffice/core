from abc import ABCMeta, abstractmethod

class ConfigNode(object):

    @abstractmethod
    def readConfiguration(self, configurationView, param):
        pass

    @abstractmethod
    def writeConfiguration(self, configurationView, param):
        pass

    @abstractmethod
    def setRoot(self, root):
        pass
