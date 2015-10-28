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
# Duplicated values are renumbered but preserved, i.e. for ...START and ...END
# definitions, and commented with "XXX was duplicate".
# To insert and renumber use a higher value for the inserted definition than
# any other used, for example 9999.
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
            a[3] = map[a[3]];
        }
        else
        {
            dup[a[3]] = a[2];
            dupmsg = "";
            ++id;
            map[a[3]] = id;
            a[3] = id;
        }
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
