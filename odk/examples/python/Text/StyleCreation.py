# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import officehelper
import sys
import traceback

from com.sun.star.awt.FontWeight import BOLD
from com.sun.star.style.BreakType import PAGE_AFTER
from com.sun.star.style.ParagraphAdjust import CENTER

NEW_STYLE_NAME = "myheading"


def create_style(component):
    properties = component.createInstance("com.sun.star.style.ParagraphStyle")
    properties.setPropertyValue("CharFontName", "Liberation Sans")
    print("set name of the font to 'Liberation Sans'")

    properties.setPropertyValue("CharHeight", float(36))
    print("Change the height of th font to 36")

    properties.setPropertyValue("CharWeight", float(BOLD))
    print("set the font attribute 'Bold'")

    properties.setPropertyValue("CharAutoKerning", True)
    print("set the paragraph attribute 'AutoKerning'")

    properties.setPropertyValue("ParaAdjust", CENTER)
    print("set the paragraph adjust to LEFT")

    properties.setPropertyValue("ParaFirstLineIndent", 0)
    print("set the first line indent to 0 cm")

    properties.setPropertyValue("BreakType", PAGE_AFTER)
    print("set the paragraph attribute Breaktype to PageAfter")

    # insert the new Paragraph style in the Paragraph style collection
    style_families = component.getStyleFamilies()
    paragraph_style_col = style_families["ParagraphStyles"]
    paragraph_style_col[NEW_STYLE_NAME] = properties
    print("create new paragraph style, with the values from the Propertyset")


def apply_style(component):
    text_range = component.getText().getStart()
    # change the value from the property 'ParaStyle' to apply the Paragraph style
    # To run the sample with StarOffice 5.2 you'll have to change
    # 'ParaStyleName' to 'ParaStyle' in the next line
    text_range.setPropertyValue("ParaStyleName", NEW_STYLE_NAME)
    print("apply the new paragraph style")


def get_desktop():
    desktop = None
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        if srv_mgr is None:
            print("Can't create a desktop. No connection, no remote office servicemanager available!")
        else:
            desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
    return desktop


def main():
    desktop = get_desktop()
    if desktop is None:
        return

    try:
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, tuple())
        create_style(doc)
        apply_style(doc)
    except Exception:
        traceback.print_exc()
        sys.exit(1)

    print("Done")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
