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

# Builds the ProfUNO lifetime example of the Developers Guide

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ProfUnoLifetime
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)
OUT_COMP_INC=$(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)

JAVAFILES  = \
	MyUnoObject.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/JavaComp/%.class,$(JAVAFILES))

CXXFILES = object_lifetime.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_COMP_OBJ)/%.$(OBJ_EXT),$(CXXFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    ProUNOLifetimeExamples

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $<

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/$(COMPONENT_NAME)$(EXE_EXT) : $(OBJFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(EXE_EXT),.map,$(@F)) \
	  $(OBJFILES) $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $(OBJFILES) \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALHELPERDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

ProUNOLifetimeExamples : $(OUT_BIN)/$(COMPONENT_NAME)$(EXE_EXT) $(OUT_COMP_CLASS)/MyUnoObject.class
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) MyUnoObject.run
	@echo $(MAKE) ProfUnoLifetime.runexe
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

%.runexe: $(OUT_BIN)/%$(EXE_EXT) 
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/$(COMPONENT_NAME)$(EXE_EXT)))
