# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# dummy UNO component to get the path of the localizations
import uno, unohelper, os, sys
from com.sun.star.lang import XMain
class LibreLogoDummyComponent(XMain, unohelper.Base):
    def __init__( self, ctx ):
        self.ctx = ctx

    def get_path(self):
        import librelogodummy_path
        return librelogodummy_path.get_path()

g_ImplementationHelper = unohelper.ImplementationHelper()
g_ImplementationHelper.addImplementation( \
    LibreLogoDummyComponent,"org.openoffice.comp.pyuno.LibreLogoDummy",("org.openoffice.LibreLogo.LibreLogoDummy",),)

# vim: set noet sw=4 ts=4:
