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

# Builds the Java DocumentHandling examples of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=JavaDocumentHandlingExamples
SAMPLE_CLASS_OUT = $(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/$(SAMPLE_NAME)

DIRFLAG=$(OUT_MISC)/$(SAMPLE_NAME)_directory.flag

APP1_NAME=DocumentConverter
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar
APP2_NAME=DocumentLoader
APP2_JAR=$(SAMPLE_CLASS_OUT)/$(APP2_NAME).jar
APP3_NAME=DocumentPrinter
APP3_JAR=$(SAMPLE_CLASS_OUT)/$(APP3_NAME).jar
APP4_NAME=DocumentSaver
APP4_JAR=$(SAMPLE_CLASS_OUT)/$(APP4_NAME).jar

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : \
	$(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(SAMPLE_CLASS_OUT)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $<

$(SAMPLE_CLASS_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(SAMPLE_CLASS_OUT)/%.jar : $(SAMPLE_CLASS_OUT)/%.mf $(SAMPLE_CLASS_OUT)/%.class
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) cvfm $(@F) $*.mf $*.class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP1_JAR) : $(SAMPLE_CLASS_OUT)/$(APP1_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP1_NAME).class
$(APP2_JAR) : $(SAMPLE_CLASS_OUT)/$(APP2_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP2_NAME).class
$(APP3_JAR) : $(SAMPLE_CLASS_OUT)/$(APP3_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP3_NAME).class
$(APP4_JAR) : $(SAMPLE_CLASS_OUT)/$(APP4_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP4_NAME).class

$(SAMPLE_NAME) : $(APP1_JAR) $(APP2_JAR) $(APP3_JAR) $(APP4_JAR)
	@echo --------------------------------------------------------------------------------
	@echo The $(APP1_NAME) search the "$(QM)./test$(QM)" directory for documents, convert
	@echo them using the "$(QM)MS Word 97$(QM)" filter and the extension "$(QM).doc$(QM)".
	@echo The converted files are store in "$(QM)$(SAMPLE_GEN_OUT)/converted_files$(QM)".
	@echo The list of possible filter names can change. Normally an updated list can be found 
	@echo on "$(QM)http://www.openoffice.org/files/documents/25/111/filter_description.html$(QM)".
	@echo -
	@echo The $(APP2_NAME) loads the document "$(QM)./test/test1.odt$(QM)".
	@echo -
	@echo The $(APP3_NAME) prints the document "$(QM)./test/test1.odt$(QM)" using the
	@echo the specified printer. If the printer is unknown, the default printer is used.
	@echo -
	@echo The $(APP5_NAME) loads the document "$(QM)./test/test1.odt$(QM)" and saves it
	@echo under "$(QM)$(SAMPLE_GEN_OUT)/savetest/testsave.odt$(QM)".
	@echo -
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo $(MAKE) $(APP3_NAME).run
	@echo $(MAKE) $(APP4_NAME).run
	@echo --------
	@echo The examples need parameters. Please use one the following commands to 
	@echo start the demo if you do not want the default parameters specified in the 
	@echo this makefile. Starting without parameters print a command line help:
	@echo --- $(APP1_NAME) ---
	@echo java -jar $(APP1_NAME).jar "$(QM)directory$(QM)" "$(QM)filter name$(QM)" "$(QM)extension$(QM)" "$(QM)output_directory$(QM)"
	@echo --- $(APP2_NAME) ---
	@echo java -jar $(APP2_NAME).jar "$(QM)Url|path$(QM)"
	@echo --- $(APP3_NAME) ---
	@echo java -jar $(APP3_NAME).jar "$(QM)printername$(QM)" "$(QM)filename$(QM)" "$(QM)pages$(QM)"
	@echo --- $(APP4_NAME) ---
	@echo java -jar $(APP4_NAME).jar "$(QM)load Url|path$(QM)" "$(QM)save Url|path$(QM)"
	@echo --------------------------------------------------------------------------------

$(APP1_NAME).run: $(APP1_JAR)
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/converted_files)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "./test" "MS Word 97" "doc" "$(SAMPLE_GEN_OUT)/converted_files"

$(APP2_NAME).run: $(APP2_JAR)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "./test/test1.odt"

$(APP3_NAME).run: $(APP3_JAR)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "my_printer" "./test/test1.odt" 1

$(APP4_NAME).run: $(APP4_JAR)
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/savetest)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "./test/test1.odt" "$(SAMPLE_GEN_OUT)/savetest/testsave.odt"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
