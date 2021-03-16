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
from com.sun.star.container import ElementExistException
from com.sun.star.container import NoSuchElementException


class XAutoTextGroup(unittest.TestCase):
    # 0 indicates the path of the Office Basis layer
    # 1 indicates the path of the user directory
    GROUP_NAME = 'atg_name1*1'

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openEmptyWriterDoc()

    def setUp(self):
        self.xAutoTextContainer = self.createAutoTextContainer()
        self.xAutoTextGroup = self.insertNewGroup(self.xAutoTextContainer)

        self.xText = self._uno.getDoc().getText()
        self.xCursor = self.xText.createTextCursor()
        self.xRange = self.xCursor.getStart()

    def tearDown(self):
        self.xAutoTextContainer.removeByName(self.GROUP_NAME)

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_XAutoTextGroup(self):
        xName = 'Name'
        xTitle = 'Title'

        titlesBefore = self.xAutoTextGroup.getTitles()

        self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)
        self.assertNotEqual(titlesBefore, self.xAutoTextGroup.getTitles())

        self.xAutoTextGroup.removeByName(xName)
        self.assertEqual(titlesBefore, self.xAutoTextGroup.getTitles())

    def test_XAutoTextGroup_NoTitle(self):
        xName = 'Name'

        titlesBefore = self.xAutoTextGroup.getTitles()

        self.xAutoTextGroup.insertNewByName(xName, xName, self.xRange)
        self.assertNotEqual(titlesBefore, self.xAutoTextGroup.getTitles())

        self.xAutoTextGroup.removeByName(xName)
        self.assertEqual(titlesBefore, self.xAutoTextGroup.getTitles())

    def test_insertNewByName_Twice(self):
        xName = 'Name'
        xTitle = 'Title'

        self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)

        with self.assertRaises(ElementExistException):
            self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)

        self.xAutoTextGroup.removeByName(xName)

    def test_renameByName(self):
        xName = 'Name'
        xTitle = 'Title'

        xNewName = 'New Name'
        xNewTitle = 'New Title'

        self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)
        self.xAutoTextGroup.renameByName(xName, xNewName, xNewTitle)

        titlesBefore = self.xAutoTextGroup.getTitles()
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextGroup.removeByName(xName)
        titlesAfter = self.xAutoTextGroup.getTitles()
        self.assertEqual(titlesBefore, titlesAfter)

        self.xAutoTextGroup.removeByName(xNewName)
        titlesAfter2 = self.xAutoTextGroup.getTitles()
        self.assertNotEqual(titlesBefore, titlesAfter2)

    def test_renameByName_Failed(self):
        xName = 'Name'
        xTitle = 'Title'

        xNewName = 'New Name'
        xNewTitle = 'New Title'

        self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)
        self.xAutoTextGroup.insertNewByName(xNewName, xNewTitle, self.xRange)

        with self.assertRaises(ElementExistException):
            self.xAutoTextGroup.renameByName(xName, xNewName, xNewTitle)

        self.xAutoTextGroup.removeByName(xName)
        self.xAutoTextGroup.removeByName(xNewName)

    def test_removeByName_Twice(self):
        xName = 'Name'
        xTitle = 'Title'

        self.xAutoTextGroup.insertNewByName(xName, xTitle, self.xRange)
        self.xAutoTextGroup.removeByName(xName)

        # 2nd attempt should fail
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextGroup.removeByName(xName)

    def test_removeByName_Empty(self):
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextGroup.removeByName('')

    def test_removeByName_NoSuchElement(self):
        with self.assertRaises(NoSuchElementException):
            self.xAutoTextGroup.removeByName('ForSureNotExistName1')

    def createAutoTextContainer(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.assertIsNotNone(xServiceManager)
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")
        self.assertIsNotNone(xAutoTextContainer)

        return xAutoTextContainer

    def insertNewGroup(self, xAutoTextContainer):
        self.assertIsNotNone(xAutoTextContainer)
        xAutoTextGroup = xAutoTextContainer.insertNewByName(self.GROUP_NAME)
        self.assertIsNotNone(xAutoTextGroup)
        return xAutoTextGroup


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
