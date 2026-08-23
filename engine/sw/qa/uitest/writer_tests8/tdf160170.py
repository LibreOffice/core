# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
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

    # Extend the list of ignored words with the hyphenations the test needs,
    # so it does not depend on the installed hyphenation patterns. This is what
    # the edit dictionary dialog did, see SvxEditDictionaryDialog::NewDelHdl.
    def set_custom_hyphenation(self):
        xDicList = self.xContext.ServiceManager.createInstanceWithContext(
            'com.sun.star.linguistic2.DictionaryList', self.xContext)

        # STR_DESCRIPTION_IGNOREALLLIST, the name GetIgnoreAllList looks up
        xDic = xDicList.getDictionaryByName('List of Ignored Words')
        self.assertIsNotNone(xDic)
        xDic.setActive(True)
        xDic.add(u'phare=tra', False, u'')

    def test_tdf160170(self):
        supported_locale = self.is_supported_locale("en", "US")
        if not supported_locale:
            self.skipTest("no hyphenation patterns for en_US available")

        # we must not depend on the installed hyphenation patterns,
        # so extend the list of ignored words with the hyphenation phare=tra
        self.set_custom_hyphenation()

        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        with self.ui_test.load_file(get_url_for_data_file("tdf160170.fodt")) as writer_doc:
            xToolkit.processEventsToIdle()
            # delete the text of the first line
            self.xUITest.executeCommand(".uno:GoToEndOfLine")
            self.xUITest.executeCommand('.uno:StartOfDocumentSel')
            self.xUITest.executeCommand('.uno:Delete')
            paragraphs = writer_doc.Text.createEnumeration()
            para1 = paragraphs.nextElement()
            # This was " pellentesque est orvi.", i.e. too much shrinking
            self.assertEqual("tra. Fusce pellentesque est orci.", para1.String)
