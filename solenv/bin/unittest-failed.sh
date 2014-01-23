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

printf '\nError: a unit test failed, please do one of:\n\nexport DEBUGCPPUNIT=TRUE            # for exception catching\nexport CPPUNITTRACE="gdb --args" # for interactive debugging on linux\nexport CPPUNITTRACE="\"[full path to devenv.exe]\" /debugexe" # for interactive debugging in Visual Studio\nexport VALGRIND=memcheck            # for memory checking\n\nand retry using: make %sTest_%s\n\n' $1 $2
exit 1

# vim: set et sw=4:
