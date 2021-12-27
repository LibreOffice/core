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
from tempfile import TemporaryDirectory
import os.path
import shutil
from contextlib import contextmanager
from uitest.uihelper.common import get_url_for_data_file

@contextmanager
def launchHTTPServer(file_name):

    with TemporaryDirectory() as tempdir:

        # Copy file to the temporary directory
        src = get_url_for_data_file(file_name).replace("file://", "")
        dst = os.path.join(tempdir, file_name)
        shutil.copy(src, dst)

        # Serve file from the temporary directory
        handler = functools.partial(SimpleHTTPRequestHandler, directory=tempdir)

        # Spawns an http.server.HTTPServer in a separate thread
        with HTTPServer(("localhost", 0), handler) as httpd:
            try:
                thread = threading.Thread(target=httpd.serve_forever)
                thread.start()
                address = "http://%s:%d/%s" % (httpd.server_name, httpd.server_port, file_name)
                yield address
            finally:
                # Call shutdown in case the testcase fails
                httpd.shutdown()
                thread.join()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
