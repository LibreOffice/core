# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import uno
from com.sun.star.connection import NoConnectException
from os.path import isfile, abspath
from sys import argv, exit

if __name__ == "__main__":
    if len(argv) < 2:
        print("usage: $OFFICE_PROGRAM_PATH/python DocumentLoader.py <path>")
        exit(1)
    if not isfile(argv[1]):
        print("%s could not be opened" % argv[1])
        exit(1)

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
    url = uno.systemPathToFileUrl(abspath(argv[1]))

    # Load a LibreOffice document, and automatically display it on the screen
    xComp = desktop.loadComponentFromURL(url, "_blank", 0, tuple([]))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
