#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:59:09 $
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
    +echo java -classpath ..$/class$/test$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.java_uno.anytest.TestRemote > $@
    $(GIVE_EXEC_RIGHTS) $@

$(OUT)$/bin$/TestJni$(SCRIPTEXT) : $(JAVACLASSFILES)
    -rm -f $@
    +echo java -classpath .$(PATH_SEPERATOR)..$/class$(PATH_SEPERATOR)$(EXEC_CLASSPATH) \
        test.java_uno.anytest.TestJni >> $@
    $(GIVE_EXEC_RIGHTS) $@

$(BIN)$/test_java_uno_anytest.rdb : types.idl
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    +regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}

$(MISC)$/gen_files.flag : $(BIN)$/test_java_uno_anytest.rdb
    +cppumaker -C -BUCR -O $(OUT)$/inc$/test -X $(SOLARBINDIR)$/udkapi.rdb $?
    +cppumaker -C -BUCR -O $(OUT)$/inc$/test -T com.sun.star.uno.XInterface $(SOLARBINDIR)$/udkapi.rdb
    +javamaker -nD -BUCR -O $(CLASSDIR) -X $(SOLARBINDIR)$/udkapi.rdb $?
    +$(TOUCH) $@
