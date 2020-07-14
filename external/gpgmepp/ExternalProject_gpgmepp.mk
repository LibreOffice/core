# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,gpgmepp))

$(eval $(call gb_ExternalProject_register_targets,gpgmepp,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,gpgmepp,build))

$(eval $(call gb_ExternalProject_use_externals,gpgmepp,\
       libgpg-error \
       libassuan \
))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,gpgmepp,build): $(call gb_Executable_get_target_for_build,cpp)
	$(call gb_Trace_StartRange,gpgmepp,EXTERNAL)
	$(call gb_ExternalProject_run,build, \
		$(gb_WIN_GPG_cross_setup_exports) \
		&& autoreconf \
		&& ./configure \
		   --disable-shared \
		   --disable-languages \
		   --disable-gpgconf-test \
		   --disable-gpg-test \
		   --disable-gpgsm-test \
		   --disable-g13-test \
		   CFLAGS='$(CFLAGS) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(call gb_Module__symbols_enabled,gpgmepp),$(gb_DEBUGINFO_FLAGS))' \
		   $(gb_WIN_GPG_platform_switches) \
		   MAKE=$(MAKE) \
	    && $(MAKE) \
	)
	$(call gb_Trace_EndRange,gpgmepp,EXTERNAL)
else
$(call gb_ExternalProject_get_state_target,gpgmepp,build):
	$(call gb_Trace_StartRange,gpgmepp,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   --disable-gpgconf-test \
		   --disable-gpg-test \
		   --disable-gpgsm-test \
		   --disable-g13-test \
		   --enable-languages="cpp" \
		   GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		   GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
		   LIBASSUAN_CFLAGS="$(LIBASSUAN_CFLAGS)" \
		   LIBASSUAN_LIBS="$(LIBASSUAN_LIBS)" \
		   CFLAGS='$(CFLAGS) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(call gb_Module__symbols_enabled,gpgmepp),$(gb_DEBUGINFO_FLAGS))' \
		   CXXFLAGS='$(CXXFLAGS) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(ENABLE_DBGUTIL),-D_GLIBCXX_DEBUG) \
				$(if $(call gb_Module__symbols_enabled,gpgmepp),$(gb_DEBUGINFO_FLAGS))' \
		   $(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		   $(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		   $(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	           $(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
	  && $(MAKE) \
	  $(if $(filter MACOSX,$(OS)),\
		  && $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			  $(EXTERNAL_WORKDIR)/lang/cpp/src/.libs/libgpgmepp.6.dylib \
			  $(EXTERNAL_WORKDIR)/src/.libs/libgpgme.11.dylib \
		) \
	)
	$(call gb_Trace_EndRange,gpgmepp,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
