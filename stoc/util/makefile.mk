#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 13:09:18 $
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

PRJNAME= bootstrap
TARGET = bootstrap.uno
ENABLE_EXCEPTIONS=TRUE

# --- openoffice.org.orig/Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

SHL1TARGET=	$(TARGET)

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/bootstrap.lib \
        $(SLB)$/security.lib \
        $(SLB)$/servicemgr.lib \
        $(SLB)$/simplereg.lib \
        $(SLB)$/nestedreg.lib \
        $(SLB)$/implreg.lib \
        $(SLB)$/shlibloader.lib \
        $(SLB)$/regtypeprov.lib \
        $(SLB)$/typemgr.lib

SHL1VERSIONMAP = bootstrap.map

SHL1STDLIBS= \
        $(CPPULIB)		\
                $(SALHELPERLIB)         \
        $(CPPUHELPERLIB)	\
                $(SALLIB)		\
        $(REGLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

SHL2TARGET=	stocservices.uno

LIB2TARGET=	$(SLB)$/stocservices.uno.lib
LIB2FILES=	\
        $(SLB)$/stocservices.lib \
        $(SLB)$/typeconverter.lib \
        $(SLB)$/uriproc.lib 

SHL2VERSIONMAP = stocservices.map

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
                $(SALLIB)		

SHL2DEPN=
SHL2IMPLIB=	istocservices.uno
SHL2LIBS=	$(SLB)$/stocservices.uno.lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)

# --- openoffice.org.orig/Targets ------------------------------------------------------

.INCLUDE :	target.mk

