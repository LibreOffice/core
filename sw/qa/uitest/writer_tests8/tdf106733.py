# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.linguistic.linguservice import get_lingu_service_manager
from uitest.uihelper.common import select_pos

# handle tdf#106733 hyphenation of words disabled by character formatting

class tdf106733(UITestCase):
    def is_supported_locale(self, language, country):
        xLinguServiceManager = get_lingu_service_manager(self.ui_test._xContext)
        xHyphenator = xLinguServiceManager.getHyphenator()
        locales = xHyphenator.getLocales()
        for locale in locales:
            if language != None:
                if locale.Language != language:
                    continue

            if country != None:
                if locale.Country != country:
                    continue

            # we found the correct combination
            return True

    def set_custom_hyphenation(self):
        with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:

            xPages = xDialog.getChild("pages")
            xLanguageEntry = xPages.getChild('2')                 # Language Settings
            xLanguageEntry.executeAction("EXPAND", tuple())
            xxLanguageEntryWritingAidsEntry = xLanguageEntry.getChild('1')
            xxLanguageEntryWritingAidsEntry.executeAction("SELECT", tuple())          # Writing Aids

            # add hyphenations to the custom dictionary to solve the non-accessible
            # hyphenation patterns for the test

            # Select an editable dictionary (list of Ignored words)
            dictionaries = xDialog.getChild("lingudicts")
            hasEditableDictionary = False
            for i in dictionaries.getChildren():
                entry = dictionaries.getChild(i)
                entry_label = get_state_as_dict(entry)["Text"]
                if entry_label == "List of Ignored Words [All]":
                    hasEditableDictionary = True
                    entry.executeAction("SELECT", tuple())          # an editable user dictionary
                    break

            self.assertEqual(True, hasEditableDictionary)

            # open Edit dialog window
            edit = xDialog.getChild("lingudictsedit")
            with self.ui_test.execute_blocking_action(edit.executeAction, args=('CLICK', ()), close_button="close") as xEdit:
                # add in=ertially and ex=cept to the custom hyphenations
                inputbox = xEdit.getChild("word")
                inputbox.executeAction("TYPE", mkPropertyValues({"TEXT": "ex=cept"}))
                add = xEdit.getChild("newreplace")
                add.executeAction("CLICK", tuple())
                inputbox.executeAction("TYPE", mkPropertyValues({"TEXT": "in=ertially"}))
                add.executeAction("CLICK", tuple())

    def test_tdf106733_disable_hyphenation(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no hyphenation patterns for en_US available")

        with self.ui_test.load_file(get_url_for_data_file("tdf106733.fodt")) as writer_doc:
            # we must not depend on the installed hyphenation patterns,
            # so extend user dictionary temporarily with the requested hyphenations
            self.set_custom_hyphenation()

            # delete the text of the first line
            for i in range(5):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # check default "ex=cept" hyphenation
            self.assertEqual(True, para1.String.startswith("cept"))

            # check disabled hyphenations (by direct character formatting)
            for i in range(6):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # This was False (the line started with "cept", because of the enabled hyphenation)
            self.assertEqual(True, para1.String.startswith(" except"))

            # check disabled hyphenations (by character style)
            for i in range(6):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # This was False (the line started with "cept", because of the enabled hyphenation)
            self.assertEqual(True, para1.String.startswith(" except"))

    def set_format_character(self, check):
            with self.ui_test.execute_dialog_through_command(".uno:FontDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")
                xNoHyphenation = xDialog.getChild("nohyphenation")
                checkValues = { False: "false", True: "true" }
                self.assertEqual(get_state_as_dict(xNoHyphenation)["Selected"], checkValues[check])
                xNoHyphenation.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xNoHyphenation)["Selected"], checkValues[not check])

    def test_tdf106733_format_character(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no hyphenation patterns for en_US available")

        with self.ui_test.load_file(get_url_for_data_file("tdf106733.fodt")) as writer_doc:

            # enable NoHyphenation on first "except"
            for i in range(38):
                self.xUITest.executeCommand(".uno:GoToNextWord")
            self.xUITest.executeCommand(".uno:WordRightSel")
            self.set_format_character(False)

            # disable NoHyphenation on second "except"
            for i in range(50):
                self.xUITest.executeCommand(".uno:GoToNextWord")
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:SelectWord")
            self.set_format_character(True)

            # disable NoHyphenation on third "except" set by its character style
            for i in range(51):
                self.xUITest.executeCommand(".uno:GoToNextWord")
            self.xUITest.executeCommand(".uno:GoRight")
            self.xUITest.executeCommand(".uno:SelectWord")

            # remove character style "Strong Emphasis" to disable NoHyphenation
            self.xUITest.executeCommand(".uno:StyleApply?Style:string=No%20Character%20Style&FamilyName:string=CharacterStyles")
            # and restore the bold formatting to hyphenate the last word
            self.xUITest.executeCommand('.uno:Bold')

            self.xUITest.executeCommand('.uno:GoToStartOfDoc')

            self.set_custom_hyphenation()
            # delete the text of the first line
            for i in range(5):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # disabled hyphenation on the first "except"
            self.assertEqual(True, para1.String.startswith(" except"))

            # check disabled hyphenations (by direct character formatting)
            for i in range(6):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # enabled hyphenation on the second "except"
            self.assertEqual(True, para1.String.startswith("cept"))

            # check disabled hyphenations (by character style)
            for i in range(6):
                self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # enabled hyphenation on the third "except"
            self.assertEqual(True, para1.String.startswith("cept"))

