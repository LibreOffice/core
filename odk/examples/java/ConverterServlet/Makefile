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

# Builds the Java ConverterServlet example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ConverterServletExample
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	ConverterServlet.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/cos.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/servlet.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : \
    ConverterServletExample

include $(SETTINGS)/stdtarget.mk

$(OFFICE_CLASSES_DIR)/cos.jar : 
	@echo --------------------------------------------------------------------------------
	@echo You need the servlet support Java archive "$(QM)cos.jar$(QM)" for this example.
	@echo You can download it from "$(QM)http://www.servlets.com/cos/index.html$(QM)"
	@echo and then please copy it into your "$(QM)$(OFFICE_CLASSES_DIR)$(QM)" directory.
	@echo --------------------------------------------------------------------------------

$(OFFICE_CLASSES_DIR)/servlet.jar : 
	@echo --------------------------------------------------------------------------------
	@echo You need the servlet API "$(QM)servlet.jar$(QM)" for this example.
	@echo You can download it from "$(QM)http://java.sun.com/products/servlet/index.html$(QM)"
	@echo and then please copy it into your "$(QM)$(OFFICE_CLASSES_DIR)$(QM)" directory.
	@echo --------------------------------------------------------------------------------

$(CLASSFILES) : $(JAVAFILES) $(OFFICE_CLASSES_DIR)/cos.jar $(OFFICE_CLASSES_DIR)/servlet.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

ConverterServletExample : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please see the README file for more information how to use the converter 
	@echo servlet.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
