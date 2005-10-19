#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-10-19 12:48:09 $
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

PRJNAME=package
TARGET=xstor

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/ocompinstream.obj\
        $(SLO)$/oseekinstream.obj\
        $(SLO)$/owriteablestream.obj\
        $(SLO)$/xstorage.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/disposelistener.obj\
        $(SLO)$/switchpersistencestream.obj\
        $(SLO)$/register.obj

EXCEPTIONSFILES= \
        $(SLO)$/ocompinstream.obj\
        $(SLO)$/oseekinstream.obj\
        $(SLO)$/owriteablestream.obj\
        $(SLO)$/xstorage.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/disposelistener.obj\
        $(SLO)$/switchpersistencestream.obj\
        $(SLO)$/register.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(SALLIB) 	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

