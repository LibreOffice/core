# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# this file provides methods to interact with the new conditional format API

def get_conditional_format_from_sheet(sheet):
    return sheet.getPropertyValue("ConditionalFormats")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
