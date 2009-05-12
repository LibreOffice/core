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

