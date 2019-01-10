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
import os

from org.libreoffice.unotest import UnoInProcess
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK

class TestVarFields(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_var_fields(self):
        """
        Reproduce fdo#55814.

        Note: this test was migrated from java (the steps numbering too)
        sw/qa/complex/writer/VarFields.java

        """

        xDoc = self.__class__._uno.openEmptyWriterDoc()
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()

        # 0. create text field
        xField = xDoc.createInstance("com.sun.star.text.textfield.SetExpression")

        # 1. fill it with properties
        self.__class__._uno.setProperties(xField,
                                          {"Content": "0",
                                           "IsVisible": True,
                                           "Hint": "trying to reproduce fdo#55814",
                                           "SubType": 0,  # VAR
                                           "Value": 0.0})

        # 2. create master field
        xMaster = xDoc.createInstance("com.sun.star.text.fieldmaster.SetExpression")

        # 3. set name of the master field to "foo"
        xMaster.setPropertyValue("Name", "foo")

        # 4. get Dependent Field
        # no op in python ;-)

        # 5. connect real field to the master
        xField.attachTextFieldMaster(xMaster)

        # 6. insert text field into the document
        xBodyText.insertTextContent(xCursor, xField, False)

        # 7. retrieve paragraph cursor
        xParagraphCursor = xCursor
        xParagraphCursor.gotoEndOfParagraph(False)  # not selected

        # 8. enter new line
        xBodyText.insertControlCharacter(xCursor, PARAGRAPH_BREAK, False)

        # 9. create new text section
        xTextSection = xDoc.createInstance("com.sun.star.text.TextSection")

        # 10. fill the properties of section
        self.__class__._uno.checkProperties(xTextSection,
                                            {"Condition": "foo EQ 1",
                                             "IsVisible": False},
                                            self)

        # 11. Insert some text to be content on the section
        xBodyText.insertString(xCursor,
                               "The quick brown fox jumps over the lazy dog",
                               True)

        # 12. insert section
        xBodyText.insertTextContent(xCursor, xTextSection, True)

        # 12.1 insert new paragraph. Note: that's here the difference
        xParagraphCursor.gotoEndOfParagraph(False)  # not select

        # TODO: how to leave the section now?
        xBodyText.insertControlCharacter(xCursor, PARAGRAPH_BREAK, False)
        xBodyText.insertString(xCursor, "new paragraph", False)

        # 13. Access fields to refresh the document
        xTextFields = xDoc.getTextFields()

        # 14. refresh document to update the fields
        xTextFields.refresh()

        # 15. retrieve the field
        xFieldEnum = xTextFields.createEnumeration()

        # Note: we have only one field here, that why nextElement() is just fine here
        xField = xFieldEnum.nextElement()

        # check
        read_content = xField.getPropertyValue("Content")
        self.assertEqual("0", read_content)
        read_content = xField.getPropertyValue("Value")
        self.assertEqual(0.0, read_content)

        # 16. change the value of the field from 0 to 1 and check
        self.__class__._uno.checkProperties(xField, {"Value": 1.0,
                                                     "Content": "1"},
                                            self)

        # 17. refresh document to update the fields again
        xTextFields.refresh()

        # 18. store document
        url = os.path.join(os.environ["TestUserDir"], "VarFields.odt")
        xDoc.storeToURL(url, tuple(list(range(0))))

        # 19. retrieve the section
        xSection = xDoc.getTextSections().getByIndex(0)

        # 20. retrieve the condition property of that section
        read_content = xSection.getPropertyValue("Condition")

        # 21. check
        self.assertEqual("foo EQ 1", read_content)

if __name__ == '__main__':
    unittest.main()
