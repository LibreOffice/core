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
from com.sun.star.container import NoSuchElementException
import uno
import time


class TestXText(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_insertAndRemoveAnnotations(self):
        xText = self._uno.getDoc().getText()
        self.assertIsNotNone(xText)

        # Insert annotation field
        xAnnotation = self.createAnnotation("John Doe")
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xAnnotation, False)

        # And the same once again, actually not inserted
        xText.insertTextContent(xCursor, xAnnotation, False)

        # Exception if we try to replace object by itself
        with self.assertRaises(IllegalArgumentException):
            xText.insertTextContent(xCursor, xAnnotation, True)
        # We expect just one annotation actually
        self.checkAnnotations(["John Doe"])

        xAnnotation2 = self.createAnnotation("Richard Roe")
        xText.insertTextContent(xCursor, xAnnotation2, True)
        self.checkAnnotations(["Richard Roe"])

        xAnnotation3 = self.createAnnotation("Jane Roe")
        xText.insertTextContent(xCursor, xAnnotation3, True)
        self.checkAnnotations(["Jane Roe", "Richard Roe"])

        # Remove annotations
        xText.removeTextContent(xAnnotation3)
        self.checkAnnotations(["Richard Roe"])
        xText.removeTextContent(xAnnotation2)
        self.checkAnnotations([])

        # Remove _already removed_ ones again
        # TODO: unexpected behaviour, it should throw an exception,
        # but let's nail down current behaviour
        xText.removeTextContent(xAnnotation2)
        xText.removeTextContent(xAnnotation)

        self.checkAnnotations([])

    def createAnnotation(self, author):
        xAnnotation = self._uno.getDoc().createInstance("com.sun.star.text.TextField.Annotation")
        self.assertIsNotNone(xAnnotation)
        xAnnotation.setPropertyValue("Author", author)
        return xAnnotation

    def checkAnnotations(self, authors):
        xFieldsEnum = self._uno.getDoc().getTextFields().createEnumeration()
        annotationsFound = 0
        for xField, author in zip(xFieldsEnum, authors):
            self.assertTrue(xField.supportsService("com.sun.star.text.TextField.Annotation"))
            self.assertEqual(xField.getPropertyValue("Author"), author)
            annotationsFound += 1
        self.assertEqual(annotationsFound, len(authors))

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
