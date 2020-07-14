# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libassuan))

$(eval $(call gb_ExternalProject_register_targets,libassuan,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,libassuan,build))

$(eval $(call gb_ExternalProject_use_externals,libassuan,\
       libgpg-error \
))


ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,libassuan,build): $(call gb_Executable_get_target_for_build,cpp)
	$(call gb_Trace_StartRange,libassuan,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
	  $(gb_WIN_GPG_cross_setup_exports) \
	  && autoreconf \
	  && ./configure \
		--enable-static \
		--disable-shared \
		--disable-doc \
		$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
		CXXFLAGS="$(CXXFLAGS)" \
		GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
		$(gb_WIN_GPG_platform_switches) \
		MAKE=$(MAKE) \
	  && $(MAKE) \
	)
	$(call gb_Trace_EndRange,libassuan,EXTERNAL)
else
$(call gb_ExternalProject_get_state_target,libassuan,build):
	$(call gb_Trace_StartRange,libassuan,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   --disable-doc \
		   GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		   GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
		   $(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		   $(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		   $(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	           $(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
	  && $(MAKE) \
	  $(if $(filter MACOSX,$(OS)),\
		  && $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			  $(EXTERNAL_WORKDIR)/src/.libs/libassuan.0.dylib \
		) \
	)
	$(call gb_Trace_EndRange,libassuan,EXTERNAL)

endif
# vim: set noet sw=4 ts=4:
