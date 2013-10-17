# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,hyphen))

ifneq ($(or $(filter NO,$(SYSTEM_HYPH)),$(filter TRUE,$(WITH_MYSPELL_DICTS))),)
$(eval $(call gb_Module_add_targets,hyphen,\
	UnpackedTarball_hyphen \
	ExternalPackage_hyphen \
))
ifeq ($(COM),MSC)

ifeq ($(SYSTEM_HYPH),NO)
$(eval $(call gb_Module_add_targets,hyphen,\
	StaticLibrary_hyphen \
))
endif

else
$(eval $(call gb_Module_add_targets,hyphen,\
	ExternalProject_hyphen \
))
endif
endif

# vim: set noet sw=4 ts=4:
