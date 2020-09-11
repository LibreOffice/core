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

from com.sun.star.text.TextContentAnchorType import AT_PARAGRAPH

class CheckDrawPage(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    """
    Test that grouping shapes also works with a single shape.
    """
    def test_group_single_shape(self):
        xDoc = self.__class__._uno.openEmptyWriterDoc()
        page = xDoc.DrawPage
        collection = self.__class__._uno.xContext.ServiceManager.createInstance( 'com.sun.star.drawing.ShapeCollection' )
        shape = xDoc.createInstance('com.sun.star.drawing.TextShape')
        shape.AnchorType = AT_PARAGRAPH
        page.add(shape)
        collection.add(shape)
        shapegroup = page.group(collection)

        self.assertEqual(shapegroup.Count, 1)

        xDoc.close(True)

if __name__ == '__main__':
    unittest.main()
