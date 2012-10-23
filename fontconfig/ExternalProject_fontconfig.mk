# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,fontconfig))

$(eval $(call gb_ExternalProject_use_unpacked,fontconfig,fontconfig))

$(eval $(call gb_ExternalProject_use_packages,fontconfig,\
	freetype \
))

$(eval $(call gb_ExternalProject_register_targets,fontconfig,\
	build \
))

$(call gb_ExternalProject_get_state_target,fontconfig,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& $(if $(debug),CFLAGS=-g) ./configure \
		--disable-shared \
		--with-arch=arm \
		--with-expat-includes=$(OUTDIR)/inc/external \
		--with-expat-lib=$(OUTDIR)/lib \
		--with-freetype-config=$(OUTDIR)/bin/freetype-config \
		--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
	&& $(GNUMAKE) \
	&& touch $@

# vim: set noet sw=4 ts=4:
