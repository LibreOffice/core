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

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_XAutoTextGroup(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'
        xTitle = 'Title'

        titlesBefore = xAutoTextGroup.getTitles()

        xAutoTextGroup.insertNewByName(xName, xTitle, xRange)
        self.assertNotEqual(titlesBefore, xAutoTextGroup.getTitles())

        xAutoTextGroup.removeByName(xName)
        self.assertEqual(titlesBefore, xAutoTextGroup.getTitles())

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_XAutoTextGroup_NoTitle(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'

        titlesBefore = xAutoTextGroup.getTitles()

        xAutoTextGroup.insertNewByName(xName, xName, xRange)
        self.assertNotEqual(titlesBefore, xAutoTextGroup.getTitles())

        xAutoTextGroup.removeByName(xName)
        self.assertEqual(titlesBefore, xAutoTextGroup.getTitles())

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_insertNewByName_Twice(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'
        xTitle = 'Title'

        xAutoTextGroup.insertNewByName(xName, xTitle, xRange)

        with self.assertRaises(ElementExistException):
            xAutoTextGroup.insertNewByName(xName, xTitle, xRange)

        xAutoTextGroup.removeByName(xName)

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_renameByName(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'
        xTitle = 'Title'

        xNewName = 'New Name'
        xNewTitle = 'New Title'

        xAutoTextGroup.insertNewByName(xName, xTitle, xRange)
        xAutoTextGroup.renameByName(xName, xNewName, xNewTitle)

        titlesBefore = xAutoTextGroup.getTitles()
        xAutoTextGroup.removeByName(xName)
        titlesAfter = xAutoTextGroup.getTitles()
        self.assertEqual(titlesBefore, titlesAfter)

        xAutoTextGroup.removeByName(xNewName)
        titlesAfter2 = xAutoTextGroup.getTitles()
        self.assertNotEqual(titlesBefore, titlesAfter2)

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_renameByName_Failed(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'
        xTitle = 'Title'

        xNewName = 'New Name'
        xNewTitle = 'New Title'

        xAutoTextGroup.insertNewByName(xName, xTitle, xRange)
        xAutoTextGroup.insertNewByName(xNewName, xNewTitle, xRange)

        with self.assertRaises(ElementExistException):
            xAutoTextGroup.renameByName(xName, xNewName, xNewTitle)

        xAutoTextGroup.removeByName(xName)
        xAutoTextGroup.removeByName(xNewName)

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_removeByName_Twice(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xRange = xCursor.getStart()

        # perform unit test
        xName = 'Name'
        xTitle = 'Title'

        xAutoTextGroup.insertNewByName(xName, xTitle, xRange)
        xAutoTextGroup.removeByName(xName)

        # no exception
        xAutoTextGroup.removeByName(xName)

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def test_removeByName_Empty(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()
        xAutoTextContainer = self.createAutoTextContainer()
        xAutoTextGroup = xAutoTextContainer.insertNewByName("atg_name1")

        # perform unit test
        xAutoTextGroup.removeByName('')

        # clean up
        xAutoTextContainer.removeByName("atg_name1")
        xDoc.close(True)

    def createAutoTextContainer(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.assertIsNotNone(xServiceManager)
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")
        self.assertIsNotNone(xAutoTextContainer)
        return xAutoTextContainer


if __name__ == '__main__':
    unittest.main()
