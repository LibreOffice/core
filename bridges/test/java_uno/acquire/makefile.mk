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

TARGET := test_javauno_acquire
PACKAGE := test$/java_uno$/acquire

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files
INCPRE += $(MISC)$/$(TARGET)$/inc

SLOFILES = $(SLO)$/testacquire.obj

SHL1TARGET = $(ENFORCEDSHLPREFIX)testacquire.uno
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1IMPLIB = itestacquire

JAVAFILES = TestAcquire.java
JARFILES = OOoRunner.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: \
    $(BIN)$/testacquire-java-client \
    $(BIN)$/testacquire-java-server \
    $(BIN)$/testacquire-native-client \
    $(BIN)$/testacquire-native-server

.IF "$(GUI)" == "WNT"
GIVE_EXEC_RIGHTS = @echo
.ELSE # GUI, WNT
GIVE_EXEC_RIGHTS = chmod +x
.ENDIF # GUI, WNT

EXEC_CLASSPATH_TMP = $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(PATH_SEPERATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

$(BIN)$/$(TARGET).rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    - $(MKDIR) $(MISC)$/$(TARGET)$/inc
    $(IDLC) -I$(SOLARIDLDIR) -O$(MISC)$/$(TARGET) $<
    $(REGMERGE) $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    $(CPPUMAKER) -BUCR -C -O$(MISC)$/$(TARGET)$/inc $@ -X$(SOLARBINDIR)$/types.rdb
    $(JAVAMAKER) -BUCR -nD -O$(CLASSDIR) $@ -X$(SOLARBINDIR)$/types.rdb
    $(REGMERGE) $@ / $(SOLARBINDIR)$/types.rdb
    $(REGCOMP) -register -r $@ -c acceptor.uno$(DLLPOST) \
        -c bridgefac.uno$(DLLPOST) -c connector.uno$(DLLPOST) \
        -c remotebridge.uno$(DLLPOST) -c uuresolver.uno$(DLLPOST)

$(SLOFILES) $(JAVACLASSFILES): $(BIN)$/$(TARGET).rdb

# Use "127.0.0.1" instead of "localhost", see #i32281#:
TEST_JAVAUNO_ACQUIRE_UNO_URL := \
    \"'uno:socket,host=127.0.0.1,port=2002;urp;test'\"

$(BIN)$/testacquire-java-client:
    echo java -classpath \
        ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)\
..$/class$/java_uno.jar$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.javauno.acquire.TestAcquire client \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-java-server:
    echo java -classpath \
        ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)\
..$/class$/java_uno.jar$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.javauno.acquire.TestAcquire server \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-native-client:
    echo '$(AUGMENT_LIBRARY_PATH)' uno \
        -c com.sun.star.test.bridges.testacquire.impl \
        -l ../lib/$(SHL1TARGETN:f) -ro $(TARGET).rdb -- \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-native-server:
    echo '$(AUGMENT_LIBRARY_PATH)' uno \
        -c com.sun.star.test.bridges.testacquire.impl \
        -l ../lib/$(SHL1TARGETN:f) -ro $(TARGET).rdb \
        -u $(TEST_JAVAUNO_ACQUIRE_UNO_URL) --singleaccept > $@
    $(GIVE_EXEC_RIGHTS) $@
