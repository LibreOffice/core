#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:03:22 $
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

PRJNAME=registry
TARGET=regtest
TARGETTYPE=CUI
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

RGTLIB = rgt.lib
.IF "$(GUI)"=="UNX" || "$(GUI)"=="OS2"
RGTLIB = -lrgt$(DLLPOSTFIX)
.ENDIF


CXXFILES= 	regtest.cxx   	\
            test.cxx		\
            regspeed.cxx


APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regtest.obj 

APP1STDLIBS=\
            $(SALLIB)	\
            $(REGLIB)

APP2TARGET= test
APP2OBJS=   $(OBJ)$/test.obj

APP2STDLIBS=\
            $(RGTLIB)

APP3TARGET= regspeed
APP3OBJS=   $(OBJ)$/regspeed.obj

APP3STDLIBS=\
            $(SALLIB)	\
            $(REGLIB) 	 

.INCLUDE :  target.mk
