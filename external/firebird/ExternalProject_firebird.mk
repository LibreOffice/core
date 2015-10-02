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
))

$(eval $(call gb_ExternalProject_register_targets,firebird,\
	build \
))

# note: this can intentionally only build against internal atomic_op
# note: this can intentionally only build against internal tommath

# do not set LDFLAGS - it is mysteriously not used by firebird on MacOSX
$(call gb_ExternalProject_get_state_target,firebird,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS \
		&& export PKG_CONFIG="" \
		&& export CPPFLAGS=" \
			$(if $(SYSTEM_LIBATOMIC_OPS),$(LIBATOMIC_OPS_CFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,libatomic_ops)/src \
			) \
			" \
		&& export CXXFLAGS=" \
			$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,boost) \
				-L$(call gb_UnpackedTarball_get_dir,boost)/source/lib \
			) \
			$(if $(SYSTEM_ICU),$(ICU_CPPFLAGS), \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
				-L$(call gb_UnpackedTarball_get_dir,icu)/source/lib \
			) \
			" \
		&& MAKE=$(MAKE) ./configure \
			--without-editline \
			--disable-superserver \
			--with-system-icu --without-fbsample --without-fbsample-db \
			$(if $(filter-out MSC,$(COM)),$(if $(ENABLE_DEBUG),--enable-debug)) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
		&& $(if $(filter WNT,$(OS)),\
			   PATH="$(shell cygpath -u $(call gb_UnpackedTarball_get_dir,icu)/source/lib):$$PATH",\
			   $(gb_Helper_set_ld_path)) \
		   $(MAKE) SHELL=$(SHELL) firebird_embedded \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_firebird)/gen/firebird/lib/libfbembed.dylib.2.5.4) \
	)
# vim: set noet sw=4 ts=4:
