# Builds the OLE activex C++ component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

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

$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_INC))
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_SLO))

# Attention: so_activex.idl is no UNOIDL file!
$(OUT_COMP_INC)/so_activex.h : so_activex.idl $(OUT_COMP_GEN)
	midl.exe /tlb $(OUT_COMP_GEN)/so_activex.tlb /h $(OUT_COMP_INC)/so_activex.h \
	/iid $(OUT_COMP_INC)/so_activex_i.c /proxy $(OUT_COMP_INC)/so_activex_p.c /dlldata \
	$(OUT_COMP_INC)/dlldata.c /Oicf $<

$(OUT_COMP_GEN)/so_activex.res : so_activex.rc $(OUT_COMP_INC)/so_activex.h $(OUT_COMP_GEN)
	rc $(CC_INCLUDES) /R /FO$@ so_activex.rc			       

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cpp $(OUT_COMP_GEN)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/so_activex.res so_activex.def $(OUT_COMP_GEN)
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(COMPONENT_NAME).map \
	/DEF:so_activex.def /IMPLIB:$(OUT_LIB)/iso_activex.lib $(SLOFILES) \
	msvcrt.lib kernel32.lib user32.lib uuid.lib advapi32.lib ole32.lib oleaut32.lib gdi32.lib \
	urlmon.lib Shlwapi.lib oldnames.lib $(OUT_COMP_GEN)/so_activex.res

OleActvieXComponentExample : $(COMPONENT_IMPL)
ifeq "$(OS)" "WIN"
	@echo --------------------------------------------------------------------------------
	@echo The activex control was created, please load the "$(QM)example.html$(QM)" file
	@echo in your InternetExplorer to use the activex control.
	@echo --------------------------------------------------------------------------------
else
	@echo --------------------------------------------------------------------------------
	@echo This example works only under Windows!
	@echo --------------------------------------------------------------------------------
endif

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst /,$(PS),$(OUT_LIB)/*$(COMPONENT_NAME).*)
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).*)
