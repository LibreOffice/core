#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2006-10-05 10:45:56 $
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

PRJNAME=	sal
TARGET=		unloading
TARGET1=samplelib1
TARGET2=samplelib2
#LIBTARGET=NO
TARGETTYPE=CUI
COMP1TYPELIST=$(TARGET1)

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

CPPUMAKERFLAGS =

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/bootstrap
INCPRE+=	$(OUT)$/inc$/bootstrap
.ELSE
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF
UNOUCROUT=	$(OUT)$/inc$/light
INCPRE+=	$(OUT)$/inc$/light
.ENDIF


#SLOFILES=	\
#		$(SLO)$/samplelib1.obj	\
#		$(SLO)$/samplelib2.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelib1.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)



SHL1DEPN=
#SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp
DEF1NAME=	$(SHL1TARGET)
#-------------------------------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2OBJFILES= \
        $(SLO)$/samplelib2.obj

SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=	$(SLB)$/$(TARGET2).lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2EXPORTFILE=	exports.dxp
DEF2NAME=	$(SHL2TARGET)

# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/unloadTest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)


LIBCIMT=MSVCPRTD.lib


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

