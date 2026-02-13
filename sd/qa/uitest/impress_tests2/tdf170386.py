# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from com.sun.star.awt import Point
import pathlib
import tempfile

class tdf170386(UITestCase):
    def test_tdf170386(self):
        # Create a Draw document, lock its controllers, add a text shape, and apply bold to it.
        # Unlock the controllers, save and reload. The expectation is, that the applied bold
        # remains applied to the text.
        # There are two levels where bold is applied to the shape and its text:
        # 1. The shape itself (draw:frame element) has a paragraph autostyle applied through
        #     draw:text-style-name attribute, and that defined the font weight;
        # 2. The paragraphs of text (text:p elements) may apply their own paragraph autoformats.
        # It is a question, why the draw:frame's draw:text-style-name apparently is not applied
        # to the paragraphs inside the shape, when the paragraphs don't define their own style;
        # but the bug was, that the paragraphs missed their autostyles, and so the text runs were
        # not bold (even though the shape-level property was present). It happened because the
        # object used to collect text properties on save wasn't notified on the changes that
        # happened when the model was locked.

        with tempfile.NamedTemporaryFile(suffix=".odg") as temp:
            url = pathlib.Path(temp.name).as_uri()

            with self.ui_test.create_doc_in_start_center("draw") as xModel:
                xPage = xModel.getDrawPages().getByIndex(0)
                xModel.lockControllers()
                xShape = xModel.createInstance("com.sun.star.drawing.TextShape")
                xPage.add(xShape)
                xShape.setString("Some Text")
                xShape.setPosition(Point(2000, 600))
                xShape.setPropertyValue("CharWeight", 150.0)
                xModel.unlockControllers()
                xModel.storeAsURL(url, [])

            with self.ui_test.load_file(url) as xModel:
                xShape = xModel.getDrawPages().getByIndex(0).getByIndex(0)
                # The shape-level property was OK even without the fix:
                self.assertEqual(150.0, xShape.getPropertyValue("CharWeight"))
                # Test that the text inside the shape (accessed using cursor) has correct weight.
                # Without the fix, this failed with "150.0 != 100.0"
                self.assertEqual(150.0, xShape.createTextCursor().getPropertyValue("CharWeight"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
