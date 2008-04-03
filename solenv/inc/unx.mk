#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unx.mk,v $
#
#   $Revision: 1.37 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 16:45:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISS"
.INCLUDE : unxsogs.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSOLARISI"
.INCLUDE : unxsogi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCSCOI"
.INCLUDE : unxscoi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXI"
.INCLUDE : unxlnxi.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXP2"
.INCLUDE : unxlngp.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCLINUXS"
.INCLUDE : unxlngs.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC295LINUXI2REDHAT60"
.INCLUDE : unxlngi3.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC300LINUXI2REDHAT60"
.INCLUDE : unxlngi4.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC322LINUXI2REDHAT60"
.INCLUDE : unxlngi5.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC341LINUXI2REDHAT60"
.INCLUDE : unxlngi6.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC341LINUXX2REDHAT60"
.INCLUDE : unxlngx6.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)" == "GCCC295LINUXP2REDHAT60"
.INCLUDE : unxlngppc.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)$(CPUNAME)" == "GCCC300LINUXP2REDHAT60POWERPC"
.INCLUDE : unxlngppc4.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)$(CPUNAME)" == "GCCC300LINUXP2REDHAT60POWERPC64"
.INCLUDE : unxlngppc64.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)$(CPUNAME)" == "GCCC300LINUX32REDHAT60S390"
.INCLUDE : unxlngs3904.mk
.ENDIF

.IF "$(COM)$(CVER)$(OS)$(CPU)$(GLIBC)$(CPUNAME)" == "GCCC300LINUX32REDHAT60S390X"
.INCLUDE : unxlngs390x.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXR2REDHAT60"
.INCLUDE : unxlngr.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)$(GLIBC)" == "GCCLINUXA2REDHAT60"
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

.IF "$(COM)$(OS)$(CPU)" == "C730IRIXM"
.INCLUDE : unxirxm3.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCIRIXM"
.INCLUDE : unxirgm.mk
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "C710IRIXM"
.INCLUDE : unxirxm.mk
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
KDE_CFLAGS:=-I$(KDE_ROOT)$/include -DQT_CLEAN_NAMESPACE
KDE_LIBS:=-lkdeui -lkdecore -lqt-mt
SOLARLIB+=-L$(KDE_ROOT)$/lib
.ENDIF          # "$(KDE_ROOT)"!=""
.ENDIF          # "$(ENABLE_KDE)" != ""
