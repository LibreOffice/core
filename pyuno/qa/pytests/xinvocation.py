# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import org.libreoffice.unotest
from org.libreoffice.embindtest import theMethodToString


class XInvocationTest(unittest.TestCase):
    # Tests that pyuno can handle an object that implements XInvocation. See tdf#173114
    def test_xinvocation(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()

        xMethodToString = theMethodToString.get(ctx)

        # The MethodToString service implements all methods via XInvocation and just returns the
        # name of the method as a string.
        self.assertEqual(xMethodToString.myMadeUpMethod(), "myMadeUpMethod")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
