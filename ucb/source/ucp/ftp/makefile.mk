#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: vg $ $Date: 2007-09-20 16:27:58 $
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

PRJ=..$/..$/..
PRJNAME=ucb
TARGET=ucpftp
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPFTP_MAJOR=1


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

.IF "$(SYSTEM_CURL)" == "YES"
CFLAGS+=-DCURL_NO_OLDIES
.ENDIF

# --- General -----------------------------------------------------

# first target ( shared library )

SLOFILES1=\
    $(SLO)$/ftpservices.obj  \
    $(SLO)$/ftpcontentprovider.obj  \
    $(SLO)$/ftpcontent.obj   \
    $(SLO)$/ftpcontentidentifier.obj   \
    $(SLO)$/ftpcontentcaps.obj \
    $(SLO)$/ftpdynresultset.obj  \
    $(SLO)$/ftpresultsetbase.obj \
    $(SLO)$/ftpresultsetI.obj \
    $(SLO)$/ftploaderthread.obj  \
    $(SLO)$/ftpinpstr.obj	\
    $(SLO)$/ftpdirp.obj     \
    $(SLO)$/ftpcfunc.obj     \
    $(SLO)$/ftpurl.obj     \
    $(SLO)$/ftpintreq.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES1)

# --- Shared-Library 1 ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCPFTP_MAJOR)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=	$(TARGET).map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(UCBHELPERLIB) \
    $(CURLLIB)

.IF "$(GUI)" == "OS2"
SHL1STDLIBS+=ssl.lib crypto.lib libz.lib
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS= \
    $(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.INCLUDE: target.mk













