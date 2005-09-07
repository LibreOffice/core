#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:12:29 $
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

PRJNAME=idlc
TARGET=idlc
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------


.INCLUDE :  settings.mk


# --- Files --------------------------------------------------------

CXXFILES=   \
            $(MISC)$/scanner.cxx \
            $(MISC)$/parser.cxx	\
            idlcmain.cxx	\
            idlc.cxx	\
            idlccompile.cxx	\
            idlcproduce.cxx	\
            errorhandler.cxx	\
            options.cxx	\
            fehelper.cxx	\
            astdeclaration.cxx \
            astscope.cxx \
            aststack.cxx \
            astdump.cxx \
            astinterface.cxx \
            aststruct.cxx \
            aststructinstance.cxx \
            astoperation.cxx \
            astconstant.cxx \
            astenum.cxx \
            astarray.cxx \
            astunion.cxx \
            astexpression.cxx

YACCTARGET=$(MISC)$/parser.cxx
YACCFILES=parser.y

OBJFILES=   \
            $(OBJ)$/scanner.obj	\
            $(OBJ)$/parser.obj	\
            $(OBJ)$/idlcmain.obj	\
            $(OBJ)$/idlc.obj	\
            $(OBJ)$/idlccompile.obj	\
            $(OBJ)$/idlcproduce.obj	\
            $(OBJ)$/errorhandler.obj	\
            $(OBJ)$/options.obj	\
            $(OBJ)$/fehelper.obj	\
            $(OBJ)$/astdeclaration.obj	\
            $(OBJ)$/astscope.obj	\
            $(OBJ)$/aststack.obj	\
            $(OBJ)$/astdump.obj	\
            $(OBJ)$/astinterface.obj	\
            $(OBJ)$/aststruct.obj	\
            $(OBJ)$/aststructinstance.obj \
            $(OBJ)$/astoperation.obj	\
            $(OBJ)$/astconstant.obj	\
            $(OBJ)$/astenum.obj	\
            $(OBJ)$/astarray.obj	\
            $(OBJ)$/astunion.obj	\
            $(OBJ)$/astexpression.obj

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJFILES)

APP1STDLIBS = \
    $(REGLIB) \
    $(SALLIB) \
    $(SALHELPERLIB)

# --- Targets ------------------------------------------------------

.IF "$(debug)" == ""
YACCFLAGS+=-l
.ELSE
YACCFLAGS+=-v
.ENDIF
    
.INCLUDE :  target.mk

$(MISC)$/stripped_scanner.ll : scanner.ll
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)" != "4nt"
    +tr -d "\015" < scanner.ll > $(MISC)$/stripped_scanner.ll
.ELSE
    +cat scanner.ll > $(MISC)$/stripped_scanner.ll
.ENDIF

$(MISC)$/scanner.cxx:	$(MISC)$/stripped_scanner.ll
    +flex -o$(MISC)$/scanner.cxx $(MISC)$/stripped_scanner.ll


