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
# $Revision: 1.6 $
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

PRJ=..$/..$/..
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

BASICANDDIALOGS_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/FirstStepsBasic.odt

CREATINGDIALOGS_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/CreatingDialogs$/SampleDialog.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/CreatingDialogs$/CreatingDialogs.odt \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/CreatingDialogs$/Makefile

TOOLKITCONTROLS_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls.odt \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/dialog.xlb \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/FileDialog.xba \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/FileDialogDlg.xdl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/MultiPage.xba \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/MultiPageDlg.xdl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/ProgressBar.xba \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/ProgressBarDlg.xdl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/script.xlb \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/ScrollBar.xba \
    $(DESTDIRDEVGUIDEEXAMPLES)$/BasicAndDialogs$/ToolkitControls$/ToolkitControls$/ScrollBarDlg.xdl

DIR_FILE_LIST= \
    $(BASICANDDIALOGS_FILES) \
    $(CREATINGDIALOGS_FILES) \
    $(TOOLKITCONTROLS_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_basicanddialogs_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_basicanddialogs.txt

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

