#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:50:21 $
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
#************************************************************************/
PRJ=..$/..
PRJNAME=i18npool
TARGET=breakiterator

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# grab all .txt files under data directory, which are breakiterator rule files.
MY_BRK_TXTFILES:=$(shell @ls data/*.txt)

# insert "OpenOffice" as icu package name in front of the  name of each rule file for searching on application provided data
MY_BRK_BRKFILES:=$(subst,data/,$(MISC)$/ $(MY_BRK_TXTFILES:s/.txt/.brk/))

# OpenOffice_dat.c is a generated file from the rule file list by gencmn
MY_MISC_CXXFILES := \
        $(MISC)$/OpenOffice_dat.c \
        $(MY_BRK_BRKFILES:s/.brk/_brk.c/)

SLOFILES=   \
        $(SLO)$/breakiteratorImpl.obj \
        $(SLO)$/breakiterator_cjk.obj \
        $(SLO)$/breakiterator_ctl.obj \
        $(SLO)$/breakiterator_th.obj \
        $(SLO)$/breakiterator_hi.obj \
        $(SLO)$/breakiterator_unicode.obj \
        $(SLO)$/xdictionary.obj \
        $(subst,$(MISC)$/,$(SLO)$/ $(MY_MISC_CXXFILES:s/.c/.obj/))

APP1TARGET = gendict

APP1OBJS   = $(OBJ)$/gendict.obj

APP1STDLIBS = $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(SYSTEM_ICU)" == "YES"
GENCMN:=$(SYSTEM_GENCMN)
GENBRK:=$(SYSTEM_GENBRK)
GENCCODE:=$(SYSTEM_GENCCODE)
.ELSE
GENCMN:=$(SOLARBINDIR)$/gencmn
GENBRK:=$(SOLARBINDIR)$/genbrk
GENCCODE:=$(SOLARBINDIR)$/genccode
.ENDIF

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_icu_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_icu_tmp.c with the -t switch.
$(MISC)$/OpenOffice_dat.c :  $(MY_BRK_BRKFILES) makefile.mk
    $(WRAPCMD) $(GENCMN) -n OpenOffice -t tmp -S -d $(MISC) O $(mktmp $(subst,$(MISC)$/, $(MY_BRK_BRKFILES:t"\n")))
    echo $(USQ)#ifdef _MSC_VER$(USQ) > $@
    echo $(USQ)#pragma warning( disable : 4229 4668 )$(USQ) >> $@
    echo $(USQ)#endif$(USQ) >> $@
    $(TYPE) $(@:s/_dat/_tmp/) >> $@

$(MISC)$/%.brk : data/%.txt
    $(WRAPCMD) $(GENBRK) -r $< -o $(MISC)$/$*.brk

$(MISC)$/%_brk.c : $(MISC)$/%.brk
    $(WRAPCMD) $(GENCCODE) -n OpenOffice -d $(MISC)$ $(MISC)$/$*.brk

