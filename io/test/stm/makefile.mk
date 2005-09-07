#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:33:31 $
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

PRJNAME=io
TARGET=teststm
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
# --- Files --------------------------------------------------------
UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

UNOTYPES =	com.sun.star.test.XSimpleTest \
        com.sun.star.beans.XPropertySet \
        com.sun.star.io.UnexpectedEOFException	\
        com.sun.star.io.WrongFormatException	\
        com.sun.star.io.XActiveDataControl	\
        com.sun.star.io.XActiveDataSink	\
        com.sun.star.io.XActiveDataSource	\
        com.sun.star.io.XConnectable	\
        com.sun.star.io.XMarkableStream	\
        com.sun.star.io.XObjectInputStream	\
        com.sun.star.io.XObjectOutputStream	\
        com.sun.star.lang.IllegalArgumentException	\
        com.sun.star.lang.XComponent	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XServiceInfo	\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.lang.XMultiComponentFactory	\
        com.sun.star.uno.XComponentContext	\
        com.sun.star.lang.XTypeProvider	\
        com.sun.star.registry.XImplementationRegistration	\
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.test.XSimpleTest	\
        com.sun.star.uno.TypeClass	\
        com.sun.star.uno.XAggregation	\
        com.sun.star.uno.XWeak

SLOFILES=	 \
        $(SLO)$/testfactreg.obj \
        $(SLO)$/pipetest.obj \
        $(SLO)$/datatest.obj \
        $(SLO)$/marktest.obj \
        $(SLO)$/pumptest.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS= \
        $(SALLIB) 	 \
        $(CPPULIB) \
        $(CPPUHELPERLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib

SHL1IMPLIB=		i$(TARGET)

SHL1DEPN=		makefile.mk $(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
