#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 04:44:27 $
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
.IF "$(OS)"=="MACOSX"
#SHL1VERSIONMAP= $(TARGET).$(DLLPOSTFIX).map
.ELSE
SHL1VERSIONMAP= $(TARGET).map
.ENDIF
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(COMPHELPERLIB)			\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALHELPERLIB)				\
    $(SALLIB)					\
    $(CPPRTLLIB)

#	$(UNOTOOLSLIB)				\
#	$(ZLIB3RDLIB) \

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

# --- Library -----------------------------------
.IF "$(SVXLIGHT)" != ""

SHL2TARGET=	$(CFGMGR_TARGET)$(CFGMGR_MAJOR)l
.IF "$(OS)"=="MACOSX"
#SHL2VERSIONMAP= $(TARGET).$(DLLPOSTFIX).map
.ELSE
SHL2VERSIONMAP= $(TARGET)l.map
.ENDIF
SHL2OBJS=$(SLOFILES)
SHL2STDLIBS=\
    $(COMPHELPERLIB)			\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALHELPERLIB)				\
    $(SALLIB)					\
    $(CPPRTLLIB)

#	$(UNOTOOLSLIB)				\

SHL2DEPN=
SHL2IMPLIB=	i$(SHL2TARGET)
SHL2LIBS=	$(SLB)$/registry.lib	\
            $(SLB)$/treecache.lib	\
            $(SLB)$/misc.lib		\
            $(SLB)$/xml.lib			\
            $(SLB)$/treemgr.lib		\
            $(SLB)$/api2.lib		\
            $(SLB)$/api.lib			\
            $(SLB)$/data.lib		\
            $(SLB)$/cm.lib

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

