# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import re
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import type_text

from libreoffice.linguistic.linguservice import get_spellchecker
from com.sun.star.lang import Locale

class SpellingAndGrammarDialog(UITestCase):

    def is_supported_locale(self, language, country):
        xSpellChecker = get_spellchecker(self.ui_test._xContext)
        locales = xSpellChecker.getLocales()
        for locale in locales:
            if language != None:
                if locale.Language != language:
                    continue

            if country != None:
                if locale.Country != country:
                    continue

            # we found the correct combination
            return True

    TDF46852_INPUT = """\
dogg
dogg
catt dogg
frogg frogg
frogg catt dogg
dogg catt
frog, dogg, catt"""

    TDF46852_REGEX = """\
([a-z]+)
\\1
([a-z]+) \\1
([a-z]+) \\3
\\3 \\2 \\1
\\1 \\2
\\3, \\1, \\2"""

    def test_tdf46852(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")
        # This automates the steps described in the bug report tdf#46852

        # Step 1: Create a document with repetitious misspelled words
        with self.ui_test.create_doc_in_start_center("writer") as document:
            cursor = document.getCurrentController().getViewCursor()
            # Inserted text must be en_US, so make sure to set language in current location
            cursor.CharLocale = Locale("en", "US", "")
            input_text = self.TDF46852_INPUT.replace('\n', '\r') # \r = para break
            document.Text.insertString(cursor, input_text, False)

            # Step 2: Place cursor on 4th line after second "frogg"
            cursor.goUp(2, False)
            cursor.goLeft(1, False)

            # Step 3: Initiate spellchecking, and make sure "Check grammar" is
            # unchecked
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="") as xDialog:
                checkgrammar = xDialog.getChild('checkgrammar')
                if get_state_as_dict(checkgrammar)['Selected'] == 'true':
                    checkgrammar.executeAction('CLICK', ())
                self.assertTrue(get_state_as_dict(checkgrammar)['Selected'] == 'false')

                # Step 4: Repetitively click on "Correct all" for each misspelling
                #         prompt until end of document is reached.
                changeall = xDialog.getChild('changeall')
                changeall.executeAction("CLICK", ())
                changeall.executeAction("CLICK", ())
                # The third time we click on changeall, the click action is going to
                # block while two message boxes are shown, so we need to do this third
                # click specially
                # Use empty close_button to open consecutive dialogs
                with self.ui_test.execute_blocking_action(
                        changeall.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    # Step 5: Confirm to "Continue check at beginning of document"
                    xYesBtn = dialog.getChild("yes")

                    with self.ui_test.execute_blocking_action(
                            xYesBtn.executeAction, args=('CLICK', ())):
                        pass

            output_text = document.Text.getString().replace('\r\n', '\n')
            self.assertTrue(re.match(self.TDF46852_REGEX, output_text))

    def test_tdf136855(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")

        with self.ui_test.load_file(get_url_for_data_file("tdf136855.odt")) as writer_doc:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="close") as xDialog:

                xChangeBtn = xDialog.getChild('change')
                for i in range(6):
                    # Without the fix in place, this test would have crashed here
                    xChangeBtn.executeAction("CLICK", ())

            output_text = writer_doc.Text.getString().replace('\n', '').replace('\r', '')
            self.assertTrue(output_text.startswith("xx xx xx xxxxxxxxxxix xxxxxxxxxxxxxxviii"))

    def test_tdf66043(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")
        with self.ui_test.load_file(get_url_for_data_file("tdf66043.fodt")) as writer_doc:
            # Step 1: Initiate spellchecking, and make sure "Check grammar" is
            # unchecked
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="close") as xDialog:
                checkgrammar = xDialog.getChild('checkgrammar')
                if get_state_as_dict(checkgrammar)['Selected'] == 'true':
                    checkgrammar.executeAction('CLICK', ())
                self.assertTrue(get_state_as_dict(checkgrammar)['Selected'] == 'false')

                # Step 2: Click on "Correct all" for each misspelling
                #         prompt until end of document is reached.
                changeall = xDialog.getChild('changeall')
                changeall.executeAction("CLICK", ())

            output_text = writer_doc.Text.getString().replace('\r\n', '\n')
            # This was "gooodgood baaad eeend" ("goood" is a deletion,
            # "good" is an insertion by fixing the first misspelling),
            # but now "goood" is not a misspelling because it is accepted
            # correctly without the redline containing a deleted "o"
            self.assertEqual(output_text, 'goood baaadbaaed eeend')

    def test_DoNotCheckURL(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")

        with self.ui_test.create_doc_in_start_center("writer") as document:
            cursor = document.getCurrentController().getViewCursor()
            # Inserted text must be en_US, so make sure to set language in current location
            cursor.CharLocale = Locale("en", "US", "")

            xMainWindow = self.xUITest.getTopFocusWindow()
            xEdit = xMainWindow.getChild("writer_edit")

            # URL is recognized during typing
            type_text(xEdit, "baaad http://www.baaad.org baaad baaad")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="close") as xDialog:
                checkgrammar = xDialog.getChild('checkgrammar')
                if get_state_as_dict(checkgrammar)['Selected'] == 'true':
                    checkgrammar.executeAction('CLICK', ())
                self.assertTrue(get_state_as_dict(checkgrammar)['Selected'] == 'false')

                change = xDialog.getChild('change')
                change.executeAction("CLICK", ())
                change.executeAction("CLICK", ())

            output_text = document.Text.getString()
            # This was "Baaed HTTP://www.baaad.org baaad baaad" (spelling URLs)
            self.assertEqual("Baaed http://www.baaad.org baaed baaad", output_text)

    def test_tdf45949(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")

        with self.ui_test.create_doc_in_start_center("writer") as document:
            cursor = document.getCurrentController().getViewCursor()
            # Inserted text must be en_US, so make sure to set language in current location
            cursor.CharLocale = Locale("en", "US", "")

            xMainWindow = self.xUITest.getTopFocusWindow()
            xEdit = xMainWindow.getChild("writer_edit")

            # URL is recognized during typing
            type_text(xEdit, "baaad http://www.baaad.org baaad")

            # add spaces before and after the word "baaad" within the URL
            cursor.goLeft(10, False)
            type_text(xEdit, " ")
            cursor.goLeft(6, False)
            type_text(xEdit, " ")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="close") as xDialog:
                checkgrammar = xDialog.getChild('checkgrammar')
                if get_state_as_dict(checkgrammar)['Selected'] == 'true':
                    checkgrammar.executeAction('CLICK', ())
                self.assertTrue(get_state_as_dict(checkgrammar)['Selected'] == 'false')

                change = xDialog.getChild('change')
                change.executeAction("CLICK", ())
                change.executeAction("CLICK", ())

            output_text = document.Text.getString()
            # This was "Baaed HTTP://www. baaad .org baaed" (skipped non-URL words of hypertext)
            self.assertEqual("Baaed http://www. baaed .org baaad", output_text)

    def test_tdf65535(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no dictionary support for en_US available")

        with self.ui_test.load_file(get_url_for_data_file("tdf65535.fodt")) as document:
            cursor = document.getCurrentController().getViewCursor()
            # Inserted text must be en_US, so make sure to set language in current location
            cursor.CharLocale = Locale("en", "US", "")

            xMainWindow = self.xUITest.getTopFocusWindow()
            xEdit = xMainWindow.getChild("writer_edit")

            # type a bad word after the word with comment
            cursor.goRight(5, False)
            type_text(xEdit, " baad")
            cursor.goLeft(10, False)

            # fix the first word using the spelling dialog
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SpellingAndGrammarDialog", close_button="close") as xDialog:
                checkgrammar = xDialog.getChild('checkgrammar')
                if get_state_as_dict(checkgrammar)['Selected'] == 'true':
                    checkgrammar.executeAction('CLICK', ())
                self.assertTrue(get_state_as_dict(checkgrammar)['Selected'] == 'false')

                change = xDialog.getChild('change')
                change.executeAction("CLICK", ())

            output_text = document.Text.getString()
            self.assertEqual("Bad baad", output_text)

            # check the original comment
            has_comment = False
            textfields = document.getTextFields()
            for textfield in textfields:
                if textfield.supportsService("com.sun.star.text.TextField.Annotation"):
                    has_comment = True

            # This was False (lost comment)
            self.assertEqual(True, has_comment)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
