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
    $(strip $(subst,!,$(PATH_SEPERATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

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
.IF "$(GUI)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    + export OO_JAVA_PROPERTIES='RuntimeLib=$(JVM_LIB_URL)' && \
        $(REGCOMP) -register -r $@ -c file://$(PWD)/$(BIN)$/$(TARGET).uno.jar \
        -br $(MISC)$/$(TARGET)$/bootstrap.rdb -classpath $(EXEC_CLASSPATH) \
        -env:URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
.ENDIF # GUI, WNT

test .PHONY: $(SHL1TARGETN) $(BIN)$/$(TARGET).uno.jar $(BIN)$/$(TARGET).rdb
.IF "$(GUI)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    $(AUGMENT_LIBRARY_PATH) uno -c test.javauno.nativethreadpool.server \
        -l $(SHL2TARGETN) -ro $(BIN)$/$(TARGET).rdb \
        -u 'uno:socket,host=localhost,port=3830;urp;test' --singleaccept &
    + $(AUGMENT_LIBRARY_PATH) OO_JAVA_PROPERTIES='RuntimeLib=$(JVM_LIB_URL)' \
        CLASSPATH=$(EXEC_CLASSPATH)$(PATH_SEPERATOR)$(BIN)$/$(TARGET).uno.jar \
        uno -c test.javauno.nativethreadpool.client -l $(SHL1TARGETN) \
        -ro $(BIN)$/$(TARGET).rdb \
        -env:URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
.ENDIF # GUI, WNT
