#*************************************************************************
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Andras Timar <atimar@suse.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
#*************************************************************************

PRJ=../..
PRJNAME=odk
TARGET=python

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)/util/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

PYTHONEXAMPLE=\
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/readme \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/Factory.xcu \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolpanels/poc.xdl \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/META-INF/manifest.xml \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolpanel.component \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolpanel.py \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/Makefile \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/description.xml \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolPanelPocBasic/Module1.xba \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolPanelPocBasic/script.xlb \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/toolPanelPocBasic/dialog.xlb \
    $(DESTDIRPYTHONEXAMPLES)/toolpanel/CalcWindowState.xcu

DIR_FILE_LIST= \
    $(PYTHONEXAMPLE)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)/ex_$(TARGET)_dirs_created.txt
DIR_FILE_FLAG=$(MISC)/ex_$(TARGET)_files.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)/util/odk_rules.pmk
