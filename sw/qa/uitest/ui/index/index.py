# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

"""Covers sw/source/ui/index/ fixes."""

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text


class TestSwuiidxmrk(UITestCase):

    def test_bibliography_page_number_insert(self):

        # Given an empty Writer document:
        self.ui_test.create_doc_in_start_center("writer")

        # When inserting a biblio entry field with a page number:
        def handle_define_entry(define_entry):
            entry = define_entry.getChild("entry")
            type_text(entry, "aaa")
            listbox = define_entry.getChild("listbox")
            select_pos(listbox, "16")  # WWW document
            pagecb = define_entry.getChild("pagecb-visible")
            pagecb.executeAction("CLICK", tuple())
            ok = define_entry.getChild("ok")
            self.ui_test.close_dialog_through_button(ok)

        self.ui_test.execute_modeless_dialog_through_command(".uno:InsertAuthoritiesEntry")
        insert_entry = self.xUITest.getTopFocusWindow()
        from_document = insert_entry.getChild("fromdocument")
        from_document.executeAction("CLICK", tuple())
        new = insert_entry.getChild("new")
        self.ui_test.execute_blocking_action(new.executeAction,
                                             args=('CLICK', ()),
                                             dialog_handler=handle_define_entry)
        insert = insert_entry.getChild("insert")
        insert.executeAction("CLICK", tuple())
        close = insert_entry.getChild("close")
        self.ui_test.close_dialog_through_button(close)

        # Then make sure the URL contains that page number:
        component = self.ui_test.get_component()
        paragraphs = component.Text.createEnumeration()
        paragraph = paragraphs.nextElement()
        portions = paragraph.createEnumeration()
        portion = portions.nextElement()
        for field in portion.TextField.Fields:
            if field.Name != "URL":
                continue
            # Without the accompanying fix in place, this test would have failed with:
            # AssertionError: '' != '#page=1'
            # i.e. the page number was not part of the URL.
            self.assertEqual(field.Value, "#page=1")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
