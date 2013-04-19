#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Builds the WriterDemo examples of the Developers Guide.

PRJ = ../../../..
SETTINGS = $(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

CSC_FLAGS = -warnaserror+ -incremental- -noconfig
VBC_FLAGS = -warnaserror+
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-Zi -MT
CSC_FLAGS += -debug+ -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -debug+ -define:DEBUG=1 -define:TRACE=1
else
CSC_FLAGS += -o
endif

WRITERDEMO_OUT:= $(OUT_BIN)/VB_dotNET_WriterDemo

# Targets
.PHONY : ALL
ALL : WriterDemo

include $(SETTINGS)/stdtarget.mk

# build executables
$(WRITERDEMO_OUT)/%.exe: %.vb 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	vbc $(VBC_FLAGS) \
		-target:exe \
		-out:$@ \
		-libpath:"$(PRJ)\cli" \
		-reference:cli_basetypes.dll \
		-reference:cli_uretypes.dll \
		-reference:cli_oootypes.dll \
		-reference:cli_ure.dll \
		-reference:cli_cppuhelper.dll \
		-reference:System.dll \
		-reference:System.Drawing.dll \
		-reference:System.Windows.Forms.dll \
		$^

ifeq "$(OS)" "WIN"
WriterDemo : $(WRITERDEMO_OUT)/WriterDemo.exe
	@echo -------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) WriterDemo.run
	@echo -------------------------------------------------------------------------------
else
WriterDemo : 
	@echo --------------------------------------------------------------------------------
	@echo This example works only under Windows!
	@echo --------------------------------------------------------------------------------
endif

%.run : $(WRITERDEMO_OUT)/%.exe
	@echo -------------------------------------------------------------------------------
	@echo Executing sample $@ $(UNORC) $(CONNECT_STRING)...
	@echo -------------------------------------------------------------------------------
	cd $(subst /,\\,$(WRITERDEMO_OUT)) && $(basename $@).exe

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_BIN)/VB_dotNET_WriterDemo)

