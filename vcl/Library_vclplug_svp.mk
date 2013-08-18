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

$(eval $(call gb_Library_Library,vclplug_svp))

$(eval $(call gb_Library_set_include,vclplug_svp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,vclplug_svp,\
    -DVCLPLUG_SVP_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_svp))

$(eval $(call gb_Library_use_libraries,vclplug_svp,\
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    basebmp \
    comphelper \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(ENABLE_JAVA), \
        jvmaccess) \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,vclplug_svp,\
	boost_headers \
	freetype_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_svp,\
    vcl/headless/svpgdi \
    vcl/headless/svpinst \
    vcl/headless/svpdata \
))

$(eval $(call gb_Library_use_static_libraries,vclplug_svp,\
    headless \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_svp,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
