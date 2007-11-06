#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:38:25 $
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

PRJ := ..$/..
PRJNAME := cppuhelper

TARGET := qa_propertysetmixin
PACKAGE = test$/cppuhelper$/propertysetmixin$/comp

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(GUI)" == "WNT"
FILEURLPREFIX = file:///
MY_URE_INTERNAL_JAVA_DIR=$(strip $(subst,\,/ file:///$(shell @$(WRAPCMD) echo $(SOLARBINDIR))))
.ELSE
FILEURLPREFIX = file://
MY_URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
.ENDIF



DLLPRE = # no leading "lib" on .so files
INCPRE += -I$(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/test_propertysetmixin.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(CPPUNITLIB) $(SALLIB)
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

.IF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL1VERSIONMAP = test.LINUXIgcc3.map
.ELSE
SHL1VERSIONMAP = test.map
.ENDIF

SHL2TARGET = $(TARGET).uno
SHL2OBJS = $(SLO)$/comp_propertysetmixin.obj
SHL2VERSIONMAP = comp.map
SHL2STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL2IMPLIB = i$(SHL2TARGET)
DEF2NAME = $(SHL2TARGET)

SLOFILES = $(SHL1OBJS) $(SHL2OBJS)

JAVAFILES = JavaSupplier.java
JARFILES = java_uno.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: test

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(@:d)
    $(IDLC) -O$(@:d) -I$(SOLARIDLDIR) -cid -we $<

$(MISC)$/$(TARGET)$/types.rdb .ERRREMOVE: $(MISC)$/$(TARGET)$/types.urd
    - $(RM) $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/uno.rdb .ERRREMOVE: $(MISC)$/$(TARGET)$/types.rdb \
        $(DLLDEST)$/$(SHL2TARGET)$(DLLPOST) \
        $(MISC)$/$(TARGET)$/$(TARGET).uno.jar $(MISC)$/$(TARGET)$/bootstrap.rdb
    - $(MKDIR) $(@:d)
    $(COPY) $(SOLARBINDIR)$/types.rdb $@
    $(REGMERGE) $@ / $(MISC)$/$(TARGET)$/types.rdb
    $(REGCOMP) -register -r $@ -c javaloader.uno$(DLLPOST) \
        -c javavm.uno$(DLLPOST) -c reflection.uno$(DLLPOST) \
        -c stocservices.uno$(DLLPOST)
    $(REGCOMP) -register -r $@ \
        -c $(subst,$/,/ $(DLLDEST)$/$(SHL2TARGET)$(DLLPOST))
    $(REGCOMP) -register -br $(MISC)$/$(TARGET)$/bootstrap.rdb -r $@ \
        -c \
   $(subst,$/,/ $(FILEURLPREFIX)$(PWD)$/$(MISC)$/$(TARGET)$/$(TARGET).uno.jar) \
        -classpath $(CLASSPATH) \
    -env:URE_INTERNAL_JAVA_DIR=$(MY_URE_INTERNAL_JAVA_DIR)

$(MISC)$/$(TARGET)$/bootstrap.rdb .ERRREMOVE:
    - $(MKDIR) $(@:d)
    $(COPY) $(SOLARBINDIR)$/types.rdb $@
    $(REGCOMP) -register -r $@ -c javaloader.uno$(DLLPOST) \
        -c javavm.uno$(DLLPOST) -c stocservices.uno$(DLLPOST)

$(MISC)$/$(TARGET)$/cppumaker.flag: $(MISC)$/$(TARGET)$/types.rdb
    $(CPPUMAKER) -O$(MISC)$/$(TARGET)$/inc -BUCR -Gc \
        -X$(SOLARBINDIR)$/types.rdb $<
    $(TOUCH) $@

$(SLOFILES): $(MISC)$/$(TARGET)$/cppumaker.flag

$(MISC)$/$(TARGET)$/javamaker.flag: $(MISC)$/$(TARGET)$/types.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -Gc -X$(SOLARBINDIR)$/types.rdb $<
    $(TOUCH) $@

# The following dependency (to execute javac whenever javamaker has run) does
# not work reliably, see #i28827#:
$(JAVAFILES) $(JAVACLASSFILES): $(MISC)$/$(TARGET)$/javamaker.flag

$(MISC)$/$(TARGET)$/$(TARGET).uno.jar: $(JAVACLASSFILES) \
        $(MISC)$/$(TARGET)$/javamaker.flag manifest
    jar cfm $@ manifest -C $(CLASSDIR) test/cppuhelper/propertysetmixin

test .PHONY: $(SHL1TARGETN) $(MISC)$/$(TARGET)$/uno.rdb
.IF "$(GUI)" == "WNT"
    set CLASSPATH=$(CLASSPATH) && \
    set URE_INTERNAL_JAVA_DIR=$(MY_URE_INTERNAL_JAVA_DIR) && \
    testshl2 $(SHL1TARGETN) -forward "$(MISC)$/$(TARGET)$/uno.rdb#$(SOLARBINDIR)"
.ELSE
    setenv CLASSPATH $(CLASSPATH) && \
    setenv URE_INTERNAL_JAVA_DIR $(MY_URE_INTERNAL_JAVA_DIR) && \
    testshl2 $(SHL1TARGETN) -forward "$(MISC)$/$(TARGET)$/uno.rdb#$(SOLARLIBDIR)"
.ENDIF
