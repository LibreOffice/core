# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys

from gen_cert_header import create_header

with open(sys.argv[1], mode='w') as f:
    # For debug purposes, ONLINEUPDATE_MAR_CERTIFICATEDER (passed in as sys.argv[2]) can be empty,
    # but create_header always expects an existing file argument, so as a hack fall back to this
    # script itself (i.e., sys.argv[0]) as the pathname of the certificate DER file (though that
    # will cause create_header to generate nonsense data, of course):
    create_header(f, sys.argv[2] or sys.argv[0])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
