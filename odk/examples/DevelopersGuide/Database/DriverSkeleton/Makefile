# Builds the Databse SDBC driver skeleton example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DatabaseSDBCDriverSkeleton
COMPONENT_IMPL_NAME=$(COMPONENT_NAME).uno.$(SHAREDLIB_EXT) 
OUT_COMP_INC=$(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_MISC=$(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)
COMPONENT_XML=skeleton.xml
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_COMP_MISC)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE = $(OUT_BIN)/$(COMPONENT_NAME).uno.zip
COMPONENT_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).uno.zip")

REGISTERFLAG = $(OUT_COMP_MISC)/devguide_$(COMPONENT_NAME)_register_component.flag
COMPONENT_TYPEFLAG = $(OUT_COMP_MISC)/devguide_$(COMPONENT_NAME)_types.flag

CXXFILES = SConnection.cxx \
	SDatabaseMetaData.cxx \
	propertyids.cxx \
	SDriver.cxx \
	SPreparedStatement.cxx \
	SResultSet.cxx \
	SResultSetMetaData.cxx \
	SServices.cxx \
	SStatement.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

TYPES = $(shell xml2cmp -types stdout $(COMPONENT_XML))
TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(OUT_COMP_INC)/$(subst .,/,$(t)).hpp)

# Targets
.PHONY: ALL
ALL : \
	DatabaseSDBCDriverSkeletonExample

include $(SETTINGS)/stdtarget.mk

$(COMPONENT_TYPEFLAG) : $(COMPONENT_XML)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(DKREGISTRYNAME)
	echo flagged > $@

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(STL_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<


$(OUT_COMP_MISC)/%.def : exports.dxp
ifeq "$(OS)" "WIN"
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	@echo EXPORTS > $@
	$(CAT) exports.dxp >> $@
endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_MISC)/$(COMPONENT_NAME).def
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_MISC)/$(COMPONENT_NAME).map \
	 /DEF:$(OUT_COMP_MISC)/$(COMPONENT_NAME).def $(SLOFILES) \
	 $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $^ \
	 $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB)
endif

$(OUT_BIN)/%.zip : $(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC)/$(PACKAGE_LIB_DIR))	 
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_MISC)/$(PACKAGE_LIB_DIR))
	cd $(OUT_COMP_MISC) && jar cvfM ../../bin/$(@F) $(PACKAGE_LIB_DIR)/$(<F)

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

DatabaseSDBCDriverSkeletonExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The Databse SDBC skeleton driver component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description and the howto of implementing a sdbc driver.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_MISC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_PACKAGE_URL))
