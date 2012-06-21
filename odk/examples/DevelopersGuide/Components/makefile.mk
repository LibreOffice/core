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

DESTJOBSADDON=$(DESTDIRDEVGUIDEEXAMPLES)$/Components$/Addons$/JobsAddon
JOBSADDON_FILES=\
    $(DESTJOBSADDON)$/Addons.xcu \
    $(DESTJOBSADDON)$/AsyncJob.java \
    $(DESTJOBSADDON)$/Jobs.xcu \
    $(DESTJOBSADDON)$/Makefile

DESTPROTOCOLHANDLERADDONCPP=$(DESTDIRDEVGUIDEEXAMPLES)$/Components$/Addons$/ProtocolHandlerAddon_cpp
PROTOCOLHANLDERADDONCPP_FILES=\
    $(DESTPROTOCOLHANDLERADDONCPP)$/addon.cxx \
    $(DESTPROTOCOLHANDLERADDONCPP)$/addon.hxx \
    $(DESTPROTOCOLHANDLERADDONCPP)$/Addons.xcu \
    $(DESTPROTOCOLHANDLERADDONCPP)$/component.cxx \
    $(DESTPROTOCOLHANDLERADDONCPP)$/Makefile \
    $(DESTPROTOCOLHANDLERADDONCPP)$/ProtocolHandler.xcu \
    $(DESTPROTOCOLHANDLERADDONCPP)$/ProtocolHandlerAddon_cpp.uno.xml

DESTPROTOCOLHANDLERADDONJAVA=$(DESTDIRDEVGUIDEEXAMPLES)$/Components$/Addons$/ProtocolHandlerAddon_java
PROTOCOLHANLDERADDONJAVA_FILES=\
    $(DESTPROTOCOLHANDLERADDONJAVA)$/Addons.xcu \
    $(DESTPROTOCOLHANDLERADDONJAVA)$/Makefile \
    $(DESTPROTOCOLHANDLERADDONJAVA)$/ProtocolHandler.xcu \
    $(DESTPROTOCOLHANDLERADDONJAVA)$/ProtocolHandlerAddon.java \
    $(DESTPROTOCOLHANDLERADDONJAVA)$/ProtocolHandlerAddon_java.uno.xml

CPPCOMPONENT_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/CppComponent.uno.xml \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/service1_impl.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/service2_impl.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/TestCppComponent.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/SimpleComponent.odt \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/some.idl

JAVACOMPONENT_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestComponentA.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestComponentB.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestServiceProvider.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestJavaComponent.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/XSomethingA.idl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/XSomethingB.idl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/SomethingA.idl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/SomethingB.idl

DESTTHUMBS=$(DESTDIRDEVGUIDEEXAMPLES)$/Components$/Thumbs
THUMBS_FILES=\
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/ImageShrink.java \
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/Thumbs.java \
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/Makefile \
    $(DESTTHUMBS)$/org$/openoffice$/test$/ImageShrink.idl \
    $(DESTTHUMBS)$/org$/openoffice$/test$/XImageShrink.idl \
    $(DESTTHUMBS)$/org$/openoffice$/test$/XImageShrinkFilter.idl \
    $(DESTTHUMBS)$/org$/openoffice$/test$/Makefile \
    $(DESTTHUMBS)$/thumbs.mk \
    $(DESTTHUMBS)$/Makefile


DIR_FILE_LIST= \
    $(JOBSADDON_FILES) \
    $(PROTOCOLHANLDERADDONCPP_FILES) \
    $(PROTOCOLHANLDERADDONJAVA_FILES) \
    $(CPPCOMPONENT_FILES) \
    $(JAVACOMPONENT_FILES) \
    $(THUMBS_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_components_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_components.txt

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

