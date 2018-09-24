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

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_insertNewByName(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['Name', 'TEST', 'Name2', '_With_underscore_', 'with space', '123456']
        for xName in xNames:
            xAutoTextContainer.insertNewByName(xName)
            xAutoTextContainer.removeByName(xName)

        # clean up
        xDoc.close(True)

    def test_insertNewByName_Spaces(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        # add
        xName = '  spaces  '
        xAutoTextContainer.insertNewByName(xName)

        # try to remove
        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName(xName)

        # remove trimmed
        xAutoTextContainer.removeByName('spaces')

        # clean up
        xDoc.close(True)

    def test_insertNewByName_Several(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("atc_name1")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("atc_name2")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("atc_name3")

        self.assertEqual("atc_name1*0", xAutoTextGroup1.getName())
        self.assertEqual("atc_name2*0", xAutoTextGroup2.getName())
        self.assertEqual("atc_name3*0", xAutoTextGroup3.getName())

        xAutoTextContainer.removeByName("atc_name1")
        xAutoTextContainer.removeByName("atc_name2")
        xAutoTextContainer.removeByName("atc_name3")

        # clean up
        xDoc.close(True)

    def test_insertNewByName_DifferentCase(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
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

        # clean up
        xDoc.close(True)

    def test_insertNewByName_Failed(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        # group name must contain a-z, A-z, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2', 'A1:B1']
        for xName in xNames:
            with self.assertRaises(IllegalArgumentException):
                xAutoTextContainer.insertNewByName(xName)

        # clean up
        xDoc.close(True)

    def test_removeByName_Unknown(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName("Some Unknown Name")

        # clean up
        xDoc.close(True)

    def test_removeByName_DifferentCases(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()

        # perform unit test
        xAutoTextContainer.insertNewByName('GroupName')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('groupname')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('GROUPNAME')

        xAutoTextContainer.removeByName('GroupName')

        # clean up
        xDoc.close(True)

    def createAutoTextContainer(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.assertIsNotNone(xServiceManager)
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")
        self.assertIsNotNone(xAutoTextContainer)
        return xAutoTextContainer


if __name__ == '__main__':
    unittest.main()
