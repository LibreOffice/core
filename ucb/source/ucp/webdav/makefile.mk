#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.42 $
#
#   last change: $Author: rt $ $Date: 2008-02-19 12:36:04 $
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

# UCP Version - Increase, if UCP libraray becomes incompatible.
UCP_VERSION=1

# Name for the UCP. Will become part of the library name (See below).
UCP_NAME=dav

# Relative path to project root.
PRJ = ..$/..$/..

# Project Name.
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

.IF "$(DISABLE_NEON)" == "TRUE"

@all:
    @echo "neon disabled...."

.ELSE

NEONINCDIR=external$/neon

.IF "$(SYSTEM_NEON)" != "YES"
.INCLUDE: $(SOLARINCDIR)$/$(NEONINCDIR)$/version.mk
.ENDIF

CFLAGS+= -DNEON_VERSION=0x$(NEON_VERSION)

.IF "$(SYSTEM_NEON)" == "YES"
CFLAGS+= $(NEON_CFLAGS)
.ELSE
CFLAGS+= -I$(SOLARINCDIR)$/$(NEONINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_OPENSSL)" == "YES"
CFLAGS+= $(OPENSSL_CFLAGS)
.ENDIF

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/webdavservices.obj    	\
    $(SLO)$/webdavprovider.obj    	\
    $(SLO)$/webdavcontent.obj     	\
    $(SLO)$/webdavcontentcaps.obj	\
    $(SLO)$/webdavresultset.obj     \
    $(SLO)$/webdavdatasupplier.obj	\
    $(SLO)$/ContentProperties.obj	\
    $(SLO)$/DAVProperties.obj	\
    $(SLO)$/DAVSessionFactory.obj	\
    $(SLO)$/DAVResourceAccess.obj	\
    $(SLO)$/NeonUri.obj		\
    $(SLO)$/NeonInputStream.obj	\
    $(SLO)$/NeonPropFindRequest.obj	\
    $(SLO)$/NeonHeadRequest.obj     \
    $(SLO)$/NeonSession.obj 	\
    $(SLO)$/DateTimeHelper.obj	\
    $(SLO)$/LinkSequence.obj	\
    $(SLO)$/LockSequence.obj	\
    $(SLO)$/LockEntrySequence.obj	\
    $(SLO)$/UCBDeadPropertyValue.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=exports.map

SHL1STDLIBS=\
        $(CPPUHELPERLIB)        \
        $(CPPULIB)              \
        $(SALLIB)               \
        $(SALHELPERLIB)         \
        $(UCBHELPERLIB)         \
        $(COMPHELPERLIB)		\
        $(NEON3RDLIB)           \
        $(LIBXML2LIB)



.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(WSOCK32LIB)
.IF "$(WINDOWS_VISTA_PSDK)" != ""
SHL1STDLIBS+= $(WS2_32LIB)
.ENDIF
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.IF "$(OS)"=="OS2"
SHL1STDLIBS+= pthread.lib libz.lib
.ENDIF # OS2
.IF "$(SYSTEM_OPENSSL)"=="YES"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
.ENDIF
.ENDIF # WNT



SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.ENDIF #"$(DISABLE_NEON)" == "TRUE"

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

