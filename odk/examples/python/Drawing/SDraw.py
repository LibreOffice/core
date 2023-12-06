# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# comment: Step 1: bootstrap UNO and get the remote component context
#          Step 2: open an empty text document
#          Step 3: get the drawpage an insert some shapes

import officehelper
import sys
import traceback
import math

from com.sun.star.awt import Size
from com.sun.star.awt import Point


def main():
    try:
        # Step 1: bootstrap UNO and get the remote component context. The
        #         context can be used to get the service manager.
        remote_context = officehelper.bootstrap()
        print("Connected to a running office ...")
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)

        # Step 2: open an empty document. In this case it's a writer document.
        #         For this purpose an instance of com.sun.star.frame.Desktop is
        #         created. It's interface XDesktop provides the XComponentLoader,
        #         which is used to open the document via loadComponentFromURL
        print("Opening an empty Draw document")
        doc_url = "private:factory/sdraw"
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())
    except Exception as e:
        print(f"Exception: {e}", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)

    generate(doc)


def generate(doc):
    # Step 3: get the drawpage an insert some shapes.
    #         the documents DrawPageSupplier supplies the DrawPage vi IndexAccess
    #         To add a shape get the MultiServiceFaktory of the document, create an
    #         instance of the ShapeType and add it to the Shapes-container
    #         provided by the drawpage

    # get the drawpage of drawing here
    print("getting Draw page")
    try:
        draw_page = doc.getDrawPages()
    except Exception as e:
        print(f"Couldn't create document: {e}", file=sys.stderr)
        traceback.print_exc()
    else:
        try:
            generate_sequence(doc, draw_page.getByIndex(0))
            # put something on the drawpage
            print("inserting some Shapes")
            draw_page.getByIndex(0).add(create_shape(doc, 2000, 1500, 1000, 1000, "Line", 0))
            draw_page.getByIndex(0).add(create_shape(doc, 3000, 4500, 15000, 1000, "Ellipse", 16711680))
            draw_page.getByIndex(0).add(create_shape(doc, 5000, 3500, 7500, 5000, "Rectangle", 6710932))
        except Exception as e:
            print(f"Exception: {e}", file=sys.stderr)
            traceback.print_exc()

    print("done")


def create_shape(doc, height: int, width: int, x: int, y: int, kind: str, col: int):
    # possible values for kind are 'Ellipse', 'Line' and 'Rectangle'
    try:
        size = Size()
        position = Point()
        shape = doc.createInstance(f"com.sun.star.drawing.{kind}Shape")
        size.Height = height
        size.Width = width
        position.X = x
        position.Y = y
        shape.setSize(size)
        shape.setPosition(position)
    except Exception as e:
        # Some exception occurs.FAILED
        print(f"Couldn't get instance: {e}", file=sys.stderr)
        traceback.print_exc()
        return

    try:
        shape.setPropertyValue("FillColor", col)
    except Exception as e:
        print(f"Can't change colors: {e}", file=sys.stderr)
        traceback.print_exc()
        return
    return shape


def generate_sequence(doc, page):
    size = Size()
    position = Point()
    height = 3000
    width = 3500
    x = 1900
    y = 20000
    ellipse_shape = None
    r = 40
    g = 0
    b = 80

    for i in range(0, 370, 25):
        try:
            ellipse_shape = doc.createInstance("com.sun.star.drawing.EllipseShape")
            size.Height = height
            size.Width = width
            position.X = (x+(i * 40))
            position.Y = int((y+(math.sin((i * math.pi) / 180)) * 5000))
            ellipse_shape.setSize(size)
            ellipse_shape.setPosition(position)
        except Exception as e:
            # Some exception occurs.FAILED
            print(f"Couldn't get Shape: {e}", file=sys.stderr)
            traceback.print_exc()
            return

        b += 8

        try:
            ellipse_shape.setPropertyValue('FillColor', get_col(r, g, b))
            ellipse_shape.setPropertyValue('Shadow', True)

        except Exception as e:
            print(f"Can't change colors: {e}", file=sys.stderr)
            traceback.print_exc()
            return

        page.add(ellipse_shape)


def get_col(r: int, g: int, b: int):
    return r * 65536 + g * 256 + b


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
