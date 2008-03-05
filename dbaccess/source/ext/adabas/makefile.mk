#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2008-03-05 08:44:12 $
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

PRJNAME=dbaccess
TARGET=adabasui
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE :	settings.mk

# --- resource -----------------------------------------------------
SRS1NAME=$(TARGET)
SRC1FILES =	\
        AdabasNewDb.src

# --- defines ------------------------------------------------------
CDEFS+=-DCOMPMOD_NAMESPACE=adabasui

# --- Files -------------------------------------
SLOFILES=\
        $(SLO)$/ASqlNameEdit.obj		\
        $(SLO)$/AdabasNewDb.obj			\
        $(SLO)$/ANewDb.obj				\
        $(SLO)$/adabasuistrings.obj		\
        $(SLO)$/Aservices.obj			\
        $(SLO)$/Acomponentmodule.obj

# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(SVTOOLLIB)				\
    $(TOOLSLIB)					\
    $(SVLLIB)					\
    $(VCLLIB)					\
    $(UCBHELPERLIB) 			\
    $(SFXLIB)					\
    $(TKLIB)					\
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
SHL1VERSIONMAP=$(TARGET).map

# --- .res file ----------------------------------------------------------
RES1FILELIST=\
    $(SRS)$/$(SRS1NAME).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- Targets ----------------------------------

.INCLUDE : target.mk

