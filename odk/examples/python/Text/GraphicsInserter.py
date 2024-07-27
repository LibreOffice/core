# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import argparse
import sys
import traceback
from os.path import isfile, dirname, join

import officehelper
from com.sun.star.beans import PropertyValue
from com.sun.star.text.TextContentAnchorType import AT_PARAGRAPH

LOG_FILE = join(dirname(__file__), "log.txt")


def insert_graphic(filename):
    remote_context = officehelper.bootstrap()
    srv_mgr = remote_context.getServiceManager()
    desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)

    doc_url = "private:factory/swriter"
    doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())

    log_file = open(LOG_FILE, "w")

    text = doc.getText()
    cursor = text.createTextCursor()

    try:
        graphic = doc.createInstance("com.sun.star.text.TextGraphicObject")
    except Exception:
        traceback.print_exc(file=log_file)
        return

    log_file.write("inserting graphic\n")
    try:
        text.insertTextContent(cursor, graphic, True)
    except Exception:
        print("Could not insert Content")
        traceback.print_exc()
        return

    log_file.write("adding graphic\n")
    try:
        graphic_url = f"file://{filename}".replace("\\", "/")
        print("insert graphic: %s", graphic_url)
        graphic_provider = srv_mgr.createInstanceWithContext(
            "com.sun.star.graphic.GraphicProvider", remote_context
        )
        loaded_graphic = graphic_provider.queryGraphic(
            (PropertyValue(Name="URL", Value=graphic_url),)
        )

        # Setting the graphic url
        graphic.setPropertyValue("Graphic", loaded_graphic)

        # Set properties for the inserted graphic
        graphic.setPropertyValue("AnchorType", AT_PARAGRAPH)
        # Setting the horizontal position
        graphic.setPropertyValue("HoriOrientPosition", 5500)
        # Setting the vertical position
        graphic.setPropertyValue("VertOrientPosition", 4200)
        # Setting the width
        graphic.setPropertyValue("Width", 4400)
        # Setting the height
        graphic.setPropertyValue("Height", 4000)
    except Exception:
        print("Couldn't set property 'GraphicURL'")
        traceback.print_exc(file=log_file)

    log_file.close()


def is_file(value):
    if not isfile(value):
        raise argparse.ArgumentTypeError(f"File {value} is not an image file.")
    return value


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=is_file, help="Path to an image file.")
    args = parser.parse_args()
    try:
        insert_graphic(args.image)
    except Exception:
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
