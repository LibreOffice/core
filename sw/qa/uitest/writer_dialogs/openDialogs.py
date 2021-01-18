# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from uitest.uihelper.testDialog import testDialog

dialogs = [
    {"command": ".uno:OpenRemote", "closeButton": "cancel"},
    {"command": ".uno:NewDoc", "closeButton": "close"},
    {"command": ".uno:SaveAsTemplate", "closeButton": "cancel"},
    {"command": ".uno:ExportToPDF", "closeButton": "cancel", "skipTestOK": True},
        # export needs filesystem
    {"command": ".uno:ExportToEPUB", "closeButton": "cancel", "skipTestOK": True},
        # export needs filesystem
    {"command": ".uno:Print", "closeButton": "cancel", "skipTestOK": True},
        # no printer in CI
        # tested in sw/qa/uitest/writer_tests5/tdf123378.py
    {"command": ".uno:PrinterSetup", "closeButton": "cancel"},
    # {"command": ".uno:SetDocumentProperties", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests2/documentProperties.py
    # {"command": ".uno:PasteSpecial", "closeButton": "cancel"},
        # would need to copy first something into the clipboard
    # {"command": ".uno:SearchDialog", "closeButton": "close"},
        # tested in sw/qa/uitest/findReplace/findReplace.py
    # {"command": ".uno:GotoPage", "closeButton": "cancel"},
        # tested in uitest/writer_tests/goToPage.py
    # {"command": ".uno:AcceptTrackedChanges", "closeButton": "close"},
        # tested in sw/qa/uitest/writer_tests/trackedChanges.py
    {"command": ".uno:ProtectTraceChangeMode", "closeButton": "cancel"},
    # {"command": ".uno:ChangeDatabaseField", "closeButton": "close"},
        # tested in sw/qa/uitest/writer_tests2/exchangeDatabase.py
    # {"command": ".uno:InsertBreak", "closeButton": "cancel"},
        # tested in uitest/writer_tests/insertBreakDialog.py
    {"command": ".uno:InsertObject", "closeButton": "cancel"},
    {"command": ".uno:InsertSection", "closeButton": "cancel"},
    {"command": ".uno:InsertFrame", "closeButton": "cancel"},
    {"command": ".uno:InsertObjectFloatingFrame", "closeButton": "cancel"},
    {"command": ".uno:FontworkGalleryFloater", "closeButton": "cancel"},
    # {"command": ".uno:HyperlinkDialog", "closeButton": "close"},
        # dialog opens but is not recognised by execute_dialog_through_command
    # {"command": ".uno:InsertBookmark", "closeButton": "close"},
        # tested in sw/qa/uitest/writer_tests2/bookmark.py
    # {"command": ".uno:InsertReferenceField", "closeButton": "close"},
        # dialog not closed
    # {"command": ".uno:InsertSymbol", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests3/specialCharacter.py
    # {"command": ".uno:InsertFootnoteDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests3/insertFootEndnote.py
    {"command": ".uno:InsertIndexesEntry", "closeButton": "close"},
    {"command": ".uno:InsertAuthoritiesEntry", "closeButton": "close"},
    {"command": ".uno:InsertMultiIndex", "closeButton": "cancel"},
        # button is labeled "Close" but is called "cancel"
    {"command": ".uno:InsertFieldCtrl", "closeButton": "cancel", "skipTestOK": True},
        # when running the test through the make command the OK button (insert) is enabled for some reason
    # {"command": ".uno:InsertEnvelope", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests3/insertEnvelope.py
    # {"command": ".uno:InsertSignatureLine", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests3/insertSignatureLine.py
    # {"command": ".uno:FontDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests2/formatCharacter.py
    # {"command": ".uno:ParagraphDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests2/formatParagraph.py
    # {"command": ".uno:BulletsAndNumberingDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests2/formatBulletsNumbering.py
    # {"command": ".uno:PageDialog", "closeButton": "cancel"},
        # tested in uitest/writer_tests/pageDialog.py
    # {"command": ".uno:TitlePageDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests5/titlePage.py
    # {"command": ".uno:RubyDialog", "closeButton": "close"},
        # dialog opens but is not recognised by execute_dialog_through_command
    # {"command": ".uno:FormatColumns", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests5/columns.py
    # {"command": ".uno:Watermark", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests/watermark.py
    # {"command": ".uno:EditStyle", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests2/horizontalLine.py
    #{"command": ".uno:LoadStyles", "closeButton": "cancel"},
        # tested in sw/qa/uitest/sidebar/stylesSidebar.py
    # {"command": ".uno:InsertTable", "closeButton": "cancel"},
        # tested in uitest/writer_tests/insertTableDialog.py
    # {"command": ".uno:SpellDialog", "closeButton": "close"},
        # an extra dialog appears
    # {"command": ".uno:ThesaurusDialog", "closeButton": "cancel"},
        # fails in CI, but works fine locally
    {"command": ".uno:Hyphenate", "closeButton": "ok", "skipTestOK": True},
        # do not test the OK button twice
    {"command": ".uno:ChineseConversion", "closeButton": "cancel"},
    # {"command": ".uno:WordCountDialog", "closeButton": "close"},
        # tested in sw/qa/uitest/writer_tests/wordCount.py
    # {"command": ".uno:AutoCorrectDlg", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests5/autocorrectOptions.py
    {"command": ".uno:EditGlossary", "closeButton": "close"},
    # {"command": ".uno:ChapterNumberingDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/chapterNumbering/chapterNumbering.py
    # {"command": ".uno:LineNumberingDialog", "closeButton": "cancel"},
        # tested in sw/qa/uitest/writer_tests3/lineNumbering.py
    {"command": ".uno:FootnoteDialog", "closeButton": "cancel"},
    # {"command": ".uno:MailMergeWizard", "closeButton": "cancel"},
        # may hang forever in e_book_client_connect_direct_sync
        # tested in sw/qa/uitest/writer_tests7/tdf134243.py
    {"command": ".uno:AddressBookSource", "closeButton": "cancel"},
    {"command": ".uno:RunMacro", "closeButton": "cancel"},
    {"command": ".uno:ScriptOrganizer", "closeButton": "close"},
    {"command": ".uno:OpenXMLFilterSettings", "closeButton": "close"},
    {"command": "service:com.sun.star.deployment.ui.PackageManagerDialog", "closeButton": "close"},
    # {"command": ".uno:ConfigureDialog", "closeButton": "cancel"},
        # tested in uitest/writer_tests/customizeDialog.py
    #{"command": ".uno:OptionsTreeDialog", "closeButton": "cancel"},
        # used in various places
    {"command": ".uno:ShowLicense", "closeButton": "close"},
    # {"command": ".uno:About", "closeButton": "close"},
        # tested in sw/qa/uitest/writer_tests5/about_test.py
]

def load_tests(loader, tests, pattern):
    return unittest.TestSuite(openDialogs(dialog)
                              for dialog in dialogs)

# Test to open all listed dialogs one by one, close it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if writer crashes by opening the dialog
class openDialogs(UITestCase):
    def check(self, dialog):
        testDialog(self, "writer", dialog)

dialogCount = 0
for dialog in dialogs:
    dialogCount = dialogCount + 1


    def ch(dialog):
        return lambda self: self.check(dialog)


    setattr(openDialogs, "test_%02d_%s" % (dialogCount, dialog["command"]), ch(dialog))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
