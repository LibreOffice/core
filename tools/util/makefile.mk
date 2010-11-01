#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=TOOLS
TARGET=tools
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

# --- STDSTRM.LIB ---
LIB3TARGET= $(LB)$/stdstrm.lib
LIB3ARCHIV= $(LB)$/libstdstrm.a
LIB3FILES=  $(LB)$/stream.lib

LIB7TARGET= $(LB)$/a$(TARGET).lib
LIB7ARCHIV= $(LB)$/liba$(TARGET).a
LIB7FILES=  $(LB)$/gen.lib          \
            $(LB)$/str.lib          \
            $(LB)$/mtools.lib       \
            $(LB)$/datetime.lib     \
            $(LB)$/fsys.lib         \
            $(LB)$/communi.lib      \
            $(LB)$/stream.lib       \
            $(LB)$/ref.lib          \
            $(LB)$/rc.lib           \
            $(LB)$/inet.lib         \
            $(LB)$/debug.lib


LIB7FILES+= $(LB)$/dll.lib

# --- TOOLS.LIB ---
LIB1TARGET:= $(SLB)$/$(TARGET).lib
LIB1FILES+=  \
            $(SLB)$/gen.lib         \
            $(SLB)$/str.lib         \
            $(SLB)$/mtools.lib      \
            $(SLB)$/datetime.lib    \
            $(SLB)$/fsys.lib        \
            $(SLB)$/communi.lib     \
            $(SLB)$/stream.lib      \
            $(SLB)$/ref.lib         \
            $(SLB)$/rc.lib          \
            $(SLB)$/debug.lib       \
            $(SLB)$/zcodec.lib      \
            $(SLB)$/inet.lib        \
            $(SLB)$/testtoolloader.lib \
            $(SLB)$/misc.lib

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS += $(CPPULIB)	 \
            $(ZLIB3RDLIB)
.ELSE
SHL1STDLIBS += $(ZLIB3RDLIB) \
            $(CPPULIB)
.ENDIF

LIB1FILES+=  $(SLB)$/dll.lib


.IF "$(BIG_TOOLS)"!=""
.IF "$(GUI)"=="WNT"
#SOLARLIBDIR=$(SOLARVER)\$((INPATH)\lib
#SOLARLIBDIR=..\$(INPATH)\lib
# bei lokalen osl rtl oder vos das SOLARLIBDIR bitte patchen !
LIB1FILES+= $(SOLARLIBDIR)\xosl.lib \
            $(SOLARLIBDIR)\xrtl.lib

SHL1STDLIBS+=   $(WSOCK32LIB)
.ENDIF
.ENDIF          # "$(BIG_TOOLS)"!=""

# --- TOOLS.DLL ---

SHL1TARGET=     tl$(DLLPOSTFIX)
SHL1LIBS=       $(LIB1TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=     itools
SHL1USE_EXPORTS=name
SHL1STDLIBS+=   $(SALLIB)  $(BASEGFXLIB) $(I18NISOLANGLIB) $(COMPHELPERLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=   $(SHELL32LIB)     \
                $(MPRLIB)         \
                $(OLE32LIB)       \
                $(UUIDLIB)        \
                $(ADVAPI32LIB)
.ENDIF

DEF1NAME        =$(SHL1TARGET)
DEF1DEPN = \
        $(MISC)$/$(SHL1TARGET).flt      \
        $(HXX1FILES)                    \
        $(HXX2FILES)                    \
        $(HXX3FILES)                    \
        $(HXX4FILES)                    \
        $(HXX5FILES)                    \
        $(HXX6FILES)                    \
        $(HXX7FILES)                    \
        $(HXX8FILES)                    \
        $(HXX9FILES)                    \
        $(HXX10FILES)                   \
        $(HXX11FILES)                   \
        $(HXX12FILES)                   \
        $(HXX13FILES)                   \
        $(HXX14FILES)                   \
        $(HXX15FILES)                   \
        $(HXX16FILES)                   \
        $(HXX17FILES)                   \
        $(HXX18FILES)                   \
        $(HXX19FILES)                   \
        $(HXX20FILES)                   \
        makefile.mk

DEFLIB1NAME =tools

# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk

# --- TOOLS.FLT ---
$(MISC)$/$(SHL1TARGET).flt:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Imp>$@
    @echo PointerList>>$@
    @echo DbgCheck>>$@
    @echo LabelList>>$@
    @echo ActionList>>$@
    @echo CBlock>>$@
    @echo DirEntryStack>>$@
    @echo readdir>>$@
    @echo closedir>>$@
    @echo opendir>>$@
    @echo volumeid>>$@
    @echo MsDos2Time>>$@
    @echo MsDos2Date>>$@
    @echo __new_alloc>>$@
    @echo __CT>>$@
    @echo unnamed>>$@
.IF "$(COM)"=="BLC"
    @echo WEP>>$@
.ENDIF
