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
from os.path import basename, abspath

import uno
import unohelper
from com.sun.star.beans import PropertyValue
from com.sun.star.connection import NoConnectException

"""
The purpose of this example is to open a specified text document and save this
file to a specified URL. The type of the saved file is "writer8".
"""

PROG = "$OFFICE_PROGRAM_PATH/python {}".format(basename(sys.argv[0]))
SOFFICE_CONNECTION_URI = "uno:socket,host=localhost,port=2083;urp;StarOffice.ComponentContext"


def connect_soffice():
    """Connect to remote running LibreOffice

    :return: an object representing the remote LibreOffice instance.
    """
    local_context = uno.getComponentContext()
    resolver = local_context.ServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", local_context
    )
    try:
        remote_context = resolver.resolve(SOFFICE_CONNECTION_URI)
    except NoConnectException:
        raise Exception("Cannot establish a connection to LibreOffice.")

    return remote_context.ServiceManager.createInstanceWithContext(
        "com.sun.star.frame.Desktop", remote_context
    )


def save_doc(src, dest):
    src_url = "file://{}".format(abspath(src)).replace("\\", "/")
    dest_url = "file://{}".format(abspath(dest)).replace("\\", "/")

    soffice = connect_soffice()
    doc = soffice.loadComponentFromURL(
        src_url, "_blank", 0, (PropertyValue(Name="Hidden", Value=True),)
    )

    save_opts = (
        PropertyValue(Name="Overwrite", Value=True),
        PropertyValue(Name="FilterName", Value="writer8"),
    )
    try:
        doc.storeAsURL(dest_url, save_opts)
        print("Document", src, "saved under", dest)
    finally:
        doc.dispose()
        print("Document closed!")


def main():
    parser = argparse.ArgumentParser(description="Document Saver", prog=PROG)
    parser.add_argument("src", help="Path to a Word document to be saved, e.g. path/to/hello.doc")
    parser.add_argument("dest", help="Save the document to here, e.g. path/to/hello.odt")

    args = parser.parse_args()
    save_doc(args.src, args.dest)


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
