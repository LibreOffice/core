# Builds the ActiveX control.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/std.mk

CC_INCLUDES+=-I$(OUT_MISC)

#######################################################
# add path to ATL headers - REQUIRES TO BE EDIT BY USER

CC_INCLUDES+=-Ir:$(PS)MSVC603$(PS)ATL$(PS)Include

#######################################################

ifeq "$(OS)" "WIN"
# Targets

.PHONY: ALL
ALL : $(OUT) $(OUT_MISC)\so_activex.h so_activex_dll

include $(SETTINGS)/stdtarget.mk

# source files
$(OUT_SLO)$(PS)so_activex.$(OBJ_EXT) : so_activex.cpp 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<

$(OUT_SLO)$(PS)SOActiveX.$(OBJ_EXT) : SOActiveX.cpp 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<

$(OUT_SLO)$(PS)SOComWindowPeer.$(OBJ_EXT) : SOComWindowPeer.cpp 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<

$(OUT_SLO)$(PS)StdAfx2.$(OBJ_EXT) : StdAfx2.cpp 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<

$(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)so_activex.$(SHAREDLIB_EXT) : 	$(OUT_SLO)$(PS)so_activex.$(OBJ_EXT) $(OUT_SLO)$(PS)SOActiveX.$(OBJ_EXT) $(OUT_SLO)$(PS)SOComWindowPeer.$(OBJ_EXT) $(OUT_SLO)$(PS)StdAfx2.$(OBJ_EXT)
	rc $(CC_INCLUDES) /R /FO$(OUT_MISC)$(PS)so_activex.res so_activex.rc
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /DEF:so_activex.def /MAP:$(OUT_MISC)$(PS)so_activex.map /IMPLIB:$(OUT_LIB)$(PS)iso_activex.lib $^ msvcrt.lib kernel32.lib user32.lib uuid.lib advapi32.lib ole32.lib oleaut32.lib gdi32.lib urlmon.lib Shlwapi.lib oldnames.lib $(OUT_MISC)$(PS)so_activex.res

so_activex_dll : $(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)so_activex.$(SHAREDLIB_EXT)

$(OUT_MISC)\so_activex.h: so_activex.idl
	midl.exe /tlb $(OUT_MISC)\so_activex.tlb /h $(OUT_MISC)\so_activex.h /iid $(OUT_MISC)\so_activex_i.c /proxy $(OUT_MISC)\so_activex_p.c /dlldata $(OUT_MISC)\dlldata.c /Oicf so_activex.idl
endif
