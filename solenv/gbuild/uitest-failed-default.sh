#!/bin/sh
# -*- Mode: sh; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
cat << EOF

Error: a unit test failed:

To rerun just this failed test without all others, use:
    make UITest_$2

Or to run just a specific test case method, use:
    make UITest_$2 UITEST_TEST_NAME="Module.ClassName.methodName"
where
    Module - the name of the python file (without the .py extension)
    Class - is the name in the "class Class" declaration

Or to do interactive debugging, run two shells with:
    SAL_USE_VCLPLUGIN=gtk make debugrun
    make gb_UITest_DEBUGRUN=T UITest_$2

The SAL_USE_VCLPLUGIN part is unnecessary if you are running on a kde desktop.
(The default gtk3 backend has issues with some uitests).

Failing that, put a
    time.sleep(60)
in the beginning of the method in the .py file,
and attach gdb to the running soffice process.

EOF

# In theory, we could add this text, but it really doesn't work very well.
#
# Or to do interactive debugging, run two shells with:
#    make debugrun
#    make gb_UITest_DEBUGRUN=T UITest_$2

exit 1

# vim: set et sw=4:
