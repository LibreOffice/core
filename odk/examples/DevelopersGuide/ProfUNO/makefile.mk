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

CPPBINDING_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/CppBinding$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/CppBinding$/office_connect.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/CppBinding$/office_connect.ini \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/CppBinding$/office_connectrc \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/CppBinding$/string_samples.cxx

INTERPROCESSCONN_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/InterprocessConn$/ConnectionAwareClient.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/InterprocessConn$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/InterprocessConn$/UrlResolver.java

LIFETIME_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/Lifetime$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/Lifetime$/MyUnoObject.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/Lifetime$/object_lifetime.cxx

SIMPLEBOOTSTRAP_JAVA_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/SimpleBootstrap_java$/SimpleBootstrap_java.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/SimpleBootstrap_java$/manifest.mf \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/SimpleBootstrap_java$/Makefile

SIMPLEBOOTSTRAP_CPP_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/SimpleBootstrap_cpp$/SimpleBootstrap_cpp.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/ProfUNO$/SimpleBootstrap_cpp$/Makefile

DIR_FILE_LIST= \
    $(CPPBINDING_FILES) \
    $(INTERPROCESSCONN_FILES) \
    $(LIFETIME_FILES) \
    $(SIMPLEBOOTSTRAP_JAVA_FILES) \
    $(SIMPLEBOOTSTRAP_CPP_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_profuno_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_profuno.txt

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

$(DESTDIREXAMPLES)$/%$/office_connectrc : $(PRJ)$/examples$/%$/office_connectrc
    -$(MKDIRHIER) $(@:d)        
    @@-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $< $(MY_TEXTCOPY_TARGETPRE) $@

