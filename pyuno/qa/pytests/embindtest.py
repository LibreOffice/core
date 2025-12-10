# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import org.libreoffice.unotest
import unittest
import uno
import unohelper
from com.sun.star.uno import RuntimeException
from org.libreoffice.embindtest import Exception
from org.libreoffice.embindtest import Struct
from org.libreoffice.embindtest import StructLong
from org.libreoffice.embindtest import StructString
from org.libreoffice.embindtest import XTest
from org.libreoffice.embindtest.Enum import E3
from org.libreoffice.embindtest.Enum import E_10
from org.libreoffice.embindtest.Enum import E_2

class Test(unohelper.Base, XTest):
    def getBoolean(self):
        return True

    def isBoolean(self, value):
        return value == True

    def getByte(self):
        return -12

    def isByte(self, value):
        return value == -12

    def getShort(self):
        return -1234

    def isShort(self, value):
        return value == -1234

    def getUnsignedShort(self):
        return 54321

    def isUnsignedShort(self, value):
        return value == 54321

    def getLong(self):
        return -123456

    def isLong(self, value):
        return value == -123456

    def getUnsignedLong(self):
        return 3456789012

    def isUnsignedLong(self, value):
        return value == 3456789012

    def getHyper(self):
        return -123456789

    def isHyper(self, value):
        return value == -123456789

    def getUnsignedHyper(self):
        return 9876543210

    def isUnsignedHyper(self, value):
        return value == 9876543210

    def getFloat(self):
        return -10.25

    def isFloat(self, value):
        return value == -10.25

    def getDouble(self):
        return 100.5

    def isDouble(self, value):
        return value == 100.5

    def getChar(self):
        return 'Ö'

    def isChar(self, value):
        return value == 'Ö'

    def getString(self):
        return 'hä'

    def isString(self, value):
        return value == 'hä'

    def getType(self):
        return uno.getTypeByName('long')

    def isType(self, value):
        return value == uno.getTypeByName('long')

    def getAnyVoid(self):
        return None

    def isAnyVoid(self, value):
        return value == None

    def getAnyBoolean(self):
        return True

    def isAnyBoolean(self, value):
        return value == True

    def getAnyByte(self):
        return uno.Any('byte', -12)

    def isAnyByte(self, value):
        return value == -12

    def getAnyShort(self):
        return uno.Any('short', -1234)

    def isAnyShort(self, value):
        return value == -1234

    def getAnyUnsignedShort(self):
        return uno.Any('unsigned short', 54321)

    def isAnyUnsignedShort(self, value):
        return value == 54321

    def getAnyLong(self):
        return -123456

    def isAnyLong(self, value):
        return value == -123456

    def getAnyUnsignedLong(self):
        return uno.Any('unsigned long', 3456789012)

    def isAnyUnsignedLong(self, value):
        return value == 3456789012

    def getAnyHyper(self):
        return uno.Any('hyper', -123456789)

    def isAnyHyper(self, value):
        return value == -123456789

    def getAnyUnsignedHyper(self):
        return uno.Any('unsigned hyper', 9876543210)

    def isAnyUnsignedHyper(self, value):
        return value == 9876543210

    def getAnyFloat(self):
        return uno.Any('float', -10.25)

    def isAnyFloat(self, value):
        return value == -10.25

    def getAnyDouble(self):
        return 100.5

    def isAnyDouble(self, value):
        return value == 100.5

    def getAnyChar(self):
        return uno.Char('Ö')

    def isAnyChar(self, value):
        return value == 'Ö'

    def getAnyString(self):
        return 'hä'

    def isAnyString(self, value):
        return value == 'hä'

    def getAnyType(self):
        return uno.getTypeByName('long')

    def isAnyType(self, value):
        return value == uno.getTypeByName('long')

    def getAnySequence(self):
        return uno.Any('[]string', ('foo', 'barr', 'bazzz'))

    def isAnySequence(self, value):
        return value == ('foo', 'barr', 'bazzz')

    def getAnyEnum(self):
        return E_2

    def isAnyEnum(self, value):
        return value == E_2

    def getAnyStruct(self):
        return Struct(
            True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
            'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
            StructLong(-123456),
            uno.createUnoStruct(
                'org.libreoffice.embindtest.Template<any,org.libreoffice.embindtest.StructString>',
                StructString('foo'), -123456, -123456, StructString('barr')),
            self)

    def isAnyStruct(self, value):
        return value == Struct(
            True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
            'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
            StructLong(-123456),
            uno.createUnoStruct(
                'org.libreoffice.embindtest.Template<any,org.libreoffice.embindtest.StructString>',
                StructString('foo'), -123456, -123456, StructString('barr')),
            self)

    def getAnyException(self):
        return Exception('error', None, -123456, 100.5, 'hä')

    def isAnyException(self, value):
        return value == Exception('error', None, -123456, 100.5, 'hä')

    def getAnyInterface(self):
        return uno.Any('org.libreoffice.embindtest.XTest', self)

    def isAnyInterface(self, value):
        return value == self

    def getEnum(self):
        return E_2

    def isEnum(self, value):
        return value == E_2

    def getStruct(self):
        return Struct(
            True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
            'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
            StructLong(-123456),
            uno.createUnoStruct(
                'org.libreoffice.embindtest.Template<any,org.libreoffice.embindtest.StructString>',
                StructString('foo'), -123456, -123456, StructString('barr')),
            self)

    def isStruct(self, value):
        return value == Struct(
            True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
            'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
            StructLong(-123456),
            uno.createUnoStruct(
                'org.libreoffice.embindtest.Template<any,org.libreoffice.embindtest.StructString>',
                StructString('foo'), -123456, -123456, StructString('barr')),
            self)

    def getStructLong(self):
        return StructLong(-123456)

    def isStructLong(self, value):
        return value == StructLong(-123456)

    def getStructString(self):
        return StructString('hä')

    def isStructString(self, value):
        return value == StructString('hä')

    def getSequenceBoolean(self):
        return (True, True, False)

    def isSequenceBoolean(self, value):
        return value == (True, True, False)

    def getSequenceByte(self):
        return (-12, 1, 12)

    def isSequenceByte(self, value):
        return value == b'\xF4\x01\x0C'

    def getSequenceShort(self):
        return (-1234, 1, 1234)

    def isSequenceShort(self, value):
        return value == (-1234, 1, 1234)

    def getSequenceUnsignedShort(self):
        return (1, 10, 54321)

    def isSequenceUnsignedShort(self, value):
        return value == (1, 10, 54321)

    def getSequenceLong(self):
        return (-123456, 1, 123456)

    def isSequenceLong(self, value):
        return value == (-123456, 1, 123456)

    def getSequenceUnsignedLong(self):
        return (1, 10, 3456789012)

    def isSequenceUnsignedLong(self, value):
        return value == (1, 10, 3456789012)

    def getSequenceHyper(self):
        return (-123456789, 1, 123456789)

    def isSequenceHyper(self, value):
        return value == (-123456789, 1, 123456789)

    def getSequenceUnsignedHyper(self):
        return (1, 10, 9876543210)

    def isSequenceUnsignedHyper(self, value):
        return value == (1, 10, 9876543210)

    def getSequenceFloat(self):
        return (-10.25, 1.5, 10.75)

    def isSequenceFloat(self, value):
        return value == (-10.25, 1.5, 10.75)

    def getSequenceDouble(self):
        return (-100.5, 1.25, 100.75)

    def isSequenceDouble(self, value):
        return value == (-100.5, 1.25, 100.75)

    def getSequenceChar(self):
        return ('a', 'B', 'Ö')

    def isSequenceChar(self, value):
        return value == ('a', 'B', 'Ö')

    def getSequenceString(self):
        return ('foo', 'barr', 'bazzz')

    def isSequenceString(self, value):
        return value == ('foo', 'barr', 'bazzz')

    def getSequenceType(self):
        return (
            uno.getTypeByName('long'), uno.getTypeByName('void'),
            uno.getTypeByName('[]org.libreoffice.embindtest.Enum'))

    def isSequenceType(self, value):
        return value == (
            uno.getTypeByName('long'), uno.getTypeByName('void'),
            uno.getTypeByName('[]org.libreoffice.embindtest.Enum'))

    def getSequenceAny(self):
        return (-123456, None, uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)))

    def isSequenceAny(self, value):
        return value == (-123456, None, (E_2, E3, E_10))

    def getSequenceSequenceString(self):
        return ((), ('foo', 'barr'), ('baz',))

    def isSequenceSequenceString(self, value):
        return value == ((), ('foo', 'barr'), ('baz',))

    def getSequenceEnum(self):
        return (E_2, E3, E_10)

    def isSequenceEnum(self, value):
        return value == (E_2, E3, E_10)

    def getSequenceStruct(self):
        return (
            Struct(
                True, -12, -1234, 1, -123456, 1, -123456789, 1, -10.25, -100.5, 'a', 'hä',
                uno.getTypeByName('long'), -123456, (), E_2, StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                self),
            Struct(
                True, 1, 1, 10, 1, 10, 1, 10, 1.5, 1.25, 'B', 'barr', uno.getTypeByName('void'),
                None, ('foo', 'barr'), E3, StructLong(1),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('baz'), 1, None, StructString('foo')),
                None),
            Struct(
                False, 12, 1234, 54321, 123456, 3456789012, 123456789, 9876543210, 10.75, 100.75,
                'Ö', 'bazzz', uno.getTypeByName('[]org.libreoffice.embindtest.Enum'),
                uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)), ('baz',), E_10,
                StructLong(123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('barr'), 123456,
                    uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)),
                    StructString('bazz')),
                self))

    def isSequenceStruct(self, value):
        return value == (
            Struct(
                True, -12, -1234, 1, -123456, 1, -123456789, 1, -10.25, -100.5, 'a', 'hä',
                uno.getTypeByName('long'), -123456, (), E_2, StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                self),
            Struct(
                True, 1, 1, 10, 1, 10, 1, 10, 1.5, 1.25, 'B', 'barr', uno.getTypeByName('void'),
                None, ('foo', 'barr'), E3, StructLong(1),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('baz'), 1, None, StructString('foo')),
                None),
            Struct(
                False, 12, 1234, 54321, 123456, 3456789012, 123456789, 9876543210, 10.75, 100.75,
                'Ö', 'bazzz', uno.getTypeByName('[]org.libreoffice.embindtest.Enum'),
                uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)), ('baz',), E_10,
                StructLong(123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('barr'), 123456,
                    uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)),
                    StructString('bazz')),
                self))

    def getNull(self):
        return None

    def isNull(self, value):
        return value == None

    def getOut(
            self, value1, value2, value3, value4, value5, value6, value7, value8, value9, value10,
            value11, value12, value13, value14, value15, value16, value17, value18):
        return (
            None, True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25,
            100.5, 'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
            Struct(
                True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
                'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
                StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                self),
            self)

    def throwRuntimeException(self):
        raise RuntimeException('test', None)

class EmbindTest(unittest.TestCase):
    def test(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()
        test = ctx.getServiceManager().createInstanceWithContext(
            'org.libreoffice.embindtest.Test', ctx)
        self.assertIsNotNone(test)

        v = test.getBoolean()
        self.assertEqual(v, True)
        self.assertTrue(test.isBoolean(v))
        self.assertTrue(test.isBoolean(True))

        v = test.getByte()
        self.assertEqual(v, -12)
        self.assertTrue(test.isByte(v))
        self.assertTrue(test.isByte(-12))

        v = test.getShort()
        self.assertEqual(v, -1234)
        self.assertTrue(test.isShort(v))
        self.assertTrue(test.isShort(-1234))

        v = test.getUnsignedShort()
        self.assertEqual(v, 54321)
        self.assertTrue(test.isUnsignedShort(v))
        self.assertTrue(test.isUnsignedShort(54321))

        v = test.getLong()
        self.assertEqual(v, -123456)
        self.assertTrue(test.isLong(v))
        self.assertTrue(test.isLong(-123456))

        v = test.getUnsignedLong()
        self.assertEqual(v, 3456789012)
        self.assertTrue(test.isUnsignedLong(v))
        self.assertTrue(test.isUnsignedLong(3456789012))

        v = test.getHyper()
        self.assertEqual(v, -123456789)
        self.assertTrue(test.isHyper(v))
        self.assertTrue(test.isHyper(-123456789))

        v = test.getUnsignedHyper()
        self.assertEqual(v, 9876543210)
        self.assertTrue(test.isUnsignedHyper(v))
        self.assertTrue(test.isUnsignedHyper(9876543210))

        v = test.getFloat()
        self.assertEqual(v, -10.25)
        self.assertTrue(test.isFloat(v))
        self.assertTrue(test.isFloat(-10.25))

        v = test.getDouble()
        self.assertEqual(v, 100.5)
        self.assertTrue(test.isDouble(v))
        self.assertTrue(test.isDouble(100.5))

        v = test.getChar()
        self.assertEqual(v, 'Ö')
        self.assertTrue(test.isChar(v))
        self.assertTrue(test.isChar('Ö'))

        v = test.getString()
        self.assertEqual(v, 'hä')
        self.assertTrue(test.isString(v))
        self.assertTrue(test.isString('hä'))

        v = test.getType()
        self.assertEqual(v, uno.getTypeByName('long'))
        self.assertTrue(test.isType(v))
        self.assertTrue(test.isType(uno.getTypeByName('long')))

        v = test.getEnum()
        self.assertEqual(v, E_2)
        self.assertTrue(test.isEnum(v))
        self.assertTrue(test.isEnum(E_2))

        v = test.getStruct()
        self.assertEqual(
            v,
            Struct(
                True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
                'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
                StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                test))
        self.assertTrue(test.isStruct(v))
        self.assertTrue(
            test.isStruct(
                Struct(
                    True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25,
                    100.5, 'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'),
                    E_2, StructLong(-123456),
                    uno.createUnoStruct(
                        'org.libreoffice.embindtest.Template<'
                            'any,org.libreoffice.embindtest.StructString>',
                        StructString('foo'), -123456, -123456, StructString('barr')),
                    test)))

        v = test.getStructLong()
        self.assertEqual(v, StructLong(-123456))
        self.assertTrue(test.isStructLong(v))
        self.assertTrue(test.isStructLong(StructLong(-123456)))

        v = test.getStructString()
        self.assertEqual(v, StructString('hä'))
        self.assertTrue(test.isStructString(v))
        self.assertTrue(test.isStructString(StructString('hä')))

        v = test.getTemplate()
        self.assertEqual(
            v,
            uno.createUnoStruct(
                'org.libreoffice.embindtest.Template<any,org.libreoffice.embindtest.StructString>',
                StructString('foo'), -123456, -123456, StructString('barr')))
        self.assertTrue(test.isTemplate(v))
        self.assertTrue(
            test.isTemplate(
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr'))))

        v = test.getAnyVoid()
        self.assertEqual(v, None)
        self.assertTrue(test.isAnyVoid(v))
        self.assertTrue(test.isAnyVoid(None))

        v = test.getAnyBoolean()
        self.assertEqual(v, True)
        self.assertTrue(test.isAnyBoolean(v))
        self.assertTrue(test.isAnyBoolean(True))

        v = test.getAnyByte()
        self.assertEqual(v, -12)
        self.assertTrue(test.isAnyByte(v))
        self.assertTrue(test.isAnyByte(-12))

        v = test.getAnyShort()
        self.assertEqual(v, -1234)
        self.assertTrue(test.isAnyShort(v))
        self.assertTrue(test.isAnyShort(-1234))

        v = test.getAnyUnsignedShort()
        self.assertEqual(v, 54321)
        self.assertFalse(test.isAnyUnsignedShort(v)) # long
        self.assertTrue(uno.invoke(test, 'isAnyUnsignedShort', (uno.Any('unsigned short', 54321),)))
            #TODO: direct call?

        v = test.getAnyLong()
        self.assertEqual(v, -123456)
        self.assertTrue(test.isAnyLong(v))
        self.assertTrue(test.isAnyLong(-123456))

        v = test.getAnyUnsignedLong()
        self.assertEqual(v, 3456789012)
        self.assertFalse(test.isAnyUnsignedLong(v)) # hyper
        self.assertTrue(
            uno.invoke(test, 'isAnyUnsignedLong', (uno.Any('unsigned long', 3456789012),)))
            #TODO: direct call?

        v = test.getAnyHyper()
        self.assertEqual(v, -123456789)
        self.assertFalse(test.isAnyHyper(v)) # long
        self.assertTrue(uno.invoke(test, 'isAnyHyper', (uno.Any('hyper', -123456789),)))
            #TODO: direct call?

        v = test.getAnyUnsignedHyper()
        self.assertEqual(v, 9876543210)
        self.assertFalse(test.isAnyUnsignedHyper(v)) # hyper
        self.assertTrue(
            uno.invoke(test, 'isAnyUnsignedHyper', (uno.Any('unsigned hyper', 9876543210),)))
            #TODO: direct call?

        v = test.getAnyFloat()
        self.assertEqual(v, -10.25)
        self.assertFalse(test.isAnyFloat(v)) # double
        self.assertTrue(uno.invoke(test, 'isAnyFloat', (uno.Any('float', -10.25),)))
            #TODO: direct call?

        v = test.getAnyDouble()
        self.assertEqual(v, 100.5)
        self.assertTrue(test.isAnyDouble(v))
        self.assertTrue(test.isAnyDouble(100.5))

        v = test.getAnyChar()
        self.assertEqual(v, 'Ö')
        self.assertTrue(test.isAnyChar(v))
        self.assertTrue(test.isAnyChar(uno.Char('Ö')))

        v = test.getAnyString()
        self.assertEqual(v, 'hä')
        self.assertTrue(test.isAnyString(v))
        self.assertTrue(test.isAnyString('hä'))

        v = test.getAnyType()
        self.assertEqual(v, uno.getTypeByName('long'))
        self.assertTrue(test.isAnyType(v))
        self.assertTrue(test.isAnyType(uno.getTypeByName('long')))

        v = test.getAnySequence()
        self.assertEqual(v, ('foo', 'barr', 'bazzz'))
        self.assertFalse(test.isAnySequence(v)) # []any
        self.assertTrue(
            uno.invoke(
                test, 'isAnySequence', (uno.Any('[]string', ('foo', 'barr', 'bazzz')),)))
            #TODO: direct call?

        v = test.getAnyEnum()
        self.assertEqual(v, E_2)
        self.assertTrue(test.isAnyEnum(v))
        self.assertTrue(test.isAnyEnum(E_2))

        v = test.getAnyStruct()
        self.assertEqual(
            v,
            Struct(
                True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
                'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
                StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                test))
        self.assertTrue(test.isAnyStruct(v))
        self.assertTrue(
            test.isAnyStruct(
                Struct(
                    True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25,
                    100.5, 'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'),
                    E_2, StructLong(-123456),
                    uno.createUnoStruct(
                        'org.libreoffice.embindtest.Template<'
                            'any,org.libreoffice.embindtest.StructString>',
                        StructString('foo'), -123456, -123456, StructString('barr')),
                    test)))

        v = test.getAnyException()
        self.assertTrue(v.Message.startswith('error')) #TODO: use Python 3.14 assertStartsWith
        self.assertEqual(v.Context, None)
        self.assertEqual(v.m1, -123456)
        self.assertEqual(v.m2, 100.5)
        self.assertEqual(v.m3, 'hä')
        self.assertTrue(test.isAnyException(v))
        self.assertTrue(test.isAnyException(Exception('error', None, -123456, 100.5, 'hä')))

        v = test.getAnyInterface()
        self.assertEqual(v, test)
        self.assertTrue(test.isAnyInterface(v))
        self.assertTrue(
            uno.invoke(
                test, 'isAnyInterface', (uno.Any('org.libreoffice.embindtest.XTest', test),)))
            #TODO: direct call?

        v = test.getSequenceBoolean()
        self.assertEqual(v, (True, True, False))
        self.assertTrue(test.isSequenceBoolean(v))
        self.assertTrue(test.isSequenceBoolean((True, True, False)))

        v = test.getSequenceByte()
        self.assertEqual(v, b'\xF4\x01\x0C')
        self.assertTrue(test.isSequenceByte(v))
        self.assertTrue(test.isSequenceByte((-12, 1, 12)))

        v = test.getSequenceShort()
        self.assertEqual(v, (-1234, 1, 1234))
        self.assertTrue(test.isSequenceShort(v))
        self.assertTrue(test.isSequenceShort((-1234, 1, 1234)))

        v = test.getSequenceUnsignedShort()
        self.assertEqual(v, (1, 10, 54321))
        self.assertTrue(test.isSequenceUnsignedShort(v))
        self.assertTrue(test.isSequenceUnsignedShort((1, 10, 54321)))

        v = test.getSequenceLong()
        self.assertEqual(v, (-123456, 1, 123456))
        self.assertTrue(test.isSequenceLong(v))
        self.assertTrue(test.isSequenceLong((-123456, 1, 123456)))

        v = test.getSequenceUnsignedLong()
        self.assertEqual(v, (1, 10, 3456789012))
        self.assertTrue(test.isSequenceUnsignedLong(v))
        self.assertTrue(test.isSequenceUnsignedLong((1, 10, 3456789012)))

        v = test.getSequenceHyper()
        self.assertEqual(v, (-123456789, 1, 123456789))
        self.assertTrue(test.isSequenceHyper(v))
        self.assertTrue(test.isSequenceHyper((-123456789, 1, 123456789)))

        v = test.getSequenceUnsignedHyper()
        self.assertEqual(v, (1, 10, 9876543210))
        self.assertTrue(test.isSequenceUnsignedHyper(v))
        self.assertTrue(test.isSequenceUnsignedHyper((1, 10, 9876543210)))

        v = test.getSequenceFloat()
        self.assertEqual(v, (-10.25, 1.5, 10.75))
        self.assertTrue(test.isSequenceFloat(v))
        self.assertTrue(test.isSequenceFloat((-10.25, 1.5, 10.75)))

        v = test.getSequenceDouble()
        self.assertEqual(v, (-100.5, 1.25, 100.75))
        self.assertTrue(test.isSequenceDouble(v))
        self.assertTrue(test.isSequenceDouble((-100.5, 1.25, 100.75)))

        v = test.getSequenceChar()
        self.assertEqual(v, ('a', 'B', 'Ö'))
        self.assertTrue(test.isSequenceChar(v))
        self.assertTrue(test.isSequenceChar(('a', 'B', 'Ö')))

        v = test.getSequenceString()
        self.assertEqual(v, ('foo', 'barr', 'bazzz'))
        self.assertTrue(test.isSequenceString(v))
        self.assertTrue(test.isSequenceString(('foo', 'barr', 'bazzz')))

        v = test.getSequenceType()
        self.assertEqual(
            v,
            (uno.getTypeByName('long'), uno.getTypeByName('void'),
             uno.getTypeByName('[]org.libreoffice.embindtest.Enum')))
        self.assertTrue(test.isSequenceType(v))
        self.assertTrue(
            test.isSequenceType(
                (uno.getTypeByName('long'), uno.getTypeByName('void'),
                 uno.getTypeByName('[]org.libreoffice.embindtest.Enum'))))

        v = test.getSequenceAny()
        self.assertEqual(v, (-123456, None, (E_2, E3, E_10)))
        self.assertFalse(test.isSequenceAny(v)) # (long, void, []any)
        self.assertTrue(
            uno.invoke(
                test, 'isSequenceAny',
                ((-123456, None, uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10))),)))
            #TODO: direct call?

        v = test.getSequenceSequenceString()
        self.assertEqual(v, ((), ('foo', 'barr'), ('baz',)))
        self.assertTrue(test.isSequenceSequenceString(v))
        self.assertTrue(test.isSequenceSequenceString(((), ('foo', 'barr'), ('baz',))))

        v = test.getSequenceEnum()
        self.assertEqual(v, (E_2, E3, E_10))
        self.assertTrue(test.isSequenceEnum(v))
        self.assertTrue(test.isSequenceEnum((E_2, E3, E_10)))

        v = test.getSequenceStruct()
        self.assertEqual(
            v,
            (Struct(
                True, -12, -1234, 1, -123456, 1, -123456789, 1, -10.25, -100.5, 'a', 'hä',
                uno.getTypeByName('long'), -123456, (), E_2, StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                test),
             Struct(
                 True, 1, 1, 10, 1, 10, 1, 10, 1.5, 1.25, 'B', 'barr', uno.getTypeByName('void'),
                 None, ('foo', 'barr'), E3, StructLong(1),
                 uno.createUnoStruct(
                     'org.libreoffice.embindtest.Template<'
                         'any,org.libreoffice.embindtest.StructString>',
                     StructString('baz'), 1, None, StructString('foo')),
                 None),
             Struct(
                 False, 12, 1234, 54321, 123456, 3456789012, 123456789, 9876543210, 10.75, 100.75,
                 'Ö', 'bazzz', uno.getTypeByName('[]org.libreoffice.embindtest.Enum'),
                 uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)), ('baz',), E_10,
                 StructLong(123456),
                 uno.createUnoStruct(
                     'org.libreoffice.embindtest.Template<'
                         'any,org.libreoffice.embindtest.StructString>',
                     StructString('barr'), 123456,
                     uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)),
                     StructString('bazz')),
                 test)))
        self.assertTrue(test.isSequenceStruct(v))
        self.assertTrue(
            test.isSequenceStruct(
                (Struct(
                    True, -12, -1234, 1, -123456, 1, -123456789, 1, -10.25, -100.5, 'a', 'hä',
                    uno.getTypeByName('long'), -123456, (), E_2, StructLong(-123456),
                    uno.createUnoStruct(
                        'org.libreoffice.embindtest.Template<'
                            'any,org.libreoffice.embindtest.StructString>',
                        StructString('foo'), -123456, -123456, StructString('barr')),
                    test),
                 Struct(
                     True, 1, 1, 10, 1, 10, 1, 10, 1.5, 1.25, 'B', 'barr',
                     uno.getTypeByName('void'), None, ('foo', 'barr'), E3, StructLong(1),
                     uno.createUnoStruct(
                         'org.libreoffice.embindtest.Template<'
                             'any,org.libreoffice.embindtest.StructString>',
                         StructString('baz'), 1, None, StructString('foo')),
                     None),
                 Struct(
                     False, 12, 1234, 54321, 123456, 3456789012, 123456789, 9876543210, 10.75,
                     100.75, 'Ö', 'bazzz', uno.getTypeByName('[]org.libreoffice.embindtest.Enum'),
                     uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)), ('baz',), E_10,
                     StructLong(123456),
                     uno.createUnoStruct(
                         'org.libreoffice.embindtest.Template<'
                             'any,org.libreoffice.embindtest.StructString>',
                         StructString('barr'), 123456,
                         uno.Any('[]org.libreoffice.embindtest.Enum', (E_2, E3, E_10)),
                         StructString('bazz')),
                     test))))

        v = test.getNull()
        self.assertEqual(v, None)
        self.assertTrue(test.isNull(v))
        self.assertTrue(test.isNull(None))

        (v, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17,
         v18) = test.getOut(
             None, None, None, None, None, None, None, None, None, None, None, None, None, None,
             None, None, None, None)
        self.assertIsNone(v)
        self.assertEqual(v1, True)
        self.assertEqual(v2, -12)
        self.assertEqual(v3, -1234)
        self.assertEqual(v4, 54321)
        self.assertEqual(v5, -123456)
        self.assertEqual(v6, 3456789012)
        self.assertEqual(v7, -123456789)
        self.assertEqual(v8, 9876543210)
        self.assertEqual(v9, -10.25)
        self.assertEqual(v10, 100.5)
        self.assertEqual(v11, 'Ö')
        self.assertEqual(v12, 'hä')
        self.assertEqual(v13, uno.getTypeByName('long'))
        self.assertEqual(v14, -123456)
        self.assertEqual(v15, ('foo', 'barr', 'bazzz'))
        self.assertEqual(v16, E_2)
        self.assertEqual(
            v17,
            Struct(
                True, -12, -1234, 54321, -123456, 3456789012, -123456789, 9876543210, -10.25, 100.5,
                'Ö', 'hä', uno.getTypeByName('long'), -123456, ('foo', 'barr', 'bazzz'), E_2,
                StructLong(-123456),
                uno.createUnoStruct(
                    'org.libreoffice.embindtest.Template<'
                        'any,org.libreoffice.embindtest.StructString>',
                    StructString('foo'), -123456, -123456, StructString('barr')),
                test))
        self.assertEqual(v18, test)

        with self.assertRaises(RuntimeException) as cm:
            test.throwRuntimeException()
        self.assertTrue(cm.exception.Message.startswith('test'))
            #TODO: use Python 3.14 assertStartsWith
        self.assertIsNone(cm.exception.Context)

        test.executeTest(Test())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
