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

.IF "$(OS)" == "WNT"
GIVE_EXEC_RIGHTS = @echo
.ELSE # GUI, WNT
GIVE_EXEC_RIGHTS = chmod +x
.ENDIF # GUI, WNT

EXEC_CLASSPATH_TMP = $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(LIBO_PATH_SEPARATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

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
        ..$/class$/test$(LIBO_PATH_SEPARATOR)..$/class$(LIBO_PATH_SEPARATOR)\
..$/class$/java_uno.jar$(LIBO_PATH_SEPARATOR)$(EXEC_CLASSPATH) \
        test.javauno.acquire.TestAcquire client \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-java-server:
    echo java -classpath \
        ..$/class$/test$(LIBO_PATH_SEPARATOR)..$/class$(LIBO_PATH_SEPARATOR)\
..$/class$/java_uno.jar$(LIBO_PATH_SEPARATOR)$(EXEC_CLASSPATH) \
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
