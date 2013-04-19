import unittest
from org.libreoffice.unotest import UnoInProcess

class TestGetExpression(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_get_expression(self):
        self.__class__._uno.checkProperties(
            self.__class__._xDoc.createInstance("com.sun.star.text.textfield.GetExpression"),
            {"Content": "foo",
             "CurrentPresentation": "bar",
             "NumberFormat": 0,
             "IsShowFormula": False,
             "SubType": 0,
             "VariableSubtype": 1,
             "IsFixedLanguage": False,
             },
            self
            )

    # property 'Value' is read only?
    @unittest.expectedFailure
    def test_get_expression_veto_read_only(self):
        self.__class__._uno.checkProperties(
            self.__class__._xDoc.createInstance("com.sun.star.text.textfield.GetExpression"),
            {"Value": 0.0},
            self
            )

if __name__ == '__main__':
    unittest.main()

