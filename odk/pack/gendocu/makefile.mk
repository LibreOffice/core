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
        $(JAVA_SRC_DIR)$/juh_src.zip \
        $(JAVA_SRC_DIR)$/jurt_src.zip \
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

JAVADOCPARAMS= -notimestamp -use -splitindex -windowtitle "Java UNO Runtime Reference" -header $(JAVADOCREFNAME) -d $(DESTDIRGENJAVAREF) -sourcepath $(JAVA_SRC_DIR) -classpath $(SOLARBINDIR)$/ridl.jar -linkoffline ../../common/ref ./uno -linkoffline http://java.sun.com/j2se/1.5/docs/api ./java $(JAVA_PACKAGES)

JAVADOCLOG = $(MISC)$/javadoc_log.txt

all :

.IF "$(DOXYGEN)" != ""
all : $(CPP_DOCU_INDEX_FILE)
.END

.IF "$(SOLAR_JAVA)" != ""
all : $(JAVA_DOCU_INDEX_FILE)
.END

$(CPP_DOCU_CLEANUP_FLAG) : $(INCLUDELIST) Doxyfile main.dox
    @@-$(MY_DELETE_RECURSIVE) $(DESTDIRGENCPPREF)
    $(TOUCH) $@

.IF "$(VERBOSE)" == "TRUE"
MY_QUIET = NO
.ELSE
MY_QUIET = YES
.END

$(CPP_DOCU_INDEX_FILE) .ERRREMOVE : $(CPP_DOCU_CLEANUP_FLAG)
    -$(MKDIRHIER) $(@:d)
    $(SED) \
        -e 's!^INPUT = %$$!INPUT = main.dox $(INCLUDETOPDIRLIST) $(INCLUDEFILELIST)!' \
        -e 's!^OUTPUT_DIRECTORY = %$$!OUTPUT_DIRECTORY = $(DESTDIRGENCPPREF)!' \
        -e 's!^PROJECT_BRIEF = %$$!PROJECT_BRIEF = $(CPPDOCREFNAME)!' \
        -e 's!^QUIET = %$$!QUIET = $(MY_QUIET)!' \
        -e 's!^STRIP_FROM_PATH = %$$!STRIP_FROM_PATH = $(SOLARINCDIR)!' \
        Doxyfile > $(MISC)/Doxyfile
    $(DOXYGEN) $(MISC)/Doxyfile

$(JAVA_SRC_FILES) : $(SOLARCOMMONPCKDIR)$/$$(@:f)
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
