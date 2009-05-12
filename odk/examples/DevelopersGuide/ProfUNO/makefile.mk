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
# $Revision: 1.8 $
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

