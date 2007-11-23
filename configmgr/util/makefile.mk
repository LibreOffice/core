#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: ihi $ $Date: 2007-11-23 14:50:36 $
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

PRJ=..
PRJNAME=configmgr
TARGET=configmgr

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk
DLLPRE = 

# --- Library -----------------------------------

SHL1TARGET=	$(CFGMGR_TARGET)$(CFGMGR_MAJOR).uno
SHL1VERSIONMAP= $(TARGET).map

SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(COMPHELPERLIB)			\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALHELPERLIB)				\
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/registry.lib	\
            $(SLB)$/treecache.lib	\
            $(SLB)$/misc.lib		\
            $(SLB)$/backend.lib		\
            $(SLB)$/localbe.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/treemgr.lib		\
            $(SLB)$/api2.lib		\
            $(SLB)$/api.lib			\
            $(SLB)$/data.lib		\
            $(SLB)$/cm.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

