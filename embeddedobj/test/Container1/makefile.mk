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
# $Revision: 1.9 $
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

PRJ = ..$/..
TARGET  = EmbedContFrame
PRJNAME = embeddedobj
PACKAGE = embeddedobj$/test

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

# EXEC_CLASSPATH_TMP = \
# 	$(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)
# EXEC_CLASSPATH = \
# 	$(strip $(subst,!,$(PATH_SEPERATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar

JAVAFILES  = \
    EmbedContApp.java\
    EmbedContFrame.java\
    NativeView.java\
    WindowHelper.java\
    JavaWindowPeerFake.java\
    BitmapPainter.java\
    PaintThread.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))


# --- Targets ------------------------------------------------------

.INCLUDE: target.mk

ALL : $(OUT)$/slo$/nativeview.obj
    JavaStorageTestExample

JavaStorageTestExample : $(CLASSFILES)
    @echo --------------------------------------------------------------------------------
    @echo "Please use following command to execute the example!"
    @echo ------
    @echo "dmake run"
    @echo --------------------------------------------------------------------------------

# $(OUT)$/slo$/nativeview.obj:
    # cd nativelib; dmake debug=t; cd ..

# echo $(SOLARBINDIR)
# echo $(EXEC_CLASSPATH)

run: $(CLASSFILES)
    +set PATH=$(PATH)$(PATH_SEPERATOR)$(JDK14PATH)$/jre$/bin && \
    java -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(JDK14PATH)$/jre$/bin;$(JDK14PATH)$/jre$/lib;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

debug: $(CLASSFILES)
    +set PATH=$(PATH)$(PATH_SEPERATOR)$(JDK14PATH)$/jre$/bin && \
    jdb -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

clean :
    -$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))

