#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXS"
.INCLUDE : unxlngs.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXI"
.INCLUDE : unxlngi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXX"
.INCLUDE : unxlngx.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(CPUNAME)" == "GCCLINUXPPOWERPC"
.INCLUDE : unxlngppc.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(CPUNAME)" == "GCCLINUXPPOWERPC64"
.INCLUDE : unxlngppc64.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCAIXP"
.INCLUDE : unxaigppc.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(CPUNAME)" == "GCCLINUX3S390"
.INCLUDE : unxlngs390.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(CPUNAME)" == "GCCLINUX3S390X"
.INCLUDE : unxlngs390x.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXR"
.INCLUDE : unxlngr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXA"
.INCLUDE : unxlnga.mk
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

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC295NETBSDI"
.INCLUDE : unxbsdi.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC300NETBSDI"
.INCLUDE : unxbsdi2.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC341NETBSDI"
.INCLUDE : unxbsdi3.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)" == "GCCC341NETBSDX"
.INCLUDE : unxbsdx3.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCNETBSDS"
.INCLUDE : unxbsds.mk
.ENDIF

.IF "$(COM)$(OS)" == "GCCFREEBSD"
.INCLUDE : unxfbsd.mk
.ENDIF

.IF "$(COM)$(OS)" == "GCCOPENBSD"
.INCLUDE : unxobsd.mk
.ENDIF

.IF "$(COM)$(OS)" == "GCCDRAGONFLY"
.INCLUDE : unxdfly.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXP"
.INCLUDE : unxmacxp.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXI"
.INCLUDE : unxmacxi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXM"
.INCLUDE : unxlngmips.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUX6"
.INCLUDE : unxlngm68k.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXH"
.INCLUDE : unxlnghppa.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXL"
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
KDE_LIBS:=-lkdeui -lkdecore -lqt-mt
SOLARLIB+=-L$(KDE_ROOT)/lib
.ENDIF          # "$(KDE_ROOT)"!=""
.ENDIF          # "$(ENABLE_KDE)" != ""

OOO_LIBRARY_PATH_VAR *= LD_LIBRARY_PATH
