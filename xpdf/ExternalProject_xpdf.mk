# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xpdf))

$(eval $(call gb_ExternalProject_use_unpacked,xpdf,xpdf))

$(eval $(call gb_ExternalProject_register_targets,xpdf,\
	build \
))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,xpdf,build):
	$(call gb_ExternalProject_run,build,\
		./configure --without-x --enable-multithreaded --enable-exceptions \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			LDFLAGS="-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2" \
			LIBS="-lgdi32" \
		&& MAKEFLAGS="$(subst r,,$(MAKEFLAGS))" $(MAKE) \
	)
else # COM=MSC
$(call gb_ExternalProject_get_state_target,xpdf,build):
	$(call gb_ExternalProject_run,build,\
		LIB="$(ILIB)" cmd.exe /d /c ms_make.bat \
	)
endif
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,xpdf,build):
	$(call gb_ExternalProject_run,build,\
		./configure --without-x --without-libpaper-library --without-t1-library --enable-multithreaded --enable-exceptions \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(SYSBASE),CFLAGS="-I$(SYSBASE)/usr/include") \
			$(if $(PTHREAD_LIBS),$(if $(filter GCC,$(COM)),LDFLAGS=$(PTHREAD_LIBS))) \
			&& MAKEFLAGS="$(subst r,,$(MAKEFLAGS))" $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
