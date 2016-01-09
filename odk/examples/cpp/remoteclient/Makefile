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

# Builds the C++ remoteclient example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMP_NAME=remoteclientsample
COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT)
COMP_IMPL=$(SHAREDLIB_OUT)/$(COMP_IMPL_NAME)
COMP_RDB_NAME = $(COMP_NAME).uno.rdb
COMP_RDB = $(OUT_BIN)/$(COMP_RDB_NAME)
COMP_SERVICES=$(SHAREDLIB_OUT)/remoteclientsample.rdb

ENV_COMP_SERVICES=-env:URE_MORE_SERVICES=$(URLPREFIX)$(COMP_SERVICES)

OUT_COMP_INC=$(OUT_INC)/$(COMP_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMP_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMP_NAME)

COMP_TYPEFLAG = $(OUT_MISC)/cpp_$(COMP_NAME)_types.flag

CXXFILES = remoteclient.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	CppClientExample

include $(SETTINGS)/stdtarget.mk

# This example type library will be extended by the URE types
$(OUT_BIN)/%.rdb :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(REGMERGE) $@ / $(URE_TYPES) 

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
	$(LINK) $(COMP_LINK_FLAGS) /OUT:$@ \
	/MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) msvcprt.lib $(LIBO_SDK_LDFLAGS_STDLIBS)
	$(LINK_MANIFEST)
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS)  $@
endif
endif	

$(COMP_SERVICES) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)components xmlns="$(QM)http://openoffice.org/2010/uno-components$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)implementation name="$(QM)com.sun.star.comp.product.example.RemoteClientSample$(QM)"$(CSEP) >> $@
	@echo $(SQM)      $(SQM)$(OSEP)service name="$(QM)com.sun.star.bridge.example.RemoteClientSample$(QM)"/$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)/implementation$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)/components$(CSEP) >> $@


#CppClientExample : $(COMP_IMPL) $(COMP_ENV_FLAG)
CppClientExample : $(COMP_IMPL) $(COMP_SERVICES)
	@echo --------------------------------------------------------------------------------
	@echo The remoteclient C++ component can be used by using the uno binary. Use the
	@echo the following command to start the example. The run target starts a remote
	@echo server and connect with the client to this server.
	@echo -
	@echo $(MAKE) remoteclient.run
	@echo -
	@echo NOTE: The example uses the $(QM)uno$(QM) tool to prepare a working UNO environment.
	@echo $(SQM)      $(SQM)The example component is made available via -env:URE_MORE_SERVICES=...
	@echo $(SQM)      $(SQM)Please check the generated $(QM)$(COMP_SERVICES)$(QM) to see how you can specify your own components in such an environment
	@echo $(SQM)      $(SQM)and how to use the passive UNO registration.
	@echo --------------------------------------------------------------------------------

%.run: $(COMP_IMPL)
	@echo Start the remote server process ...
	@echo -
ifeq "$(OS)" "WIN"
	start uno $(ENV_COMP_SERVICES) --singleaccept -s com.sun.star.io.Pipe \
	-u "uno:socket,host=localhost,port=2083;urp;MyPipe"
else
	uno $(ENV_COMP_SERVICES) --singleaccept -s com.sun.star.io.Pipe \
	-u "uno:socket,host=localhost,port=2083;urp;MyPipe" &
	@echo waiting on the server process ...
	sleep 5
endif
	@echo -
	@echo ... remote server process runs ...
	@echo -
	@echo Start remote client process ...
	@echo -
	uno $(ENV_COMP_SERVICES) -s com.sun.star.bridge.example.RemoteClientSample \
		-- "uno:socket,host=localhost,port=2083;urp;MyPipe"
	@echo -

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_TYPEFLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_SERVICES)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SHAREDLIB_OUT)/$(COMP_NAME).*))
#	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/remoteserver.rdb))
