#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2003-04-15 17:39:14 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************
CALL_CDECL=TRUE

#mozilla specific stuff.
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
#MOZ_LIB=$(PRJ)$/$(INPATH)$/lib
#MOZ_INC=$(PRJ)$/$(INPATH)$/inc$/mozilla
#End of mozilla specific stuff.

PRJ=..$/..$/..$/..
PRJINC=..$/..$/..
PRJNAME=connectivity
TARGET=mozabsrc

USE_LDUMP2=TRUE
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
LDUMP=ldump2.exe

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "		Not building the mozillasrc stuff in OpenOffice.org build"
    @echo "		dependency to Mozilla developer snapshots not feasable at the moment"

.ELSE

.INCLUDE :  $(PRJ)$/version.mk


#mozilla specific stuff.
# --- Files -------------------------------------

SLOFILES = \
    $(SLO)$/MNSInit.obj			            \
    $(SLO)$/MQueryHelper.obj			    \
    $(SLO)$/MDatabaseMetaDataHelper.obj		\
    $(SLO)$/MQuery.obj			            \
    $(SLO)$/MTypeConverter.obj              \
    $(SLO)$/MNameMapper.obj



.IF "$(GUI)" == "WNT"
.IF "$(DBG_LEVEL)" == "0"
INCPRE += . -I.. -I$(MOZ_INC)  -I$(MOZ_INC)$/nspr -I$(MOZ_INC)$/xpcom \
        -I$(MOZ_INC)$/string -I$(MOZ_INC)$/rdf -I$(MOZ_INC)$/msgbase \
        -I$(MOZ_INC)$/addrbook -I$(MOZ_INC)$/mork -I$(MOZ_INC)$/locale \
        -I$(MOZ_INC)$/pref -I$(MOZ_INC)$/mime -I$(MOZ_INC)$/chrome \
        -I$(MOZ_INC)$/necko -I$(MOZ_INC)$/intl -I$(MOZ_INC)$/profile \
        -I$(MOZ_INC)$/embed_base -I$(MOZ_INC)$/mozldap
CDEFS +=    -DWINVER=0x400 -DMOZILLA_CLIENT \
        -DNS_NET_FILE -DCookieManagement -DSingleSignon -DClientWallet \
            -DTRACING -DXP_PC -DXP_WIN -DXP_WIN32 -DHW_THREADS \
            -DDMSVC4 -DNS_MT_SUPPORTED -DNETLIB_THREAD \
            -DOJI -DWIN32 -D_X86_ -D_WINDOWS \
        -DMOZ_XUL -DMOZ_REFLOW_PERF -DMOZ_REFLOW_PERF_DSP \
        -DNSPR20 -DOS_HAS_DLL -DNO_JNI_STUBS \
        -DNETSCAPE -DMOZILLA_CLIENT -DJS_THREADSAFE -DNECKO -DINCLUDE_XUL
CFLAGS +=   -GR- -W3 -Gy -MD -UDEBUG
.ELSE
INCPRE += . -I.. -I$(MOZ_INC)  -I$(MOZ_INC)$/nspr -I$(MOZ_INC)$/xpcom \
        -I$(MOZ_INC)$/string -I$(MOZ_INC)$/rdf -I$(MOZ_INC)$/msgbase \
        -I$(MOZ_INC)$/addrbook -I$(MOZ_INC)$/mork -I$(MOZ_INC)$/locale \
        -I$(MOZ_INC)$/pref -I$(MOZ_INC)$/mime -I$(MOZ_INC)$/chrome \
        -I$(MOZ_INC)$/necko -I$(MOZ_INC)$/intl -I$(MOZ_INC)$/profile \
        -I$(MOZ_INC)$/embed_base -I$(MOZ_INC)$/mozldap
CDEFS +=    -DWINVER=0x400 -DMOZILLA_CLIENT \
        -DNS_NET_FILE -DCookieManagement -DSingleSignon -DClientWallet \
            -DTRACING -DXP_PC -DXP_WIN -DXP_WIN32 -DHW_THREADS \
            -DDMSVC4 -DDEVELOPER_DEBUG -DNS_MT_SUPPORTED -DNETLIB_THREAD \
            -DOJI -DWIN32 -D_X86_ -D_WINDOWS \
        -DMOZ_XUL -DMOZ_REFLOW_PERF -DMOZ_REFLOW_PERF_DSP \
        -DDEBUG_Administrator -DNSPR20 -DOS_HAS_DLL -DNO_JNI_STUBS \
        -DNETSCAPE -DMOZILLA_CLIENT -DJS_THREADSAFE -DNECKO -DINCLUDE_XUL
CFLAGS +=   -Zi -GR- -W3 -Gy -MDd -UNDEBUG
.ENDIF
.ENDIF
.IF "$(GUI)" == "UNX"
INCPOST += . -I.. -I$(MOZ_INC)  -I$(MOZ_INC)$/nspr -I$(MOZ_INC)$/xpcom \
        -I$(MOZ_INC)$/string -I$(MOZ_INC)$/rdf -I$(MOZ_INC)$/msgbase \
        -I$(MOZ_INC)$/addrbook -I$(MOZ_INC)$/mork -I$(MOZ_INC)$/locale \
        -I$(MOZ_INC)$/pref -I$(MOZ_INC)$/mime -I$(MOZ_INC)$/chrome \
        -I$(MOZ_INC)$/necko -I$(MOZ_INC)$/intl -I$(MOZ_INC)$/profile \
        -I$(MOZ_INC)$/embed_base -I$(MOZ_INC)$/mozldap
CDEFS+=	    -DMOZILLA_CLIENT \
            -DOSTYPE=\"Linux2.2.14-5\" -DOJI
.IF "$(OS)" == "LINUX"
CFLAGS +=   -fPIC -g
CFLAGSCXX += \
            -fno-rtti -Wall -Wconversion -Wpointer-arith \
            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long -pthread
CDEFS     += -DTRACING
.ELIF "$(OS)" == "NETBSD"
CFLAGS +=   -fPIC
CFLAGSCXX += \
            -fno-rtti -Wall -Wconversion -Wpointer-arith \
            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
            -Wno-long-long
CDEFS     += -DTRACING
.ENDIF
.ENDIF

SHL1TARGET_NAME=$(TARGET)$(MOZAB_MAJOR)

.ENDIF # MACOSX

# --- Targets ----------------------------------

.INCLUDE : target.mk


killdpc:
    -+$(RM) $(DPCTARGET)
    -+$(RM) $(DEPFILES)
    @+echo Dependency files removed

