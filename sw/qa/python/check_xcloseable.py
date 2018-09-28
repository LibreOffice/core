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
from com.sun.star.util import CloseVetoException
import time


listenerCalls = 0
listenerBlocks = 0


class XCloseListenerExtended(unohelper.Base, XCloseListener):
    @classmethod
    def queryClosing(self, Source, GetsOwnership):
        global listenerCalls
        listenerCalls += 1

    @classmethod
    def notifyClosing(self, Source):
        pass

    @classmethod
    def disposing(self, Event):
        pass


class XCloseListenerBlocker(unohelper.Base, XCloseListener):
    @classmethod
    def queryClosing(self, Source, GetsOwnership):
        global listenerBlocks
        print("queryClosing(" + str(GetsOwnership) + ") was called, listenerBlocks=" + str(listenerBlocks))
        # 1->2: we allow to close the object
        if listenerBlocks == 1:
            listenerBlocks = 2
            Source.Source.close(True)
        # 0->1: we blocked to close the object
        if listenerBlocks == 0:
            listenerBlocks = 1
            raise CloseVetoException(Source)

    @classmethod
    def notifyClosing(self, Source):
        print("notifyClosing was called")
        pass

    @classmethod
    def notifyEvent(self, Source):
        print("notifyEvent was called")
        
    @classmethod
    def disposing(self, Param):
        print("disposing was called")
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

    def test_closeWithBlock(self):
        global listenerBlocks
        listenerBlocks = 0
        xDoc = self._uno.openEmptyWriterDoc()
        xListener = XCloseListenerBlocker()
        xDoc.addCloseListener(xListener)

        print("Try to close document: 1 of 2")
        xDoc.close(True)
        print("Doc was closed")
        xDoc = None
#        with self.assertRaises(CloseVetoException):
        
        self.assertEqual(1, listenerBlocks)
        time.sleep(1)
        print("Finished")
        print(a)

#        print("Try to close document: 2 of 2")
#        xDoc.close(True)
#        self.assertEqual(2, listenerBlocks)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
