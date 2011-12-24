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

if test -n "`which gdb`"
then
    if test `ls "${WORKDIR}"/core* 2>/dev/null | wc -l` -eq 1
    then
        COREFILE=`ls "${WORKDIR}"/core*`
        echo
        echo "It seems like soffice.bin crashed during the test excution!"
        echo "Found a core dump at ${COREFILE}"
        echo "Stacktrace:"
        GDBCOMMANDFILE=`mktemp`
        echo "thread apply all bt" > ${GDBCOMMANDFILE}
        gdb -x $GDBCOMMANDFILE --batch ${OFFICEFILE}.bin ${COREFILE}
        rm ${GDBCOMMANDFILE}
        echo
        exit 1
    else
        echo
        echo "No core dump at ${WORKDIR}, to create core dumps (and stack traces)"
        echo "for crashed soffice instances, enable core dumps with:"
        echo
        echo "   ulimit -c unlimited"
        echo
        exit 0
    fi
else
    echo "You need gdb in your path to generate stacktraces."
    exit 0
fi

# vim: set et sw=4 sts=4:
