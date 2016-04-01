import unittest
from org.libreoffice.unotest import UnoInProcess

#@unittest.skip("that seems to work")
class TestSetExpresion(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_set_expression(self):
        self.__class__._uno.checkProperties(
            self.__class__._xDoc.createInstance("com.sun.star.text.textfield.SetExpression"),
            {"NumberingType": 0,
             "Content": "foo",
             "CurrentPresentation": "bar",
             "NumberFormat": 0,
             "NumberingType": 0,
             "IsShowFormula": False,
             "IsInput": False,
             "IsVisible": True,
             "SequenceValue": 0,
             "SubType": 0,
             "Value": 1.0,
             "IsFixedLanguage": False
             },
            self
            )

if __name__ == '__main__':
    unittest.main()

