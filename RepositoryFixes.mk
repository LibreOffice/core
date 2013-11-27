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



# fixes for all the libraries that are named with too much creativity and do
# not follow any of the established nameschemes

ifeq ($(OS),LINUX)
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:libucbhelper%,ucbhelper:libucbhelper4%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:libstdc++.so,$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif


ifeq ($(OS),MACOSX)
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:libucbhelper%,ucbhelper:libucbhelper4%,$(gb_Library_FILENAMES))

ifneq ($(MACOSX_DEPLOYMENT_TARGET),10.4) # default to use libc++
gb_Library_FILENAMES := $(patsubst stl:%,stl:libc++.dylib,$(gb_Library_FILENAMES))
else # use libstdc++ instead on ancient MacOSX targets
gb_Library_FILENAMES := $(patsubst stl:%,stl:libstdc++.dylib,$(gb_Library_FILENAMES))
endif
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif


ifeq ($(OS),WNT)
ifneq ($(USE_MINGW),)
gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppunit:icppunit%,cppunit:libcppunit.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cui:icui%,cui:icuin%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:iii18nisolang1%,i18nisolang1:iii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:ivos3%,vos3:ivos%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:istl%,stl:msvcprt%,$(gb_Library_FILENAMES))

gb_Library_NOILIBFILENAMES:=\
    icule \
    icuuc \
    sot \
    uwinapi \

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))

gb_Library_ILIBFILENAMES:=\
    gdiplus \
    uuid \

gb_Library_DLLFILENAMES := $(filter-out $(foreach lib,$(gb_Library_ILIBFILENAMES),$(lib):%),$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES += $(foreach lib,$(gb_Library_ILIBFILENAMES),$(lib):$(PSDK_HOME)/lib/$(lib)$(gb_Library_ILIBEXT))

gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icule:icule%,icule:icule40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst jvmaccess:jvmaccess%,jvmaccess:jvmaccess$(gb_Library_MAJORVER)%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbhelper4%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:$(gb_Library_IARCSYSPRE)stdc++_s$(gb_Library_IARCSYSPRE),$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

else
gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppunit:icppunit%,cppunit:icppunit_dll%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cui:icui%,cui:icuin%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:iii18nisolang1%,i18nisolang1:iii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:ivos3%,vos3:ivos%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))
gb_StaticLibrary_FILENAMES := $(patsubst graphite:graphite%,graphite:graphite_dll%,$(gb_StaticLibrary_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:istl%,stl:msvcprt%,$(gb_Library_FILENAMES))

gb_Library_NOILIBFILENAMES:=\
    advapi32 \
    gdi32 \
    gdiplus \
    gnu_getopt \
    icuuc \
    icule \
    imm32\
    kernel32 \
    msimg32 \
    msvcrt \
    mpr \
    oldnames \
    ole32 \
    oleaut32 \
    shell32 \
    sot \
    user32 \
    uuid \
    uwinapi \
    winspool \

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))
#ifneq ($(gb_PRODUCT),$(true))
#gb_Library_FILENAMES := $(patsubst msvcrt:msvcrt%,msvcrt:msvcrtd%,$(gb_Library_FILENAMES))
#endif

gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbhelper4%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))

endif

endif

ifeq ($(OS),OS2)
gb_Library_FILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelpgcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppu:cppu%,cppu:cppu3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:cppuhelper%,cppuhelper:cppuh3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppunit:icppunit%,cppunit:cppunit%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cui:iicuc%,cui:icuc%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:i18nisolang1%,i18nisolang1:i18nisolang1gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmaccess:jvmaccess%,jvmaccess:jvmaccess3gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:jvmfwk%,jvmfwk:jvmfwk3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:rdf%,rdf:librdf%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:tl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sal:sal%,sal:sal3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:salhelper%,salhelper:salhelp3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svl:isvl%,svl:svl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vcl:ivcl%,vcl:vcl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbh4%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:vos3%,vos3:vos3gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xo:ixo%,xo:xo%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:xml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst stl:stl%,stl:stdc++%,$(gb_Library_FILENAMES))

# translate to 8.3 naming
gb_Library_DLLFILENAMES := $(patsubst sw:sw%,sw:swoo%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst drawinglayer:drawinglayer%,drawinglayer:drawingl%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst fsstorage:fsstorage.uno%,fsstorage:fsstorag%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst hatchwindowfactory:hatchwindowfactory.uno%,hatchwindowfactory:hatchwin%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst msforms:msforms.uno%,msforms:msforms%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst passwordcontainer:passwordcontainer.uno%,passwordcontainer:password%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst productregistration:productregistration.uno%,productregistration:productr%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst textconversiondlgs:textconversiondlgs%,textconversiondlgs:txcnvdlg%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst vbahelper:vbahelper%,vbahelper:vbahelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst vbaswobj:vbaswobj.uno%,vbaswobj:vbaswobj%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst sfx:test_sfx2_metadatable%,sfx:tstsfx2m%,$(gb_Library_DLLFILENAMES))

gb_Library_NOILIBFILENAMES:=\
    ft2lib \
    icuuc \
    icule

gb_Library_ILIBFILENAMES:=\
    ft2lib \
    uuid \

endif

ifeq ($(OS),SOLARIS)
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:libucbhelper%,ucbhelper:libucbhelper4%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
#$(info libnames: $(gb_Library_FILENAMES))

endif

ifeq ($(OS),FREEBSD)
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:libucbhelper%,ucbhelper:libucbhelper4%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:libstdc++.so,$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif

# we do not require a known rule for these, when using system libs

gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

ifeq ($(SYSTEM_LIBXML),YES)
gb_Library_TARGETS := $(filter-out xml2,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_LIBXSLT),YES)
gb_Library_TARGETS := $(filter-out xslt,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_REDLAND),YES)
gb_Library_TARGETS := $(filter-out rdf,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_ICU),YES)
gb_Library_TARGETS := $(filter-out icuuc,$(gb_Library_TARGETS))
endif

# vim: set noet sw=4 ts=4:
