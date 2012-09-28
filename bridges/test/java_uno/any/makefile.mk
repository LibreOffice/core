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
TARGET = test_javauno_any

PACKAGE = test$/java_uno$/any

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

.IF "$(GUI)" == "WNT"
GIVE_EXEC_RIGHTS = @echo
.ELSE
GIVE_EXEC_RIGHTS = chmod +x
.ENDIF

JAVAFILES = \
    $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

# Make sure TestBed.class is found under $(CLASSDIR)$/test:
.IF "$(XCLASSPATH)" == ""
XCLASSPATH := $(CLASSDIR)$/test
.ELSE
XCLASSPATH !:= $(XCLASSPATH)$(PATH_SEPERATOR)$(CLASSDIR)$/test
.ENDIF

EXEC_CLASSPATH_TMP = \
    $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)$(XCLASSPATH)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(PATH_SEPERATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

JARFILES = juh.jar jurt.jar ridl.jar
JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/TestAny.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestRemote.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestJni.class

#--------------------------------------------------

USE_DEFFILE = TRUE
ENABLE_EXCEPTIONS = TRUE
INCPRE += $(OUT)$/inc$/test

.IF "$(debug)" != ""
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

SLOFILES= \
    $(SLO)$/transport.obj

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
    $(CPPUHELPERLIB)		\
    $(SALHELPERLIB)			\
    $(JVMACCESSLIB)			\
    $(CPPULIB)			\
    $(SALLIB)

SHL1VERSIONMAP=$(TARGET).map
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.INCLUDE: target.mk

#--------------------------------------------------

$(SLOFILES) : $(MISC)$/gen_files.flag
$(JAVACLASSFILES) : $(MISC)$/gen_files.flag

ALLTAR : \
    $(OUT)$/bin$/TestRemote \
    $(OUT)$/bin$/TestJni

$(OUT)$/bin$/TestRemote : $(JAVACLASSFILES)
    -rm -f $@
    echo java -classpath ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.java_uno.anytest.TestRemote > $@
    $(GIVE_EXEC_RIGHTS) $@

$(OUT)$/bin$/TestJni : $(JAVACLASSFILES)
    -rm -f $@
    echo '$(AUGMENT_LIBRARY_PATH)' java -classpath \
        .$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        -Djava.library.path=..$/lib test.java_uno.anytest.TestJni >> $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/test_java_uno_anytest.rdb : types.idl
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}

$(MISC)$/gen_files.flag : $(BIN)$/test_java_uno_anytest.rdb
    $(CPPUMAKER) -C -BUCR -O $(OUT)$/inc$/test -X $(SOLARBINDIR)$/udkapi.rdb $?
    $(CPPUMAKER) -C -BUCR -O $(OUT)$/inc$/test -T com.sun.star.uno.XInterface $(SOLARBINDIR)$/udkapi.rdb
    $(JAVAMAKER) -nD -BUCR -O $(CLASSDIR) -X $(SOLARBINDIR)$/udkapi.rdb $?
    $(TOUCH) $@
