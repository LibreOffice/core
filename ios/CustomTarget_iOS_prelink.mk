#  -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSLIB = ''
IOSLD = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ld
IOSCLANG = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
IOSOBJ = $(WORKDIR)/CObject/ios/Kit.o

ifeq ($(ENABLE_DEBUG),TRUE)
ifeq ($(CPUNAME),X86_64)
IOSKIT = $(IOSGEN)/simulator/libKit
else
IOSKIT = $(IOSGEN)/debug/libKit
endif
else
ifeq ($(CPUNAME),ARM64)
IOSKIT = $(IOSGEN)/release/libKit
endif
endif



#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_prelink))

$(call gb_CustomTarget_get_target,ios/iOS_prelink): $(IOSKIT).dylib



#- build  ---------------------------------------------------------------------
.PHONY: FORCE
FORCE:


IOSPREBUILD: FORCE
	$(eval IOSLIBS = `$(SRCDIR)/bin/lo-all-static-libs`)



$(IOSKIT).dylib: IOSPREBUILD $(WORKDIR)/ios $(call gb_StaticLibrary_get_target,iOS_kitBridge) $(IOSLIBS)
	echo "JAN" $(IOSLIBS)
	$(IOSLD) -r -ios_version_min $(IOS_DEPLOYMENT_VERSION) \
	    -syslibroot $(MACOSX_SDK_PATH) \
	    -arch `echo $(CPUNAME) |  tr '[:upper:]' '[:lower:]'` \
	    -o $(IOSOBJ) \
	    $(WORKDIR)/CObject/ios/source/LibreOfficeKit.o \
	    $(IOSLIBS)
	$(AR) -r $(IOSKIT).a $(IOSOBJ)

	$(IOSCLANG) -dynamiclib -mios-simulator-version-min=$(IOS_DEPLOYMENT_VERSION) \
	    -arch `echo $(CPUNAME) |  tr '[:upper:]' '[:lower:]'` \
	    -isysroot $(MACOSX_SDK_PATH) \
	    -Xlinker -rpath -Xlinker @executable_path/Frameworks \
	    -Xlinker -rpath -Xlinker @loader_path/Frameworks \
	    -dead_strip \
	    -Xlinker -export_dynamic \
	    -Xlinker -no_deduplicate \
	    -Xlinker -objc_abi_version -Xlinker 2 \
	    -fobjc-link-runtime \
	    -framework CoreFoundation \
	    -framework CoreGraphics \
	    -framework CoreText \
	    -liconv \
	    -lc++ \
	    -lz \
	    -lpthread \
	    -single_module \
	    -compatibility_version 1 \
	    -current_version 1 \
	    $(IOSOBJ) \
	    -o $(IOSKIT).dylib
ifeq ($(origin IOS_CODEID),undefined)
	@echo "please define environment variable IOS_CODEID as\n" \
	      "export IOS_CODEID=<your apple code identifier>"
	@exit -1
else
	codesign -s "$(IOS_CODEID)" $(IOSKIT).dylib
endif




#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_prelink):
	rm -f $(IOSKIT).a $(IOSKIT).dylib



# vim: set noet sw=4 ts=4:
