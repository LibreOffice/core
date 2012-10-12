# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,expat))

ifeq ($(SYSTEM_EXPAT),NO)

$(eval $(call gb_Module_add_targets,expat,\
	ExternalPackage_expat \
	UnpackedTarball_expat \
	StaticLibrary_ascii_expat_xmlparse \
	StaticLibrary_expat_xmlparse \
	StaticLibrary_expat_xmltok \
))

# Actually it wasn't that clear if we should
# run configure step on mingw or not:
# CONFIGURATION_ACTION wasn't set, but
# CROSS_COMPILE case was handled in original makefile

# tested it on mingw and it doesn't hurt ;-)
#ifneq ($(OS),WNT)
ifneq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Module_add_targets,expat,\
	ExternalProject_expat \
))
endif

# ---------------- X64 stuff special ---------------------
#  use UNICODE only because shell/shlxthandler
#  doesn't link against ascii_expat_xmlparse
#---------------------------------------------------------
ifeq ($(BUILD_X64),TRUE)
$(eval $(call gb_Module_add_targets,expat,\
	StaticLibrary_expat_xmlparse_x64 \
	StaticLibrary_expat_xmltok_x64 \
))
endif

endif

# vim: set noet sw=4 ts=4:
