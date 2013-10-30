#!/usr/bin/awk -f
#
# -*- Mode: awk; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Consecutively number a series of defines, for example sc/inc/globstr.hrc
# WARNING: this does not expect other defines in between and would mess around
# with them.

BEGIN {
    id = 0;
    lastline = ""
}

{
    if ($1 ~ /#define/ && lastline !~ /#ifndef/)
    {
        n = split( $0, a, / +/, s);
        if (dup[a[3]])
        {
            dupmsg = " // XXX was duplicate " a[3] " of " dup[a[3]];
        }
        else
        {
            dup[a[3]] = a[2];
            dupmsg = "";
        }
        a[3] = ++id;
        lastline = s[0];
        for (i=1; i<=n; ++i)
        {
            lastline = lastline a[i] s[i];
        }
        lastline = lastline dupmsg;
    }
    else
    {
        lastline = $0;
    }
    print lastline;
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
