#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:02:10 $
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

# grab all .txt files under data directory, which are breakiterator rule files.
MY_BRK_TXTFILES:=$(shell ls data/*.txt)

# insert "OpenOffice" as icu package name in front of the  name of each rule file for searching on application provided data
MY_BRK_BRKFILES:=$(subst,data/,$(MISC)$/OpenOffice_ $(MY_BRK_TXTFILES:s/.txt/.brk/))

# OpenOffice_icu_dat.c is a generated file from the rule file list by gencmn
MY_MISC_CXXFILES := \
        $(MISC)$/OpenOffice_icu_dat.c \
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

APP1STDLIBS = $(SALLIB) \
        $(TOOLSLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
$(MISC)$/OpenOffice_icu_dat.c :  $(MY_BRK_BRKFILES)
    +$(WRAPCMD) $(SOLARBINDIR)$/gencmn -e OpenOffice_icu -n OpenOffice_icu -S -d $(MISC) O $(mktmp $(MY_BRK_BRKFILES:t"\n"))
$(MISC)$/OpenOffice_%.brk : data/%.txt
    +$(WRAPCMD) $(SOLARBINDIR)$/genbrk -r $< -o $(MISC)$/OpenOffice_$*.brk
$(MISC)$/%_brk.c : $(MISC)$/%.brk
    +$(WRAPCMD) $(SOLARBINDIR)$/genccode -d $(MISC)$ $(MISC)$/$*.brk
