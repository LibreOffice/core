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

CFLAGS += -I$(PRJ)/source/layout

# Allow zoom and wordcount to be built without depending on svx,sv,sfx2
CFLAGS += -I../$(PRJ)/svx/inc -I../$(PRJ)/svtools/inc -I../$(PRJ)/sfx2/inc

.INCLUDE : $(PRJ)$/util$/makefile.pmk

CFLAGS += -DENABLE_LAYOUT=1 -DTEST_LAYOUT=1

.IF "$(COMNAME)" == "gcc3"
CFLAGS+=-Wall -Wno-non-virtual-dtor
.ENDIF

CXXFILES=\
    editor.cxx \
    recover.cxx \
    wordcountdialog.cxx \
    test.cxx \
    zoom.cxx

OBJFILES=\
    $(OBJ)$/editor.obj \
    $(OBJ)$/recover.obj \
    $(OBJ)$/test.obj \
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
        $(TKLIB)

svtools = $(INCCOM)/svtools
all: $(svtools) ALLTAR

.INCLUDE :  target.mk

XML_FILES=\
    recover.xml\
    wordcount.xml\
    zoom.xml\

TRALAY=$(AUGMENT_LIBRARY_PATH) tralay
XML_LANGS=$(alllangiso)

#ALL_XMLS=$(foreach,i,$(XML_LANGS) $(foreach,j,$(XML_FILES) $i/$j))
ALLTAR: $(foreach,i,$(XML_FILES) en-US/$i)

$(XML_LANGS:f:t"/%.xml ")/%.xml: %.xml
    $(TRALAY) -m localize.sdf -o . -l $(XML_LANGS:f:t" -l ") $<
    rm -rf en-US

$(svtools):
# FIXME: there's a bug in svtools layout or usage
# Include files are in svtools/inc, but are referenced as <svtools/..>
# They probably should be in svtools/inc/svtools
# This means that include files can only be included after svtools
# is built, which would mean a circular dependency,
# because svtools depends on toolkit.
    ln -sf ..$/$(PRJ)$/svtools$/inc $(INCCOM)$/svtools

dist .PHONY :
    $(SHELL) ./un-test.sh zoom.cxx > ../$(PRJ)/svx/source/dialog/zoom.cxx
    $(SHELL) ./un-test.sh zoom.hxx > ../$(PRJ)/svx/source/dialog/zoom.hxx
    touch ../$(PRJ)/svx/source/dialog/dlgfact.cxx
    $(SHELL) ./un-test.sh wordcountdialog.cxx > ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.cxx
    $(SHELL) ./un-test.sh wordcountdialog.hxx > ../$(PRJ)/sw/source/ui/inc/wordcountdialog.hxx
    touch ../$(PRJ)/sw/source/ui/dialog/swdlgfact.cxx
# FIXME: broken setup
    ln -sf ../inc/wordcountdialog.hxx ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.hxx 

.ELSE # ENABLE_LAYOUT != TRUE
all .PHONY:
.ENDIF # ENABLE_LAYOUT != TRUE
