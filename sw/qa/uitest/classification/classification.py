# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
# import org.libreoffice.unotest
# import pathlib
from uitest.path import get_srcdir_url
def get_url_for_data_file(file_name):
#    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

#TSCP: add advanced classification dialog https://cgit.freedesktop.org/libreoffice/core/commit/?id=71ee09947d5a71105d64fd225bb3672dfa7ce834
# This adds a advanced classification dialog, which enables the user
# to manually construct a header/footer message from classification
# properties in cases where the user needs more control. All the
# text is inserted as fields into the end document and can be changed
# by changing the document properties. The simple classification
# still functions as it did before, which is what most users will
# want to use in this case.

#[Bug 122491] Classification: dialog (Paragraph) Classification doesn't show previously chosen classification
#https://wiki.documentfoundation.org/TSCP-classification

        #new file; open Doc classification dialog
        #add classification Confidential, save
        #reopen and verify
        #verify watermark Bug 122586 - Classification: by using the dialog, Watermark text from policy is not placed in the document
        #open dialog and add intellectualProperty text, save
        #verify in doc properties text of IP (in API, cannot verify in UI)
        #reopen and verify classification and Content
        #do the same for Paragraph classification
        # verify the text on characters 0-6 : "(Conf)"
        #+ new file and do it only for Paragraph classification (no watermark!)
        #+ bug with bold text Bug 122565 - Classification dialog - Button "bold" works only in dialog

        #variants of content written manually, IP strings, classification (Conf, NB, GB, IO)
        #[Bug 122491] with testdoc

class classification(UITestCase):
    def test_document_classification_dialog(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.execute_dialog_through_command(".uno:ClassificationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        classificationEditWindow = xDialog.getChild("classificationEditWindow")
        recentlyUsedCB = xDialog.getChild("recentlyUsedCB")
        classificationCB = xDialog.getChild("classificationCB")
        internationalClassificationCB = xDialog.getChild("internationalClassificationCB")
        intellectualPropertyPartEntry = xDialog.getChild("intellectualPropertyPartEntry")
        intellectualPropertyPartAddButton = xDialog.getChild("intellectualPropertyPartAddButton")

        props = {"TEXT": "Confidential"}
        actionProps = mkPropertyValues(props)
        classificationCB.executeAction("SELECT", actionProps)
        #verify International is set too
        self.assertEqual(get_state_as_dict(internationalClassificationCB)["SelectEntryText"], "Confidential")
        #verify textBox Content
        # self.assertEqual(get_state_as_dict(classificationEditWindow)["Text"], "Conf")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify watermark
        #Bug 122586 - Classification: by using the dialog, Watermark text from policy is not placed in the document
        self.ui_test.execute_dialog_through_command(".uno:Watermark")
        xDialog = self.xUITest.getTopFocusWindow()
        xTextInput = xDialog.getChild("TextInput")
        xAngle = xDialog.getChild("Angle")
        xTransparency = xDialog.getChild("Transparency")
        self.assertEqual(get_state_as_dict(xTextInput)["Text"], "Confidential")
        self.assertEqual(get_state_as_dict(xAngle)["Text"], "45°")
        self.assertEqual((get_state_as_dict(xTransparency)["Text"])[0:2], "50")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        #TODO
        #open dialog and add intellectualProperty text, save
        #verify
        #reopen and verify classification and Content

        #do the same for Paragraph classification
        self.ui_test.execute_dialog_through_command(".uno:ParagraphClassificationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        classificationEditWindow = xDialog.getChild("classificationEditWindow")
        recentlyUsedCB = xDialog.getChild("recentlyUsedCB")
        classificationCB = xDialog.getChild("classificationCB")
        internationalClassificationCB = xDialog.getChild("internationalClassificationCB")
        intellectualPropertyPartEntry = xDialog.getChild("intellectualPropertyPartEntry")
        intellectualPropertyPartAddButton = xDialog.getChild("intellectualPropertyPartAddButton")

        props = {"TEXT": "Confidential"}
        actionProps = mkPropertyValues(props)
        classificationCB.executeAction("SELECT", actionProps)
        #verify International is set too
        self.assertEqual(get_state_as_dict(internationalClassificationCB)["SelectEntryText"], "Confidential")
        #verify textBox Content  TODO textbox not supported
#        self.assertEqual(get_state_as_dict(classificationEditWindow)["Text"], "Conf")
        # print(get_state_as_dict(classificationEditWindow))

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # verify the text on characters 0-6 : "(Conf)"
        self.assertEqual(document.Text.String[0:6], "(Conf)")

        self.ui_test.close_doc()

    def test_paragraph_classification_dialog(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #+ new file and do it only for Paragraph classification (no watermark!)
        self.ui_test.execute_dialog_through_command(".uno:ParagraphClassificationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        classificationEditWindow = xDialog.getChild("classificationEditWindow")
        recentlyUsedCB = xDialog.getChild("recentlyUsedCB")
        classificationCB = xDialog.getChild("classificationCB")
        internationalClassificationCB = xDialog.getChild("internationalClassificationCB")
        intellectualPropertyPartEntry = xDialog.getChild("intellectualPropertyPartEntry")
        intellectualPropertyPartAddButton = xDialog.getChild("intellectualPropertyPartAddButton")

        props = {"TEXT": "Confidential"}
        actionProps = mkPropertyValues(props)
        classificationCB.executeAction("SELECT", actionProps)
        #verify International is set too
        self.assertEqual(get_state_as_dict(internationalClassificationCB)["SelectEntryText"], "Confidential")
        #verify textBox Content  TODO  - texbox not supported yet
        # self.assertEqual(get_state_as_dict(classificationEditWindow)["Text"], "Conf")
        # print(get_state_as_dict(classificationEditWindow))

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # verify the text on characters 0-6 : "(Conf)"
        self.assertEqual(document.Text.String[0:6], "(Conf)")

        self.ui_test.close_doc()

    def test_paragraph_classification_dialog_text(self):
        writer_doc = self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #+ new file and do it only for Paragraph classification (no watermark!)
        self.ui_test.execute_dialog_through_command(".uno:ParagraphClassificationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        classificationEditWindow = xDialog.getChild("classificationEditWindow")
        recentlyUsedCB = xDialog.getChild("recentlyUsedCB")
        classificationCB = xDialog.getChild("classificationCB")
        internationalClassificationCB = xDialog.getChild("internationalClassificationCB")
        intellectualPropertyPartEntry = xDialog.getChild("intellectualPropertyPartEntry")
        intellectualPropertyPartAddButton = xDialog.getChild("intellectualPropertyPartAddButton")
        #type text AA
        intellectualPropertyPartEntry.executeAction("TYPE", mkPropertyValues({"TEXT":"AA"}))
        intellectualPropertyPartAddButton.executeAction("CLICK", tuple())
        props = {"TEXT": "Internal Only"}
        actionProps = mkPropertyValues(props)
        classificationCB.executeAction("SELECT", actionProps)
        #verify International is set too
        self.assertEqual(get_state_as_dict(internationalClassificationCB)["SelectEntryText"], "Internal Only")
        #verify textBox Content  TODO  - texbox not supported yet
        # self.assertEqual(get_state_as_dict(classificationEditWindow)["Text"], "Conf")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # verify the text on characters 0-6 : "(AAIO)"
        self.assertEqual(document.Text.String[0:6], "(AAIO)")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
