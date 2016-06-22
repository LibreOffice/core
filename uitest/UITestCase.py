# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest

from uitest_helper import UITest

from connection import PersistentConnection, OfficeConnection

class UITestCase(unittest.TestCase):

    def __init__(self, test_name, opts):
        unittest.TestCase.__init__(self, test_name)
        self.opts = opts

    def setUp(self):
        self.connection = PersistentConnection(self.opts)
        self.connection.setUp()
        self.xContext = self.connection.getContext()
        self.xUITest = self.xContext.ServiceManager.createInstanceWithContext(
                "org.libreoffice.uitest.UITest", self.xContext)

        self.ui_test = UITest(self.xUITest, self.xContext)

    def tearDown(self):
        self.connection.tearDown()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
