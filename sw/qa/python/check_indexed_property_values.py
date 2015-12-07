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
import unohelper
import os
from com.sun.star.beans import PropertyValue
from com.sun.star.container import XIndexContainer

class CheckIndexedPropertyValues(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()


    def checkIndexedPropertyValues():
        xCont = UnoRuntime.queryInterface(type(XIndexContainer).__name__,(connection.getComponentContext().getServiceManager().createInstanceWithContext("com.sun.star.document.IndexedPropertyValues",connection.getComponentContext())))
        self.assertTrue("XIndexContainer was queried but returned null.", xCont)
        p1 = PropertyValue()
        prop1 = []
        prop1.append(p1)
        prop1[0].Name  = "Jupp"
        prop1[0].Value = "GoodGuy"

        p2 = PropertyValue()
        prop2 = []
        prop2.append(p2)
        prop2[0].Name  = "Horst"
        prop2[0].Value = "BadGuy"

        p3 = PropertyValue
        prop3 = []
        prop3.append(p3)
        prop3[0].Name  = "Peter"
        prop3[0].Value = "FamilyGuy"

        t = xCont.getElementType()
        self.assertEqual("Initial container is not empty", 0, xCont.getCount())
        xCont.insertByIndex(0, prop1)
        
        ret = xCont.getByIndex(0)
        self.assertEqual(prop1[0].Name, ret[0].Name)
        self.assertEqual(prop1[0].Value, ret[0].Value)
        xCont.replaceByIndex(0, prop2)
        ret = xCont.getByIndex(0)
        self.assertEqual(prop2[0].Name, ret[0].Name)
        self.assertEqual(prop2[0].Value, ret[0].Value)
        xCont.removeByIndex(0)
        self.assertTrue("Could not remove PropertyValue.", not(xCont.hasElements()) and xCont.getCount()==0)
        xCont.insertByIndex(0, prop1)
        xCont.insertByIndex(1, prop2)
        self.assertTrue("Did not insert PropertyValue.", xCont.hasElements() and xCont.getCount()==2)
        try:
            xCont.removeByIndex(1)
        except:
            self.fail("Could not remove last PropertyValue")
        
        xCont.insertByIndex(1, prop2)
        xCont.insertByIndex(1, prop3)
        ret = xCont.getByIndex(1)
        self.assertEqual(prop3[0].Name, ret[0].Name)
        self.assertEqual(prop3[0].Value, ret[0].Value)

        try:
            xCont.insertByIndex(25, prop2)
        except:
            self.fail("IllegalArgumentException was not thrown.")
     
        try:
            xCont.removeByIndex(25)
        except:
            self.fail("IndexOutOfBoundsException was not thrown.")
   
        try:
            xCont.insertByIndex(3, "Example String")
        except:
            self.fail("IllegalArgumentException was not thrown.")
   
