# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,python3))

ifneq ($(DISABLE_PYTHON),TRUE)
ifeq ($(SYSTEM_PYTHON),)

$(eval $(call gb_Module_add_targets,python3,\
	UnpackedTarball_python3 \
	ExternalProject_python3 \
	$(if $(filter MACOSX,$(OS)),GeneratedPackage_python3,ExternalPackage_python3) \
))

endif
endif

# vim: set noet sw=4 ts=4:
