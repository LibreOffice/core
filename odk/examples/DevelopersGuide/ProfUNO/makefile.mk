#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:27:43 $
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
    -rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $< $(MY_TEXTCOPY_TARGETPRE) $@

