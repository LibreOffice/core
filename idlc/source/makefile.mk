#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..

PRJNAME=idlc
TARGET=idlc
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE
INCPRE=$(MISC)

.IF "$(GUI)" == "OS2"
STL_OS2_BUILDING=1
.ENDIF

# --- Settings -----------------------------------------------------


.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CXXFILES=   \
            wrap_scanner.cxx \
            wrap_parser.cxx	\
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
            astexpression.cxx \
            astservice.cxx

YACCTARGET=$(MISC)$/parser.cxx
YACCFILES=parser.y

OBJFILES=   \
            $(OBJ)$/wrap_scanner.obj	\
            $(OBJ)$/wrap_parser.obj	\
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
            $(OBJ)$/astexpression.obj \
            $(OBJ)$/astservice.obj

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
    tr -d "\015" < scanner.ll > $(MISC)$/stripped_scanner.ll
.ELSE
    cat scanner.ll > $(MISC)$/stripped_scanner.ll
.ENDIF

$(MISC)$/scanner.cxx:	$(MISC)$/stripped_scanner.ll
    flex -o$(MISC)$/scanner.cxx $(MISC)$/stripped_scanner.ll

$(OBJ)$/wrap_parser.obj: $(MISC)$/parser.cxx
$(OBJ)$/wrap_scanner.obj: $(MISC)$/scanner.cxx
