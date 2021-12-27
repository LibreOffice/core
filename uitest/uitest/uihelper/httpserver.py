# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from http.server import HTTPServer, SimpleHTTPRequestHandler
import threading
import functools
import os
from contextlib import contextmanager
import time

@contextmanager
def launchHTTPServer():
    # Spawns an http.server.HTTPServer in a separate thread
    # Serve in TDOC directory
    handler = functools.partial(SimpleHTTPRequestHandler,
            directory=os.getenv("TDOC"))

    with HTTPServer(("localhost", 0), handler) as httpd:
        try:
            time.sleep(2)
            thread = threading.Thread(target=httpd.serve_forever)
            time.sleep(2)
            thread.start()
            time.sleep(2)
            yield httpd
        finally:
            # Call shutdown in case the testcase fails
            time.sleep(2)
            httpd.shutdown()
            time.sleep(2)
            thread.join()
            time.sleep(2)

    time.sleep(2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
