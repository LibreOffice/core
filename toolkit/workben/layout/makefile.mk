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
# $Revision: 1.3 $
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
PRJNAME=toolkit
TARGET=test
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(ENABLE_LAYOUT)" == "TRUE"

# Allow zoom and wordcount to be built without depending on svx,sv,sfx2
CFLAGS += -I../$(PRJ)/svx/inc -I../$(PRJ)/svtools/inc -I../$(PRJ)/sfx2/inc -I../$(PRJ)/sc/inc -I../$(PRJ)/sc/source/ui/inc -I../$(PRJ)/sw/inc

.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(COMNAME)" == "gcc3"
CFLAGS+=-Wall -Wno-non-virtual-dtor
.ENDIF

CXXFILES=\
    editor.cxx \
    plugin.cxx \
    recover.cxx \
    wordcountdialog.cxx \
    test.cxx \
    zoom.cxx

OBJFILES=\
    $(OBJ)$/editor.obj \
    $(OBJ)$/plugin.obj \
    $(OBJ)$/recover.obj \
    $(OBJ)$/test.obj \
    $(OBJ)$/tpsort.obj \
    $(OBJ)$/sortdlg.obj \
    $(OBJ)$/wordcountdialog.obj \
    $(OBJ)$/zoom.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1STDLIBS= \
        $(TOOLSLIB)			\
        $(COMPHELPERLIB)		\
        $(VCLLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)		\
        $(SALLIB)			\
        $(XMLSCRIPTLIB)			\
        $(TKLIB) \
        $(SVXLIB) \
        $(ISCLIB) \
#

svtools = $(INCCOM)/svtools
default: ALLTAR

.INCLUDE : target.mk

XML_FILES=\
    insert-sheet.xml\
    message-box.xml\
    move-copy-sheet.xml\
    recover.xml\
    sort-options.xml\
    string-input.xml\
    tab-dialog.xml\
    wordcount.xml\
    zoom.xml\

TRALAY=$(AUGMENT_LIBRARY_PATH) tralay
XML_LANGS=$(alllangiso)

ALLTAR: localize.sdf $(BIN)/testrc $(svtools) $(foreach,i,$(XML_FILES) en-US/$i)

$(XML_LANGS:f:t"/%.xml ")/%.xml: %.xml
    $(TRALAY) -m localize.sdf -o . -l $(XML_LANGS:f:t" -l ") $<
    rm -rf en-US

$(BIN)/%: %.in
    cp $< $@

$(svtools):
# FIXME: there's a bug in svtools layout or usage
# Include files are in svtools/inc, but are referenced as <svtools/..>
# They probably should be in svtools/inc/svtools
# This means that include files can only be included after svtools
# is built, which would mean a circular dependency,
# because svtools depends on toolkit.
    ln -sf ..$/$(PRJ)$/svtools$/inc $(INCCOM)$/svtools

dist .PHONY :
    cp -pv message-box.xml $(PRJ)/uiconfig/layout
    cp -pv tab-dialog.xml $(PRJ)/uiconfig/layout
    $(SHELL) ./un-test.sh zoom.cxx > ../$(PRJ)/svx/source/dialog/zoom.cxx
    $(SHELL) ./un-test.sh zoom.hxx > ../$(PRJ)/svx/source/dialog/zoom.hxx
    touch ../$(PRJ)/svx/source/dialog/dlgfact.cxx
    cp -pv zoom.xml ../$(PRJ)/svx/uiconfig/layout
    $(SHELL) ./un-test.sh wordcountdialog.cxx > ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.cxx
    $(SHELL) ./un-test.sh wordcountdialog.hxx > ../$(PRJ)/sw/source/ui/inc/wordcountdialog.hxx
    touch ../$(PRJ)/sw/source/ui/dialog/swdlgfact.cxx
    cp -pv wordcount.xml ../$(PRJ)/sw/uiconfig/layout
    # FIXME: broken setup
    ln -sf ../inc/wordcountdialog.hxx ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.hxx 
    $(SHELL) ./un-test.sh tpsort.cxx > ../$(PRJ)/sc/source/ui/dbgui/tpsort.cxx
    $(SHELL) ./un-test.sh tpsort.hxx > ../$(PRJ)/sc/source/ui/inc/tpsort.hxx
    $(SHELL) ./un-test.sh sortdlg.cxx > ../$(PRJ)/sc/source/ui/dbgui/sortdlg.cxx
    $(SHELL) ./un-test.sh sortdlg.hxx > ../$(PRJ)/sc/source/ui/inc/sortdlg.hxx
    touch ../$(PRJ)/sc/source/ui/attrdlg/scdlgfact.cxx
    touch ../$(PRJ)/sc/source/ui/view/cellsh2.cxx
    cp -pv insert-sheet.xml ../$(PRJ)/sc/uiconfig/layout
    cp -pv move-copy-sheet.xml ../$(PRJ)/sc/uiconfig/layout
    cp -pv sort-options.xml ../$(PRJ)/sc/uiconfig/layout
    cp -pv string-input.xml ../$(PRJ)/sc/uiconfig/layout

localize.sdf: $(PRJ)/../svx/source/dialog/localize.sdf $(PRJ)/../sw/source/ui/dialog/localize.sdf $(PRJ)/../sc/source/ui/src/localize.sdf
    grep sortdlg.src $(PRJ)/../sc/source/ui/src/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "sc\\uiconfig\\layout\\sort-options.xml", $$3, "layout", $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\t_label/\tRID_SCDLG_SORT_title/' > sort-options-$@
    grep wordcountdialog.src $(PRJ)/../sw/source/ui/dialog/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "sw\\uiconfig\\layout\\wordcount.xml", $$3, "layout", $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\t_label/\tDLG_WORDCOUNT_title/' > wordcount-$@
    grep zoom.src $(PRJ)/source/dialog/localize.sdf | awk -F'\t' '{{printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "layout", "svx\\uiconfig\\layout\\zoom.xml", $$3, "layout", $$6 "_label", "", "", $$8, "0", $$10, $$11, $$12, "", $$14, $$15}}' | sed -e 's/\(\(FL\|STR\)_[^\t]*\)_label/\1_text/' -e 's/\t_label/\tRID_SVXDLG_ZOOM_title/' > zoom-$@
    echo '#empty' | cat - sort-options-$@ wordcount-$@ zoom-$@ > $@
    rm -f *-$@

.ELSE # ENABLE_LAYOUT != TRUE
all .PHONY:
.ENDIF # ENABLE_LAYOUT != TRUE
