#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.3 $
#
#  last change: $Author: hr $ $Date: 2003-06-30 15:44:40 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
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

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

TYPES := \
	com.sun.star.uno.XNamingService \
	com.sun.star.uno.XComponentContext \
	com.sun.star.uno.XWeak \
	com.sun.star.uno.XAggregation \
	com.sun.star.lang.XMain \
	com.sun.star.lang.XMultiServiceFactory \
	com.sun.star.lang.XSingleComponentFactory \
	com.sun.star.lang.XTypeProvider \
	com.sun.star.lang.XComponent \
	com.sun.star.registry.XSimpleRegistry \
	com.sun.star.registry.XImplementationRegistration \
	com.sun.star.bridge.XBridgeFactory \
	com.sun.star.bridge.XUnoUrlResolver \
	com.sun.star.container.XHierarchicalNameAccess 

TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(OUT_COMP_INC)/$(subst .,/,$(t)).hpp)


# Targets
.PHONY: ALL
ALL : \
    ProUNOLifetimeExamples

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $<

$(GENHPPFILES) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(DKREGISTRYNAME)

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(GENHPPFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/%$(EXE_EXT) : $(OBJFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(EXE_EXT),.map,$(@F)) \
	  $(OBJFILES) $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $(OBJFILES) \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)
endif

ProUNOLifetimeExamples : $(OUT_BIN)/$(COMPONENT_NAME)$(EXE_EXT) $(OUT_COMP_CLASS)/MyUnoObject.class
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make MyUnoObject.run
	@echo make ProfUnoLifetime.runexe
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

%.runexe: $(OUT_BIN)/%$(EXE_EXT) 
	cd $(OUT_BIN) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst /,$(PS),$(OUT_BIN)/$(COMPONENT_NAME).*)
