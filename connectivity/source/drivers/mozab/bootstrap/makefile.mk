#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.13 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
CALL_CDECL=TRUE

#mozilla specific stuff.
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
#End of mozilla specific stuff.

PRJ=..$/..$/..$/..
PRJINC=..$/..$/..
PRJNAME=connectivity
TARGET=mozbootstrap

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF


.INCLUDE : settings.mk

.IF ("$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES") || "$(WITH_MOZILLA)" == "NO"
dummy:
    @echo "		Not building the mozillasrc stuff in OpenOffice.org build"
    @echo "		dependency to Mozilla developer snapshots not feasable at the moment"

.ELSE

.INCLUDE :  $(PRJ)$/version.mk


#mozilla specific stuff.
# --- Files -------------------------------------

SLOFILES = \
    $(SLO)$/MMozillaBootstrap.obj	\
    $(SLO)$/MNSFolders.obj	\
    $(SLO)$/MNSProfileDiscover.obj				\
    $(SLO)$/MNSInit.obj			            \
    $(SLO)$/MNSProfileManager.obj	\
    $(SLO)$/MNSINIParser.obj	\
    $(SLO)$/MNSRunnable.obj	\
    $(SLO)$/MNSProfile.obj					\
    $(SLO)$/MNSProfileDirServiceProvider.obj


.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
CFLAGS += -features=tmplife
#This flag is needed to build mozilla 1.7 code
.ENDIF		# "$(COMNAME)"=="sunpro5"
.ENDIF

MOZINC = . -I.. -I..$/mozillasrc -I$(MOZ_INC)  -I$(MOZ_INC)$/nspr -I$(MOZ_INC)$/xpcom \
        -I$(MOZ_INC)$/string -I$(MOZ_INC)$/rdf -I$(MOZ_INC)$/msgbase \
        -I$(MOZ_INC)$/addrbook -I$(MOZ_INC)$/mork -I$(MOZ_INC)$/locale \
        -I$(MOZ_INC)$/pref -I$(MOZ_INC)$/mime -I$(MOZ_INC)$/chrome \
        -I$(MOZ_INC)$/necko -I$(MOZ_INC)$/intl -I$(MOZ_INC)$/profile \
        -I$(MOZ_INC)$/embed_base -I$(MOZ_INC)$/mozldap -I$(MOZ_INC)$/uconv\
        -I$(MOZ_INC)$/xpcom_obsolete -I$(MOZ_INC)$/content
        
.IF "$(GUI)" == "WNT"
.IF "$(COM)" == "GCC"
INCPOST += $(MOZINC)
CDEFS +=    -DWINVER=0x400 -DMOZILLA_CLIENT \
        -DNS_NET_FILE -DCookieManagement -DSingleSignon -DClientWallet \
            -DTRACING -DXP_PC -DXP_WIN -DXP_WIN32 -DHW_THREADS \
            -DNS_MT_SUPPORTED -DNETLIB_THREAD \
            -DOJI -DWIN32 -D_X86_ -D_WINDOWS \
        -DMOZ_XUL -DMOZ_REFLOW_PERF -DMOZ_REFLOW_PERF_DSP \
        -DNSPR20 -DOS_HAS_DLL -DNO_JNI_STUBS \
        -DNETSCAPE -DMOZILLA_CLIENT -DJS_THREADSAFE -DNECKO -DINCLUDE_XUL
CFLAGSCXX += \
            -fno-rtti -Wall -Wconversion -Wpointer-arith \
            -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long
.ELSE
.IF "$(DBG_LEVEL)" == "0"
INCPRE += $(MOZINC)
CDEFS +=    -DWINVER=0x400 -DMOZILLA_CLIENT \
        -DNS_NET_FILE -DCookieManagement -DSingleSignon -DClientWallet \
            -DTRACING -DXP_PC -DXP_WIN -DXP_WIN32 -DHW_THREADS \
            -DDMSVC4 -DNS_MT_SUPPORTED -DNETLIB_THREAD \
            -DOJI -DWIN32 -D_X86_ -D_WINDOWS \
        -DMOZ_XUL -DMOZ_REFLOW_PERF -DMOZ_REFLOW_PERF_DSP \
        -DNSPR20 -DOS_HAS_DLL -DNO_JNI_STUBS \
        -DNETSCAPE -DMOZILLA_CLIENT -DJS_THREADSAFE -DNECKO -DINCLUDE_XUL	\
        -UDEBUG
CFLAGS +=   -GR- -W3 -Gy -MD -UDEBUG
.ELSE
INCPRE += $(MOZINC)
CDEFS +=    -DWINVER=0x400 -DMOZILLA_CLIENT \
        -DNS_NET_FILE -DCookieManagement -DSingleSignon -DClientWallet \
            -DTRACING -DXP_PC -DXP_WIN -DXP_WIN32 -DHW_THREADS \
            -DDMSVC4 -DDEVELOPER_DEBUG -DNS_MT_SUPPORTED -DNETLIB_THREAD \
            -DOJI -DWIN32 -D_X86_ -D_WINDOWS \
        -DMOZ_XUL -DMOZ_REFLOW_PERF -DMOZ_REFLOW_PERF_DSP \
        -DDEBUG_Administrator -DNSPR20 -DOS_HAS_DLL -DNO_JNI_STUBS \
        -DNETSCAPE -DMOZILLA_CLIENT -DJS_THREADSAFE -DNECKO -DINCLUDE_XUL	\
        -UDEBUG
CFLAGS +=   -Zi -GR- -W3 -Gy -MDd -UDEBUG
.IF "$(CCNUMVER)" >= "001399999999"
CDEFS  +=   -D_STL_NOFORCE_MANIFEST
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.IF "$(GUI)" == "UNX"
INCPOST += $(MOZINC)
CDEFS+=	    -DMOZILLA_CLIENT \
            -DXP_UNIX
.IF "$(OS)" == "LINUX"
CFLAGS +=   -fPIC -g
CDEFS+=     -DOJI

CFLAGSCXX += \
            -fno-rtti -Wconversion -Wpointer-arith \
            -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long -pthread
CDEFS     += -DTRACING
.ELIF "$(OS)" == "NETBSD" || "$(OS)" == "MACOSX"
CFLAGS +=   -fPIC
CFLAGSCXX += \
            -fno-rtti -Wconversion -Wpointer-arith \
            -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long
CDEFS     += -DTRACING
.ENDIF
.ENDIF

SHL1TARGET_NAME=$(TARGET)$(MOZAB_MAJOR)

.ENDIF # MACOSX

# --- Targets ----------------------------------

.INCLUDE : target.mk

