#!/bin/sh
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# invocation for coverity to collect c/c++ and javascript

set -e
cd "$(dirname "$0")/.."

# Deliberately a few modules from the bottom of the dependency graph, so the
# warnings each newly added module brings can be judged on their own.
ENGINE_MODULES="sal salhelper store registry unoidl xmlreader cppu cppuhelper"

online_include='browser/(src|admin|welcome)'
engine_include="engine/($(echo "$ENGINE_MODULES" | tr ' ' '|'))/"

# Both capture runs add to cov-int, so remove it once here.
rm -rf cov-int

# Capture watches for compiler invocations, so an already-built tree gives it
# nothing to see. Clean first. The rest of the engine stays built, so the make
# below only compiles these modules again.
make -C engine $(for m in $ENGINE_MODULES; do printf '%s.clean ' "$m"; done)

# A full make, not "make <module>". The per-module target builds only that one
# module and takes the libraries it links against from workdir, which the clean
# above just emptied.
coverity capture --dir cov-int --language c-family \
    --file-include-regex "$engine_include" \
    -- make -C engine

make clean

coverity capture --dir cov-int --language c-family --language javascript \
    --file-include-regex "$online_include" \
    -- make -j `nproc`
