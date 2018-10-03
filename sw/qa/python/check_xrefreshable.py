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
import unohelper
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.util import XRefreshListener


refreshed = 0


class XRefreshListenerExtended(unohelper.Base, XRefreshListener):
    @classmethod
    def refreshed(self, Event):
        global refreshed
        refreshed += 1

    @classmethod
    def disposing(self, event):
        pass


class XRefreshable(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_None(self):
        global refreshed
        refreshed = 0
        xDoc = self._uno.openEmptyWriterDoc()

        # attempt to add/remove none
        xDoc.addRefreshListener(None)
        xDoc.removeRefreshListener(None)

        # refresh without listeners
        xDoc.refresh()
        xDoc.close(True)

    def test_refresh(self):
        global refreshed
        refreshed = 0

        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XRefreshListenerExtended()
        xDoc.addRefreshListener(xListener)
        xDoc.refresh()
        xDoc.removeRefreshListener(xListener)
        self.assertEqual(1, refreshed)
        xDoc.refresh()
        self.assertEqual(1, refreshed)
        xDoc.close(True)

    def test_refreshTwice(self):
        global refreshed
        refreshed = 0

        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XRefreshListenerExtended()
        xDoc.addRefreshListener(xListener)
        xDoc.refresh()
        xDoc.refresh()
        xDoc.removeRefreshListener(xListener)
        self.assertEqual(2, refreshed)
        xDoc.refresh()
        self.assertEqual(2, refreshed)
        xDoc.close(True)

    def test_DoubleInstances(self):
        global refreshed
        refreshed = 0

        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XRefreshListenerExtended()

        xDoc.addRefreshListener(xListener)
        xDoc.addRefreshListener(xListener)
        xDoc.addRefreshListener(xListener)

        xDoc.refresh()
        xDoc.refresh()
        self.assertEqual(3*2, refreshed)

        refreshed = 0
        xDoc.removeRefreshListener(xListener)
        xDoc.refresh()
        # two instances should remain in the list
        self.assertEqual(2, refreshed)

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
