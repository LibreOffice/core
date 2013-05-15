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

PRJ = ..$/..$/..
PRJNAME = bridges

TARGET = test_javauno_equals
PACKAGE = test$/java_uno$/equals

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

# Make sure TestBed.class is found under $(CLASSDIR)$/test:
.IF "$(XCLASSPATH)" == ""
XCLASSPATH := $(CLASSDIR)$/test
.ELSE
XCLASSPATH !:= $(XCLASSPATH)$(PATH_SEPARATOR)$(CLASSDIR)$/test
.ENDIF

DLLPRE = # no leading "lib" on .so files
INCPRE += $(MISC)$/$(TARGET)$/inc

SLOFILES = $(SLO)$/testequals.obj

SHL1TARGET = $(ENFORCEDSHLPREFIX)testequals.uno
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1IMPLIB = itestequals

JAVAFILES = TestEquals.java
JARFILES = juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: $(BIN)$/testequals

.IF "$(OS)" == "WNT"
GIVE_EXEC_RIGHTS = @echo
.ELSE # GUI, WNT
GIVE_EXEC_RIGHTS = chmod +x
.ENDIF # GUI, WNT

EXEC_CLASSPATH_TMP = $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(PATH_SEPARATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

$(MISC)$/$(TARGET).rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    - $(MKDIR) $(MISC)$/$(TARGET)$/inc
    $(IDLC) -I$(SOLARIDLDIR) -O$(MISC)$/$(TARGET) $<
    $(REGMERGE) $(MISC)$/$(TARGET).rdb /UCR $(MISC)$/$(TARGET)$/types.urd
    $(CPPUMAKER) -BUCR -C -O$(MISC)$/$(TARGET)$/inc $@ -X$(SOLARBINDIR)$/types.rdb
    $(JAVAMAKER) -BUCR -nD -O$(CLASSDIR) $@ -X$(SOLARBINDIR)$/types.rdb

$(SLOFILES) $(JAVACLASSFILES): $(MISC)$/$(TARGET).rdb

$(BIN)$/testequals: $(BIN)$/testequals_services.rdb
    echo '$(AUGMENT_LIBRARY_PATH)' java -classpath \
        ..$/class$/test$(PATH_SEPARATOR)..$/class$(PATH_SEPARATOR)\
..$/class$/java_uno.jar$(PATH_SEPARATOR)$(EXEC_CLASSPATH) \
        test.java_uno.equals.TestEquals $(SOLARBINDIR)$/types.rdb \
        testequals_services.rdb > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testequals_services.rdb:
    - rm $@
    $(REGCOMP) -register -r $@ -c bridgefac.uno
    $(REGCOMP) -register -r $@ -c connector.uno
    $(REGCOMP) -register -r $@ -c remotebridge.uno
