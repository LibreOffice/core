#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2003-08-27 16:43:36 $
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
    $(DESTPROTOCOLHANDLERADDONCPP)$/exports.dxp \
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
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/exports.dxp \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/service1_impl.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/service2_impl.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/SimpleComponent.sxw \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/CppComponent$/some.idl

JAVACOMPONENT_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/RunComponent.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestComponentA.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestComponentB.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/TestServiceProvider.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/XSomethingA.idl \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Components$/JavaComponent$/XSomethingB.idl

DESTTHUMBS=$(DESTDIRDEVGUIDEEXAMPLES)$/Components$/Thumbs
THUMBS_FILES=\
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/ImageShrink.java \
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/Thumbs.java \
    $(DESTTHUMBS)$/org$/openoffice$/comp$/test$/Makefile \
    $(DESTTHUMBS)$/org$/openoffice$/test$/ImageShrink.idl \
    $(DESTTHUMBS)$/org$/openoffice$/test$/XImageShrink.idl \
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

