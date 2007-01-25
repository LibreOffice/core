#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:35:43 $
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

PRJNAME=FlatXml
TARGET=FlatXml
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(GUI)"=="WNT"
DLLPOSTFIX=.dll
DLLPREFIX=
.ELSE
DLLPOSTFIX=.so
DLLPREFIX=lib
.ENDIF

# --- Files --------------------------------------------------------
ALLIDLFILES = test_bridge.idl
#CPPUMAKERFLAGS += -C

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb 

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/examples

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

SLOFILES = $(SLO)$/FlatXml.obj

SHL1TARGET= FlatXml

SHL1STDLIBS= \
        $(SALLIB)	\
        $(VOSLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(SHL1TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR :	$(MISC)$/component_officeclient_registered.flag

.IF "$(GUI)"=="WNT"
PATHPREFIX = .
CURRENTDIR = $(BIN)
.ELSE
PATHPREFIX = ..$/bin
CURRENTDIR = $(LB)
.ENDIF

$(MISC)$/component_officeclient_registered.flag .SETDIR=$(CURRENTDIR): 
    -rm -f ..$/misc$/component_officeclient_registered.flag
    $(COPY) $(SOLARBINDIR)$/applicat.rdb $(PATHPREFIX)$/officeclient.rdb
    
