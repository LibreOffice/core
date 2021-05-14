# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,lockfile))

$(eval $(call gb_Executable_set_warnings_not_errors,lockfile))

$(eval $(call gb_Executable_set_include,lockfile, \
    -I$(SRCDIR)/solenv/lockfile \
    -I$(BUILDDIR)/solenv/lockfile \
))

$(eval $(call gb_Executable_add_cobjects,lockfile, \
    solenv/lockfile/dotlockfile \
    solenv/lockfile/lockfile \
    , $(gb_COMPILEROPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
