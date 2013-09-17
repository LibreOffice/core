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
gb_Library_ICU_LIBRARIES := icudt icuuc icuin icutu
gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_ICU_LIBRARIES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_ICU_LIBRARIES),$(lib):$(lib)$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll)
gb_Library_FILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:rdf%,rdf:librdf%,$(gb_Library_FILENAMES))

# libpyuno_wrapper.dll => pyuno.pyd
gb_Library_FILENAMES := $(patsubst pyuno:pyuno.dll,pyuno:pyuno$(if $(MSVC_USE_DEBUG_RUNTIME),_d).pyd,$(gb_Library_FILENAMES))

ifeq ($(COM),GCC)
gb_Library_ILIBFILENAMES := $(patsubst crypto:icrypto%,crypto:crypto%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst exslt:iexslt%,exslt:libexslt$(gb_Library_IARCEXT),$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst libexttextcat:liblibext%,libexttextcat:libilibext%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst rdf:irdf%,rdf:librdf.dll$(gb_Library_IARCEXT),$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst ssl:issl%,ssl:ssl%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2$(gb_Library_IARCEXT),$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt$(gb_Library_IARCEXT),$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_ILIBFILENAMES))

# Libraries not provided by mingw(-w64), available only in the Windows
# SDK. So if these actually are liked with somewhere, we can't
# cross-compile that module then using MinGW. That needs to be fixed
# then, and we need to use these libraries through run-time lookup of
# APIs. Or something.
gb_Library_SDKLIBFILENAMES:=

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):$(WINDOWS_SDK_HOME)/lib/$(lib).lib)

gb_Library_FILENAMES := $(patsubst crypto:crypto%,crypto:libeay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:ssl%,ssl:ssleay32%,$(gb_Library_FILENAMES))

else # $(COM) != GCC
gb_Library_ILIBFILENAMES := $(patsubst cppunit:icppunit%,cppunit:icppunit_dll%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst crypto:icrypto%,crypto:libeay32%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst crypto:libcrypto%,crypto:libcrypto_static%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst ssl:issl%,ssl:ssleay32%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst ssl:libssl%,ssl:libssl_static%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst xmlsec1:ixmlsec1%,xmlsec1:libxmlsec%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst xmlsec1-mscrypto:ixmlsec1-mscrypto%,xmlsec1-mscrypto:libxmlsec-mscrypto%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst xmlsec1-nss:ixmlsec1-nss%,xmlsec1-nss:libxmlsec-nss%,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_ILIBFILENAMES))


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

gb_Library_ILIBFILENAMES := \
	$(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%) \
				 $(foreach lib,$(gb_Library_NOILIBFILENAMES_D),$(lib):%) \
				 $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):%) \
		,$(gb_Library_ILIBFILENAMES))
gb_Library_ILIBFILENAMES += \
	$(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_NOILIBFILENAMES_D),$(lib):$(lib)$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):lib$(lib)$(gb_Library_PLAINEXT))

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_LIBLIBFILENAMES),$(lib):lib$(lib).dll)
gb_Library_FILENAMES := $(patsubst cppunit:cppunit%,cppunit:cppunit$(if $(MSVC_USE_DEBUG_RUNTIME),d)_dll%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:crypto%,crypto:libeay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:ssl%,ssl:ssleay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst fbembed:fbembed%,fbembed:ifbembed%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1:xmlsec1%,xmlsec1:libxmlsec%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-mscrypto:xmlsec1-mscrypto%,xmlsec1-mscrypto:libxmlsec-mscrypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-nss:xmlsec1-nss%,xmlsec1-nss:libxmlsec-nss%,$(gb_Library_FILENAMES))

endif # ifeq ($(COM),GCC)

endif # ifeq ($(OS),WNT)

# vim: set noet sw=4 ts=4:
