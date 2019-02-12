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
from com.sun.star.lang import IllegalArgumentException

class TestXText(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._uno.openEmptyWriterDoc()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_insert_and_remove_annotations(self):
        x_text = self._uno.getDoc().getText()
        self.assertIsNotNone(x_text)

        # Insert annotation field
        x_annotation = self.create_annotation("John Doe")
        x_cursor = x_text.createTextCursor()
        x_text.insertTextContent(x_cursor, x_annotation, False)

        # And the same once again, actually not inserted
        x_text.insertTextContent(x_cursor, x_annotation, False)

        # Exception if we try to replace object by itself
        with self.assertRaises(IllegalArgumentException):
            x_text.insertTextContent(x_cursor, x_annotation, True)

        # We expect just one annotation actually
        self.check_annotations(["John Doe"])

        x_annotation_2 = self.create_annotation("Richard Roe")
        x_text.insertTextContent(x_cursor, x_annotation_2, True)
        self.check_annotations(["Richard Roe"])

        x_annotation_3 = self.create_annotation("Jane Roe")
        x_text.insertTextContent(x_cursor, x_annotation_3, True)
        self.check_annotations(["Jane Roe", "Richard Roe"])

        # Remove annotations
        x_text.removeTextContent(x_annotation_3)
        self.check_annotations(["Richard Roe"])
        x_text.removeTextContent(x_annotation_2)
        self.check_annotations([])

        # Remove _already removed_ ones again
        # TODO: unexpected behaviour, it should throw an exception,
        # but let's nail down current behaviour
        # NOTE: reported as tdf#123404
        x_text.removeTextContent(x_annotation_2)
        x_text.removeTextContent(x_annotation)

        self.check_annotations([])

    def create_annotation(self, author):
        x_annotation = self._uno.getDoc().createInstance("com.sun.star.text.TextField.Annotation")
        self.assertIsNotNone(x_annotation)
        x_annotation.setPropertyValue("Author", author)
        return x_annotation

    def check_annotations(self, authors):
        x_fields_enum = self._uno.getDoc().getTextFields().createEnumeration()

        annotations_found = 0

        for x_field, author in zip(x_fields_enum, authors):
            self.assertTrue(x_field.supportsService("com.sun.star.text.TextField.Annotation"))
            self.assertEqual(x_field.getPropertyValue("Author"), author)
            annotations_found += 1

        self.assertEqual(annotations_found, len(authors))

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
