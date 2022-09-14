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
from os.path import abspath, basename

import uno
import unohelper
from com.sun.star.beans import PropertyValue
from com.sun.star.connection import NoConnectException

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


def print_(doc_path, printer, pages):
    soffice = connect_soffice()
    doc_url = "file://{}".format(abspath(doc_path)).replace("\\", "/")
    # Load a Writer document, which will be automatically displayed
    doc = soffice.loadComponentFromURL(doc_url, "_blank", 0, tuple([]))
    try:
        doc.setPrinter((PropertyValue(Name="Name", Value=printer),))
        print_opts = (
            PropertyValue(Name="Pages", Value=pages),
        )
        doc.print(print_opts)
    finally:
        doc.dispose()


def main():
    parser = argparse.ArgumentParser(description="Document Printer", prog=PROG)
    parser.add_argument("printer", help="Printer name")
    parser.add_argument("doc_path", help="Path to a document to be printed")
    parser.add_argument("pages", help="Page range to be printed, e.g. 1-3")

    args = parser.parse_args()
    print_(args.doc_path, args.printer, args.pages)


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
