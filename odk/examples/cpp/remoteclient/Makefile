# Builds the C++ remoteclient example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=remoteclientsample
COMPONENT_IMPL_NAME=$(SHAREDLIB_PRE)$(COMPONENT_NAME).$(SHAREDLIB_EXT) 
COMPONENT_IMPL=$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).$(SHAREDLIB_EXT) 
COMPONENT_RDB_NAME = $(COMPONENT_NAME).rdb
COMPONENT_RDB = $(OUT_BIN)/$(COMPONENT_RDB_NAME)
COMPONENT_XML=$(COMPONENT_NAME).xml

OUT_COMP_INC=$(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)

COMPOENNT_ENV_FLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_prepare_env.flag
COMPONENT_TYPEFLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_types.flag

CXXFILES = remoteclient.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

TYPES = $(shell xml2cmp -types stdout $(COMPONENT_XML))
TYPESLIST = $(foreach t,$(TYPES),-T$(t))

# Targets
.PHONY: ALL
ALL : \
	CppOfficeClientExample

include $(SETTINGS)/stdtarget.mk

# create all component sepcific output directories
# and use OUT_COMP_GEN for dependencies 
$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_INC))
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_SLO))

# This example type library will be extended by the office types
$(OUT_BIN)/%.rdb : $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ / $(DKREGISTRYNAME) 

$(COMPONENT_TYPEFLAG) : $(COMPONENT_RDB) $(COMPONENT_XML) $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(COMPONENT_RDB)
	echo flagged > $@

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG) $(OUT_COMP_GEN)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_COMP_GEN)/%.def : exports.dxp $(OUT_COMP_GEN)
ifeq "$(OS)" "WIN"
	@echo EXPORTS > $@
	$(CAT) exports.dxp >> $@
endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/$(COMPONENT_NAME).def $(OUT_COMP_GEN)
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(COMPONENT_NAME).map \
	/DEF:$(OUT_COMP_GEN)/$(COMPONENT_NAME).def $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES)
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $^\
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB)
endif	

$(COMPOENNT_ENV_FLAG) : $(COMPONENT_RDB) $(OUT_BIN)/remoteserver.rdb
	-$(DEL) $@
	@echo --------------------------------------------------------------------------------
	@echo   Register necessary runtime components in remoteserver/remoteclientsample rdb
	@echo --------------------------------------------------------------------------------
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c $(SHAREDLIB_PRE)connectr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c $(SHAREDLIB_PRE)remotebridge.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c $(SHAREDLIB_PRE)brdgfctr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c $(SHAREDLIB_PRE)uuresolver.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c $(SHAREDLIB_PRE)stm.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c $(SHAREDLIB_PRE)connectr.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c $(SHAREDLIB_PRE)remotebridge.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c $(SHAREDLIB_PRE)brdgfctr.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c $(SHAREDLIB_PRE)uuresolver.$(SHAREDLIB_EXT)
	@echo bla > $@

CppOfficeClientExample : $(COMPONENT_IMPL) $(COMPOENNT_ENV_FLAG)
	@echo --------------------------------------------------------------------------------
	@echo The remoteclient C++ component can be used by using the uno binary. Use the 
	@echo the follwong command to start the example. The run target starts a remote 
	@echo server and connect with the client to this server.
	@echo - 
	@echo make remoteclient.run
	@echo --------------------------------------------------------------------------------

%.run: $(COMPONENT_IMPL) $(COMPOENNT_ENV_FLAG)
	@echo Start the remote server process ...
	@echo -
ifeq "$(OS)" "WIN"
	start uno -rw $(OUT_BIN)/remoteserver.rdb -s com.sun.star.io.Pipe \
	-u "uno:socket,host=localhost,port=8100;urp;MyPipe"
else
	uno -rw $(OUT_BIN)/remoteserver.rdb -s com.sun.star.io.Pipe \
	-u "uno:socket,host=localhost,port=8100;urp;MyPipe" &
endif
	@echo -
	@echo ... remote server process runs ...
	@echo -
	@echo Start remote client process ...
	@echo -
	uno -l $(COMPONENT_IMPL) -c com.sun.star.comp.product.example.RemoteClientSample \
	-rw $(COMPONENT_RDB) -- "uno:socket,host=localhost,port=8100;urp;MyPipe"
	@echo -
	@echo ... remote process returned succesful, please kill the server process.
	@echo - Windows: activate the server shell and press ctrl-c, 
	@echo - Unix: use the kill command to kill the server process

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).*)
	-$(DEL) $(subst /,$(PS),$(OUT_BIN)/remoteserver.rdb)
