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


class XModifiable2(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_XModifiable2(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # clean up
        xDoc.close(True)

    def test_XModifiable2_Double(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to disable
        # Expected return value:
        #   `TRUE` the changing of the modified state was disabled
        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to disable twice
        # Expected return value:
        #   `FALSE` the changing of the modified state was already disabled
        self.assertFalse(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to disable third time
        # Expected return value:
        #   `FALSE` the changing of the modified state was already disabled
        #   i.e. the same as in previous call
        self.assertFalse(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        # try to enable
        # Expected return value:
        #   `FALSE` the changing of the modified state was enabled
        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to enable twice
        # Expected return value:
        #   `TRUE` the changing of the modified state was already enabled
        self.assertTrue(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # try to enable third time
        # Expected return value:
        #   `TRUE` the changing of the modified state was already enabled
        #   i.e. the same as in previous call
        self.assertTrue(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # clean up
        xDoc.close(True)

    def test_XModifiable2_setModified(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test
        # try to set modified flag when modification enabled
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertTrue(xDoc.isModified())

        xDoc.setModified(False)
        self.assertFalse(xDoc.isModified())

        # try to set modified flag when modification disabled
        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertFalse(xDoc.isModified())

        self.assertFalse(xDoc.enableSetModified())
        self.assertTrue(xDoc.isSetModifiedEnabled())

        # document is still not modified
        self.assertFalse(xDoc.isModified())

        # try to set modified flag when modification enabled
        # and when we have changed the modification possibility
        self.assertTrue(xDoc.isSetModifiedEnabled())

        self.assertFalse(xDoc.isModified())
        xDoc.setModified(True)
        self.assertTrue(xDoc.isModified())

        xDoc.setModified(False)
        self.assertFalse(xDoc.isModified())

        # clean up
        xDoc.close(True)

    def test_setModified_ByContent(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test:
        #        set modified flag using text editing
        #        when modification of the flag is enabled
        self.assertTrue(xDoc.isSetModifiedEnabled())
        self.assertFalse(xDoc.isModified())

        cursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertString(cursor, "The first paragraph", 0)

        self.assertTrue(xDoc.isSetModifiedEnabled())
        self.assertTrue(xDoc.isModified())

        # clean up
        xDoc.close(True)

    def test_setModified_ByContent_Blocked(self):
        # initialization
        xDoc = self._uno.openEmptyWriterDoc()

        # perform unit test:
        #       it is unable to set modified flag using text editing
        #       when modification of the flag was disabled
        self.assertTrue(xDoc.disableSetModified())
        self.assertFalse(xDoc.isSetModifiedEnabled())
        self.assertFalse(xDoc.isModified())

        cursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertString(cursor, "The first paragraph", 0)

        self.assertFalse(xDoc.isSetModifiedEnabled())
        self.assertFalse(xDoc.isModified())

        # clean up
        xDoc.close(True)

    def test_WriteProtectedDocument(self):
        # initialization
        xDoc = self._uno.openTemplateFromTDOC('WriteProtected.odt')

        # perform unit test:
        #       it is unable to set modified flag using text editing
        #       when modification of the flag was disabled as
        #       ODT file was marked to be opened as read-only
        self.assertFalse(xDoc.isSetModifiedEnabled())
        self.assertFalse(xDoc.isModified())

        cursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertString(cursor, "The first paragraph", 0)

        self.assertFalse(xDoc.isSetModifiedEnabled())
        self.assertFalse(xDoc.isModified())

        # clean up
        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
