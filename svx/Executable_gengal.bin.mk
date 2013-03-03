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

ifeq ($(OS),WNT)
svx_GENGALBIN := gengal
else
svx_GENGALBIN := gengal.bin
endif


$(eval $(call gb_Executable_Executable,$(svx_GENGALBIN)))

$(eval $(call gb_Executable_set_include,$(svx_GENGALBIN),\
    $$(INCLUDE) \
    -I$(SRCDIR)/svx/inc/ \
    -I$(SRCDIR)/svx/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/udkapi \
    -I$(OUTDIR)/inc/offapi \
))

$(eval $(call gb_Executable_use_external,$(svx_GENGALBIN),boost_headers))

$(eval $(call gb_Executable_use_libraries,$(svx_GENGALBIN),\
    basegfx \
    sal \
    tl \
    svl \
    svt \
    comphelper \
    cppu \
    cppuhelper \
    utl \
    vcl \
    svxcore \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,$(svx_GENGALBIN),\
    svx/source/gengal/gengal \
))

$(eval $(call gb_Executable_use_static_libraries,$(svx_GENGALBIN),\
    vclmain \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_use_system_win32_libs,$(svx_GENGALBIN),\
	kernel32 \
	msvcrt \
	oldnames \
	user32 \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,$(svx_GENGALBIN),\
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
