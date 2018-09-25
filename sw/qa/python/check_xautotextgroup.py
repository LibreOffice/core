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


class XAutoTextGroup(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls.xDoc = cls._uno.openEmptyWriterDoc()

        xServiceManager = cls._uno.xContext.ServiceManager
        cls.xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")

        cls.xAutoTextGroup = cls.xAutoTextContainer.insertNewByName("atg_name1")

        xText = cls.xDoc.getText()
        xCursor = xText.createTextCursor()
        cls.xRange = xCursor.getStart()

    @classmethod
    def tearDownClass(cls):
        cls.xAutoTextContainer.removeByName("atg_name1")
        cls.xDoc.close(True)
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

        # no exception
        self.xAutoTextGroup.removeByName(xName)

    def test_removeByName_Empty(self):
        self.xAutoTextGroup.removeByName('')


if __name__ == '__main__':
    unittest.main()
