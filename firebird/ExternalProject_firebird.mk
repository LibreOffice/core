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

$(call gb_ExternalProject_get_state_target,firebird,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS \
		&& export PKG_CONFIG="" \
		&& export CXXFLAGS=" \
			$(if $(filter NO,$(SYSTEM_BOOST)),-I$(call gb_UnpackedTarball_get_dir,boost),$(BOOST_CPPFLAGS)) \
			$(if $(filter NO,$(SYSTEM_ICU)), \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
				-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
				,$(ICU_CPPFLAGS)) \
				-L$(OUTDIR)/lib \
				-L$(call gb_UnpackedTarball_get_dir,boost)/source/lib" \
		&& export LD_LIBRARY_PATH="$(OUTDIR)/lib:$(call gb_UnpackedTarball_get_dir,boost)/source/lib" \
		&& export DYLD_LIBRARY_PATH="$(OUTDIR)/lib:$(call gb_UnpackedTarball_get_dir,boost)/source/lib" \
		$(if $(filter WNT,$(OS)), && export PATH="$(PATH):$(shell cygpath $(OUTDIR)/lib):$(shell cygpath $(call gb_UnpackedTarball_get_dir,icu)/source/lib)") \
		&& ./configure \
			--without-editline \
			--disable-superserver \
			--with-system-icu --without-fbsample --without-fbsample-db \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
		&& export CHANGE_INSTALL_NAMES="$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl fb OOO" \
		&& $(MAKE) firebird_embedded \
	)
# vim: set noet sw=4 ts=4:
