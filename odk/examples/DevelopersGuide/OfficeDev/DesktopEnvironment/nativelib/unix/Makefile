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
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(SDK_JAVA_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(SHL_NAME).$(SHAREDLIB_EXT) : $(SLOFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) $(LINK_JAVA_LIBS) -o $@ $< \
	-ljawt $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_SHL_SLO))
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT))$(PS)*$(SHL_NAME)*.*
