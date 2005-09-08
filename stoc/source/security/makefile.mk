#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:10:14 $
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
PRJ=..$/..

PRJNAME=stoc
TARGET = security.uno
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC = TRUE
BOOTSTRAP_SERVICE=TRUE
COMP1TYPELIST = sec

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES= \
        $(SLO)$/permissions.obj			\
        $(SLO)$/access_controller.obj		\
        $(SLO)$/file_policy.obj

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(CPPULIB)		\
        $(SALLIB)
SHL1VERSIONMAP = sec.map

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.IF "$(debug)" != ""

# some diagnose
.IF "$(diag)" == "full"
CFLAGS += -D__DIAGNOSE -D__CACHE_DIAGNOSE
.ELIF "$(diag)" == "cache"
CFLAGS += -D__CACHE_DIAGNOSE
.ELIF "$(diag)" != ""
CFLAGS += -D__DIAGNOSE
.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

