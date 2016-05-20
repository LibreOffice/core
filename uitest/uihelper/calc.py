# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from helper import mkPropertyValues

def enter_text_to_cell(gridwin, cell, text):
    gridwin.executeAction("SELECT", mkPropertyValues({"CELL": cell}))
    gridwin.executeAction("TYPE", mkPropertyValues({"TEXT": text}))
    gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
