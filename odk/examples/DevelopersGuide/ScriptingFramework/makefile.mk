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

DESTSF=$(DESTDIRDEVGUIDEEXAMPLES)$/ScriptingFramework
SF_FILES=\
    $(DESTSF)$/SayHello$/SayHello$/parcel-descriptor.xml \
    $(DESTSF)$/SayHello$/SayHello$/SayHello.java \
    $(DESTSF)$/SayHello$/Makefile \
    $(DESTSF)$/SayHello$/SayHello.odt \
    $(DESTSF)$/SayHello$/build.xml \
    $(DESTSF)$/SayHello$/META-INF$/manifest.xml \
    $(DESTSF)$/ScriptSelector$/ScriptSelector.odt \
    $(DESTSF)$/ScriptSelector$/Makefile \
    $(DESTSF)$/ScriptSelector$/build.xml \
    $(DESTSF)$/ScriptSelector$/META-INF$/manifest.xml \
    $(DESTSF)$/ScriptSelector$/ScriptSelector$/container.gif \
    $(DESTSF)$/ScriptSelector$/ScriptSelector$/script.gif \
    $(DESTSF)$/ScriptSelector$/ScriptSelector$/soffice.gif \
    $(DESTSF)$/ScriptSelector$/ScriptSelector$/ScriptSelector.java \
    $(DESTSF)$/ScriptSelector$/ScriptSelector$/parcel-descriptor.xml

DIR_FILE_LIST= \
    $(SF_FILES) \

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_sf_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_sf.txt

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
    

