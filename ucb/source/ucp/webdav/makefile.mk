#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: vg $ $Date: 2003-07-25 11:39:54 $
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

.INCLUDE: svpre.mk
.INCLUDE: settings.mk
.INCLUDE: sv.mk

NEONINCDIR=external$/neon
CFLAGS+= -I$(SOLARINCDIR)$/$(NEONINCDIR)

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
    $(SLO)$/DAVResourceAccessThread.obj	\
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
.IF "$(OS)"=="MACOSX"
.ELSE
SHL1VERSIONMAP=exports.map
.ENDIF

SHL1STDLIBS=\
        $(CPPUHELPERLIB)        \
        $(CPPULIB)              \
        $(SALLIB)               \
        $(SALHELPERLIB)         \
        $(VOSLIB)               \
        $(UCBHELPERLIB)         \
        $(NEON3RDLIB)           \
    $(EXPATASCII3RDLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= wsock32.lib
.ENDIF # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket
.ENDIF # SOLARIS

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=$(TARGET)$(UCP_VERSION)
.ENDIF

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

DEF1DES=UCB WebDAV Content Provider

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

