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

# Makefile template for directories that contain Java unit tests.
#
# Before including this makefile, the following variables should be set:
# - PRJ, PRJNAME, TARGET
# - PACKAGE
# - JAVATESTFILES: the unit test classes;
#     each one of the form <basename>.java;
#     these java files must only define classes within the package corresponding
#     to PACKAGE, the name of each defined (outer) class should end with "_Test"
# - JAVAFILES: optional java files, corresponding class files are generated
#     that can be used when compiling or running the tests;
#     each one of the form <basename>.java;
#     these java files must only define classes within the package corresponding
#     to PACKAGE, the name of each defined (outer) class should start with
#     "Test"
# - IDLTESTFILES: optional idl files, corresponding class files are generated
#     that can be used when compiling or running the tests;
#     each one of the form <basename>.idl;
#     these idl files must only define entities within the module corresponding
#     to PACKAGE, the name of each defined entity should start with "Test"
# - JARFILES: optional jar files that can be used when compiling or running the
#     tests;
#     each one of the form <basename>.jar
# All generated class files are placed into <platform>/class/test/ instead of
# <platform>/class/, so that they are not accidentally included in jar files
# packed from <platform>/class/ subdirectories.

JAVAFILES +:= $(JAVATESTFILES)
EXTRAJARFILES += $(OOO_JUNIT_JAR)

.INCLUDE: settings.mk

.IF "$(XCLASSPATH)" == ""
XCLASSPATH := $(CLASSDIR)
.ELSE
XCLASSPATH !:= $(XCLASSPATH)$(PATH_SEPERATOR)$(CLASSDIR)
.ENDIF
CLASSDIR !:= $(CLASSDIR)/test

.INCLUDE: target.mk

$(JAVATARGET) : $(MISC)/$(TARGET).classdir.flag

$(MISC)/$(TARGET).classdir.flag:
    - $(MKDIR) $(CLASSDIR)
    $(TOUCH) $@

.IF "$(JAVATESTFILES)" != ""
ALLTAR : test
.END

.IF "$(SOLAR_JAVA)" == "TRUE" && "$(OOO_JUNIT_JAR)" != ""
test .PHONY : $(JAVATARGET)
    $(JAVAI) $(JAVAIFLAGS) $(JAVACPS) \
        '$(OOO_JUNIT_JAR)$(PATH_SEPERATOR)$(CLASSPATH)' \
        org.junit.runner.JUnitCore \
        $(foreach,i,$(JAVATESTFILES) $(subst,/,. $(PACKAGE)).$(i:s/.java//))
.ELSE
test .PHONY :
    echo 'test needs SOLAR_JAVA=TRUE and OOO_JUNIT_JAR'
.END

.IF "$(IDLTESTFILES)" != ""

# The following dependency (to execute javac whenever javamaker has run) does
# not work reliably, see #i28827#:
$(JAVAFILES) $(JAVACLASSFILES): $(MISC)/$(TARGET).javamaker.flag

$(MISC)/$(TARGET).javamaker.flag: $(MISC)/$(TARGET).rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

$(MISC)/$(TARGET).rdb .ERRREMOVE: \
        $(foreach,i,$(IDLTESTFILES) $(subst,.idl,.urd $(MISC)/$(TARGET)/$i))
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(foreach,i,$(IDLTESTFILES) $(subst,.idl,.urd $(MISC)/$(TARGET)/$i)): \
        $(IDLTESTFILES)
    - $(MKDIR) $(MISC)/$(TARGET)
    $(IDLC) -O$(MISC)/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

.ENDIF
