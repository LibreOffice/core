# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
#from uitest.uihelper.common import type_text
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path
#Bug 90401 - remove personal info on redlines and annotations

class tdf90401(UITestCase):

   def test_tdf90401_remove_personal_info(self):

        # load a test document with a tracked change, and add a comment

        writer_doc = self.ui_test.load_file(get_url_for_data_file('redline-autocorrect.fodt'))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild('writer_edit')

        document = self.ui_test.get_component()
        selection = self.xUITest.executeCommand('.uno:SelectAll')
        self.xUITest.executeCommand('.uno:InsertAnnotation')

        # enable remove personal info security option

        self.ui_test.execute_dialog_through_command('.uno:OptionsTreeDialog')  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild('pages')
        xGenEntry = xPages.getChild('0')
        xSecurityPage = xGenEntry.getChild('6')
        xSecurityPage.executeAction('SELECT', tuple())
        # Click Button Options...
        xOptions = xDialog.getChild('options')
        def handle_options_dlg(dialog):
            xRemovePersonal = dialog.getChild('removepersonal')
            if get_state_as_dict(xRemovePersonal)['Selected'] == "false":
                xRemovePersonal.executeAction('CLICK', tuple())
            self.ui_test.wait_until_property_is_updated(xRemovePersonal, "Selected", "true")
            self.assertEqual(get_state_as_dict(xRemovePersonal)["Selected"], "true")
            #Click on its button Close
            xOkBtn = dialog.getChild('ok')
            xOkBtn.executeAction('CLICK', tuple())

        self.ui_test.execute_blocking_action(xOptions.executeAction, args=('CLICK', ()),
                dialog_handler=handle_options_dlg)

        xOKBtn = xDialog.getChild('ok')
        self.ui_test.close_dialog_through_button(xOKBtn)

        # save and reload the document to remove personal info

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, 'tdf90401-tmp.fodt')

            # Save Copy as
            self.ui_test.execute_dialog_through_command('.uno:SaveAs')
            xDialog = self.xUITest.getTopFocusWindow()

            xFileName = xDialog.getChild('file_name')
            xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
            xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
            xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            xOpenBtn = xDialog.getChild('open')
            self.ui_test.close_dialog_through_button(xOpenBtn)

            # Close the Writer document
            self.ui_test.close_doc()

            self.ui_test.load_file(systemPathToFileUrl(xFilePath))
            document = self.ui_test.get_component()

            # Wait for async events to be processed
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check removed personal info on comments

            textfields = document.getTextFields()
            author = ""
            year = -1
            for textfield in textfields:
                if textfield.supportsService("com.sun.star.text.TextField.Annotation"):
                    author = textfield.Author
                    year = textfield.Date.Year
            # This was 'Unknown Author'
            self.assertEqual(author, 'Author2')
            # This was 2021
            self.assertEqual(year, 0)

            # check removed personal info on tracked changes

            self.ui_test.execute_modeless_dialog_through_command('.uno:AcceptTrackedChanges')
            xTrackDlg = self.xUITest.getTopFocusWindow()
            xTreeList = xTrackDlg.getChild('writerchanges')
            state = get_state_as_dict(xTreeList)
            # This was 'NL\t11/03/2020 19:19:05\t', containing personal info
            self.assertEqual(state['SelectEntryText'], 'Author1\t01/01/1970 00:00:00\t')

        self.ui_test.close_doc()

   def test_tdf142902_remove_personal_info_in_DOCX(self):

        # load a test document with a tracked change, and add a comment

        writer_doc = self.ui_test.load_file(get_url_for_data_file('redline-para-join.docx'))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild('writer_edit')

        document = self.ui_test.get_component()
        selection = self.xUITest.executeCommand('.uno:SelectAll')
        self.xUITest.executeCommand('.uno:InsertAnnotation')

        # enable remove personal info security option

        self.ui_test.execute_dialog_through_command('.uno:OptionsTreeDialog')  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild('pages')
        xGenEntry = xPages.getChild('0')
        xSecurityPage = xGenEntry.getChild('6')
        xSecurityPage.executeAction('SELECT', tuple())
        # Click Button Options...
        xOptions = xDialog.getChild('options')
        def handle_options_dlg(dialog):
            xRemovePersonal = dialog.getChild('removepersonal')
            if get_state_as_dict(xRemovePersonal)['Selected'] == "false":
                xRemovePersonal.executeAction('CLICK', tuple())
            self.ui_test.wait_until_property_is_updated(xRemovePersonal, "Selected", "true")
            self.assertEqual(get_state_as_dict(xRemovePersonal)["Selected"], "true")
            #Click on its button Close
            xOkBtn = dialog.getChild('ok')
            xOkBtn.executeAction('CLICK', tuple())

        self.ui_test.execute_blocking_action(xOptions.executeAction, args=('CLICK', ()),
                dialog_handler=handle_options_dlg)

        xOKBtn = xDialog.getChild('ok')
        self.ui_test.close_dialog_through_button(xOKBtn)

        # save and reload the document to remove personal info

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, 'redline-para-join-tmp.docx')

            # Save Copy as
            self.ui_test.execute_dialog_through_command('.uno:SaveAs')
            xDialog = self.xUITest.getTopFocusWindow()

            xFileName = xDialog.getChild('file_name')
            xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
            xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
            xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            xOpenBtn = xDialog.getChild('open')
            self.ui_test.close_dialog_through_button(xOpenBtn)

            # DOCX confirmation dialog is displayed
            xWarnDialog = self.xUITest.getTopFocusWindow()
            xOK = xWarnDialog.getChild("save")
            self.ui_test.close_dialog_through_button(xOK)

            # Close the Writer document
            self.ui_test.close_doc()

            self.ui_test.load_file(systemPathToFileUrl(xFilePath))
            document = self.ui_test.get_component()

            # Wait for async events to be processed
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            # check removed personal info on comments

            textfields = document.getTextFields()
            author = ""
            year = -1
            for textfield in textfields:
                if textfield.supportsService("com.sun.star.text.TextField.Annotation"):
                    author = textfield.Author
                    year = textfield.Date.Year
            # This was 'Unknown Author'
            self.assertEqual(author, 'Author2')
            # This was 2021
            self.assertEqual(year, 0)

            # check removed personal info on tracked changes

            self.ui_test.execute_modeless_dialog_through_command('.uno:AcceptTrackedChanges')
            xTrackDlg = self.xUITest.getTopFocusWindow()
            xTreeList = xTrackDlg.getChild('writerchanges')
            state = get_state_as_dict(xTreeList)
            # This was 'NL\t11/03/2020 19:19:05\t', containing personal info
            self.assertEqual(state['SelectEntryText'], 'Author1\t01/01/1970 00:00:00\t')

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
