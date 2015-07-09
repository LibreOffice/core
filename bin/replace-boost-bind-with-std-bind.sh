#! /bin/bash -x
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This script renames the most annoying abbreviations in Writer (and partially
# in the shared code too).  Just run it in the source directory.

for I in "boost/bind.hpp#functional" "boost::bind#std::bind" "boost::ref#std::ref" "boost::cref#std::cref" "boost::mem_fn#std::mem_fn"
do
    S="${I%#*}"
    git grep -l "$S" | grep -v -e '\.mk' -e 'replace-boost-bind-with-std-bind.sh' | xargs sed -i "s#$I#g"
done

# vim: set et sw=4 ts=4 textwidth=0:
