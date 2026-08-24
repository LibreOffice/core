import os
import sys
import unittest

class CompatTest(unittest.TestCase):

    # test if the compatibility shim that allows importing
    # UNO types that have moved from com:sun::star to cpo works.
    def test_com_sun_star_alias_import(self):
        import uno
        from com.sun.star.uno.TypeClass import STRING
        string_type = uno.Type("string", STRING)

if __name__ == '__main__':
    unittest.main()
