# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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

    def test_bibliography_local_page_number_insert(self):

        # Given an empty Writer document:
        with self.ui_test.create_doc_in_start_center("writer") as component:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertAuthoritiesEntry", close_button="close") as insert_entry:
                from_document = insert_entry.getChild("fromdocument")
                from_document.executeAction("CLICK", tuple())
                new = insert_entry.getChild("new")

                # When inserting a biblio entry field with a page number:
                with self.ui_test.execute_blocking_action(new.executeAction, args=('CLICK', ())) as define_entry:
                    entry = define_entry.getChild("entry")
                    type_text(entry, "aaa")
                    listbox = define_entry.getChild("listbox")
                    select_pos(listbox, "16")  # WWW document, just select a valid position
                    pagecb = define_entry.getChild("pagecb-local-visible")
                    pagecb.executeAction("CLICK", tuple())

                insert = insert_entry.getChild("insert")
                insert.executeAction("CLICK", tuple())

            # Then make sure the local URL contains that page number:
            paragraphs = component.Text.createEnumeration()
            paragraph = paragraphs.nextElement()
            portions = paragraph.createEnumeration()
            portion = portions.nextElement()
            for field in portion.TextField.Fields:
                if field.Name != "LocalURL":
                    continue
                # Without the accompanying fix in place, this test would have failed with:
                # Could not find child with id: pagecb-local-visible
                self.assertEqual(field.Value, "#page=1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
