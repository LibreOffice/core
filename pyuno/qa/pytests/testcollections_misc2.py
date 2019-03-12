
# execute run procedure as Python macro for testing

import uno
import sys
import unittest

from com.sun.star.awt.FontSlant import ITALIC
from com.sun.star.awt.FontSlant import NONE
from com.sun.star.uno.TypeClass import STRING
from com.sun.star.uno.TypeClass import LONG
from com.sun.star.awt import Point

class Test124953(unittest.TestCase):

    def test_Enum(self):
        italic = uno.Enum("com.sun.star.awt.FontSlant", "ITALIC")
        none_ = uno.Enum("com.sun.star.awt.FontSlant", "NONE")
        self.assertEqual(ITALIC, ITALIC)
        self.assertEqual(ITALIC, italic)
        self.assertFalse((ITALIC != italic))
        self.assertNotEqual(ITALIC, NONE)
        self.assertEqual(NONE, none_)

    def test_Type(self):

        STRING_TYPE = uno.getTypeByName("string")
        LONG_TYPE = uno.getTypeByName("long")
        string_type = uno.Type("string", STRING)
        long_type = uno.Type("long", LONG)
        self.assertEqual(STRING_TYPE, STRING_TYPE)
        self.assertEqual(STRING_TYPE, string_type)
        self.assertFalse((STRING_TYPE != string_type))
        self.assertNotEqual(STRING_TYPE, LONG)
        self.assertEqual(LONG_TYPE, long_type)

    def test_Char(self):
        if sys.version_info[0] == 3:
            char_a = uno.Char("a")
            char_a2 = uno.Char("a")
            char_b = uno.Char("b")
        else:
            char_a = uno.Char(u"a")
            char_a2 = uno.Char(u"a")
            char_b = uno.Char(u"b")
        self.assertEqual(char_a, char_a)
        self.assertEqual(char_a, char_a2)
        self.assertFalse((char_a != char_a2))
        self.assertNotEqual(char_a, char_b)

    def test_ByteSequence(self):
        if sys.version_info[0] == 3:
            b1 = uno.ByteSequence(bytes("abcdefg", encoding="utf8"))
            b2 = uno.ByteSequence(bytes("abcdefg", encoding="utf8"))
            b3 = uno.ByteSequence(bytes("1234567", encoding="utf8"))
        else:
            b1 = uno.ByteSequence("abcdefg")
            b2 = uno.ByteSequence("abcdefg")
            b3 = uno.ByteSequence("1234567")
        self.assertEqual(b1, b1)
        self.assertEqual(b1, b2)
        self.assertFalse(b1 != b2)
        self.assertNotEqual(b1, b3)

    def test_Struct(self):
        point1 = Point(100, 200)
        point2 = Point(100, 200)
        point3 = Point(0, 10)
        self.assertEqual(point1, point1)
        self.assertEqual(point1, point2)
        self.assertFalse((point1 != point2))
        self.assertNotEqual(point1, point3)
