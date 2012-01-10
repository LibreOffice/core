#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    +set PATH=$(PATH)$(PATH_SEPERATOR)$(JDK14PATH)$/jre$/bin && \
    java -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(JDK14PATH)$/jre$/bin;$(JDK14PATH)$/jre$/lib;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

debug: $(CLASSFILES)
    +set PATH=$(PATH)$(PATH_SEPERATOR)$(JDK14PATH)$/jre$/bin && \
    jdb -classpath "$(OUT)$/class;$(OUT)$/lib;$(OUT)$/bin;$(CLASSPATH)" embeddedobj.test.EmbedContFrame

clean :
    -$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))

