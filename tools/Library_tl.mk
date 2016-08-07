#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,tl))

$(eval $(call gb_Library_add_package_headers,tl,tools_inc))

$(eval $(call gb_Library_add_precompiled_header,tl,$(SRCDIR)/tools/inc/pch/precompiled_tools))

$(eval $(call gb_Library_set_include,tl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc \
    -I$(SRCDIR)/tools/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(SRCDIR)/solenv/inc/Xp31 \
    -I$(OUTDIR)/inc/stl \
))

$(eval $(call gb_Library_add_defs,tl,\
    -DTOOLS_DLLIMPLEMENTATION \
    -DVCL \
))

$(eval $(call gb_Library_add_api,tl,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,tl,\
    basegfx \
    comphelper \
    i18nisolang1 \
    stl \
    cppu \
    sal \
    vos3 \
    $(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/source/communi/geninfo \
    tools/source/communi/parser \
    tools/source/datetime/datetime \
    tools/source/datetime/tdate \
    tools/source/datetime/ttime \
    tools/source/debug/debug \
    tools/source/debug/stcktree \
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
    tools/source/generic/svlibrary \
    tools/source/inet/inetmime \
    tools/source/inet/inetmsg \
    tools/source/inet/inetstrm \
    tools/source/memtools/contnr \
    tools/source/memtools/mempool \
    tools/source/memtools/multisel \
    tools/source/memtools/table \
    tools/source/memtools/unqidx \
    tools/source/misc/appendunixshellword \
    tools/source/misc/extendapplicationenvironment \
    tools/source/misc/getprocessworkingdir \
    tools/source/misc/solarmutex \
    tools/source/rc/isofallback \
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
    tools/source/string/debugprint \
    tools/source/string/tenccvt \
    tools/source/string/tstring \
    tools/source/string/tustring \
    tools/source/testtoolloader/testtoolloader \
    tools/source/zcodec/zcodec \
))

ifeq ($(GUI),UNX)
$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/unx/source/dll/toolsdll \
))
endif

ifeq ($(GUI),OS2)
$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/os2/source/dll/toolsdll \
))
endif

$(call gb_Library_use_external,tl,zlib)

ifeq ($(OS),OS2)
# YD FIXME above is not working... needs ldflags hack...
$(eval $(call gb_Library_add_libs,tl,\
    -lz \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Library_set_include,tl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/win/inc \
))

$(eval $(call gb_Library_add_exception_objects,tl,\
    tools/win/source/dll/toolsdll \
))

$(eval $(call gb_Library_add_linked_libs,tl,\
    mpr \
    ole32 \
    shell32 \
    user32 \
    uuid \
))

endif

# tools/source/string/debugprint -DDEBUG -DEXCEPTIONS_OFF -DOSL_DEBUG_LEVEL=2 -DSHAREDLIB -DTOOLS_DLLIMPLEMENTATION -D_DLL_ -O0 -fno-exceptions -fpic -fvisibility=hidden -g
# -DOPTIMIZE
# no -DTOOLS_DLLIMPLEMENTATION on toolsdll
# -DEXCEPTIONS_OFF -fno-exceptions on geninfo parser datetime tdate ttime bigint color config fract gen line link poly2 svborder toolsin inetmime inetmsg inetstrm contnr mempool multisel table unqidx cachestr stream strmsys vcompat tenccvt tstring tustring testtoolloader
# vim: set noet sw=4 ts=4:
