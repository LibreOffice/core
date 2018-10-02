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
from com.sun.star.uno import RuntimeException


class XAutoTextGroup(unittest.TestCase):
    # 0 indicates the path of the Office Basis layer
    # 1 indicates the path of the user directory
    GROUP_NAME = 'atg_name1*1'

    @classmethod
    def setUpClass(self):
        self._uno = UnoInProcess()
        self._uno.setUp()
        self._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(self):
        self._uno.tearDown()

    def test_XAutoTextGroup(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_XAutoTextGroup_NoTitle(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_insertNewByName_Twice(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_renameByName(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_renameByName_Failed(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_removeByName_Twice(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        xText = self._uno.getDoc().getText()
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
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_removeByName_Empty(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        # perform unit test
        xAutoTextGroup.removeByName('')

        # clean up
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def test_removeByName_NoSuchElement(self):
        # initialization
        xAutoTextContainer = self.createAutoTextContainer()
        if xAutoTextContainer is None:
            return
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)

        # perform unit test
        xAutoTextGroup.removeByName('ForSureNotExistName1')

        # clean up
        xAutoTextContainer.removeByName(self.GROUP_NAME)

    def createAutoTextContainer(self):
        xServiceManager = self._uno.xContext.ServiceManager
        self.assertIsNotNone(xServiceManager)
        xAutoTextContainer = xServiceManager.createInstance("com.sun.star.text.AutoTextContainer")
        self.assertIsNotNone(xAutoTextContainer)

        # Note that in some systems the user may lack of
        # write access to the Office Basis directory
        xAutoTextGroup = self.insertNewGroup(xAutoTextContainer)
        try:
            titles = xAutoTextGroup.getTitles()
        except RuntimeException:
            return None
        xAutoTextContainer.removeByName(self.GROUP_NAME)

        # ok, we have permissions
        return xAutoTextContainer

    def insertNewGroup(self, xAutoTextContainer):
        self.assertIsNotNone(xAutoTextContainer)
        xAutoTextGroup = xAutoTextContainer.insertNewByName(self.GROUP_NAME)
        self.assertIsNotNone(xAutoTextGroup)
        return xAutoTextGroup


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
