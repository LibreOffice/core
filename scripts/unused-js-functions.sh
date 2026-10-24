#!/usr/bin/env bash
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

find browser/src/ -name "*.js" -exec grep ': function' \{\} \; | sed -e 's/:.*//' -e 's/^[\t ]*//' -e 's/[\t ]*$//' | grep -v " " | sort | uniq | \
    while read FUNC ; do
        NUM=`git grep "\<$FUNC\>" browser/src/ | wc -l`
        #echo "Trying $FUNC: $NUM"
        if [ "$NUM" = "1" ] ; then
            git --no-pager grep "\<$FUNC\>" browser/src/
        fi
    done
