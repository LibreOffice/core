#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.6 $
#
#  last change: $Author: hr $ $Date: 2003-06-30 15:53:19 $
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

# Builds the C++ remoteclient example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=remoteclientsample
COMPONENT_IMPL_NAME=$(COMPONENT_NAME).uno.$(SHAREDLIB_EXT) 
COMPONENT_IMPL=$(SHAREDLIB_OUT)/$(COMPONENT_IMPL_NAME)
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_BIN)/$(COMPONENT_RDB_NAME)
COMPONENT_XML=$(COMPONENT_NAME).uno.xml

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

# This example type library will be extended by the office types
$(OUT_BIN)/%.rdb :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ / $(DKREGISTRYNAME) 

$(COMPONENT_TYPEFLAG) : $(COMPONENT_RDB) $(COMPONENT_XML)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(COMPONENT_RDB)
	echo flagged > $@

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_COMP_GEN)/%.def : exports.dxp
ifeq "$(OS)" "WIN"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo EXPORTS > $@
	$(CAT) exports.dxp >> $@
endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/%.def
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) \
	/DEF:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),def,$(@F)) $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $^\
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB)
endif	

$(COMPOENNT_ENV_FLAG) : $(COMPONENT_RDB) $(OUT_BIN)/remoteserver.rdb
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $@
	@echo --------------------------------------------------------------------------------
	@echo   Register necessary runtime components in remoteserver/remoteclientsample rdb
	@echo --------------------------------------------------------------------------------
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c connector.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c remotebridge.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c bridgefac.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c uuresolver.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/remoteserver.rdb -c streams.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c connector.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c remotebridge.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c bridgefac.uno.$(SHAREDLIB_EXT)
	regcomp -register -r $(COMPONENT_RDB) -c uuresolver.uno.$(SHAREDLIB_EXT)
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
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT)/$(COMPONENT_NAME).*)
	-$(DEL) $(subst /,$(PS),$(OUT_BIN)/remoteserver.rdb)
