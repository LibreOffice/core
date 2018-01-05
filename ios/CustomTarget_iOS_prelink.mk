# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSLD = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld
IOSOBJ = $(WORKDIR)/CObject/ios/Kit.o

ifeq ($(ENABLE_DEBUG),TRUE)
IOSKIT = $(SRCDIR)/ios/generated/libKit_$(CPUNAME)_debug.a
else
IOSKIT = $(SRCDIR)/ios/generated/libKit_$(CPUNAME).a
endif



#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_prelink))

$(call gb_CustomTarget_get_target,ios/iOS_prelink): $(IOSKIT)



#- build  ---------------------------------------------------------------------
.PHONY: FORCE
FORCE:

$(IOSKIT): $(call gb_StaticLibrary_get_target,iOS_kitBridge) FORCE
	$(IOSLD) -r -ios_version_min 11.2 \
	    -syslibroot $(MACOSX_SDK_PATH) \
	    -arch `echo $(CPUNAME) |  tr '[:upper:]' '[:lower:]'` \
	    -o $(IOSOBJ) \
	    $(WORKDIR)/CObject/ios/source/LibreOfficeKit.o \
	    `$(SRCDIR)/bin/lo-all-static-libs`
	$(AR) -r $(IOSKIT) $(IOSOBJ)



#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_prelink):
	rm $(IOSKIT)



# vim: set noet sw=4 ts=4:
