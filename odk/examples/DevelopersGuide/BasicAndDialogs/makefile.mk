#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-01-31 16:03:30 $
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

