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
    # 0 indicates the path of the Office Basis layer
    # 1 indicates the path of the user directory
    GROUP_POSTFIX = '*1'

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openEmptyWriterDoc()

    def setUp(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.xAutoTextContainer = xServiceManager.createInstance(
            "com.sun.star.text.AutoTextContainer")

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_insertNewByName(self):
        # group name must contain a-z, A-z, 0-9, '_', ' ' only
        xNames = ['Name', 'TEST', 'Name2', '_With_underscore_', 'with space', '123456']
        for xName in xNames:
            self.xAutoTextContainer.insertNewByName(xName+self.GROUP_POSTFIX)
            self.xAutoTextContainer.removeByName(xName+self.GROUP_POSTFIX)

    def test_insertNewByName_Spaces(self):
        # add
        xName = '  spaces  '
        self.xAutoTextContainer.insertNewByName(xName+self.GROUP_POSTFIX)

        # try to remove
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextContainer.removeByName(xName+self.GROUP_POSTFIX)

        # remove trimmed
        self.xAutoTextContainer.removeByName('spaces'+self.GROUP_POSTFIX)

    def test_insertNewByName_Several(self):
        xAutoTextGroup1 = self.xAutoTextContainer.insertNewByName(
            "atc_name1"+self.GROUP_POSTFIX)
        xAutoTextGroup2 = self.xAutoTextContainer.insertNewByName(
            "atc_name2"+self.GROUP_POSTFIX)
        xAutoTextGroup3 = self.xAutoTextContainer.insertNewByName(
            "atc_name3"+self.GROUP_POSTFIX)

        self.assertEqual("atc_name1"+self.GROUP_POSTFIX, xAutoTextGroup1.getName())
        self.assertEqual("atc_name2"+self.GROUP_POSTFIX, xAutoTextGroup2.getName())
        self.assertEqual("atc_name3"+self.GROUP_POSTFIX, xAutoTextGroup3.getName())

        self.xAutoTextContainer.removeByName("atc_name1"+self.GROUP_POSTFIX)
        self.xAutoTextContainer.removeByName("atc_name2"+self.GROUP_POSTFIX)
        self.xAutoTextContainer.removeByName("atc_name3"+self.GROUP_POSTFIX)

    def test_insertNewByName_DifferentCase(self):
        xAutoTextGroup1 = self.xAutoTextContainer.insertNewByName("myname"+self.GROUP_POSTFIX)
        xAutoTextGroup2 = self.xAutoTextContainer.insertNewByName("MYNAME"+self.GROUP_POSTFIX)
        xAutoTextGroup3 = self.xAutoTextContainer.insertNewByName("MyName"+self.GROUP_POSTFIX)

        self.assertEqual("myname"+self.GROUP_POSTFIX, xAutoTextGroup1.getName())

        # Note: different platforms could support different cases
        #       in container names
        validName2 = False
        validName2 |= (xAutoTextGroup2.getName() == "MYNAME"+self.GROUP_POSTFIX)
        validName2 |= (xAutoTextGroup2.getName()[:5] == "group")

        validName3 = False
        validName3 |= (xAutoTextGroup3.getName() == "MyName"+self.GROUP_POSTFIX)
        validName3 |= (xAutoTextGroup3.getName()[:5] == "group")

        self.assertTrue(validName2)
        self.assertTrue(validName3)

        self.xAutoTextContainer.removeByName("myname"+self.GROUP_POSTFIX)

        xName = xAutoTextGroup2.getName()
        xName = xName[:xName.find('*')]
        self.xAutoTextContainer.removeByName(xName)

        xName = xAutoTextGroup3.getName()
        xName = xName[:xName.find('*')]
        self.xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Failed(self):
        # group name must contain a-z, A-z, 0-9, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2', 'A1:B1']
        for xName in xNames:
            with self.assertRaises(IllegalArgumentException):
                self.xAutoTextContainer.insertNewByName(xName)

    def test_removeByName_Unknown(self):
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextContainer.removeByName("Some Unknown Name")

    def test_removeByName_DifferentCases(self):
        self.xAutoTextContainer.insertNewByName('GroupName'+self.GROUP_POSTFIX)

        with self.assertRaises(NoSuchElementException):
            self.xAutoTextContainer.removeByName('groupname'+self.GROUP_POSTFIX)

        with self.assertRaises(NoSuchElementException):
            self.xAutoTextContainer.removeByName('GROUPNAME'+self.GROUP_POSTFIX)

        self.xAutoTextContainer.removeByName('GroupName'+self.GROUP_POSTFIX)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
