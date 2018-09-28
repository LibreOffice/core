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
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.util import XCloseListener
from com.sun.star.document import XEventListener


listenerCalls = 0
print("hello")

class XCloseListenerExtended(unohelper.Base, XCloseListener, XEventListener):
    @classmethod
    def queryClosing(self, Source, GetsOwnership):
        print("queryClosing was called")
        global listenerCalls
        listenerCalls += 1
        super(XCloseListenerExtended, self).queryClosing(Source, GetsOwnership)


class XCloseable(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_closeTrue(self):
        global listenerCalls
        listenerCalls = 0
        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XCloseListenerExtended()
        xDoc.addCloseListener(xListener)
        xDoc.close(True)
        self.assertEqual(1, listenerCalls)

    def test_closeFalse(self):
        global listenerCalls
        listenerCalls = 0
        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XCloseListenerExtended()
        xDoc.addCloseListener(xListener)
        xDoc.close(False)
        self.assertEqual(1, listenerCalls)


if __name__ == '__main__':
    unittest.main()
