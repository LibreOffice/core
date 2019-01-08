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
gb_Library_FILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:libhyphen.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst lnth:lnth.uno%,lnth:liblnth.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst reg:libuno_reg%,reg:libreg%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst spell:spell.uno%,spell:libspell.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst store:libuno_store%,store:libstore%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sunjavaplugin:libsunjavaplugin%,sunjavaplugin:sunjavaplugin%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:libstdc++.so,$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif # ifeq ($(OS),LINUX)

ifeq ($(OS),SOLARIS)

gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:libhyphen.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst lnth:lnth.uno%,lnth:liblnth.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst reg:libuno_reg%,reg:libreg%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst spell:spell.uno%,spell:libspell.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst store:libuno_store%,store:libstore%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sunjavaplugin:libsunjavaplugin%,sunjavaplugin:sunjavaplugin%,$(gb_Library_FILENAMES))
#$(info libnames: $(gb_Library_FILENAMES))

endif

ifeq ($(OS),MACOSX)

gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:libhyphen.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst lnth:lnth.uno%,lnth:liblnth.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst reg:libuno_reg%,reg:libreg%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst spell:spell.uno%,spell:libspell.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst store:libuno_store%,store:libstore%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sunjavaplugin:libsunjavaplugin%,sunjavaplugin:sunjavaplugin%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jurt:libjpipe.dylib%,jurt:libjpipe.jnilib%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:libc++.dylib,$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif # ifeq ($(OS),MACOSX)

ifeq ($(OS),WNT)

ifneq ($(USE_MINGW),)

gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:iii18nisolang1%,i18nisolang1:iii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:ivos3%,vos3:ivos%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:crypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:istl%,stl:msvcprt%,$(gb_Library_FILENAMES))

# handle libraries in msvc format that don't use an "i" prefix for their import library
# these are libraries built by AOO, but only a few of them
# all other libraries built by AOO and all platform libraries (exceptions see below) are used without an import library
# we link against their dlls in gcc format directly
gb_Library_NOILIBFILENAMES:=\
    icudt \
    icuin \
    icule \
    icutu \
    icuuc \
    uwinapi \
    winmm \

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))

# some Windows platform libraries are missing in mingw library set
# we have to use them from the PSDK by linking against their ilibs
gb_Library_ILIBFILENAMES:=\
    gdiplus \
    uuid \
    winmm \

gb_Library_DLLFILENAMES := $(filter-out $(foreach lib,$(gb_Library_ILIBFILENAMES),$(lib):%),$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES += $(foreach lib,$(gb_Library_ILIBFILENAMES),$(lib):$(PSDK_HOME)/lib/$(lib)$(gb_Library_ILIBEXT))

gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:hyphen.uno%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icudt:icudt%,icudt:icudt40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuin:icuin%,icuin:icuin40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icule:icule%,icule:icule40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icutu:icutu%,icutu:icutu40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst jvmaccess:jvmaccess%,jvmaccess:jvmaccess$(gb_Library_MAJORVER)%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:$(gb_Library_IARCSYSPRE)stdc++_s$(gb_Library_IARCSYSPRE),$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

else #ifneq ($(USE_MINGW),)

gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:iii18nisolang1%,i18nisolang1:iii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:ivos3%,vos3:ivos%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:libeay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssleay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf%,$(gb_Library_FILENAMES))
gb_StaticLibrary_FILENAMES := $(patsubst graphite:graphite%,graphite:graphite_dll%,$(gb_StaticLibrary_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:istl%,stl:msvcprt%,$(gb_Library_FILENAMES))

# change the names of all import libraries that don't have an "i" prefix as in our standard naming schema
gb_Library_NOILIBFILENAMES := $(gb_Library_PLAINLIBS_NONE)
gb_Library_NOILIBFILENAMES += \
    icudt \
    icuin \
    icule \
    icutu \
    icuuc

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))
gb_Library_FILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_FILENAMES))

#ifneq ($(gb_PRODUCT),$(true))
#gb_Library_FILENAMES := $(patsubst msvcrt:msvcrt%,msvcrt:msvcrtd%,$(gb_Library_FILENAMES))
#endif

gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:hyphen.uno%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icudt:icudt%,icudt:icudt40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuin:icuin%,icuin:icuin40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icule:icule%,icule:icule40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icutu:icutu%,icutu:icutu40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))

endif # ifneq ($(USE_MINGW),)

else # ifeq ($(OS),WNT)

gb_Library_FILENAMES := $(patsubst vbaobj:vbaobj.uno%,vbaobj:libvbaobj.uno%,$(gb_Library_FILENAMES))

endif # ifeq ($(OS),WNT)

ifeq ($(OS),OS2)
gb_Library_FILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelpgcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppu:cppu%,cppu:cppu3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:cppuhelper%,cppuhelper:cppuh3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cui:iicuc%,cui:icuc%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:i18nisolang1%,i18nisolang1:i18nisolang1gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmaccess:jvmaccess%,jvmaccess:jvmaccess3gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:jvmfwk%,jvmfwk:jvmfwk3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:tl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sal:sal%,sal:sal3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:salhelper%,salhelper:salhelp3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svl:isvl%,svl:svl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vcl:ivcl%,vcl:vcl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vos3:vos3%,vos3:vos3gcc3%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xo:ixo%,xo:xo%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:xml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst stl:stl%,stl:stdc++%,$(gb_Library_FILENAMES))

# translate to 8.3 naming
gb_Library_DLLFILENAMES := $(patsubst sw:sw%,sw:swoo%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst drawinglayer:drawinglayer%,drawinglayer:drawingl%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst fsstorage:fsstorage%,fsstorage:fsstorag%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst hatchwindowfactory:hatchwindowfactory%,hatchwindowfactory:hatchwin%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:hyphen_u%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst lnth:lnth.uno%,lnth:lnth%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst msforms:msforms.uno%,msforms:msforms%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst passwordcontainer:passwordcontainer%,passwordcontainer:password%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst productregistration:productregistration.uno%,productregistration:productr%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst textconversiondlgs:textconversiondlgs%,textconversiondlgs:txcnvdlg%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst vbahelper:vbahelper%,vbahelper:vbahelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst vbaswobj:vbaswobj.uno%,vbaswobj:vbaswobj%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst sfx:test_sfx2_metadatable%,sfx:tstsfx2m%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst xmlreader:xmlreader%,xmlreader:xmlreade%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbhelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst cppcanvas:cppcanvas%,cppcanvas:cppcanva%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst canvasfactory:canvasfactory%,canvasfactory:canvasfa%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst canvastools:canvastools%,canvastools:canvasto%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst cairocanvas:cairocanvas%,cairocanvas:cairocnv%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst simplecanvas:simplecanvas%,simplecanvas:simpleca%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst spell:spell.uno%,spell:spell%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst vclcanvas:vclcanvas%,vclcanvas:vclcanva%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst writerfilter:writerfilter%,writerfilter:wfilt%,$(gb_Library_DLLFILENAMES))

gb_Library_DLLFILENAMES := $(patsubst connector:connector%,connector:connecto%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst configmgr:configmgr%,configmgr:configmg%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst emboleobj:emboleobj%,emboleobj:emboleob%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst fps_office:fps_office%,fps_office:fps_offi%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst textinstream:textinstream%,textinstream:textouts%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst textoutstream:textoutstream%,textoutstream:textouts%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst sysdtrans:sysdtrans%,sysdtrans:sysdtran%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst slideshow:slideshow%,slideshow:slidesho%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst uuresolver:uuresolver%,uuresolver:uuresolv%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucpexpand1:ucpexpand1%,ucpexpand1:ucpexpan%,$(gb_Library_DLLFILENAMES))

gb_Library_NOILIBFILENAMES:=\
    ft2lib \
    icudt \
    icuin \
    icule \
    icutu \
    icuuc

gb_Library_ILIBFILENAMES:=\
    ft2lib \
    uuid \

endif

ifeq ($(OS),FREEBSD)
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst hyphen_uno:hyphen_uno.uno%,hyphen_uno:libhyphen.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst lnth:lnth.uno%,lnth:liblnth.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst reg:libuno_reg%,reg:libreg%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst spell:spell.uno%,spell:libspell.uno%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst store:libuno_store%,store:libstore%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sunjavaplugin:libsunjavaplugin%,sunjavaplugin:sunjavaplugin%,$(gb_Library_FILENAMES))

gb_Library_FILENAMES := $(patsubst stl:%,stl:libstdc++.so,$(gb_Library_FILENAMES))
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

endif

# we do not require a known rule for these, when using system libs

gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))

ifeq ($(SYSTEM_EXPAT),YES)
gb_Library_TARGETS := $(filter-out expat,$(gb_Library_TARGETS))
endif

# vim: set noet sw=4 ts=4:
