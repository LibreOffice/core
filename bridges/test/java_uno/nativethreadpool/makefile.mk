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

PRJ := ..$/..$/..
PRJNAME := bridges

TARGET := test_javauno_nativethreadpool
PACKAGE := test$/java_uno$/nativethreadpool

ENABLE_EXCEPTIONS := TRUE

#TODO:
.IF "$(OS)" == "LINUX"
JVM_LIB_URL := file:///net/grande.germany/develop6/update/dev/Linux_JDK_1.4.1_03/jre/lib/i386/client/libjvm.so
.ELSE
ERROR -- missing platform
.ENDIF

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files
INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(ENFORCEDSHLPREFIX)$(TARGET)_client.uno
SHL1OBJS = $(SLO)$/testnativethreadpoolclient.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(TARGET)_client

SHL2TARGET = $(ENFORCEDSHLPREFIX)$(TARGET)_server.uno
SHL2OBJS = $(SLO)$/testnativethreadpoolserver.obj
SHL2STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL2VERSIONMAP = version.map
SHL2IMPLIB = i$(TARGET)_server

SLOFILES = $(SHL1OBJS) $(SHL2OBJS)

JAVAFILES = Relay.java
JARFILES = java_uno.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: test

EXEC_CLASSPATH_TMP = $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(LIBO_PATH_SEPARATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

$(MISC)$/$(TARGET)$/types.rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    - $(MKDIR) $(MISC)$/$(TARGET)$/inc
    $(IDLC) -I$(SOLARIDLDIR) -O$(MISC)$/$(TARGET) $<
    $(REGMERGE) $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    $(CPPUMAKER) -BUCR -C -O$(MISC)$/$(TARGET)$/inc $@ -X$(SOLARBINDIR)$/types.rdb
    $(JAVAMAKER) -BUCR -nD -O$(CLASSDIR) $@ -X$(SOLARBINDIR)$/types.rdb

$(SLOFILES) $(JAVACLASSFILES): $(MISC)$/$(TARGET)$/types.rdb

$(BIN)$/$(TARGET).uno.jar: $(JAVACLASSFILES) relay.manifest
    jar cfm $@ relay.manifest -C $(CLASSDIR) test/javauno/nativethreadpool

$(BIN)$/$(TARGET).rdb .ERRREMOVE: $(MISC)$/$(TARGET)$/types.rdb \
        $(BIN)$/$(TARGET).uno.jar
    cp $(MISC)$/$(TARGET)$/types.rdb $@
    $(REGMERGE) $@ / $(SOLARBINDIR)$/types.rdb
    $(REGCOMP) -register -r $@ -c acceptor.uno$(DLLPOST) \
        -c bridgefac.uno$(DLLPOST) -c connector.uno$(DLLPOST) \
        -c remotebridge.uno$(DLLPOST) -c uuresolver.uno$(DLLPOST) \
        -c javaloader.uno$(DLLPOST) -c javavm.uno$(DLLPOST) \
        -c stocservices.uno$(DLLPOST)
    cp $(SOLARBINDIR)$/types.rdb $(MISC)$/$(TARGET)$/bootstrap.rdb
    $(REGCOMP) -register -r $(MISC)$/$(TARGET)$/bootstrap.rdb \
        -c javaloader.uno$(DLLPOST) -c javavm.uno$(DLLPOST) \
        -c stocservices.uno$(DLLPOST)
.IF "$(OS)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    + export OO_JAVA_PROPERTIES='RuntimeLib=$(JVM_LIB_URL)' && \
        $(REGCOMP) -register -r $@ -c file://$(PWD)/$(BIN)$/$(TARGET).uno.jar \
        -br $(MISC)$/$(TARGET)$/bootstrap.rdb -classpath $(EXEC_CLASSPATH) \
        -env:URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
.ENDIF # GUI, WNT

test .PHONY: $(SHL1TARGETN) $(BIN)$/$(TARGET).uno.jar $(BIN)$/$(TARGET).rdb
.IF "$(OS)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    $(AUGMENT_LIBRARY_PATH) uno -c test.javauno.nativethreadpool.server \
        -l $(SHL2TARGETN) -ro $(BIN)$/$(TARGET).rdb \
        -u 'uno:socket,host=localhost,port=3830;urp;test' --singleaccept &
    + $(AUGMENT_LIBRARY_PATH) OO_JAVA_PROPERTIES='RuntimeLib=$(JVM_LIB_URL)' \
        CLASSPATH=$(EXEC_CLASSPATH)$(LIBO_PATH_SEPARATOR)$(BIN)$/$(TARGET).uno.jar \
        uno -c test.javauno.nativethreadpool.client -l $(SHL1TARGETN) \
        -ro $(BIN)$/$(TARGET).rdb \
        -env:URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
.ENDIF # GUI, WNT
