#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:57:45 $
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

PRJ := ..$/..$/..
PRJNAME := bridges

TARGET := test_javauno_acquire
PACKAGE := test$/javauno$/acquire

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files
INCPRE += $(MISC)$/$(TARGET)$/inc

SLOFILES = $(SLO)$/testacquire.obj

SHL1TARGET = testacquire.uno
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1VERSIONMAP = testacquire.map
SHL1IMPLIB = itestacquire

JAVAFILES = TestAcquire.java
JARFILES = OOoRunner.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: \
    $(BIN)$/testacquire-java-client$(SCRIPTEXT) \
    $(BIN)$/testacquire-java-server$(SCRIPTEXT) \
    $(BIN)$/testacquire-native-client$(SCRIPTEXT) \
    $(BIN)$/testacquire-native-server$(SCRIPTEXT)

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
    idlc -I$(SOLARIDLDIR) -O$(MISC)$/$(TARGET) $<
    regmerge $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    cppumaker -BUCR -C -O$(MISC)$/$(TARGET)$/inc $@ -X$(SOLARBINDIR)$/types.rdb
    javamaker -BUCR -nD -O$(CLASSDIR) $@ -X$(SOLARBINDIR)$/types.rdb
    regmerge $@ / $(SOLARBINDIR)$/types.rdb
    regcomp -register -r $@ -c acceptor.uno$(DLLPOST) \
        -c bridgefac.uno$(DLLPOST) -c connector.uno$(DLLPOST) \
        -c remotebridge.uno$(DLLPOST) -c uuresolver.uno$(DLLPOST)

$(SLOFILES) $(JAVACLASSFILES): $(BIN)$/$(TARGET).rdb

# Use "127.0.0.1" instead of "localhost", see #i32281#:
TEST_JAVAUNO_ACQUIRE_UNO_URL := \
    \"'uno:socket,host=127.0.0.1,port=2002;urp;test'\"

$(BIN)$/testacquire-java-client$(SCRIPTEXT):
    echo java -classpath \
        ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)\
..$/class$/java_uno.jar$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.javauno.acquire.TestAcquire client \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-java-server$(SCRIPTEXT):
    echo java -classpath \
        ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)\
..$/class$/java_uno.jar$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.javauno.acquire.TestAcquire server \
        $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-native-client$(SCRIPTEXT):
    echo uno -c com.sun.star.test.bridges.testacquire.impl -l $(SHL1TARGETN:f) \
        -ro $(TARGET).rdb -- $(TEST_JAVAUNO_ACQUIRE_UNO_URL) > $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/testacquire-native-server$(SCRIPTEXT):
    echo uno -c com.sun.star.test.bridges.testacquire.impl -l $(SHL1TARGETN:f) \
        -ro $(TARGET).rdb -u $(TEST_JAVAUNO_ACQUIRE_UNO_URL) --singleaccept > $@
    $(GIVE_EXEC_RIGHTS) $@
