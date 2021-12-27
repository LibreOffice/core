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
import org.libreoffice.unotest
from contextlib import contextmanager

class SignalingHTTPServer(HTTPServer):

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.ready_event = threading.Event()

    def service_actions(self):
        self.ready_event.set()

@contextmanager
def launchHTTPServer():
    # Spawns an http.server.HTTPServer in a separate thread
    # Serve in TDOC directory
    handler = functools.partial(SimpleHTTPRequestHandler,
            directory=org.libreoffice.unotest.getTDOC())

    with SignalingHTTPServer(("localhost", 0), handler) as httpd:
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()

        # Wait until the event is ready
        httpd.ready_event.wait()
        try:
            yield httpd
        finally:
            # Call shutdown in case the testcase fails
            httpd.shutdown()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
