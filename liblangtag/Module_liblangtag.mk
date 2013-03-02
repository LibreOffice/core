# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,liblangtag))

ifeq ($(ENABLE_LIBLANGTAG),YES)
ifeq ($(SYSTEM_LIBLANGTAG),NO)

$(eval $(call gb_Module_add_targets,liblangtag,\
	UnpackedTarball_langtag \
	ExternalPackage_langtag_data \
	Zip_liblangtag_data \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,liblangtag,\
	StaticLibrary_langtag \
))
else
$(eval $(call gb_Module_add_targets,liblangtag,\
	ExternalPackage_langtag \
	ExternalProject_langtag \
))
endif
endif
endif

# vim: set noet sw=4 ts=4:
