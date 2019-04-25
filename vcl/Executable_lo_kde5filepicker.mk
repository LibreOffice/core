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

$(eval $(call gb_Executable_Executable,lo_kde5filepicker))

# FIXME: how to find the moc files automatically?!
$(eval $(call gb_Executable_set_include,lo_kde5filepicker,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(WORKDIR)/CustomTarget/vcl/unx/gtk3_kde5 \
))

$(eval $(call gb_Executable_add_cxxflags,lo_kde5filepicker,\
    $$(INCLUDE) \
    $$(BOOST_CXXFLAGS) \
))

$(eval $(call gb_Executable_use_custom_headers,lo_kde5filepicker,\
	officecfg/registry \
))

$(eval $(call gb_Executable_use_sdk_api,lo_kde5filepicker))

$(eval $(call gb_Executable_add_libs,lo_kde5filepicker,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Executable_use_libraries,lo_kde5filepicker,\
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

$(eval $(call gb_Executable_use_externals,lo_kde5filepicker,\
	boost_headers \
	epoxy \
	kde5 \
	dbus \
))

$(eval $(call gb_Executable_add_defs,lo_kde5filepicker,\
    $(QT5_CFLAGS) \
    $(QT5_GLIB_CFLAGS) \
    $(KF5_CFLAGS) \
))
$(eval $(call gb_Executable_add_libs,lo_kde5filepicker,\
    $(QT5_LIBS) \
    $(QT5_GLIB_LIBS) \
    $(KF5_LIBS) \
    $(BOOST_PROCESS_LIB) \
    $(BOOST_FILESYSTEM_LIB) \
))

$(eval $(call gb_Executable_add_exception_objects,lo_kde5filepicker,\
	vcl/unx/gtk3_kde5/kde5_lo_filepicker_main \
	vcl/unx/gtk3_kde5/kde5_filepicker \
	vcl/unx/gtk3_kde5/kde5_filepicker_ipc \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,lo_kde5filepicker,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
