#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ=..$/..
PRJNAME=odk
TARGET=gendocu

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
.IF "$(L10N_framework)"==""

CPPDOCREFNAME="UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) C/C++ API Reference"
JAVADOCREFNAME="UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) Java API Reference"

CPP_DOCU_INDEX_FILE = $(DESTDIRGENCPPREF)$/index.html
JAVA_DOCU_INDEX_FILE = $(DESTDIRGENJAVAREF)$/index.html

CPP_DOCU_CLEANUP_FLAG = $(MISC)$/cpp_docu_cleanup.flag

JAVA_SRC_DIR=$(MISC)$/java_src
JAVA_PACKAGES=\
        com.sun.star.comp.helper \
        com.sun.star.lib.uno.helper \
        com.sun.star.lib.unoloader \
        com.sun.star.uno \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/lib$/util$/UrlToFileMapper.java
#        com.sun.star.beans \

JAVA_SRC_FILES=\
        $(JAVA_SRC_DIR)$/jurt_src.zip \
        $(JAVA_SRC_DIR)$/juh_src.zip \
        $(JAVA_SRC_DIR)$/ridl_src.zip \
        $(JAVA_SRC_DIR)$/unoloader_src.zip

JAVA_BEAN_SRC_FILES=\
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/ContainerFactory.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/JavaWindowPeerFake.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/LocalOfficeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/LocalOfficeWindow.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/NativeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/NativeService.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/OfficeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/OfficeWindow.java

AUTODOCPARAMS= -lg c++ \
        -p sal $(INCOUT) -t sal -t osl -t rtl \
        -p store $(INCOUT) -t store \
        -p registry $(INCOUT) -t registry \
        -p cppu $(INCOUT) -t cppu -t com -t typelib -t uno \
        -p cppuhelper $(INCOUT) -t cppuhelper \
        -p salhelper $(INCOUT) -t salhelper \
        -p bridges $(INCOUT) -t bridges

JAVADOCPARAMS= -use -splitindex -windowtitle "Java UNO Runtime Reference" -header $(JAVADOCREFNAME) -d $(DESTDIRGENJAVAREF) -sourcepath $(JAVA_SRC_DIR) -classpath $(SOLARBINDIR)$/ridl.jar -linkoffline ../../common/ref ./uno -linkoffline http://java.sun.com/j2se/1.4.1/docs/api ./java $(JAVA_PACKAGES)

JAVADOCLOG = $(MISC)$/javadoc_log.txt

.IF "$(SOLAR_JAVA)"!=""
all: \
    $(CPP_DOCU_INDEX_FILE) \
    $(JAVA_DOCU_INDEX_FILE)

.ELSE
all: $(CPP_DOCU_INDEX_FILE)
.ENDIF


$(CPP_DOCU_CLEANUP_FLAG) : $(INCLUDELIST) $(PRJ)$/docs$/cpp$/ref$/cpp.css
    @@-$(MY_DELETE_RECURSIVE) $(DESTDIRGENCPPREF)
    $(TOUCH) $@

$(CPP_DOCU_INDEX_FILE) : $(CPP_DOCU_CLEANUP_FLAG)
    -$(MKDIRHIER) $(@:d)        
    $(MY_AUTODOC) -html $(DESTDIRGENCPPREF) -name $(CPPDOCREFNAME) $(AUTODOCPARAMS)
    -rm $(@:d:d)$/cpp.css
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(PRJ)$/docs$/cpp$/ref$/cpp.css $(MY_TEXTCOPY_TARGETPRE) $(@:d:d)$/cpp.css

$(JAVA_SRC_FILES) : $(SOLARCOMMONBINDIR)$/$$(@:f)
    -$(MKDIRHIER) $(@:d)        
    $(MY_COPY) $< $@
    cd $(JAVA_SRC_DIR) && unzip -quo $(@:f)

#$(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/%.java : $(PRJ)$/source$/bean$/com$/sun$/star$/beans$/%.java 
#	-$(MKDIRHIER) $(@:d)        
#	$(MY_COPY) $< $@

#$(JAVA_DOCU_INDEX_FILE) .SEQUENTIAL : $(JAVA_SRC_FILES) $(JAVA_BEAN_SRC_FILES)
.IF "$(SOLAR_JAVA)"!=""
$(JAVA_DOCU_INDEX_FILE) .SEQUENTIAL : $(JAVA_SRC_FILES)
    -$(MKDIRHIER) $(@:d)        
    $(JAVADOC) -J-Xmx120m $(JAVADOCPARAMS) > $(JAVADOCLOG)
.ENDIF

.ELSE
pseudo:
.ENDIF
