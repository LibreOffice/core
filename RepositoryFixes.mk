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

gb_Executable_FILENAMES := $(patsubst uno:uno,uno:uno.bin,$(gb_Executable_FILENAMES))
gb_Executable_FILENAMES := $(patsubst gengal:gengal,gengal:gengal.bin,$(gb_Executable_FILENAMES))

ifeq ($(OS),MACOSX)
gb_Executable_FILENAMES := $(patsubst soffice.bin:soffice.bin,soffice.bin:soffice,$(gb_Executable_FILENAMES))
endif

gb_Executable_FILENAMES_FOR_BUILD := $(subst $(gb_Executable_EXT),$(gb_Executable_EXT_for_build),$(gb_Executable_FILENAMES))

# fixes for all the libraries that are named with too much creativity and do
# not follow any of the established nameschemes

# Make has no support for 'or' clauses in conditionals,
# we use a filter expression instead.
ifneq (,$(filter SOLARIS GCC,$(OS) $(COM)))
gb_Library_FILENAMES := $(patsubst affine_uno:libaffine_uno%,affine_uno:libaffine_uno_uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst log_uno:liblog_uno%,log_uno:liblog_uno_uno%,$(gb_Library_FILENAMES))
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
gb_Library_FILENAMES := $(patsubst unsafe_uno:libunsafe_uno%,unsafe_uno:libunsafe_uno_uno%,$(gb_Library_FILENAMES))
endif

ifeq ($(OS),WNT)
gb_Library_DLLFILENAMES := $(patsubst affine_uno:affine_uno%,affine_uno:affine_uno_uno%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst log_uno:log_uno%,log_uno:log_uno_uno%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst unsafe_uno:unsafe_uno%,unsafe_uno:unsafe_uno_uno%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst rdf:rdf%,rdf:librdf%,$(gb_Library_DLLFILENAMES))

# libpyuno_wrapper.dll => pyuno.pyd
gb_Library_DLLFILENAMES := $(patsubst pyuno:pyuno.dll,pyuno:pyuno$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd,$(gb_Library_DLLFILENAMES))

ifeq ($(COM),GCC)
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:crypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst exslt:iexslt%,exslt:libexslt$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst libexttextcat:liblibext%,libexttextcat:libilibext%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))

# Libraries not provided by mingw(-w64), available only in the Windows
# SDK. So if these actually are liked with somewhere, we can't
# cross-compile that module then using MinGW. That needs to be fixed
# then, and we need to use these libraries through run-time lookup of
# APIs. Or something.
gb_Library_SDKLIBFILENAMES:=

gb_Library_DLLFILENAMES := $(filter-out $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):%),$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES += $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):$(WINDOWS_SDK_HOME)/lib/$(lib).lib)

gb_Library_DLLFILENAMES := $(patsubst crypto:crypto%,crypto:libeay32%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ssl:ssl%,ssl:ssleay32%,$(gb_Library_DLLFILENAMES))

else # $(COM) != GCC

gb_Library_FILENAMES := $(patsubst cppunit:icppunit%,cppunit:icppunit_dll%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:libeay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:libcrypto%,crypto:libcrypto_static%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssleay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:libssl%,ssl:libssl_static%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1:ixmlsec1%,xmlsec1:libxmlsec%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-mscrypto:ixmlsec1-mscrypto%,xmlsec1-mscrypto:libxmlsec-mscrypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-nss:ixmlsec1-nss%,xmlsec1-nss:libxmlsec-nss%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_FILENAMES))


gb_Library_NOILIBFILENAMES_D := \
    icudt icuin icuuc \
    lcms2 \

# change the names of all import libraries that don't have an "i" prefix as in our standard naming schema
gb_Library_NOILIBFILENAMES := \
    langtag \
    lpsolve55 \
    xpcom \
    xpcom_core \
    nspr4 \
    nss3 \
    smime3 \

# these have prefix "lib" instead of "i"
gb_Library_LIBLIBFILENAMES := \
	curl \
	exslt \
	rdf \
	xml2 \
	xslt \

gb_Library_FILENAMES := \
	$(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%) \
				 $(foreach lib,$(gb_Library_NOILIBFILENAMES_D),$(lib):%) \
				 $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):%) \
		,$(gb_Library_FILENAMES))
gb_Library_FILENAMES += \
	$(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_NOILIBFILENAMES_D),$(lib):$(lib)$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):lib$(lib)$(gb_Library_PLAINEXT))

endif # ifeq ($(COM),GCC)

endif # ifeq ($(OS),WNT)

# vim: set noet sw=4 ts=4:
