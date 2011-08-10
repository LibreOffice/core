from abc import ABCMeta, abstractmethod

class ConfigNode(object):

    root = None

    @abstractmethod
    def readConfiguration(self, configurationView, param):
        pass

    @abstractmethod
    def writeConfiguration(self, configurationView, param):
        pass
