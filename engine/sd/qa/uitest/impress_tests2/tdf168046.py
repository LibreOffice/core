# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from com.sun.star.awt import Point, Size
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit, get_url_for_data_file, select_pos


class tdf168046(UITestCase):

    @staticmethod
    def get_test_group(document):
        # Attachment 202439's copied test-area group contains formatted text.
        return document.getDrawPages().getByIndex(0).getByIndex(8)

    @staticmethod
    def get_text_runs(shape):
        runs = []
        paragraphs = shape.createEnumeration()
        while paragraphs.hasMoreElements():
            portions = paragraphs.nextElement().createEnumeration()
            while portions.hasMoreElements():
                portion = portions.nextElement()
                text = portion.getString()
                if text:
                    runs.append((
                        text,
                        portion.getPropertyValue("CharFontName"),
                        portion.getPropertyValue("CharHeight"),
                        portion.getPropertyValue("CharColor"),
                        portion.getPropertyValue("CharWeight"),
                        portion.getPropertyValue("CharPosture"),
                        portion.getPropertyValue("CharEscapement"),
                    ))
        return runs

    def test_tdf168046_accept_and_undo_preserves_text_formatting(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf168046.odg")) as document:
            group = self.get_test_group(document)
            text = group.getByIndex(3)
            document.getCurrentController().select(group)

            before = self.get_text_runs(text)
            self.assertTrue(before)
            self.assertGreater(len({run[2] for run in before}), 1)
            self.assertGreater(len({run[3] for run in before}), 1)

            # Accepting the Position and Size dialog without edits reproduces the report.
            with self.ui_test.execute_dialog_through_command(".uno:TransformDialog"):
                pass

            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(before, self.get_text_runs(self.get_test_group(document).getByIndex(3)))

    def test_tdf168046_resize_undo_preserves_text_formatting(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf168046.odg")) as document:
            group = self.get_test_group(document)
            text = group.getByIndex(3)
            document.getCurrentController().select(group)

            before_runs = self.get_text_runs(text)
            before_position = group.getPosition()
            before_size = group.getSize()

            with change_measurement_unit(self, "Millimeter"):
                with self.ui_test.execute_dialog_through_command(".uno:TransformDialog") as dialog:
                    width = dialog.getChild("MTR_FLD_WIDTH")
                    width.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                    width.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
                    width.executeAction("TYPE", mkPropertyValues({"TEXT": "300"}))

            self.assertNotEqual(before_size.Width, group.getSize().Width)
            self.xUITest.executeCommand(".uno:Undo")
            restored_group = self.get_test_group(document)
            self.assertEqual((before_position.X, before_position.Y),
                             (restored_group.getPosition().X, restored_group.getPosition().Y))
            self.assertEqual((before_size.Width, before_size.Height),
                             (restored_group.getSize().Width, restored_group.getSize().Height))
            self.assertEqual(before_runs, self.get_text_runs(restored_group.getByIndex(3)))

    def test_tdf168046_text_in_shape_remains_formatted(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf168046-shape-text.odg")) as document:
            # Attachment 202464 groups a text object with a rectangle containing text.
            group = document.getDrawPages().getByIndex(0).getByIndex(2)
            text_object = group.getByIndex(0)
            shape = group.getByIndex(1)
            before_text_object = self.get_text_runs(text_object)
            before_shape = self.get_text_runs(shape)
            self.assertTrue(before_text_object)
            self.assertTrue(before_shape)

            document.getCurrentController().select(group)
            with self.ui_test.execute_dialog_through_command(".uno:TransformDialog"):
                pass

            self.xUITest.executeCommand(".uno:Undo")
            restored_group = document.getDrawPages().getByIndex(0).getByIndex(2)
            self.assertEqual(before_text_object, self.get_text_runs(restored_group.getByIndex(0)))
            self.assertEqual(before_shape, self.get_text_runs(restored_group.getByIndex(1)))

    def test_tdf159736_transform_dispatch_undo_preserves_subscript(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf159736.odg")) as document:
            group = document.getDrawPages().getByIndex(0).getByIndex(0)
            text = group.getByIndex(1)
            before_runs = self.get_text_runs(text)
            self.assertTrue(any(run[6] < 0 for run in before_runs))
            before_angle = group.getPropertyValue("RotateAngle")
            document.getCurrentController().select(group)

            # The sidebar sends preset arguments through this transform slot.
            position = group.getPosition()
            size = group.getSize()
            self.xUITest.executeCommandWithParameters(".uno:TransformDialog", mkPropertyValues({
                "TransformRotationAngle": 4500,
                "TransformRotationX": position.X + size.Width // 2,
                "TransformRotationY": position.Y + size.Height // 2,
            }))

            self.assertNotEqual(before_angle, group.getPropertyValue("RotateAngle"))
            self.xUITest.executeCommand(".uno:Undo")
            restored_group = document.getDrawPages().getByIndex(0).getByIndex(0)
            self.assertEqual(before_angle, restored_group.getPropertyValue("RotateAngle"))
            self.assertEqual(before_runs, self.get_text_runs(restored_group.getByIndex(1)))

    def test_tdf168046_caption_dialog_still_applies_caption_attributes(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            template_dialog = self.xUITest.getTopFocusWindow()
            self.ui_test.close_dialog_through_button(template_dialog.getChild("close"))

            caption = document.createInstance("com.sun.star.drawing.CaptionShape")
            caption.setPosition(Point(1000, 1000))
            caption.setSize(Size(10000, 8000))
            caption.setString("Caption")
            page = document.getDrawPages().getByIndex(0)
            page.add(caption)
            caption_index = page.getCount() - 1
            document.getCurrentController().select(caption)

            before_length = caption.getPropertyValue("CaptionLineLength")
            before_fit = caption.getPropertyValue("CaptionIsFitLineLength")
            with self.ui_test.execute_dialog_through_command(".uno:TransformDialog") as dialog:
                select_pos(dialog.getChild("tabcontrol"), "1")
                optimal = dialog.getChild("optimal")
                if before_fit:
                    optimal.executeAction("CLICK", tuple())
                length = dialog.getChild("length")
                length.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                length.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
                length.executeAction("TYPE", mkPropertyValues({"TEXT": "20"}))

            self.assertNotEqual(before_length, caption.getPropertyValue("CaptionLineLength"))
            self.assertFalse(caption.getPropertyValue("CaptionIsFitLineLength"))
            self.xUITest.executeCommand(".uno:Undo")
            restored_caption = page.getByIndex(caption_index)
            self.assertEqual(before_length, restored_caption.getPropertyValue("CaptionLineLength"))
            self.assertEqual(before_fit, restored_caption.getPropertyValue("CaptionIsFitLineLength"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
