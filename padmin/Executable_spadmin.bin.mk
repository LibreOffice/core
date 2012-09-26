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

$(eval $(call gb_Executable_Executable,spadmin.bin))

$(eval $(call gb_Executable_set_include,spadmin.bin,\
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,spadmin.bin,\
    vclmain \
))

$(eval $(call gb_Executable_use_sdk_api,spadmin.bin))

$(eval $(call gb_Executable_use_libraries,spadmin.bin,\
    spa \
    svt \
    vcl \
    utl \
    ucbhelper \
    comphelper \
    tl \
    cppuhelper \
    cppu \
    sal \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,spadmin.bin,\
    padmin/source/desktopcontext \
    padmin/source/pamain \
))

ifneq (,$(filter LINUX DRAGONFLY OPENBSD FREEBSD NETBSD, $(OS)))
$(eval $(call gb_Executable_use_libraries,spadmin.bin,\
    pthread \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_use_libraries,spadmin.bin,\
    dl \
))
endif

# vim: set noet sw=4 ts=4:
