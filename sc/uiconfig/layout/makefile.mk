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

PRJ=../..
PRJNAME=sc
TARGET=layout

.INCLUDE : settings.mk

.IF "$(ENABLE_LAYOUT)" == "TRUE"

all: ALLTAR

XML_FILES=\
  insert-sheet.xml\
  move-copy-sheet.xml\
  sort-options.xml\
  string-input.xml\

#

.INCLUDE : layout.mk

.ELSE # ENABLE_LAYOUT != TRUE
all .PHONY:
.ENDIF # ENABLE_LAYOUT != TRUE

.INCLUDE :  target.mk

localize.sdf: $(PRJ)/source/ui/miscdlgs/localize.sdf $(PRJ)/source/ui/src/localize.sdf
    grep instbdlg.src $(PRJ)/source/ui/miscdlgs/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "sc\\uiconfig\\layout\\insert-sheet.xml", $$3, "layout", $$5 $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\tRID_SCDLG_INSERT_TABLE/\t/' -e 's/\t_label/\tRID_SCDLG_INSERT_TABLE_title/' > insert-sheet-$@
    grep -E 'miscdlgs.src.*(FT_DEST|FT_INSERT|STR_NEWDOC|RID_SCDLG_MOVETAB|BTN_COPY)' $(PRJ)/source/ui/src/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "sc\\uiconfig\\layout\\move-copy-sheet.xml", $$3, "layout", $$5 $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\tRID_SCDLG_MOVETAB/\t/' -e 's/\t_label/\tRID_SCDLG_MOVETAB_title/'> move-copy-sheet-$@
    grep sortdlg.src $(PRJ)/source/ui/src/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "sc\\uiconfig\\layout\\sort-options.xml", $$3, "layout", $$5 $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\tRID_SCDLG_SORT/\t/' -e 's/\t_label/\tRID_SCDLG_SORT_title/' -e 's/\tRID_SCPAGE_SORT_OPTIONS/\t/' -e 's/\tRID_SCPAGE_SORT_FIELDS/\t/' > sort-options-$@
    echo '#empty' | cat - insert-sheet-$@ move-copy-sheet-$@ sort-options-$@ > $@
    rm -f *-$@
