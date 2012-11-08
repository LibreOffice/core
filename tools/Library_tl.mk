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

$(eval $(call gb_Library_use_package,tl,tools_inc))

$(eval $(call gb_Library_use_custom_headers,tl,tools/reversemap))

$(eval $(call gb_Library_set_include,tl,\
    -I$(SRCDIR)/tools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,tl,\
    -DTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,tl))

$(eval $(call gb_Library_use_libraries,tl,\
    basegfx \
    comphelper \
    i18nisolang1 \
    cppu \
    sal \
	$(gb_UWINAPI) \
))


$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/source/datetime/datetime \
    tools/source/datetime/tdate \
    tools/source/datetime/ttime \
    tools/source/debug/debug \
    tools/source/fsys/comdep \
    tools/source/fsys/dirent \
    tools/source/fsys/filecopy \
    tools/source/fsys/fstat \
    tools/source/fsys/tdir \
    tools/source/fsys/tempfile \
    tools/source/fsys/urlobj \
    tools/source/fsys/wldcrd \
    tools/source/generic/b3dtrans \
    tools/source/generic/bigint \
    tools/source/generic/color \
    tools/source/generic/config \
    tools/source/generic/fract \
    tools/source/generic/gen \
    tools/source/generic/line \
    tools/source/generic/link \
    tools/source/generic/poly \
    tools/source/generic/poly2 \
    tools/source/generic/svborder \
    tools/source/generic/toolsin \
    tools/source/inet/inetmime \
    tools/source/inet/inetmsg \
    tools/source/inet/inetstrm \
    tools/source/memtools/mempool \
    tools/source/memtools/multisel \
    tools/source/memtools/unqidx \
    tools/source/misc/appendunixshellword \
    tools/source/misc/extendapplicationenvironment \
    tools/source/misc/getprocessworkingdir \
    tools/source/misc/solarmutex \
    tools/source/rc/rc \
    tools/source/rc/resary \
    tools/source/rc/resmgr \
    tools/source/ref/errinf \
    tools/source/ref/globname \
    tools/source/ref/pstm \
    tools/source/ref/ref \
    tools/source/stream/cachestr \
    tools/source/stream/stream \
    tools/source/stream/strmsys \
    tools/source/stream/vcompat \
    tools/source/string/tenccvt \
    tools/source/string/tustring \
    tools/source/string/reversemap \
    tools/source/zcodec/zcodec \
))

ifeq ($(GUI),UNX)
$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/unx/source/dll/toolsdll \
))
endif

$(eval $(call gb_Library_use_external,tl,zlib))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_set_include,tl,\
    -I$(SRCDIR)/tools/win/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/win/source/dll/toolsdll \
))

$(eval $(call gb_Library_use_system_win32_libs,tl,\
	mpr \
	ole32 \
	shell32 \
	uuid \
))

endif

# vim: set noet sw=4 ts=4:
