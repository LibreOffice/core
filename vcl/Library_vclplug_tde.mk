# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2012, Timothy Pearson <kb9vqf@pearsoncomputing.net>
#

$(eval $(call gb_Library_Library,vclplug_tde))

$(eval $(call gb_Library_set_include,vclplug_tde,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/solenv/inc \
    -I$(SRCDIR)/vcl/inc/unx/tde \
))

$(eval $(call gb_Library_set_include,vclplug_tde,\
	$$(INCLUDE) \
    $$(TDE_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_tde,\
    -DVCLPLUG_TDE_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_tde))

$(eval $(call gb_Library_add_libs,vclplug_tde,\
	$(TDE_LIBS) \
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_use_libraries,vclplug_tde,\
    vclplug_gen \
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(ENABLE_JAVA), \
        jvmaccess) \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,vclplug_tde,\
	boost_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_tde,\
    vcl/unx/kde/kdedata \
    vcl/unx/kde/salnativewidgets-kde \
	vcl/unx/kde/UnxCommandThread \
	vcl/unx/kde/UnxFilePicker \
	vcl/unx/kde/UnxNotifyThread \
))

# TDE/TQt consider -Wshadow more trouble than benefit
$(eval $(call gb_Library_add_cxxflags,vclplug_tde,\
        -Wno-shadow \
))

$(eval $(call gb_Library_set_warnings_not_errors,vclplug_tde))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_tde,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
