#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import pathlib
import unittest

from org.libreoffice.unotest import UnoInProcess, makeCopyFromTDOC
from com.sun.star.beans import PropertyValue


class TestXStyleLoader(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_loadStyleFromStream(self):
        xDoc = self.__class__._uno.openEmptyWriterDoc()
        self.assertIsNotNone(xDoc)

        xServiceManager = self.__class__._uno.xContext.ServiceManager
        simpleFileAccess = xServiceManager.createInstance(
            "com.sun.star.ucb.SimpleFileAccess")
        xInputStream = simpleFileAccess.openFileRead(
            pathlib.Path(makeCopyFromTDOC("xstyleloader.odt")).as_uri())

        p1 = PropertyValue(Name="InputStream", Value=xInputStream)
        p2 = PropertyValue(Name="LoadTextStyles", Value=True)

        styles = xDoc.getStyleFamilies()
        styles.loadStylesFromURL("private:stream", [p1, p2])
        textStyles = styles.getByName("ParagraphStyles")
        self.assertTrue(textStyles.hasByName("Test_Template"))

        xDoc.close(True)


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
