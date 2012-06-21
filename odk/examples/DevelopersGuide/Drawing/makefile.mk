#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

DRAWING_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/ChangeOrderDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/ControlAndSelectDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/CustomShowDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/DrawingDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/DrawViewDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/FillAndLineStyleDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/GluePointDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/GraphicExportDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/Helper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/LayerDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/ObjectTransformationDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/Organigram.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/PageHelper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/PresentationDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/ShapeHelper.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/SimplePresentation.odp \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/StyleDemo.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Drawing$/TextDemo.java

DIR_FILE_LIST= \
    $(DRAWING_FILES) \

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_drawing_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_drawing.txt

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
    
