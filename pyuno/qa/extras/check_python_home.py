# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from com.sun.star.task import XJob
from com.sun.star.uri import UriReferenceFactory
from com.sun.star.util import theMacroExpander
import os.path
import sys
import uno
import unohelper


g_ImplementationHelper = unohelper.ImplementationHelper()
g_implName = "org.libreoffice.pyuno.check_python_home"


class CheckPythonHome(unohelper.Base, XJob):
    def __init__(self, ctx):
        self.ctx = ctx

    def execute(self, args):
        # Make sure that the Python prefix is a subdirectory of $BRAND_BASE_DIR

        xServiceManager = self.ctx.ServiceManager
        xUriFactory = UriReferenceFactory.create(self.ctx)
        xUriReference = xUriFactory.parse("vnd.sun.star.expand:$BRAND_BASE_DIR")
        uri = xUriReference.expand(theMacroExpander.get(self.ctx))
        brand_path = os.path.realpath(uno.fileUrlToSystemPath(uri))
        prefix_path = os.path.realpath(sys.prefix)

        assert prefix_path.startswith(brand_path)


g_ImplementationHelper.addImplementation(
    CheckPythonHome,
    g_implName,
    (
    ),
)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
