# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

from com.sun.star.beans import StringPair
from com.sun.star.frame import InfobarType
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.container import NoSuchElementException


# Test for Infobar API

class tdf126627(UITestCase):
    def test_infobar_add(self):
        self.ui_test.create_doc_in_start_center("writer")
        controller = self.ui_test.get_component().getCurrentController()
        buttons = [StringPair("Close", ".uno:CloseDoc")]
        controller.appendInfobar(
            "my", "Hello world", "The quick, brown fox jumps over a lazy dog.", InfobarType.INFO, buttons, True)

        # Adding another infobar with the same ID should throw an exception
        with self.assertRaises(IllegalArgumentException):
            controller.appendInfobar(
                "my", "Hello world", "The quick, brown fox jumps over a lazy dog.", InfobarType.INFO, buttons, True)
        self.ui_test.close_doc()

    def test_infobar_update(self):
        self.ui_test.create_doc_in_start_center("writer")
        controller = self.ui_test.get_component().getCurrentController()
        buttons = [StringPair("Close", ".uno:CloseDoc")]
        controller.appendInfobar(
            "my", "Hello world", "The quick, brown fox jumps over a lazy dog.", InfobarType.INFO, buttons, True)
        controller.updateInfobar("my", "Hello universe", "", InfobarType.WARNING)

        # Updating non-existing infobars should throw an exception
        with self.assertRaises(NoSuchElementException):
            controller.updateInfobar("notexisting", "", "", InfobarType.WARNING)

        # Passing invalid values for InfobarType should throw an exception
        with self.assertRaises(IllegalArgumentException):
            controller.updateInfobar("my", "", "", 120)

    def test_infobar_remove(self):
        self.ui_test.create_doc_in_start_center("writer")
        controller = self.ui_test.get_component().getCurrentController()
        buttons = [StringPair("Close", ".uno:CloseDoc")]
        controller.appendInfobar(
            "my", "Hello world", "The quick, brown fox jumps over a lazy dog.", InfobarType.INFO, buttons, True)

        controller.removeInfobar("my")

        # Removing an already removed infobar shoud throw an exception
        with self.assertRaises(NoSuchElementException):
            controller.removeInfobar("my")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
