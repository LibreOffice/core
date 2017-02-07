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



# --- Unix-Environment ---------------------------------------
# Used if "$(GUI)" == "UNX"


# Dieses Define gilt fuer alle  WNT- Plattformen
# fuer Compiler, wo kein Java unterstuetzt wird, muss dies ge'undeft werden
#JAVADEF=-DSOLAR_JAVA

# --- Compiler ---

.IF "$(COM)$(OS)$(CPU)" == "C50SOLARISS"
.INCLUDE : unxsols2.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "C50SOLARISI"
.INCLUDE : unxsoli2.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISS3"
.INCLUDE : unxsols3.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISI3"
.INCLUDE : unxsoli3.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISS4"
.INCLUDE : unxsols4.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISI4"
.INCLUDE : unxsoli4.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISU4"
.INCLUDE : unxsolu4.mk
.ENDIF

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISX4"
.INCLUDE : unxsolx4.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISS"
.INCLUDE : unxsogs.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISI"
.INCLUDE : unxsogi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSCOI"
.INCLUDE : unxscoi.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXS"
.INCLUDE : unxlngs.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXI"
.INCLUDE : unxlngi.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXX"
.INCLUDE : unxlngx.mk
.ENDIF

.IF "$(OS)$(CPU)$(CPUNAME)" == "LINUXPPOWERPC"
.INCLUDE : unxlngppc.mk
.ENDIF

.IF "$(OS)$(CPU)$(CPUNAME)" == "LINUXPPOWERPC64"
.INCLUDE : unxlngppc64.mk
.ENDIF

.IF "$(OS)$(CPU)$(CPUNAME)" == "LINUX3S390"
.INCLUDE : unxlngs390.mk
.ENDIF

.IF "$(OS)$(CPU)$(CPUNAME)" == "LINUX3S390X"
.INCLUDE : unxlngs390x.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXR"
.INCLUDE : unxlngr.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXA"
.INCLUDE : unxlnga.mk
.ENDIF

# FreeBSD includes the specific platforms in its makefile
.IF "$(OS)" == "FREEBSD"
.INCLUDE : unxfbsd.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "ACCHPUXR"
.INCLUDE : unxhpxr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCHPUXR"
.INCLUDE : unxhpgr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCNETBSDA"
.INCLUDE : unxbsda.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "ICCAIXP"
.INCLUDE : unxaixp.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC295NETBSDI"
.INCLUDE : unxbsdi.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC300NETBSDI"
.INCLUDE : unxbsdi2.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCNETBSDS"
.INCLUDE : unxbsds.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXP"
.INCLUDE : unxmacxp.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXI"
.INCLUDE : unxmacxi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "CLANGMACOSXI"
.INCLUDE : unxmacci.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "CLANGMACOSXX"
.INCLUDE : unxmaccx.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXM"
.INCLUDE : unxlngmips.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUX6"
.INCLUDE : unxlngm68k.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXH"
.INCLUDE : unxlnghppa.mk
.ENDIF

.IF "$(OS)$(CPU)" == "LINUXL"
.INCLUDE : unxlngaxp.mk
.ENDIF

# --- general *ix settings ---
HC=hc
HCFLAGS=
PATH_SEPERATOR*=:
CDEFS+=-D__DMAKE

CDEFS+=-DUNIX

# fuer linux: bison -y -d
YACC*=yacc
YACCFLAGS*=-d -t

EXECPOST=
SCPPOST=.ins
DLLDEST=$(LB)
SOLARSHAREDBIN=$(SOLARLIBDIR)
SONAME_SWITCH*=-h

.IF "$(UNIXVERSIONNAMES)"!=""
DLLPOST!:=$(DLLPOST).$($(UNIXVERSIONNAMES)_MAJOR)
.ENDIF			# "$(UNIXVERSIONNAMES)"!=""

# enable building/linking KDE-dependent code in both OOo and SO build environment
.IF "$(ENABLE_KDE)" != ""
.IF "$(KDE_ROOT)"!=""
KDE_CFLAGS:=-I$(KDE_ROOT)/include -DQT_CLEAN_NAMESPACE
KDE_LIBS:=-L$(KDE_ROOT)/lib -lkdeui -lkdecore -lqt-mt
.IF "$(OS)$(CPU)" == "LINUXX"
SOLARLIB+=-L$(KDE_ROOT)/lib64
.ENDIF          # "$(OS)$(CPU)" == "LINUXX"
.ENDIF          # "$(KDE_ROOT)"!=""
.ENDIF          # "$(ENABLE_KDE)" != ""

OOO_LIBRARY_PATH_VAR *= LD_LIBRARY_PATH
