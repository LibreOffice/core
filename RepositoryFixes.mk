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

# fixes for executables

gb_Executable_FILENAMES := $(foreach group,$(gb_Executable_VALIDGROUPS),\
	$(foreach exe,$(gb_Executable_$(group)),$(exe):$(exe)$(gb_Executable_EXT)))

ifneq (,$(filter-out MACOSX WNT,$(OS)))
gb_Executable_FILENAMES := $(patsubst uno:uno,uno:uno.bin,$(gb_Executable_FILENAMES))
endif
gb_Executable_FILENAMES := $(patsubst unopkg_bin:unopkg_bin%,unopkg_bin:unopkg$(if $(filter-out MACOSX,$(OS)),.bin),$(gb_Executable_FILENAMES))
gb_Executable_FILENAMES := $(patsubst unopkg_com:unopkg_com%,unopkg_com:unopkg.com,$(gb_Executable_FILENAMES))
ifneq ($(OS),MACOSX)
gb_Executable_FILENAMES := $(patsubst gengal:gengal,gengal:gengal.bin,$(gb_Executable_FILENAMES))
endif

ifeq ($(OS),MACOSX)
gb_Executable_FILENAMES := $(patsubst soffice_bin:soffice_bin,soffice_bin:soffice,$(gb_Executable_FILENAMES))
else
gb_Executable_FILENAMES := $(patsubst soffice_bin:soffice_bin%,soffice_bin:soffice.bin,$(gb_Executable_FILENAMES))
endif

gb_Executable_FILENAMES_FOR_BUILD := $(subst $(gb_Executable_EXT),$(gb_Executable_EXT_for_build),$(gb_Executable_FILENAMES))

# fixes for .jnilibs on Mac OS X that are not also needed as .dylibs:
ifeq ($(OS),MACOSX)
gb_Library_FILENAMES := \
    $(subst jpipe:libjpipe.dylib,jpipe:libjpipe.jnilib,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := \
    $(subst juh:libjuh.dylib,juh:libjuh.jnilib,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := \
    $(subst hsqldb:libhsqldb.dylib,hsqldb:libhsqldb.jnilib,$(gb_Library_FILENAMES))
endif

# fixes for all the libraries that are named with too much creativity and do
# not follow any of the established nameschemes

# Make has no support for 'or' clauses in conditionals,
# we use a filter expression instead.
ifneq (,$(filter SOLARIS GCC,$(OS) $(COM)))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst purpenvhelper:libpurpen%,purpenvhelper:libuno_purpen%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))

ifeq ($(OS),MACOSX)
# libpyuno_wrapper.dylib => pyuno.so
gb_Library_FILENAMES := $(patsubst pyuno_wrapper:libpyuno_wrapper.dylib,pyuno_wrapper:pyuno.so,$(gb_Library_FILENAMES))
else
# libpyuno_wrapper.so => pyuno.so
gb_Library_FILENAMES := $(patsubst pyuno_wrapper:libpyuno_wrapper.so,pyuno_wrapper:pyuno.so,$(gb_Library_FILENAMES))
endif

ifneq ($(OS),ANDROID)
gb_Library_FILENAMES := $(patsubst unobootstrapprotector:libuno%,unobootstrapprotector:uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst unoexceptionprotector:libuno%,unoexceptionprotector:uno%,$(gb_Library_FILENAMES))
endif
endif

ifeq ($(OS),WNT)
gb_Library_FILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:rdf%,rdf:librdf%,$(gb_Library_FILENAMES))

# libpyuno_wrapper.dll => pyuno.pyd
gb_Library_FILENAMES := $(patsubst pyuno:pyuno.dll,pyuno:pyuno$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd,$(gb_Library_FILENAMES))

gb_Library_ILIBFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_ILIBFILENAMES))

# these have prefix "lib" instead of "i"
gb_Library_LIBLIBFILENAMES := \
	rdf \

gb_Library_ILIBFILENAMES := \
	$(filter-out $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):%) \
		,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES += \
	$(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):lib$(lib)$(gb_Library_PLAINEXT))

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):lib$(lib).dll)

endif # ifeq ($(OS),WNT)

# vim: set noet sw=4 ts=4:
