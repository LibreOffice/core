#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
