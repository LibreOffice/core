# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import os
import uno

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file


def make_policy(name, group_sizes):
    """A SPIF policy with one classification and a category group per size, named "Group 1"
    onwards, each with that many categories."""
    tags = []
    for group, size in enumerate(group_sizes, start=1):
        categories = "".join(
            '<spif:tagCategory name="G{0}C{1}" lacv="{1}" obsolete="false" />'.format(group, category)
            for category in range(1, size + 1))
        tags.append('<spif:securityCategoryTag name="Group {0}" tagType="enumerated" '
                    'enumType="permissive" singleSelection="false">{1}'
                    '</spif:securityCategoryTag>'.format(group, categories))

    return ('<?xml version="1.0" encoding="utf-8"?>'
            '<spif:SPIF xmlns:spif="http://www.xmlspif.org/spif" schemaVersion="1.0" version="1">'
            '<spif:securityPolicyId name="{0}" id="1.2.3.4" />'
            '<spif:securityClassifications>'
            '<spif:securityClassification name="OFFICIAL" lacv="1" hierarchy="1" obsolete="false" />'
            '</spif:securityClassifications>'
            '<spif:securityCategoryTagSets>'
            '<spif:securityCategoryTagSet name="Tags" id="1.2.3.4.0">{1}</spif:securityCategoryTagSet>'
            '</spif:securityCategoryTagSets>'
            '</spif:SPIF>').format(name, "".join(tags))


class securityLabelDialog(UITestCase):

    def write_policy(self, policy):
        """Provisions the policy in the user profile, where the dialog loads policies from."""
        substitution = self.xContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.util.PathSubstitution", self.xContext)
        directory = os.path.join(
            uno.fileUrlToSystemPath(substitution.substituteVariables("$(user)", True)), "spif")
        os.makedirs(directory, exist_ok=True)
        path = os.path.join(directory, "policy.xml")
        with open(path, "w", encoding="utf-8") as policyFile:
            policyFile.write(policy)
        return path

    def open_dialog_and_read_error(self, policy, name):
        """Opens the dialog on an OOXML document with only the given policy provisioned, checks
        that the policy with the given name is the active one, and returns whether the error
        about what the dialog cannot show is visible, and its text."""
        path = self.write_policy(policy)
        try:
            with self.ui_test.load_file(get_url_for_data_file("date_picker.docx")):
                with self.ui_test.execute_dialog_through_command(
                        ".uno:SecurityLabel", close_button="cancel") as xDialog:
                    self.assertEqual(
                        name, get_state_as_dict(xDialog.getChild("policy"))["SelectEntryText"])
                    state = get_state_as_dict(xDialog.getChild("seclabeloverflow"))
                    return state["Visible"] == "true", state["Text"]
        finally:
            os.remove(path)

    def test_policy_that_fits_shows_no_error(self):
        visible, _ = self.open_dialog_and_read_error(
            make_policy("Small Policy", [2, 3]), "Small Policy")
        self.assertFalse(visible)

    def test_policy_too_big_names_what_is_not_shown(self):
        # Eleven groups, one more than the dialog has room for, and a first group with two
        # categories more than a group can show.
        visible, text = self.open_dialog_and_read_error(
            make_policy("Big Policy", [302] + [1] * 10), "Big Policy")
        self.assertTrue(visible)
        self.assertEqual(
            "Policy 'Big Policy' has 11 category groups, but this dialog can show only 10. "
            "Not shown: 'Group 11'.\n"
            "Category group 'Group 1' has 302 categories, but this dialog can show only 300. "
            "The other 2 are not shown.", text)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
