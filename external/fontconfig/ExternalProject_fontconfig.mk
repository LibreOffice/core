# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,fontconfig))

$(eval $(call gb_ExternalProject_use_externals,fontconfig,\
	expat \
	freetype \
))

$(eval $(call gb_ExternalProject_register_targets,fontconfig,\
	build \
))

$(call gb_ExternalProject_get_state_target,fontconfig,build) :
	$(call gb_Trace_StartRange,fontconfig,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		CFLAGS="$(if $(debug),-g) $(gb_VISIBILITY_FLAGS) $(if $(filter EMSCRIPTEN,$(OS)),-pthread)" $(if $(filter ANDROID,$(OS)),LIBS="-lm") \
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-shared \
			--disable-silent-rules \
			$(if $(filter ANDROID,$(OS)),--with-arch=arm) \
			--with-expat-includes=$(call gb_UnpackedTarball_get_dir,expat)/lib \
			--with-expat-lib=$(gb_StaticLibrary_WORKDIR) \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			$(if $(filter EMSCRIPTEN,$(OS)),ac_cv_func_fstatfs=no ac_cv_func_fstatvfs=no) \
		&& $(MAKE) -C src \
	)
	$(call gb_Trace_EndRange,fontconfig,EXTERNAL)

# vim: set noet sw=4 ts=4:
