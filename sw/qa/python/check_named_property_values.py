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
from com.sun.star.container import XNameContainer


class CheckNamedPropertyValues(unittest.TestCase):
    _uno = None
    _xDoc = None

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()


    def checkNamedPropertyValues(self):
        xCont = UnoRuntime.queryInterface(type(XNameContainer).__name__, (connection.getComponentContext().getServiceManager().createInstanceWithContext("com.sun.star.document.NamedPropertyValues",connection.getComponentContext())))

        self.assertTrue("XNameContainer was queried but returned null.", xCont)

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

        t = xCont.getElementType() 
        self.assertFalse("Initial container is not empty.", xCont.hasElements())

        xCont.insertByName("prop1", prop1) 
        ret = xCont.getByName("prop1") 
        self.assertEqual(prop1[0].Name, ret[0].Name)
        self.assertEqual(prop1[0].Value, ret[0].Value)
        xCont.replaceByName("prop1", prop2)
        ret = xCont.getByName("prop1")
        self.assertEqual(prop2[0].Name, ret[0].Name)
        self.assertEqual(prop2[0].Value, ret[0].Value)
        xCont.removeByName("prop1")
        self.assertFalse("Could not remove PropertyValue.", xCont.hasElements())
        xCont.insertByName("prop1", prop1)
        xCont.insertByName("prop2", prop2)
        self.assertTrue("Did not insert PropertyValue.", xCont.hasElements())
        names = xCont.getElementNames()
        self.assertEqual("Not all element names were returned.", 2, len(names))
        for i in range(len(names)):
            self.assertTrue("Got a wrong element name",names[i]=="prop1" or names[i]=="prop2")
      
        try:
            xCont.insertByName("prop2", prop1)
        except:
            self.fail("ElementExistException was not thrown.")

        try:
            xCont.insertByName("prop3", "Example String")
        except:
            self.fail("IllegalArgumentException was not thrown.")
       
        try:
            xCont.removeByName("prop3")
        except:
            self.fail("NoSuchElementException was not thrown.")
    
