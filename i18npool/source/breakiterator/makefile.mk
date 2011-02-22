#*************************************************************************
#*
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
        $(SLO)$/breakiterator_unicode.obj \
        $(SLO)$/xdictionary.obj \
        $(subst,$(MISC)$/,$(SLO)$/ $(MY_MISC_CXXFILES:s/.c/.obj/))

OBJFILES   = $(OBJ)$/gendict.obj

APP1TARGET = gendict
APP1RPATH = NONE

DEPOBJFILES   = $(OBJ)$/gendict.obj 
APP1OBJS   = $(DEPOBJFILES)

APP1STDLIBS = $(SALLIB)

# --- Targets ------------------------------------------------------

.IF "$(SYSTEM_ICU)" == "YES"
GENCMN:=$(SYSTEM_GENCMN)
GENBRK:=$(SYSTEM_GENBRK)
GENCCODE:=$(SYSTEM_GENCCODE)
.ELSE
GENCMN:=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)$/gencmn
GENBRK:=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)$/genbrk
GENCCODE:=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)$/genccode
.ENDIF

.INCLUDE .IGNORE :  icuversion.mk

$(MISC)$/%.txt : data/%.txt
# fdo#31271 ")" reclassified in more recent ICU/Unicode Standards
.IF "$(ICU_MAJOR)" >= "5" || ("$(ICU_MAJOR)" == "4" && "$(ICU_MINOR)" >= "4")
    $(SED) "s#\[:LineBreak =  Close_Punctuation:\]#\[\[:LineBreak =  Close_Punctuation:\] \[:LineBreak = Close_Parenthesis:\]\]#" $< > $@
.ELSE
    $(COPY) $< $@
.ENDIF

$(MISC)$/%.brk : $(MISC)/%.txt
    $(WRAPCMD) $(GENBRK) -r $< -o $(MISC)$/$*.brk

$(MISC)$/%_brk.c : $(MISC)$/%.brk
    $(WRAPCMD) $(GENCCODE) -n OpenOffice -d $(MISC)$ $(MISC)$/$*.brk

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_icu_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_icu_tmp.c with the -t switch.
$(MISC)$/OpenOffice_%.c : $(MY_BRK_BRKFILES:s/.brk/_brk.c/)
    $(WRAPCMD) $(GENCMN) -n OpenOffice -t tmp -S -d $(MISC) O $(mktmp $(subst,$(MISC)$/, $(MY_BRK_BRKFILES:t"\n")))
    echo $(USQ)#ifdef _MSC_VER$(USQ) > $@
    echo $(USQ)#pragma warning( disable : 4229 4668 )$(USQ) >> $@
    echo $(USQ)#endif$(USQ) >> $@
    $(TYPE) $(@:s/_dat/_tmp/) >> $@

.INCLUDE :	target.mk

