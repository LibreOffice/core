# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This is a dummy library with one C source that is compiled to make
# sure that the <tools/UnixWrappers.h> stays compilable as plain C. If
# you can think of a better way to ensure that (before we actually use
# UnixWrappers.h in some (external) C file that is compiled in normal
# Windows builds), go ahead.

$(eval $(call gb_StaticLibrary_StaticLibrary,UseUnixWrappers))

$(eval $(call gb_StaticLibrary_add_cobjects,UseUnixWrappers,\
    tools/source/misc/UseUnixWrappers \
))

# vim: set noet sw=4 ts=4:
