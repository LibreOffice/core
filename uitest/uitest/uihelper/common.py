# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import convert_property_values_to_dict, mkPropertyValues

def get_state_as_dict(ui_object):
    return convert_property_values_to_dict(ui_object.getState())

def type_text(ui_object, text):
    ui_object.executeAction("TYPE", mkPropertyValues({"TEXT": text}))

def select_pos(ui_object, pos):
    ui_object.executeAction("SELECT", mkPropertyValues({"POS": pos}))

def select_text(ui_object, from_pos, to):
    ui_object.executeAction("SELECT", mkPropertyValues({"FROM": from_pos, "TO": to}))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
