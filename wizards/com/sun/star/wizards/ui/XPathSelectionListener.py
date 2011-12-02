from abc import ABCMeta, abstractmethod

class XPathSelectionListener(object):

    @abstractmethod
    def validatePath(self):
        pass
