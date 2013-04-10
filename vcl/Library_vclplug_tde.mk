# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Timothy Pearson <kb9vqf@pearsoncomputing.net> (C) 2012, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
#*************************************************************************

$(eval $(call gb_Library_Library,vclplug_tde))

$(eval $(call gb_Library_use_packages,vclplug_tde,\
	shell_inc \
))

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
    $(if $(filter TRUE,$(SOLAR_JAVA)), \
        jvmaccess) \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,vclplug_tde,\
	boost_headers \
	icule \
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
