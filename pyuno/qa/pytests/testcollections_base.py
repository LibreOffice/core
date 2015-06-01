import unittest
import uno

from org.libreoffice.unotest import pyuno
from com.sun.star.beans import PropertyValue

class CollectionsTestBase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.context = pyuno.getComponentContext()
        pyuno.private_initTestEnvironment()

    def setUp(self):
        self._components = []

    def tearDown(self):
        for component in self._components:
            try:
                component.close(True)
            except Exception:
                pass

    def createHiddenWindow(self, url):
        serviceManager = self.context.ServiceManager
        desktop = serviceManager.createInstanceWithContext('com.sun.star.frame.Desktop', self.context)
        loadProps = (
          PropertyValue(Name='Hidden', Value=True),
          PropertyValue(Name='ReadOnly', Value=False)
        )
        component = desktop.loadComponentFromURL(url, '_blank', 0, loadProps)
        return component

    def createBlankTextDocument(self):
        component = self.createHiddenWindow('private:factory/swriter')
        self._components.append(component)
        return component

    def createBlankSpreadsheet(self):
        component = self.createHiddenWindow('private:factory/scalc')
        self._components.append(component)
        return component

    def createBlankDrawing(self):
        component = self.createHiddenWindow('private:factory/sdraw')
        self._components.append(component)
        return component

# vim:set shiftwidth=4 softtabstop=4 expandtab: