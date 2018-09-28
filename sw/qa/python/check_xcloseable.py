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
from com.sun.star.util import XCloseListener


listenerCallsOwner = 0
listenerCallsNoOwner = 0


class XCloseListenerExtended(unohelper.Base, XCloseListener):
    @classmethod
    def queryClosing(self, Source, GetsOwnership):
        if GetsOwnership is True:
            global listenerCallsOwner
            listenerCallsOwner += 1
        else:
            global listenerCallsNoOwner
            listenerCallsNoOwner += 1

    @classmethod
    def notifyClosing(self, Source):
        pass

    @classmethod
    def disposing(self, Event):
        pass


class XCloseable(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_closeTrue(self):
        # prepare
        global listenerCallsOwner
        global listenerCallsNoOwner
        listenerCallsOwner = 0
        listenerCallsNoOwner = 0

        # run
        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XCloseListenerExtended()
        xDoc.addCloseListener(xListener)
        xDoc.close(True)

        # verify results
        self.assertEqual(1, listenerCallsOwner)
        self.assertEqual(0, listenerCallsNoOwner)

    def test_closeFalse(self):
        # prepare
        global listenerCallsOwner
        global listenerCallsNoOwner
        listenerCallsOwner = 0
        listenerCallsNoOwner = 0

        # run
        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XCloseListenerExtended()
        xDoc.addCloseListener(xListener)
        xDoc.close(False)

        # verify results
        self.assertEqual(0, listenerCallsOwner)
        self.assertEqual(1, listenerCallsNoOwner)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
