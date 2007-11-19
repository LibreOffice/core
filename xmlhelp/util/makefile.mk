#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: ihi $ $Date: 2007-11-19 13:01:28 $
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

UCP_VERSION=1
UCP_NAME=chelp

PRJ=..
PRJNAME=xmlhelp
TARGET=ucp$(UCP_NAME)
UCPHELP_MAJOR=1

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

.IF "$(GUI)"=="WNT"
CFLAGS+=-GR
.ENDIF

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=exports.map

# Add additional libs here.
SHL1STDLIBS=                     \
    $(CPPUHELPERLIB)         \
    $(CPPULIB)               \
    $(COMPHELPERLIB)         \
    $(SALLIB)                \
    $(EXPATASCII3RDLIB)      \
    $(UCBHELPERLIB)          \
    $(SVTOOLLIB)             	 \
    $(BERKELEYLIB)		 	 \
    $(XSLTLIB)

SHL1LIBS =                       \
    $(SLB)$/jaqe.lib         \
    $(SLB)$/jautil.lib       \
    $(SLB)$/jadb.lib         \
    $(SLB)$/chelp.lib

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

