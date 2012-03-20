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
OUT_SHL_MISC=$(OUT_MISC)/nativeview

CFILES = nativeview.c

SLOFILES = $(patsubst %.c,$(OUT_SHL_SLO)/%.$(OBJ_EXT),$(CFILES))

# Targets
.PHONY: ALL
ALL : \
    $(SHL_LIBRARY)

include $(SETTINGS)/stdtarget.mk

$(OUT_SHL_SLO)/%.$(OBJ_EXT) : %.c
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS_JNI) $(CC_INCLUDES) $(SDK_JAVA_INCLUDES) $(CC_DEFINES_JNI) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(SHL_NAME).$(SHAREDLIB_EXT) : $(SLOFILES) $(SHL_NAME).def
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_SHL_MISC))
	$(LINK) $(LIBRARY_LINK_FLAGS) /DEF:$(SHL_NAME).def /OUT:$@ \
	/MAP:$(OUT_SHL_MISC)/$(SHL_NAME).map $(LINK_JAVA_LIBS) \
	$(SLOFILES) jawt.lib msvcrt.lib kernel32.lib user32.lib
	$(LINK_MANIFEST)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_SLO))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_MISC))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SHAREDLIB_OUT))$(PS)*$(SHL_NAME)*.*)
