#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:04:57 $
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

