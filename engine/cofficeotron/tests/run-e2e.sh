#!/bin/sh
# cofficeotron - a C++ port of the Java(tm) Office-o-tron
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Runs cofficeotron over every document in etc/test-data and compares the
# output with the expected copies in tests/expected. With --regen the
# expected copies are rewritten instead. The COFFICEOTRON environment
# variable names the validator binary to run, and COFFICEOTRON_SCHEMA_DIR
# the assembled schema directory (the validator reads the variable
# itself).
#
# The expected outputs embed RNV's validation message wording, so an RNV
# or schema update can shift them; regenerate and review the diff then.

cd "$(dirname "$0")/.." || exit 1

if test -z "$COFFICEOTRON"; then
    echo "set COFFICEOTRON to the cofficeotron binary to run" >&2
    exit 2
fi

regen=false
[ "$1" = "--regen" ] && regen=true

mkdir -p tests/expected
status=0

for f in etc/test-data/*; do
    name=$(basename "$f")
    expected="tests/expected/$name.txt"
    if $regen; then
        "$COFFICEOTRON" "$f" > "$expected"
        echo "regenerated $expected"
    else
        if "$COFFICEOTRON" "$f" | diff -u "$expected" - ; then
            echo "ok $name"
        else
            echo "FAIL $name"
            status=1
        fi
    fi
done

exit $status
