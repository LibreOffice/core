#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 05:20:59 $
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

PRJNAME=remotebridges
TARGET=officeclientsample
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------
ALLIDLFILES = test_bridge.idl
#CPPUMAKERFLAGS += -C


UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb 

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/examples

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

UNOTYPES = \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.uno.XNamingService \
        com.sun.star.uno.XWeak \
        com.sun.star.lang.XMain \
        com.sun.star.uno.XAggregation \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.frame.XComponentLoader\
        com.sun.star.registry.XSimpleRegistry \
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.lang.XComponent \
        com.sun.star.bridge.XBridgeFactory\
        com.sun.star.bridge.XUnoUrlResolver\
        com.sun.star.connection.XAcceptor\
        com.sun.star.connection.XConnector\
        com.sun.star.lang.XServiceInfo \
         com.sun.star.text.XTextDocument

SLOFILES = $(SLO)$/officeclient.obj

SHL1TARGET= officeclientsample

SHL1STDLIBS= \
        $(SALLIB)	\
        $(VOSLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1LIBS=		$(SLB)$/$(SHL1TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

