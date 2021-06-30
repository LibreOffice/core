#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
# Purpose: Getting a file name via arguments, and opening it in LibreOffice
#

import uno
from com.sun.star.connection import NoConnectException
from os.path import isfile, abspath
from sys import argv, exit

if __name__ == "__main__":
    if len(argv) < 2:
        print("usage: /opt/libreoffice7.1/program/python DocumentLoader.py <path>")
        exit(1)
    if not isfile(argv[1]):
        print("%s could not be opened" % argv[1])
        exit(1)

    # UNO component context for initializing the Python runtime
    localContext = uno.getComponentContext()

    # Create an instance of a service implementation
    resolver = localContext.ServiceManager.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", localContext)

    try:
        context = resolver.resolve("uno:socket,host=localhost,port=2083;urp;StarOffice.ComponentContext")
    except NoConnectException:
        raise Exception("Error: cannot establish a connection to LibreOffice.")

    desktop = context.ServiceManager.createInstanceWithContext("com.sun.star.frame.Desktop", context)
    url = uno.systemPathToFileUrl(abspath(argv[1]))

    # Load a LibreOffice document, and automatically display it on the screen
    xComp = desktop.loadComponentFromURL(url, "_blank", 0, tuple([]))

