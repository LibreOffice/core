#!/bin/sh
# -*- Mode: sh; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .

DEBUGGING=

if [ "$3" = "WNT" ]; then
    DEBUGGING='
export CPPUNITTRACE="'$DEVENV' /debugexe" # for interactive debugging in Visual Studio
export CPPUNITTRACE="drmemory -free_max_frames 20"            # for memory checking (install Dr.Memory first, and put it to your PATH)
'
elif [ "$3" = "MACOSX" ]; then
    DEBUGGING='
export CPPUNITTRACE="lldb --"       # for interactive debugging on OS X
export VALGRIND=memcheck            # for memory checking
'
else
    DEBUGGING='
export DEBUGCPPUNIT=TRUE            # for exception catching
export CPPUNITTRACE="gdb --args"    # for interactive debugging on Linux
export VALGRIND=memcheck            # for memory checking
'
fi

# print the actual error message
cat << EOF

Error: a unit test failed, please do one of:
$DEBUGGING
and retry using: make $1Test_$2

EOF

exit 1

# vim: set et sw=4:
