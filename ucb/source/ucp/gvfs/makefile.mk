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



PRJ=..$/..$/..
PRJNAME=ucb
# Version
UCPGVFS_MAJOR=1
TARGET=ucpgvfs
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

.IF "$(ENABLE_GNOMEVFS)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gnome-vfs-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS:s/ -llinc//:s/ -lbonobo-activation//:s/ -lgconf-2//:s/ -lORBit-2//:s/ -lIDL-2//:s/ -lgmodule-2.0//:s/ -lgobject-2.0//:s/ -lgthread-2.0//)
.ENDIF          # "$(OS)" == "LINUX"

# no "lib" prefix
DLLPRE =

SLOFILES=\
    $(SLO)$/gvfs_content.obj	\
    $(SLO)$/gvfs_directory.obj	\
    $(SLO)$/gvfs_stream.obj	\
    $(SLO)$/gvfs_provider.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)$(UCPGVFS_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""
.ENDIF # L10N_framework

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpgvfs.component

$(MISC)/ucpgvfs.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpgvfs.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpgvfs.component
