#*************************************************************************
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright 2000, 2010 Oracle and/or its affiliates.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the Java NotesAccess example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=NotesAccessExample
OUT_APP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)

APP_NAME=NotesAccess
APP_JAR=$(OUT_APP_CLASS)/$(APP_NAME).jar

JAVAFILES  = \
	NotesAccess.java

CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_APP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaNotesAccessExample

include $(SETTINGS)/stdtarget.mk

NotesAPIs : 
	@echo --------------------------------------------------------------------------------
	@echo This example uses external APIs from Lotus Notes/Domino. You can download a 
	@echo trial version from "$(QM)http://www.lotus.com/downloads$(QM)". After installation
	@echo please do the following:
	@echo  1. Add the install path to the PATH or LD_LIBRARY_PATH of this shell.
	@echo     e.g "$(QM)PATH=C:\Program Files\lotus\notes;...$(QM)"
	@echo  2. Add the "$(QM)Notes.jar$(QM)" to your CLASSPATH
	@echo     e.g. "$(QM)CLASSPATH=C:\Program Files\lotus\notes\Notes.jar;$(QM)"
	@echo --------------------------------------------------------------------------------


$(CLASSFILES) : $(JAVAFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $(JAVAFILES) 

$(OUT_APP_CLASS)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Class-Path: Notes.jar> $@
	@echo Main-Class: com.sun.star.lib.loader.Loader>> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(APP_JAR) : $(OUT_APP_CLASS)/$(APP_NAME).mf $(OUT_APP_CLASS)/$(APP_NAME).class
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(APP_NAME).mf $(APP_NAME).class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

JavaNotesAccessExample : NotesAPIs $(APP_JAR)
	@echo --------------------------------------------------------------------------------
	@echo NOTE: Before you run this example make sure that the exaternal "$(QM)Notes.jar$(QM)" can
	@echo be found besides "$(QM)$(APP_JAR)"$(QM)
	@echo and that the PATH variable includes the Notes installation directory.
	@echo -   
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) NotesAccess.run
	@echo --------------------------------------------------------------------------------

%.run: $(APP_JAR)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "$(QM)$(QM)" "$(QM)$(QM)" "$(QM)$(QM)" Stocks.nsf

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst \\,\,$(subst /,$(PS),$(OUT_APP_CLASS)))
