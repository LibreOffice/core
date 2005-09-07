#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:14:48 $
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

PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

ACTIVEX_FILES=\
    $(DESTDIROLEEXAMPLES)$/activex$/example.html \
    $(DESTDIROLEEXAMPLES)$/activex$/Makefile \
    $(DESTDIROLEEXAMPLES)$/activex$/README.txt \
    $(DESTDIROLEEXAMPLES)$/activex$/resource.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOActiveX.rgs \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.h \
    $(DESTDIROLEEXAMPLES)$/activex$/SOComWindowPeer.rgs \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.def \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.idl \
    $(DESTDIROLEEXAMPLES)$/activex$/so_activex.rc \
    $(DESTDIROLEEXAMPLES)$/activex$/StdAfx2.cpp \
    $(DESTDIROLEEXAMPLES)$/activex$/StdAfx2.h

DELPHI_FILES=\
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/Project1.dpr \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/Project1.res \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleCode.pas \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleUI.dfm \
    $(DESTDIROLEEXAMPLES)$/delphi$/InsertTables$/SampleUI.pas \
    $(DESTDIROLEEXAMPLES)$/delphi$/StarOffice_and_Delphi.pdf \
    $(DESTDIROLEEXAMPLES)$/delphi$/StarOffice_Delphi.sxw

VBSCRIPT_FILES=\
    $(DESTDIROLEEXAMPLES)$/vbscript$/readme.txt \
    $(DESTDIROLEEXAMPLES)$/vbscript$/WriterDemo.vbs

DIR_FILE_LIST= \
    $(ACTIVEX_FILES) \
    $(DELPHI_FILES) \
    $(VBSCRIPT_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/ex_ole_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/ex_ole_files.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk
    
