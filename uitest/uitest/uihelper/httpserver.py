# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import http.server
import threading
import functools
import org.libreoffice.unotest
from contextlib import contextmanager

@contextmanager
def launchHTTPServer():
    #Spawns an http.server.HTTPServer in a separate thread
    try:
        # serve in TDOC directory
        handler = functools.partial(
                http.server.SimpleHTTPRequestHandler,
                directory=org.libreoffice.unotest.getTDOC())

        httpd = http.server.HTTPServer(("localhost", 0), handler, False)

        httpd.server_bind()

        httpd.server_activate()

        def serve(httpd):
            with httpd:  # to make sure httpd.server_close is called
                httpd.serve_forever()

        thread = threading.Thread(target=serve, args=(httpd, ))
        thread.setDaemon(True)
        thread.start()

        yield httpd
    finally:
        # Call shutdown in case the testcase fails
        httpd.shutdown()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
