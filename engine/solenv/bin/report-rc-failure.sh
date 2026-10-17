#!/usr/bin/env bash
# -*- Mode: sh; tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Prints what the resource compiler could see, after it failed: its command line, its include
# search path, every config_version.h on that path, and whether an old workdir is still being
# deleted. Arguments: the build directory, then the compiler command line.

builddir=$1
shift
headers_found=0

report_dir()
{
    local header=$1/config_version.h
    if [ -e "$header" ]; then
        headers_found=$((headers_found + 1))
        echo "found: $(ls -l --full-time "$header")"
        grep -n 'LIBO_VERSION_MAJOR' "$header" || echo "  no LIBO_VERSION_MAJOR in it"
    elif [ ! -d "$1" ]; then
        echo "missing include directory: $1"
    fi
}

{
    echo "=== resource compiler failed at $(date '+%Y-%m-%d %H:%M:%S.%N')"
    echo "command: $*"
    for arg in "$@"; do
        case $arg in
            -I*) report_dir "${arg#-I}" ;;
        esac
    done
    # The compiler searches the directories in INCLUDE after the -I ones. When INCLUDE holds the -I
    # list itself, make passed its own variable of that name down, and those are reported above.
    echo "INCLUDE=$INCLUDE"
    case $INCLUDE in
        *-I*) ;;
        *)
            IFS=';' read -r -a include_dirs <<< "$INCLUDE"
            for dir in "${include_dirs[@]}"; do
                [ -n "$dir" ] && report_dir "$dir"
            done
            ;;
    esac
    [ $headers_found -eq 0 ] && echo "no config_version.h in any include directory"
    if [ -e "$builddir/workdir.deleting" ]; then
        echo "workdir.deleting still exists"
    else
        echo "workdir.deleting is gone"
    fi
    echo "==="
} >&2

# vim: set shiftwidth=4 softtabstop=4 expandtab:
