# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from org.libreoffice.unotest import systemPathToFileUrl
from tempfile import TemporaryDirectory
import os.path


class exportToPDF(UITestCase):

    def test_checkDefaultValues(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, 'exportToPDFFromImpress-tmp.pdf')

            with self.ui_test.create_doc_in_start_center("impress"):

                xTemplateDlg = self.xUITest.getTopFocusWindow()
                xCancelBtn = xTemplateDlg.getChild("close")
                self.ui_test.close_dialog_through_button(xCancelBtn)

                xDoc = self.xUITest.getTopFocusWindow()
                xEdit = xDoc.getChild("impress_win")

                xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"Hello World"}))

                # Export as PDF
                with self.ui_test.execute_dialog_through_command('.uno:ExportToPDF', close_button="") as xDialog:

                    selectedChildren = ['bookmarks', 'display', 'effects', 'enablea11y',
                                        'enablecopy', 'exporturl', 'forms', 'reduceresolution', 'tagged']

                    for child in selectedChildren:
                        self.assertEqual("true", get_state_as_dict(xDialog.getChild(child))['Selected'])

                    nonSelectedChildren = ['allowdups', 'center', 'comments', 'convert', 'embed', 'emptypages', 'export', 'exportplaceholders',
                            'firstonleft', 'hiddenpages', 'menubar', 'notes', 'onlynotes', 'open', 'pdfa', 'pdfua', 'resize', 'singlepagesheets',
                            'toolbar', 'usereferencexobject', 'viewpdf', 'watermark', 'window']

                    for child in nonSelectedChildren:
                        self.assertEqual("false", get_state_as_dict(xDialog.getChild(child))['Selected'])

                    checkedChildren = ['all', 'allbookmarks', 'changeany', 'default', 'defaultlayout', 'fitdefault', 'jpegcompress', 'outline', 'printhigh']

                    for child in checkedChildren:
                        self.assertEqual("true", get_state_as_dict(xDialog.getChild(child))['Checked'])

                    nonCheckedChildren = ['changecomment', 'changeform', 'changeinsdel', 'changenone', 'contfacinglayout', 'contlayout', 'fitvis',
                            'fitwidth', 'fitwin', 'fitzoom', 'losslesscompress', 'openinternet', 'openpdf', 'pageonly', 'printlow', 'printnone', 'range',
                            'selection', 'singlelayout', 'thumbs', 'visiblebookmark']

                    for child in nonCheckedChildren:
                        self.assertEqual("false", get_state_as_dict(xDialog.getChild(child))['Checked'])

                    self.assertEqual("300 DPI", get_state_as_dict(xDialog.getChild("resolution"))['Text'])
                    self.assertEqual("90", get_state_as_dict(xDialog.getChild("quality"))['Value'])
                    self.assertEqual("FDF", get_state_as_dict(xDialog.getChild("format"))['DisplayText'])

                    xOk = xDialog.getChild("ok")
                    with self.ui_test.execute_dialog_through_action(xOk, "CLICK", close_button="open") as xSaveDialog:
                        xFileName = xSaveDialog.getChild('file_name')
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                        xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            # only run this if optional pdfimport service is available
            if self.xContext.ServiceManager.createInstance("org.libreoffice.comp.documents.DrawPDFImport"):
                with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as document:

                    self.assertEqual("", document.DrawPages[0].getByIndex(0).String)
                    self.assertEqual(" ", document.DrawPages[0].getByIndex(1).String)
                    self.assertEqual(" ", document.DrawPages[0].getByIndex(2).String)
                    self.assertEqual("Hello World", document.DrawPages[0].getByIndex(3).String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
