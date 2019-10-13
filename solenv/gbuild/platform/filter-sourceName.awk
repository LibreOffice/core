#!/usr/bin/gawk -f
# -*- tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Filter out the "sourcefile.cxx" message printed by cl.exe,
# as there is no way to disable it. This file is used
# in place of filter-showIncludes.awk (which does the same)
# when --disable-dependency-tracking is used.

BEGIN {
    firstline = 1
}

{
    if (firstline) {
        # ignore
    } else {
        # because MSVC stupidly prints errors on stdout, it's
        # necessary to forward everything that isn't matched by the pattern
        # so users get to see them.
        print $0 > "/dev/stderr"
    }
    firstline = 0
}

# vim: set noet sw=4 ts=4:
