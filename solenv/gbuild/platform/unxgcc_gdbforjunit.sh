#!/bin/sh
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 Canonical,
# Ltd. . All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

# we expect our work directory to be where soffice had been started
# $(1) should be the path to the executable

OFFICEFILE=${1}
WORKDIR=${2}

if test -e ${WORKDIR}/core
then
    STORELOCATION=`mktemp --tmpdir=${WORKDIR} core.XXXX`
    echo "Found a core dump at ${WORKDIR}, moving it to ${STORELOCATION}"
    mv ${WORKDIR}/core ${STORELOCATION}
    echo "Stacktrace:"
    GDBCOMMANDFILE=`mktemp`
    echo "bt" > ${GDBCOMMANDFILE}
    gdb -x $GDBCOMMANDFILE --batch ${OFFICEFILE}.bin ${STORELOCATION}
    rm ${GDBCOMMANDFILE}
    exit 1
else
    exit 0
fi

# vim: set et sw=4 sts=4:
