#*************************************************************************
#
#   $RCSfile: javaunittest.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2004-07-23 15:10:42 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
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

TESTS := $(subst,.java,.test $(JAVATESTFILES))
JAVAFILES +:= $(JAVATESTFILES)
JARFILES +:= OOoRunner.jar

.INCLUDE: settings.mk

.IF "$(XCLASSPATH)" == ""
XCLASSPATH := $(CLASSDIR)
.ELSE
XCLASSPATH !:= $(XCLASSPATH)$(PATH_SEPERATOR)$(CLASSDIR)
.ENDIF
CLASSDIR !:= $(CLASSDIR)$/test

.INCLUDE: target.mk

ALLTAR: $(TESTS)

$(JAVAFILES): $(MISC)$/$(TARGET).classdir.flag

$(MISC)$/$(TARGET).classdir.flag:
    - $(MKDIR) $(CLASSDIR)
    $(TOUCH) $@

.IF "$(TESTS)" != ""
$(TESTS): $(JAVACLASSFILES)
.ENDIF

%.test .PHONY: %.java
    java -classpath $(CLASSPATH) org.openoffice.Runner -TestBase java_complex \
        -NoOffice yes -o $(subst,$/,. $(subst,.test, $(PACKAGE).$@))

.IF "$(IDLTESTFILES)" != ""

# The following dependency (to execute javac whenever javamaker has run) does
# not work reliably, see #i28827#:
$(JAVAFILES) $(JAVACLASSFILES): $(MISC)$/$(TARGET).javamaker.flag

$(MISC)$/$(TARGET).javamaker.flag: $(MISC)$/$(TARGET).rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD $< -X$(SOLARBINDIR)$/types.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb .ERRREMOVE: \
        $(foreach,i,$(IDLTESTFILES) $(subst,.idl,.urd $(MISC)$/$(TARGET)$/$i))
    - rm $@
    $(REGMERGE) $@ /UCR $<

$(foreach,i,$(IDLTESTFILES) $(subst,.idl,.urd $(MISC)$/$(TARGET)$/$i)): \
        $(IDLTESTFILES)
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

.ENDIF
