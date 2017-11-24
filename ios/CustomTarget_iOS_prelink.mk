# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSDEV = /Applications/Xcode.app/Contents/Developer/
IOSLD = $(IOSDEV)Toolchains/XcodeDefault.xctoolchain/usr/bin/ld

ifeq ($(ENABLE_DEBUG),TRUE)
IOSKIT = $(SRCDIR)/ios/generated/libLibreOfficeKit_$(CPUNAME)_debug.o
else
IOSKIT = l$(SRCDIR)/ios/generated/libLibreOfficeKit_$(CPUNAME).o
endif



#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_prelink))

$(call gb_CustomTarget_get_target,ios/iOS_prelink): $(IOSKIT)



#- build  ---------------------------------------------------------------------
.PHONY: FORCE
FORCE:

$(IOSKIT): $(call gb_StaticLibrary_get_target,iOS_kitBridge) FORCE
	$(IOSLD) -r -ios_version_min 11.1 \
	    -syslibroot $(MACOSX_SDK_PATH) \
	    -arch `echo $(CPUNAME) |  tr '[:upper:]' '[:lower:]'` \
	    -o $(IOSKIT) \
	    $(WORKDIR)/CObject/ios/source/LibreOfficeKit.o  \
	    `$(SRCDIR)/bin/lo-all-static-libs`



# prelink parms: -ios_version_min 11.0 -lz -liconv -g -Wl
#	    -e "s'@SYMROOT@'$(WORKDIR)/ios/build'g" \
#	        -sdk $(IOS_SDK) \



#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_prelink):
	rm $(IOSKIT)



# vim: set noet sw=4 ts=4:
