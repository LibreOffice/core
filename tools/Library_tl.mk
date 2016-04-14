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

$(eval $(call gb_Library_Library,tl))

$(eval $(call gb_Library_set_include,tl,\
    -I$(SRCDIR)/tools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,tl,$(SRCDIR)/tools/inc/pch/precompiled_tl))

$(eval $(call gb_Library_add_defs,tl,\
    -DTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,tl))

$(eval $(call gb_Library_use_libraries,tl,\
    basegfx \
    comphelper \
    i18nlangtag \
    cppu \
    sal \
    sax \
	$(gb_UWINAPI) \
))


$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/source/datetime/datetime \
    tools/source/datetime/datetimeutils \
    tools/source/datetime/tdate \
    tools/source/datetime/ttime \
    tools/source/debug/debug \
    tools/source/fsys/urlobj \
    tools/source/fsys/wldcrd \
    tools/source/generic/b3dtrans \
    tools/source/generic/bigint \
    tools/source/generic/color \
    tools/source/generic/config \
    tools/source/generic/fract \
    tools/source/generic/gen \
    tools/source/generic/line \
    tools/source/generic/point \
    tools/source/generic/poly \
    tools/source/generic/poly2 \
    tools/source/generic/svborder \
    tools/source/inet/inetmime \
    tools/source/inet/inetmsg \
    tools/source/inet/inetstrm \
    tools/source/memtools/mempool \
    tools/source/memtools/multisel \
    tools/source/memtools/unqidx \
    tools/source/misc/cpuid \
    tools/source/misc/extendapplicationenvironment \
    tools/source/misc/getprocessworkingdir \
    tools/source/misc/toolsdll \
    tools/source/rc/rc \
    tools/source/rc/resary \
    tools/source/rc/resmgr \
    tools/source/ref/errinf \
    tools/source/ref/globname \
    tools/source/ref/pstm \
    tools/source/ref/ref \
    tools/source/stream/stream \
    tools/source/stream/strmsys \
    tools/source/stream/vcompat \
    tools/source/string/tenccvt \
    tools/source/zcodec/zcodec \
))

$(eval $(call gb_Library_add_generated_exception_objects,tl,\
    CustomTarget/tools/string/reversemap \
))

$(eval $(call gb_Library_use_externals,tl,\
	boost_headers \
	zlib \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,tl,\
        -lrt \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,tl,\
	mpr \
	ole32 \
	shell32 \
	uuid \
))

endif

# vim: set noet sw=4 ts=4:
