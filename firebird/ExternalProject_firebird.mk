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

# Dependency in 3.0
# $(eval $(call gb_ExternalProject_use_packages,firebird, \
#     tommath \
# ))

$(eval $(call gb_ExternalProject_register_targets,firebird,\
	build \
))

# note: this can intentionally only build against internal atomic_op
# note: this can intentionally only build against internal tommath

$(call gb_ExternalProject_get_state_target,firebird,build):
	$(call gb_ExternalProject_run,build,\
		unset MAKEFLAGS && \
		export CFLAGS="$(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden)" \
		&& LDFLAGS="-L$(OUTDIR)/lib \
			$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib") \
			$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		&& CPPFLAGS="-I$(OUTDIR)/inc/external $(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		&& export PKG_CONFIG="" \
		&& ./configure \
			--without-editline \
			--disable-superserver \
			$(if $(filter NO,$(SYSTEM_BOOST)),CXXFLAGS=-I$(call gb_UnpackedTarball_get_dir,boost),CXXFLAGS=$(BOOST_CPPFLAGS)) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
		&& $(MAKE) firebird_embedded \
	)

# vim: set noet sw=4 ts=4:
