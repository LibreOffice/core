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

# Builds the OfficeDevDesktopEnvironment native library of the Developers Guide.

PRJ=../../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

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
	$(SLOFILES) jawt.lib $(LIBO_SDK_LDFLAGS_STDLIBS) user32.lib
	$(LINK_MANIFEST)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_SLO))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_MISC))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SHAREDLIB_OUT))$(PS)*$(SHL_NAME)*.*)
