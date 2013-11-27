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

# Builds the OfficeDevDesktopEnvironment native library of the Developers Guide.

PRJ=../../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
SHL_NAME=nativeview
SHL_LIBRARY=$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(SHL_NAME).$(SHAREDLIB_EXT)

OUT_SHL_SLO=$(OUT_SLO)/nativeview

CFILES = nativeview.c

SLOFILES = $(patsubst %.c,$(OUT_SHL_SLO)/%.$(OBJ_EXT),$(CFILES))


# Targets
.PHONY: ALL
ALL : $(SHL_LIBRARY)

include $(SETTINGS)/stdtarget.mk

$(OUT_SHL_SLO)/%.$(OBJ_EXT) : %.c
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS_JNI) $(CC_INCLUDES) $(SDK_JAVA_INCLUDES) $(CC_DEFINES_JNI) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(SHL_NAME).$(SHAREDLIB_EXT) : $(SLOFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) $(LINK_JAVA_LIBS) -o $@ $< \
	-ljawt $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STDC++LIB)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_SLO))
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT))$(PS)*$(SHL_NAME)*.*
