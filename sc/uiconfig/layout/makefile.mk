#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
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
#     Foundation, Inc., 51 Franklin Street, 5th Floor, Boston,
#     MA  02110-1301  USA
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
