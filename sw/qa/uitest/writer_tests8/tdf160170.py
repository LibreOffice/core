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

# handle tdf#160170 overshrank lines with smart justify and hyphenation

class tdf160170(UITestCase):
    def is_supported_locale(self, language, country):
        xLinguServiceManager = get_lingu_service_manager(self.ui_test._xContext)
        xHyphenator = xLinguServiceManager.getHyphenator()
        locales = xHyphenator.getLocales()
        for locale in locales:
            if language is not None:
                if locale.Language != language:
                    continue

            if country is not None:
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

            # add hyphenation "phare=tra" to the custom dictionary
            # to solve the non-accessible hyphenation patterns for the test

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
                # write phare=tra into the input box
                inputbox = xEdit.getChild("word")
                inputbox.executeAction("TYPE", mkPropertyValues({"TEXT": "phare=tra"}))
                add = xEdit.getChild("newreplace")
                add.executeAction("CLICK", tuple())

    def test_tdf160170(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no hyphenation patterns for en_US available")

        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        with self.ui_test.load_file(get_url_for_data_file("tdf160170.fodt")) as writer_doc:
            # we must not depend on the installed hyphenation patterns,
            # so extend user dictionary temporarily with the hyphenation phare=tra
            self.set_custom_hyphenation()
            xToolkit.processEventsToIdle()
            # delete the text of the first line
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # This was " pellentesque est orvi.", i.e. too much shrinking
            self.assertEqual("tra. Fusce pellentesque est orci.", para1.String)
