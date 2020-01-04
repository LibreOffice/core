# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.path import get_srcdir_url
import os
from tempfile import TemporaryDirectory


def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/calc_tests/data/" + file_name


# Bug 120161: PRINTING, PDF Export: Problem with selected cells which cross pages
class tdf120161(UITestCase):
    def getFileContent(self, pathAndFileName):
        with open(pathAndFileName, 'rb') as theFile:  # b is important -> binary
            # Return as binary string
            data = theFile.read()
        return data

    def verifyExportToFile(self, xDoc, xContext, xRange, xFontName, xFilename):
        # set selection
        xGridWin = xDoc.getChild("grid_window")
        xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": xRange}))

        # set print area
        self.xUITest.executeCommand(".uno:DefinePrintArea")

        # create temp file name
        xURL = 'file:///' + xFilename

        # prepare to export into pdf
        xServiceManager = xContext.ServiceManager
        xDispatcher = xServiceManager.createInstanceWithContext(
            'com.sun.star.frame.DispatchHelper', xContext)
        xDocFrame = self.ui_test.get_desktop().getCurrentFrame()
        document = self.ui_test.get_component()

        # get selection
        xSelection = document.Sheets.getByName("Sheet1").getCellRangeByName(xRange)
        self.assertIsNotNone(xSelection)

        # run export into pdf
        xFilterData = mkPropertyValues(
            {'Selection': xSelection, 'ViewPDFAfterExport': True, 'Printing': '2'})
        xParams = mkPropertyValues(
            {'URL': xURL, 'FilterName': 'calc_pdf_Export', 'FilterData': xFilterData})
        xDispatcher.executeDispatch(xDocFrame, '.uno:ExportToPDF', '', 0, xParams)

        # check resulting pdf file
        xFileContent = self.getFileContent(xFilename)
        position = xFileContent.find(xFontName)
        return position > 0

    # create temp directory and filename inside it
    def verifyExport(self, xDoc, xContext, xRange, xFontName):
        with TemporaryDirectory() as tempdir:
            if os.altsep:  # we need URL so replace "\" with "/"
                tempdir = tempdir.replace(os.sep, os.altsep)
            xFilename = tempdir + "/tdf120161-temp.pdf"
            return self.verifyExportToFile(xDoc, xContext, xRange, xFontName, xFilename)
        return False

    def test_tdf120161(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf120161.ods"))
        xDoc = self.xUITest.getTopFocusWindow()
        xContext = self.xContext

        # check different areas to be printed without any lost cell
        # note:
        # 1. Visually in GridView G1 is on page-1 and H1 is on page-2
        # 2. DejaVuSans is used only in H1
        self.assertFalse(self.verifyExport(xDoc, xContext, "A1:G1", b"DejaVuSans"))
        self.assertTrue(self.verifyExport(xDoc, xContext, "H1:I1", b"DejaVuSans"))
        self.assertTrue(self.verifyExport(xDoc, xContext, "G1:H1", b"DejaVuSans"))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
