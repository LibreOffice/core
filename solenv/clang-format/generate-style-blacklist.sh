#!/bin/bash
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Generates a blacklist containing all existing cxx/hxx files.

git ls-files |egrep '\.(c|cpp|cxx|h|hxx|inl)$' > solenv/clang-format/blacklist

# vi:set shiftwidth=4 expandtab:
