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

# Builds the OfficeDevDestopEnvironment example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=OfficeDevDesktopEnv
OUT_APP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)

APP1_NAME=DesktopExample
APP1_JAR=$(OUT_APP_CLASS)/$(APP1_NAME).jar

APP1_JAVAFILES  = \
		   CustomizeView.java \
		   Desk.java \
		   DocumentView.java \
		   FunctionHelper.java \
		   Interceptor.java \
		   IOnewayLink.java \
		   IShutdownListener.java \
		   JavaWindowPeerFake.java \
		   NativeView.java \
		   OfficeConnect.java \
		   OnewayExecutor.java \
		   StatusListener.java \
		   StatusView.java \
		   ViewContainer.java

APP1_CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(APP1_JAVAFILES))
APP1_CLASSNAMES = $(patsubst %.java,%.class,$(APP1_JAVAFILES)) \
		CustomizeView$(QUOTE)$$ClickListener.class \
		DocumentView$(QUOTE)$$1.class \
		DocumentView$(QUOTE)$$Reactor.class


SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_APP_CLASS))

ifeq "$(OS)" "WIN"
SUBDIR= nativelib/windows 
else
SUBDIR= nativelib/unix 
endif

# Targets
.PHONY: ALL
ALL : $(SUBDIR) \
	$(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

.PHONY : $(SUBDIR)
$(SUBDIR) :
	$(MAKE) -C $@	  

$(APP1_CLASSFILES) : $(APP1_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $(APP1_JAVAFILES) 

$(OUT_APP_CLASS)/$(APP1_NAME).mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: Desk>> $@

$(APP1_JAR) : $(OUT_APP_CLASS)/$(APP1_NAME).mf $(APP1_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(basename $(@F)).mf $(APP1_CLASSNAMES)
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(EXAMPLE_NAME) : $(APP1_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) DesktopExample.run
	@echo ------
	@echo If you want to run the $(JAR1_JAR) file please set your
	@echo CLASSPATH = $(SDK_CLASSPATH)
	@echo Start the example with jar -jar $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_APP_CLASS)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

.PHONY: clean
clean :
	$(MAKE) -C $(SUBDIR) clean
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_CLASS))
