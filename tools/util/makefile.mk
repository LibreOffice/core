#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.25 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:22:46 $
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
SHL1STDLIBS += $(UCBHELPERLIB) \
            $(CPPULIB)	 \
            $(ZLIB3RDLIB)
.ELSE
SHL1STDLIBS += $(ZLIB3RDLIB) \
            $(UCBHELPERLIB) \
            $(CPPULIB)
.ENDIF

LIB1FILES+=  $(SLB)$/dll.lib


.IF "$(BIG_TOOLS)"!=""
.IF "$(GUI)"=="WNT"
#SOLARLIBDIR=$(SOLARVER)\$((INPATH)\lib
#SOLARLIBDIR=..\$(INPATH)\lib
# bei lokalen osl rtl oder vos das SOLARLIBDIR bitte patchen !
LIB1FILES+= $(SOLARLIBDIR)\xosl.lib \
            $(SOLARLIBDIR)\xrtl.lib \
            $(SOLARLIBDIR)\xvos.lib
SHL1STDLIBS+=   $(WSOCK32LIB)
.ENDIF
.ENDIF          # "$(BIG_TOOLS)"!=""

# --- TOOLS.DLL ---

SHL1TARGET=     tl$(DLLPOSTFIX)
SHL1LIBS=       $(LIB1TARGET)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=     itools
SHL1USE_EXPORTS=name
SHL1STDLIBS+=   $(SALLIB) $(VOSLIB) $(BASEGFXLIB) $(I18NISOLANGLIB)

.IF "$(WITH_LIBART)"=="YES"
SHL1STDLIBS+=   $(LIBART_LIBS)
.ELSE
SHL1STDLIBS+=   $(GPC3RDLIB)
.ENDIF


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
