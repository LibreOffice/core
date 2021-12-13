# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,desktop_detector))

$(eval $(call gb_Library_set_plugin_for,desktop_detector,vcl))

$(eval $(call gb_Library_set_include,desktop_detector,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,desktop_detector,\
    -DDESKTOP_DETECTOR_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,desktop_detector))

$(eval $(call gb_Library_use_libraries,desktop_detector,\
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

$(eval $(call gb_Library_use_externals,desktop_detector,\
	boost_headers \
	icuuc \
))

$(eval $(call gb_Library_add_libs,desktop_detector,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_add_exception_objects,desktop_detector,\
    vcl/unx/generic/desktopdetect/desktopdetector \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Library_add_libs,desktop_detector,\
	-lm $(DLOPEN_LIBS) \
))
endif

# vim: set noet sw=4 ts=4:
