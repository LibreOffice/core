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
    def setUpClass(self):
        self._uno = UnoInProcess()
        self._uno.setUp()
        self._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(self):
        self._uno.tearDown()

    def test_insertNewByName(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        # group name must contain a-z, A-z, 0-9, '_', ' ' only
        xNames = ['Name', 'TEST', 'Name2', '_With_underscore_', 'with space', '123456']
        for xName in xNames:
            xAutoTextContainer.insertNewByName(xName)
            xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Spaces(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        # add
        xName = '  spaces  '
        xAutoTextContainer.insertNewByName(xName)

        # try to remove
        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName(xName)

        # remove trimmed
        xAutoTextContainer.removeByName('spaces')

    def test_insertNewByName_Several(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

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

    def test_insertNewByName_DifferentCase(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        xAutoTextGroup1 = xAutoTextContainer.insertNewByName("myname")
        xAutoTextGroup2 = xAutoTextContainer.insertNewByName("MYNAME")
        xAutoTextGroup3 = xAutoTextContainer.insertNewByName("MyName")

        self.assertEqual("myname*0", xAutoTextGroup1.getName())

        # Note: different platforms could support different cases
        #       in container names
        validName2 = False
        validName2 |= (xAutoTextGroup2.getName() == "MYNAME*0")
        validName2 |= (xAutoTextGroup2.getName()[:5] == "group")

        validName3 = False
        validName3 |= (xAutoTextGroup3.getName() == "MyName*0")
        validName3 |= (xAutoTextGroup3.getName()[:5] == "group")

        self.assertTrue(validName2)
        self.assertTrue(validName3)

        xAutoTextContainer.removeByName("myname")

        xName = xAutoTextGroup2.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

        xName = xAutoTextGroup3.getName()
        xName = xName[:xName.find('*')]
        xAutoTextContainer.removeByName(xName)

    def test_insertNewByName_Failed(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        # group name must contain a-z, A-z, 0-9, '_', ' ' only
        xNames = ['', 'Name!!!', 'Red & White', 'Name.With.Dot', 'Name-2', 'A1:B1']
        for xName in xNames:
            with self.assertRaises(IllegalArgumentException):
                xAutoTextContainer.insertNewByName(xName)

    def test_removeByName_Unknown(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName("Some Unknown Name")

    def test_removeByName_DifferentCases(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return

        # perform unit test
        xAutoTextContainer.insertNewByName('GroupName')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('groupname')

        with self.assertRaises(NoSuchElementException):
            xAutoTextContainer.removeByName('GROUPNAME')

        xAutoTextContainer.removeByName('GroupName')

    def createAutoTextContainer(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.assertIsNotNone(xServiceManager)
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")
        self.assertIsNotNone(xAutoTextContainer)

        # Note that in some systems the user may lack of
        # write access to the Office Basis directory
        xAutoTextGroup = xAutoTextContainer.insertNewByName("_PermCheck1")
        try:
            titles = xAutoTextGroup.getTitles()
        except RuntimeException:
            return None
        xAutoTextContainer.removeByName("_PermCheck1")

        # ok, we have permissions
        return xAutoTextContainer


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
