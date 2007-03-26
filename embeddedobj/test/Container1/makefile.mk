#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2007-03-26 11:58:04 $
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

