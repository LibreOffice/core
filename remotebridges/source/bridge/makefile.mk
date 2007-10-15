#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2007-10-15 12:22:06 $
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

PRJNAME=remotebridges
TARGET = remotebridge.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = remotebridge

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
DLLPRE =
# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/remote_bridge.obj \
        $(SLO)$/bridge_connection.obj\
        $(SLO)$/bridge_provider.obj 
SHL1TARGET= $(TARGET)
SHL1VERSIONMAP = remotebridge.map

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        $(RMCXTLIB)

SHL1DEPN=
SHL1IMPLIB=		i$(TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=      URELIB

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk
