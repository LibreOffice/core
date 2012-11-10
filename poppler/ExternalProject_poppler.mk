# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,poppler))

$(eval $(call gb_ExternalProject_use_unpacked,poppler,poppler))

$(eval $(call gb_ExternalProject_register_targets,poppler,\
	build \
))

$(call gb_ExternalProject_get_state_target,poppler,build):
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure --without-x --disable-shared --disable-introspection \
	--disable-libopenjpeg --disable-libtiff --disable-libjpeg --disable-libpng --disable-splash-output \
	--disable-cairo-output --disable-poppler-glib --disable-poppler-qt4 --disable-poppler-cpp --disable-gtk-test \
	--disable-utils --disable-cms \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
	$(if $(filter NO,$(SYSTEM_CAIRO)),CAIRO_CFLAGS="-I$(OUTDIR)/inc/cairo")\
	&& $(MAKE) \
	&& touch $@

# vim: set noet sw=4 ts=4:
