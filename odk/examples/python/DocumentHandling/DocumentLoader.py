# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import argparse
import uno
from com.sun.star.connection import NoConnectException
from os.path import isfile, abspath, basename
from sys import argv


def is_file(value):
    if not isfile(value):
        raise argparse.ArgumentTypeError("{} could not be opened".format(value))
    return value


PROG = "$OFFICE_PROGRAM_PATH/python {}".format(basename(argv[0]))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog=PROG)
    parser.add_argument("--writer", action="store_true", required=False, help="Open an empty Writer document")
    parser.add_argument("--calc", action="store_true", required=False, help="Open an empty Calc document")
    parser.add_argument("--draw", action="store_true", required=False, help="Open an empty Draw document")
    parser.add_argument("path",
                        type=is_file,
                        nargs="?",
                        help="Path to a document to load. If omitted, an empty document is opened accordingly.")
    args = parser.parse_args()

    # UNO component context for initializing the Python runtime
    localContext = uno.getComponentContext()

    # Create an instance of a service implementation
    resolver = localContext.ServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", localContext)

    try:
        context = resolver.resolve(
            "uno:socket,host=localhost,"
            "port=2083;urp;StarOffice.ComponentContext")
    except NoConnectException:
        raise Exception("Error: cannot establish a connection to LibreOffice.")

    desktop = context.ServiceManager.createInstanceWithContext(
        "com.sun.star.frame.Desktop", context)

    if args.path:
        url = uno.systemPathToFileUrl(abspath(args.path))
    elif args.writer:
        url = "private:factory/swriter"
    elif args.calc:
        url = "private:factory/scalc"
    elif args.draw:
        url = "private:factory/sdraw"
    else:
        url = "private:factory/swriter"

    # Load a LibreOffice document, and automatically display it on the screen
    xComp = desktop.loadComponentFromURL(url, "_blank", 0, tuple([]))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
