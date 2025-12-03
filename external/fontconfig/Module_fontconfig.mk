# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,fontconfig))

$(eval $(call gb_Module_add_targets,fontconfig,\
	UnpackedTarball_fontconfig \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Module_add_targets,fontconfig,\
	StaticLibrary_fontconfig \
))

else

$(eval $(call gb_Module_add_targets,fontconfig,\
	ExternalProject_fontconfig \
	$(if $(filter EMSCRIPTEN,$(OS)),ExternalPackage_fontconfig_data) \
	$(if $(filter MACOSX,$(OS)),ExternalPackage_fontconfig_data) \
	$(if $(filter FONTCONFIG,$(BUILD_TYPE)),ExternalPackage_fontconfig) \
))

endif

# vim: set noet sw=4 ts=4:
