# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,firebird))

$(eval $(call gb_ExternalProject_use_autoconf,firebird,build))

$(eval $(call gb_ExternalProject_use_externals,firebird,\
	boost_headers \
	icu \
	libatomic_ops \
	libtommath \
))

$(eval $(call gb_ExternalProject_register_targets,firebird,\
	build \
))

ifneq ($(OS),WNT)
INVOKE_FPA:="CPU=\$$(EMPTY) $${FB_CPU_ARG}"
endif

ifeq ($(COM_IS_CLANG),TRUE)
firebird_NO_CXX11_NARROWING := -Wno-c++11-narrowing
endif

MAKE_PRE=LC_ALL=C

MAKE_POST=$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(EXTERNAL_WORKDIR)/gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/plugins/libEngine12.dylib \
			$(EXTERNAL_WORKDIR)/gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/lib/libfbclient.dylib.3.0.0)

$(call gb_ExternalProject_get_state_target,firebird,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS \
		&& FB_CPU_ARG='$(filter --jobserver-fds=%,$(MAKEFLAGS))' \
		&& export PKG_CONFIG="" \
		&& export CPPFLAGS=" \
			$(if $(SYSTEM_LIBATOMIC_OPS),$(LIBATOMIC_OPS_CFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,libatomic_ops)/src \
			) \
			$(if $(SYSTEM_LIBTOMMATH),$(LIBTOMMATH_CFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,libtommath) \
				-L$(call gb_UnpackedTarball_get_dir,libtommath) \
			) \
			$(if $(SYSTEM_ICU),$(ICU_CPPFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
			) \
			$(if $(filter GCC-INTEL,$(COM)-$(CPUNAME)),-Di386=1) \
			" \
		&& export CXXFLAGS=" \
			$(if $(filter MSC,$(COM)),$(if $(MSVC_USE_DEBUG_RUNTIME),-DMSVC_USE_DEBUG_RUNTIME)) \
			$(if $(HAVE_GCC_FNO_SIZED_DEALLOCATION),-fno-sized-deallocation -fno-delete-null-pointer-checks) \
			$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS), \
				$(BOOST_CPPFLAGS) \
				-L$(call gb_UnpackedTarball_get_dir,boost)/source/lib \
			) \
			$(if $(SYSTEM_ICU),$(ICU_CPPFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
			) \
			$(if $(SYSTEM_LIBTOMMATH),$(LIBTOMMATH_CFLAGS), \
				-L$(call gb_UnpackedTarball_get_dir,libtommath) \
			) \
			$(CXXFLAGS_CXX11) \
			$(firebird_NO_CXX11_NARROWING) \
			$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS)) \
		" \
		&& export LDFLAGS=" \
			$(if $(SYSTEM_ICU),$(ICU_LIBS), \
				-L$(call gb_UnpackedTarball_get_dir,icu)/source/lib \
			) \
		" \
		&& export LIBREOFFICE_ICU_LIB="$(call gb_UnpackedTarball_get_dir,icu)/source/lib" \
		&& MAKE=$(MAKE) ./configure \
			--without-editline \
			--with-wire-compress=no \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			$(if $(filter MACOSX,$(OS)), \
				$(if $(filter 1, \
						$(shell expr '$(MAC_OS_X_VERSION_MIN_REQUIRED)' \
							'<' 101200)), \
					ac_cv_func_clock_gettime=no)) \
		&& if [ -n "$${FB_CPU_ARG}" ]; then \
				$(MAKE_PRE) $(MAKE) \
					$(if $(filter LINUX,$(OS)),CXXFLAGS="$$CXXFLAGS -std=gnu++11") \
					$(if $(ENABLE_DEBUG),Debug) $(INVOKE_FPA) SHELL='$(SHELL)' \
					LIBO_TUNNEL_LIBRARY_PATH='$(subst ','\'',$(subst $$,$$$$,$(call gb_Helper_extend_ld_path,$(call gb_UnpackedTarball_get_dir,icu)/source/lib)))' \
				$(MAKE_POST); \
			else \
				$(MAKE_PRE) $(MAKE) \
					$(if $(filter LINUX,$(OS)),CXXFLAGS="$$CXXFLAGS -std=gnu++11") \
					$(if $(ENABLE_DEBUG),Debug) SHELL='$(SHELL)' \
					LIBO_TUNNEL_LIBRARY_PATH='$(subst ','\'',$(subst $$,$$$$,$(call gb_Helper_extend_ld_path,$(call gb_UnpackedTarball_get_dir,icu)/source/lib)))' \
				$(MAKE_POST); \
			fi \
	)
# vim: set noet sw=4 ts=4:
