#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Date: 2004-10-04 19:57:43 $
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
#
#*************************************************************************

PRJ = ..$/..
TARGET  = EmbedContFrame
PRJNAME = $(TARGET)
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

