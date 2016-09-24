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

# Builds the Database examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=DataBaseExamples
OUT_APP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)

APP1_NAME=CodeSamples
APP1_JAR=$(OUT_APP_CLASS)/$(APP1_NAME).jar
APP2_NAME=OpenQuery
APP2_JAR=$(OUT_APP_CLASS)/$(APP2_NAME).jar
APP3_NAME=RowSet
APP3_JAR=$(OUT_APP_CLASS)/$(APP3_NAME).jar

APP1_JAVAFILES  = \
	Sales.java \
	SalesMan.java \
	CodeSamples.java \
	sdbcx.java

APP3_JAVAFILES  = \
	RowSetEventListener.java \
	RowSet.java

APP1_CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(APP1_JAVAFILES))
APP1_CLASSNAMES = $(patsubst %.java,%.class,$(APP1_JAVAFILES))

APP3_CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(APP3_JAVAFILES))
APP3_CLASSNAMES = $(patsubst %.java,%.class,$(APP3_JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_APP_CLASS))

# Targets
.PHONY: ALL
ALL : \
	$(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(APP1_CLASSFILES) : $(APP1_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $(APP1_JAVAFILES) 

$(APP3_CLASSFILES) : $(APP3_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $(APP3_JAVAFILES) 

$(OUT_APP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $<

$(OUT_APP_CLASS)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(APP1_JAR) : $(OUT_APP_CLASS)/$(APP1_NAME).mf $(APP1_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(basename $(@F)).mf $(APP1_CLASSNAMES)
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP2_JAR) : $(OUT_APP_CLASS)/$(APP2_NAME).mf $(OUT_APP_CLASS)/$(APP2_NAME).class
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(basename $(@F)).mf $(basename $(@F)).class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP3_JAR) : $(OUT_APP_CLASS)/$(APP3_NAME).mf $(APP3_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(basename $(@F)).mf $(APP3_CLASSNAMES)
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(EXAMPLE_NAME) : $(APP1_JAR) $(APP2_JAR) $(APP3_JAR)
	@echo --------------------------------------------------------------------------------
	@echo This examples needs a running database with name "$(QM)MYDB0$(QM)". Make sure that you 
	@echo have created such a database and have established a connection to it.
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) CodeSamples.run
	@echo $(MAKE) OpenQuery.run
	@echo $(MAKE) RowSet.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_APP_CLASS)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_CLASS))
