#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2003-08-27 16:46:03 $
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
    
