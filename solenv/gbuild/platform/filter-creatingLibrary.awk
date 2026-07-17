#!/usr/bin/gawk -f
# -*- tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Filter out the "Creating library" message printed by link.exe,
# as there is no way to disable it.

BEGIN {
    creatinglibrary_prefix = ENVIRON["CREATINGLIBRARY_PREFIX"];
    generatingcode_message = ENVIRON["GENERATINGCODE_MESSAGE"];
    finishedgeneratingcode_message = ENVIRON["FINISHEDGENERATINGCODE_MESSAGE"];
    if (!creatinglibrary_prefix) {
        creatinglibrary_prefix = "\\.lib.*\\.exp"
    }
    if (!generatingcode_message) {
        generatingcode_message = "Generating code"
    }
    if (!finishedgeneratingcode_message) {
        finishedgeneratingcode_message = "Finished generating code"
    }
    firstline = 1
}

{
    if (firstline && match($0, creatinglibrary_prefix)) {
        # ignore
    } else if (match($0, generatingcode_message)) {
        # ignore
    } else if (match($0, finishedgeneratingcode_message)) {
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
