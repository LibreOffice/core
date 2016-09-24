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

# Builds the OLE activex C++ component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=so_activex
COMPONENT_IMPL_NAME=$(SHAREDLIB_PRE)$(COMPONENT_NAME).$(SHAREDLIB_EXT) 
COMPONENT_IMPL=$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).$(SHAREDLIB_EXT) 
OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)

MIDLFILES = so_activex.idl

CXXFILES = SOActiveX.cpp \
	 SOComWindowPeer.cpp \
	 so_activex.cpp \
	 StdAfx2.cpp

SLOFILES = $(patsubst %.cpp,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	OleActvieXComponentExample

include $(SETTINGS)/stdtarget.mk

# Attention: so_activex.idl is no UNOIDL file!
$(OUT_COMP_INC)/so_activex.h : so_activex.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
	midl.exe /tlb $(OUT_COMP_GEN)/so_activex.tlb /h $(OUT_COMP_INC)/so_activex.h \
	/iid $(OUT_COMP_INC)/so_activex_i.c /proxy $(OUT_COMP_INC)/so_activex_p.c \
	/dlldata $(OUT_COMP_INC)/dlldata.c /Oicf $<

$(OUT_COMP_GEN)/so_activex.res : so_activex.rc $(OUT_COMP_INC)/so_activex.h
	-$(MKDIR) $(subst /,$(PS),$(@D))
	rc $(CC_INCLUDES) /R /FO$@ so_activex.rc			       

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cpp
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<


# Note: If you don't use the MS Visual Studio .Net, please comment out the next line.
CL_NEW_LIB=atls.lib

$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/so_activex.res so_activex.def
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_LIB))
	@echo .
	@echo -----------------------------------------------------------------------------------
	@echo ATTENTION: If you have problems with linking the library and if you do not use the
	@echo MS .Net compiler please check the makefile and comment out the variable CL_NEW_LIB.
	@echo -----------------------------------------------------------------------------------
	@echo .
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) \
	/DEF:so_activex.def /IMPLIB:$(OUT_LIB)/iso_activex.lib $(SLOFILES) \
	$(LIBO_SDK_LDFLAGS_STDLIBS) user32.lib uuid.lib advapi32.lib ole32.lib oleaut32.lib gdi32.lib \
	urlmon.lib Shlwapi.lib oldnames.lib $(CL_NEW_LIB) $(OUT_COMP_GEN)/so_activex.res
	$(LINK_MANIFEST)

ifeq "$(OS)" "WIN"
OleActvieXComponentExample : $(COMPONENT_IMPL)
	@echo --------------------------------------------------------------------------------
	@echo The activex control was created, please load the "$(QM)example.html$(QM)" file
	@echo in your InternetExplorer to use the activex control.
	@echo --------------------------------------------------------------------------------
else
OleActvieXComponentExample :
	@echo --------------------------------------------------------------------------------
	@echo This example works only under Windows!
	@echo --------------------------------------------------------------------------------
endif

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_LIB)/*$(COMPONENT_NAME).*))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).*))
