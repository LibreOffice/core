# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import officehelper

try:
    xContext = officehelper.bootstrap()
    print("Connected to a running office ...")
    xMCF = xContext.getServiceManager()
    available = "not available" if xMCF is None else "available"
    print("remote ServiceManager is " + available)

except Exception as e:
    print(e)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

