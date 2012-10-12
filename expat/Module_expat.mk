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
	ExternalProject_expat \
	StaticLibrary_ascii_expat_xmlparse \
	StaticLibrary_expat_xmlparse \
	StaticLibrary_expat_xmltok \
))

endif

# vim: set noet sw=4 ts=4:
