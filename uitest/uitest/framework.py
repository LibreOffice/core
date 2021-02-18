# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import signal
import unittest
import time

from uitest.test import UITest

from libreoffice.connection import PersistentConnection

class UITestCase(unittest.TestCase):

    def __init__(self, test_name, opts):
        unittest.TestCase.__init__(self, test_name)
        self.opts = opts

    def setUp(self):
        self.setSignalHandler()
        self.connection = PersistentConnection(self.opts)
        self.connection.setUp()
        self.xContext = self.connection.getContext()
        self.xUITest = self.xContext.ServiceManager.createInstanceWithContext(
                "org.libreoffice.uitest.UITest", self.xContext)

        self.ui_test = UITest(self.xUITest, self.xContext)
        self.startTime = time.time()

    def tearDown(self):
        try:
            t = time.time() - self.startTime
            print("Execution time for %s: %.3f" % (self.id(), t))
            if self.xContext is not None:
                try:
                    desktop = self.ui_test.get_desktop()
                    components = desktop.getComponents()
                    for component in components:
                            component.close(False)
                except Exception as e:
                    print(e)

            self.connection.tearDown()
        finally:
            self.resetSignalHandler()
            self.connection.kill()

    def signalHandler(self, signum, frame):
        if self.connection:
            self.connection.kill()

    def setSignalHandler(self):
        signal.signal(signal.SIGABRT, self.signalHandler)
        signal.signal(signal.SIGSEGV, self.signalHandler)
        signal.signal(signal.SIGTERM, self.signalHandler)
        signal.signal(signal.SIGILL, self.signalHandler)

    def resetSignalHandler(self):
        signal.signal(signal.SIGABRT, signal.SIG_IGN)
        signal.signal(signal.SIGSEGV, signal.SIG_IGN)
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
        signal.signal(signal.SIGILL, signal.SIG_IGN)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
