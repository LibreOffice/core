# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Steps on the Color page of an Area or Background tab, which every fill starts from.

from contextlib import contextmanager
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, select_by_text, select_pos

# The positions of the Fill Effects tabs when the dialog offers all three effects.
FILL_EFFECT_GRADIENT = "0"
FILL_EFFECT_HATCH = "1"
FILL_EFFECT_IMAGE = "2"

def select_palette_color(dialog, palette, pos):
    """Fills with the color at position pos of the palette named palette."""
    select_by_text(dialog.getChild("paletteselector"), palette)
    dialog.getChild("iconview_colors").getChild(pos).executeAction("SELECT", mkPropertyValues({}))

@contextmanager
def open_fill_effects(ui_test, dialog, tab, close_button="ok"):
    """Opens Fill Effects through More effects, on the tab at position tab. The effect showing
    when it closes with OK becomes the fill."""
    with ui_test.execute_dialog_through_action(
            dialog.getChild("moreeffects"), "CLICK", close_button=close_button) as xEffects:
        select_pos(xEffects.getChild("tabcontrol"), tab)
        yield xEffects

def set_custom_color(ui_test, dialog, hex_color):
    """Fills with hex_color, six hex digits typed into the Pick a Color dialog."""
    with ui_test.execute_dialog_through_action(dialog.getChild("customcolor"), "CLICK") as xPicker:
        xHex = xPicker.getChild("hexEntry")
        xHex.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xHex.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xHex.executeAction("TYPE", mkPropertyValues({"TEXT": hex_color}))

def select_fill(test, dialog, fill):
    """Sets the fill named fill with its first preset: "none", "color" (Light Blue 2 of the
    Standard palette), "gradient", "hatch", "image" or "pattern"."""
    if fill == "none":
        select_by_text(dialog.getChild("paletteselector"), "None")
    elif fill == "color":
        select_palette_color(dialog, "Standard", "56")
    elif fill == "pattern":
        dialog.getChild("patternstyle").executeAction("OPENLIST", tuple())
        # Position 0 is None, so 1 is the first pattern.
        xPattern = test.xUITest.getFloatWindow().getChild("patternstylelist").getChild("1")
        xPattern.executeAction("SELECT", mkPropertyValues({}))
        xPattern.executeAction("DOUBLECLICK", tuple())
    else:
        tab = {"gradient": FILL_EFFECT_GRADIENT, "hatch": FILL_EFFECT_HATCH,
               "image": FILL_EFFECT_IMAGE}[fill]
        with open_fill_effects(test.ui_test, dialog, tab):
            pass

def get_fill_caption(dialog):
    """The caption under the fill preview, such as "Color: Yellow (#FFFF00)" or "None"."""
    return get_state_as_dict(dialog.getChild("previewcaption"))["Text"]

# vim: set shiftwidth=4 softtabstop=4 expandtab:
