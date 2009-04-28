#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.11 $
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

PRJ = ..$/..$/..
PRJNAME = bridges
TARGET = test_javauno_any

PACKAGE = test$/java_uno$/anytest

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
    $(OUT)$/bin$/TestRemote$(SCRIPTEXT) \
    $(OUT)$/bin$/TestJni$(SCRIPTEXT)

$(OUT)$/bin$/TestRemote$(SCRIPTEXT) : $(JAVACLASSFILES)
    -rm -f $@
    echo java -classpath ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.java_uno.anytest.TestRemote > $@
    $(GIVE_EXEC_RIGHTS) $@

$(OUT)$/bin$/TestJni$(SCRIPTEXT) : $(JAVACLASSFILES)
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
