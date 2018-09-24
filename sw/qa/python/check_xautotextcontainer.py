#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.container import NoSuchElementException
from com.sun.star.lang import IllegalArgumentException


class XAutoTextContainer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xDoc = cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls.xDoc.close(True)
        # HACK in case cls.xDoc holds a UNO proxy to an SwXTextDocument (whose dtor calls
        # Application::GetSolarMutex via sw::UnoImplPtrDeleter), which would potentially only be
        # garbage-collected after VCL has already been deinitialized:
        cls.xDoc = None
        cls._uno.tearDown()

    def test_insertNewByName(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['Name', 'TEST', 'Name2', '_With_underscore_', 'with space', '123456']
        for xName in xNames:
            xAutoTextContainer.insertNewByName(xName)
            xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Spaces(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # add
        xName = '  spaces  '
        xAutoTextContainer.insertNewByName(xName)

        # try to remove
        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName(xName)

        # remove trimmed
        xAutoTextContainer.removeByName('spaces')

    def test_insertNewByName_Several(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("atc_name1")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("atc_name2")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("atc_name3")

        self.assertEqual("atc_name1*0", xAutoTextGroup1.getName())
        self.assertEqual("atc_name2*0", xAutoTextGroup2.getName())
        self.assertEqual("atc_name3*0", xAutoTextGroup3.getName())

        xAutoTextContainer.removeByName("atc_name1")
        xAutoTextContainer.removeByName("atc_name2")
        xAutoTextContainer.removeByName("atc_name3")

    def test_insertNewByName_DifferentCase(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("myname")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("MYNAME")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("MyName")

        self.assertEqual("myname*0", xAutoTextGroup1.getName())
        self.assertNotEqual("MYNAME*0", xAutoTextGroup2.getName())
        self.assertNotEqual("MyName*0", xAutoTextGroup3.getName())
        self.assertEqual("group", xAutoTextGroup2.getName()[:5])
        self.assertEqual("group", xAutoTextGroup3.getName()[:5])

        xAutoTextContainer.removeByName("myname")

        xName = xAutoTextGroup2.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

        xName = xAutoTextGroup3.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Failed(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2', 'A1:B1']
        for xName in xNames:
            with self.assertRaises(IllegalArgumentException):
                xAutoTextContainer.insertNewByName(xName)

    def test_removeByName_Unknown(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName("Some Unknown Name")

    def test_removeByName_DifferentCases(self):
        xServiceManager = self.__class__._uno.xContext.ServiceManager
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        xAutoTextContainer.insertNewByName('GroupName')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('groupname')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('GROUPNAME')

        xAutoTextContainer.removeByName('GroupName')


if __name__ == '__main__':
    unittest.main()
