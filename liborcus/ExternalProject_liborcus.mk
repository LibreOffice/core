# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,liborcus))

$(eval $(call gb_ExternalProject_use_unpacked,liborcus,orcus))

$(eval $(call gb_ExternalProject_register_targets,liborcus,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	cd $(EXTERNAL_WORKDIR)/vsprojects/liborcus-static-nozip \
	&& export BOOST_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
	&& $(COMPATH)/vcpackages/vcbuild.exe liborcus-static-nozip.vcproj "Release|Win32" \
	&& touch $@

else

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& $(if $(filter ANDROID,$(OS)),LIBS='-lgnustl_shared -lm') \
	./configure \
		--with-pic \
		--enable-static \
		--disable-shared \
		--without-libzip \
		--disable-debug \
		--disable-spreadsheet-model \
		CXXFLAGS="$(if $(filter NO,$(SYSTEM_BOOST)),-I$(OUTDIR)/inc/external) $(gb_CXXFLAGS)" \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	&& $(GNUMAKE) \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
