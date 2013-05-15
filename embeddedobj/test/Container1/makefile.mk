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

PRJ = ..$/..
TARGET  = EmbedContFrame
PRJNAME = embeddedobj
PACKAGE = embeddedobj$/test

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

# EXEC_CLASSPATH_TMP = \
# 	$(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)$(LIBO_PATH_SEPARATOR)
# EXEC_CLASSPATH = \
# 	$(strip $(subst,!,$(LIBO_PATH_SEPARATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar

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
    +set PATH=$(PATH)$(LIBO_PATH_SEPARATOR)$(JDK14PATH)$/jre$/bin && \
    java -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(JDK14PATH)$/jre$/bin;$(JDK14PATH)$/jre$/lib;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

debug: $(CLASSFILES)
    +set PATH=$(PATH)$(LIBO_PATH_SEPARATOR)$(JDK14PATH)$/jre$/bin && \
    jdb -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

clean :
    -$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))

