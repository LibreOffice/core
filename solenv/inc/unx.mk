#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unx.mk,v $
#
# $Revision: 1.40 $
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

.IF "$(COM)$(OS)" == "GCCFREEBSD"
.INCLUDE : unxfbsd.mk
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

# Add SOLARLIBDIR to the end of a (potentially previously undefined)
# LD_LIBRARY_PATH (there is no real reason to prefer adding at the end over
# adding at the start); the ": &&" in the bash case enables this to work at the
# start of a recipe line that is not prefixed by "+" as well as in the middle of
# an existing && chain; the tcsh case is somewhat imprecise in that it
# potentially affects multiple commands following on the recipe line:
.IF "$(USE_SHELL)" == "bash"
AUGMENT_LIBRARY_PATH *= : && \
    LD_LIBRARY_PATH=$${{LD_LIBRARY_PATH+$${{LD_LIBRARY_PATH}}:}}$(SOLARLIBDIR)
.ELSE
AUGMENT_LIBRARY_PATH *= if ($$?LD_LIBRARY_PATH == 1) \
    eval 'setenv LD_LIBRARY_PATH "$${{LD_LIBRARY_PATH}}:$(SOLARLIBDIR)"' && \
    if ($$?LD_LIBRARY_PATH == 0) setenv LD_LIBRARY_PATH "$(SOLARLIBDIR)" &&
.ENDIF
