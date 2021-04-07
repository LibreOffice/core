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
from com.sun.star.container import ElementExistException
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.container import NoSuchElementException


class CheckNamedPropertyValues(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xContext = cls._uno.getContext()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_checkNamedPropertyValues(self):

        xServiceManager = self.xContext.ServiceManager
        xCont = xServiceManager.createInstanceWithContext('com.sun.star.document.NamedPropertyValues',
                                                          self.xContext)

        p1 = PropertyValue(Name="Jupp", Value="GoodGuy")
        prop1 = uno.Any("[]com.sun.star.beans.PropertyValue", (p1,))

        p2 = PropertyValue(Name="Horst", Value="BadGuy")
        prop2 = uno.Any("[]com.sun.star.beans.PropertyValue", (p2,))

        t = xCont.getElementType()
        self.assertFalse(xCont.hasElements(),  "Initial container is not empty")

        uno.invoke(xCont, "insertByName", ("prop1", prop1))
        ret = xCont["prop1"]
        self.assertEqual(p1.Name, ret[0].Name)
        self.assertEqual(p1.Value, ret[0].Value)

        uno.invoke(xCont, "replaceByName", ("prop1", prop2))
        ret = xCont["prop1"]
        self.assertEqual(p2.Name, ret[0].Name)
        self.assertEqual(p2.Value, ret[0].Value)

        xCont.removeByName("prop1")
        self.assertFalse(xCont.hasElements(), "Could not remove PropertyValue.")
        uno.invoke(xCont, "insertByName", ("prop1", prop1))
        uno.invoke(xCont, "insertByName", ("prop2", prop2))
        self.assertTrue(xCont.hasElements(), "Did not insert PropertyValue")
        names = xCont.getElementNames()
        self.assertEqual(2, len(names), "Not all element names were returned")

        for i in range(len(names)):
            self.assertIn(names[i], ["prop1", "prop2"], "Got a wrong element name")

        with self.assertRaises(ElementExistException):
            uno.invoke(xCont, "insertByName", ("prop2", prop1))

        with self.assertRaises(IllegalArgumentException):
            uno.invoke(xCont, "insertByName", ("prop3", "Example String"))

        with self.assertRaises(NoSuchElementException):
            xCont.removeByName("prop3")
