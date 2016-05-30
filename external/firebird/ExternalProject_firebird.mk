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

MAKE_PRE=$(if $(filter WNT,$(OS)),\
			   PATH="$(shell cygpath -u $(call gb_UnpackedTarball_get_dir,icu)/source/lib):$$PATH",\
			   $(gb_Helper_set_ld_path))

MAKE_POST=$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_firebird)/gen/firebird/lib/libfbembed.dylib.2.5.5)

# do not set LDFLAGS - it is mysteriously not used by firebird on MacOSX
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
			$(if $(filter WNT,$(OS)), \
				$(if $(SYSTEM_ICU),$(ICU_CPPFLAGS), \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
					-L$(call gb_UnpackedTarball_get_dir,icu)/source/lib \
				)) \
			" \
		&& export CXXFLAGS=" \
			$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS), \
				$(BOOST_CPPFLAGS) \
				-L$(call gb_UnpackedTarball_get_dir,boost)/source/lib \
			) \
			$(if $(filter-out WNT,$(OS)), \
				$(if $(SYSTEM_ICU),$(ICU_CPPFLAGS), \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
					-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
					-L$(call gb_UnpackedTarball_get_dir,icu)/source/lib \
				)) \
			$(if $(SYSTEM_LIBTOMMATH),$(LIBTOMMATH_CFLAGS), \
				-L$(call gb_UnpackedTarball_get_dir,libtommath) \
			) \
		" \
		&& MAKE=$(MAKE) ./configure \
			--without-editline \
			--without-fbsample --without-fbsample-db \
			--with-wire-compress=no \
			$(if $(filter-out MSC,$(COM)),$(if $(ENABLE_DEBUG),--enable-debug)) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
		&& if [ -n "$${FB_CPU_ARG}" ]; then \
			   $(MAKE_PRE) $(MAKE) $(INVOKE_FPA) SHELL='$(SHELL)' $(MAKE_POST); \
			else \
			   $(MAKE_PRE) $(MAKE) SHELL='$(SHELL)' $(MAKE_POST); \
			fi \
	)
# vim: set noet sw=4 ts=4:
