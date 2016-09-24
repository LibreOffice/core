# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import convert_property_values_to_dict

def get_state_as_dict(ui_object):
    return convert_property_values_to_dict(ui_object.getState())

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
