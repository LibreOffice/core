# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gc
import pyuno
import unittest

class LoTestResult(unittest.TextTestResult):
    def stopTestRun(self):
        # HACK calling gc.collect() to get rid of as many still existing UNO proxies to
        # SwXTextDocument and friends as possible; those C++ classes' dtors call
        # Application::GetSolarMutex via sw::UnoImplPtrDeleter, so the dtors must be called before
        # DeInitVCL in the call to pyuno.private_deinitTestEnvironment(); any remaining proxies
        # that are still referenced (UnoInProcess' self.xDoc in
        # unotest/source/python/org/libreoffice/unotest.py, or per-class variables in the various
        # PythonTests) need to be individually released (each marked as "HACK" in the code):
        gc.collect()
        pyuno.private_deinitTestEnvironment()

if __name__ == '__main__':
    unittest.main(module=None, testRunner=unittest.TextTestRunner(resultclass=LoTestResult))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
