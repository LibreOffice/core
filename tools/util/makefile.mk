#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:03:10 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=TOOLS
TARGET=tools
VERSION=$(UPD)

USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Allgemein ----------------------------------------------------

.IF "$(depend)" == ""

# --- TOOLS.HXX ---
HXX1FILES=	$(INC)$/debug.hxx		\
            $(INC)$/new.hxx 		\
            $(INC)$/shl.hxx 		\
            $(INC)$/link.hxx		\
            $(INC)$/fract.hxx		\
            $(INC)$/string.hxx		\
            $(INC)$/contnr.hxx		\
            $(INC)$/list.hxx		\
            $(INC)$/table.hxx		\
            $(INC)$/unqidx.hxx		\
            $(INC)$/unqid.hxx		\
            $(INC)$/mempool.hxx 	\
            $(INC)$/simplecm.hxx	\
            $(INC)$/stream.hxx		\
            $(INC)$/cachestr.hxx	\
            $(INC)$/errcode.hxx 	\
            $(INC)$/lang.hxx		\
            $(INC)$/ref.hxx 		\
            $(INC)$/tools.h
HXX2FILES=	$(INC)$/solar.h
HXX4FILES=	$(INC)$/stdstrm.hxx
HXX5FILES=	$(INC)$/ref.hxx
HXX6FILES=	$(INC)$/pstm.hxx  \
            $(INC)$/globname.hxx
HXX7FILES=	$(INC)$/rtti.hxx
HXX8FILES=	$(INC)$/solver.h
.IF "$(GUIBASE)" == "WIN"
HXX9FILES=	$(INCGUI)$/svwin.h		 \
            $(INCGUI)$/postwin.h \
            $(INCGUI)$/prewin.h
.ENDIF
.IF "$(GUI)" == "OS2"
HXX9FILES=	$(INCGUI)$/svpm.h
.ENDIF
HXX10FILES= $(INC)$/errinf.hxx $(INC)$/errcode.hxx $(INC)$/string.hxx
HXX11FILES= $(INC)$/urlobj.hxx
HXX12FILES= $(INC)$/bigint.hxx
HXX13FILES= $(INC)$/multisel.hxx
HXX14FILES= $(INC)$/date.hxx $(INC)$/time.hxx $(INC)$/datetime.hxx $(INC)$/intn.hxx $(INC)$/timestamp.hxx
HXX15FILES= $(INC)$/wldcrd.hxx $(INC)$/fsys.hxx
HXX16FILES= $(INC)$/queue.hxx $(INC)$/dynary.hxx $(INC)$/stack.hxx
HXX17FILES= $(INC)$/stream.hxx $(INC)$/cachestr.hxx
HXX18FILES= $(HXX1FILES)
HXX19FILES= $(INC)$/zcodec.hxx

.IF "$(UPDATER)"!=""
# --- STDSTRM.LIB ---
LIB3TARGET= $(LB)$/stdstrm.lib
LIB3ARCHIV= $(LB)$/libstdstrm.a
LIB3FILES=	$(LB)$/stream.lib
.ENDIF					# "$(UPDATER)"!=""

.IF "$(GUI)"!="UNX"
MEMMGRLIB=$(LB)$/memmgr.lib
MEMMGRSLB=$(SLB)$/memmgr.lib
.ELSE
MEMMGRLIB=
MEMMGRSLB=
.ENDIF

.IF "$(UPDATER)"!=""

LIB7TARGET= $(LB)$/a$(TARGET).lib
LIB7ARCHIV= $(LB)$/liba$(TARGET).a
LIB7FILES=	$(LB)$/gen.lib			\
            $(LB)$/str.lib			\
            $(LB)$/mtools.lib		\
            $(MEMMGRLIB)			\
            $(LB)$/datetime.lib 	\
            $(LB)$/timestamp.lib	\
            $(LB)$/intntl.lib		\
            $(LB)$/fsys.lib 		\
            $(LB)$/communi.lib		\
            $(LB)$/stream.lib		\
            $(LB)$/ref.lib			\
            $(LB)$/rc.lib			\
            $(LB)$/inet.lib			\
            $(LB)$/debug.lib


LIB7FILES+= $(LB)$/dll.lib

.ENDIF		# "$(UPDATER)"!=""

# --- TOOLS.LIB ---
LIB1TARGET:= $(SLB)$/$(TARGET).lib
LIB1FILES+=  \
            $(MEMMGRSLB)			\
            $(SLB)$/gen.lib 		\
            $(SLB)$/str.lib 		\
            $(SLB)$/mtools.lib		\
            $(SLB)$/datetime.lib	\
            $(SLB)$/timestamp.lib	\
            $(SLB)$/intntl.lib		\
            $(SLB)$/fsys.lib		\
            $(SLB)$/communi.lib 	\
            $(SLB)$/stream.lib		\
            $(SLB)$/ref.lib 		\
            $(SLB)$/rc.lib			\
            $(SLB)$/debug.lib		\
            $(SLB)$/zcodec.lib		\
            $(SLB)$/inet.lib

SHL1STDLIBS += $(ZLIB3RDLIB)

LIB1FILES+=  $(SLB)$/dll.lib


.IF "$(BIG_TOOLS)"!=""
.IF "$(GUI)"=="WNT"
#SOLARLIBDIR=$(SOLARVER)\$(UPD)\$(INPATH)\lib
#SOLARLIBDIR=..\$(INPATH)\lib
# bei lokalen osl rtl oder vos das SOLARLIBDIR bitte patchen !
LIB1FILES+= $(SOLARLIBDIR)\xosl.lib \
            $(SOLARLIBDIR)\xrtl.lib \
            $(SOLARLIBDIR)\xvos.lib
SHL1STDLIBS+=	wsock32.lib
.ENDIF
.ENDIF			# "$(BIG_TOOLS)"!=""

# --- TOOLS.DLL ---
SHL1TARGET= 	tl$(VERSION)$(DLLPOSTFIX)
#SHL1TARGET=	$(TOOLSDLLPREFIX)$(VERSION)$(DLLPOSTFIX)
SHL1LIBS=		$(LIB1TARGET)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB= 	itools
SHL1STDLIBS+=	$(SALLIB) $(VOSLIB)

.IF "$(GUI)" == "OS2"
SHL1OBJS=		$(SLO)\memmgr.obj
.ENDIF
.IF "$(GUI)"=="WIN"
SHL1OBJS=		$(SLO)\memmgr.obj
.ENDIF			# "$(GUI)"=="WIN"


.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=	shell32.lib 	\
                mpr.lib 		\
                ole32.lib		\
                uuid.lib		\
                advapi32.lib
.ENDIF

DEF1NAME		=$(SHL1TARGET)
DEF1DEPN = \
        $(MISC)$/$(SHL1TARGET).flt		\
        $(HXX1FILES)					\
        $(HXX2FILES)					\
        $(HXX3FILES)					\
        $(HXX4FILES)					\
        $(HXX5FILES)					\
        $(HXX6FILES)					\
        $(HXX7FILES)					\
        $(HXX8FILES)					\
        $(HXX9FILES)					\
        $(HXX10FILES)					\
        $(HXX11FILES)					\
        $(HXX12FILES)					\
        $(HXX13FILES)					\
        $(HXX14FILES)					\
        $(HXX15FILES)					\
        $(HXX16FILES)					\
        $(HXX17FILES)					\
        $(HXX18FILES)					\
        $(HXX19FILES)					\
        $(HXX20FILES)					\
        makefile.mk

DEFLIB1NAME =tools

ALL:		$(LIB1TARGET)			\
            ALLTAR					\
            $(INCCOM)$/dll.hxx

# --- Targets ------------------------------------------------------

# --- TOOLS.FLT ---
$(MISC)$/$(SHL1TARGET).flt:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Imp>$@
    @echo PointerList>>$@
    @echo DbgCheck>>$@
    @echo DbgName>>$@
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
.ENDIF							# "$(depend)" == ""

# --- Targets ------------------------------------------------------

.INCLUDE :		target.mk

# --- TOOLS.LIB ---
$(LB)$/$(TARGET).lib : $(LB)$/itools.lib
    @echo ------------------------------
    @echo Making: $@
    @+$(COPY) $(LB)$/itools.lib $@


# --- DLL.HXX ---

.IF "$(GUI)"=="UNX"
$(INCCOM)$/dll.hxx:
    @echo ------------------------------
    @echo Making: $@
    @echo > $@
.ELSE
$(INCCOM)$/dll.hxx: 	 $(INCGUI)$/dll.hxx
    @echo ------------------------------
    @echo Making: $@
    @+$(COPY) $(INCGUI)$/dll.hxx $@
.ENDIF

