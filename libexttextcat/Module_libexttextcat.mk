# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,libexttextcat))

ifeq ($(SYSTEM_LIBEXTTEXTCAT),NO)
$(eval $(call gb_Module_add_targets,libexttextcat,\
	ExternalPackage_fingerprint \
	UnpackedTarball_exttextcat \
))
ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,libexttextcat,\
	StaticLibrary_exttextcat \
))
else
$(eval $(call gb_Module_add_targets,libexttextcat,\
	ExternalProject_exttextcat \
))
endif
endif

# vim: set noet sw=4 ts=4:
