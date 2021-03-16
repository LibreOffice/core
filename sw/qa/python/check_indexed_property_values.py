'''
  This file is part of the LibreOffice project.

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  This file incorporates work covered by the following license notice:

    Licensed to the Apache Software Foundation (ASF) under one or more
    contributor license agreements. See the NOTICE file distributed
    with this work for additional information regarding copyright
    ownership. The ASF licenses this file to you under the Apache
    License, Version 2.0 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.apache.org/licenses/LICENSE-2.0 .
'''

import unittest
import uno
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.beans import PropertyValue
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.lang import IndexOutOfBoundsException


class CheckIndexedPropertyValues(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xContext = cls._uno.getContext()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_checkIndexedPropertyValues(self):
        xServiceManager = self.xContext.ServiceManager
        xCont = xServiceManager.createInstanceWithContext('com.sun.star.document.IndexedPropertyValues',
                                                          self.xContext)

        p1 = PropertyValue(Name="Jupp", Value="GoodGuy")
        prop1 = uno.Any("[]com.sun.star.beans.PropertyValue", (p1,))

        p2 = PropertyValue(Name="Horst", Value="BadGuy")
        prop2 = uno.Any("[]com.sun.star.beans.PropertyValue", (p2,))

        p3 = PropertyValue(Name="Peter", Value="FamilyGuy")
        prop3 = uno.Any("[]com.sun.star.beans.PropertyValue", (p3,))

        t = xCont.getElementType()
        self.assertEqual(0, len(xCont), "Initial container is not empty")
        uno.invoke(xCont, "insertByIndex", (0, prop1))

        ret = xCont[0]
        self.assertEqual(p1.Name, ret[0].Name)
        self.assertEqual(p1.Value, ret[0].Value)

        uno.invoke(xCont, "replaceByIndex", (0, prop2))
        ret = xCont[0]
        self.assertEqual(p2.Name, ret[0].Name)
        self.assertEqual(p2.Value, ret[0].Value)

        xCont.removeByIndex(0)
        self.assertTrue(not(xCont.hasElements()) and len(xCont) == 0, "Could not remove PropertyValue")
        uno.invoke(xCont, "insertByIndex", (0, prop1))
        uno.invoke(xCont, "insertByIndex", (1, prop2))
        self.assertTrue(xCont.hasElements() and len(xCont) == 2, "Did not insert PropertyValue")

        uno.invoke(xCont, "insertByIndex", (1, prop2))
        uno.invoke(xCont, "insertByIndex", (1, prop3))
        ret = xCont[1]
        self.assertEqual(p3.Name, ret[0].Name)
        self.assertEqual(p3.Value, ret[0].Value)

        with self.assertRaises(IndexOutOfBoundsException):
            uno.invoke(xCont, "insertByIndex", (25, prop2))

        with self.assertRaises(IndexOutOfBoundsException):
            xCont.removeByIndex(25)

        with self.assertRaises(IllegalArgumentException):
            uno.invoke(xCont, "insertByIndex", (3, "Example String"))
