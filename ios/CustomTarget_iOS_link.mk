#  -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


#- Env ------------------------------------------------------------------------
IOSGEN   = $(SRCDIR)/ios/generated
IOSLIBS := $(shell \
        (export INSTDIR=$(INSTDIR);export OS=$(OS); \
         export WORKDIR=$(WORKDIR);export LIBO_LIB_FOLDER=$(LIBO_LIB_FOLDER); \
         $(SRCDIR)/bin/lo-all-static-libs))
IOSOBJ = $(WORKDIR)/ios/Kit.o
IOSSRC = $(SRCDIR)/ios/source/LibreOfficeKit.c

ifeq ($(ENABLE_DEBUG),TRUE)
ifeq ($(CPUNAME),X86_64)
IOSKIT = $(IOSGEN)/simulator/libKit.dylib
else
IOSKIT = $(IOSGEN)/debug/libKit.dylib
endif
else
ifeq ($(CPUNAME),ARM64)
IOSKIT = $(IOSGEN)/release/libKit.dylib
endif
endif



#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_link))

$(call gb_CustomTarget_get_target,ios/iOS_link): $(IOSKIT)


#- build  ---------------------------------------------------------------------
$(IOSOBJ): $(IOSSRC) $(call gb_CustomTarget_get_target,ios/iOS_setup)
	$(call gb_Output_announce,iOS compile interface,$(true),C,2)
	$(gb_CC) $(gb_COMPILERDEFS) $(gb_OSDEFS) $(gb_CFLAGS) \
		-DDISABLE_DYNLOADING -DLIBO_INTERNAL_ONLY \
		-fvisibility=hidden -Werror -O0 -fstrict-overflow \
		$(if $(ENABLE_DEBUG),$(gb_DEBUG_CFLAGS) -g) \
		-c $(IOSSRC) -o $(IOSOBJ) \
		-I$(SRCDIR)/include -I$(BUILDDIR)/config_host \

$(IOSKIT): $(IOSOBJ) $(IOSLIBS)
	$(call gb_Output_announce,iOS dylib,$(true),LNK,2)
	$(gb_CC) -dynamiclib \
	    -Xlinker -rpath -Xlinker @executable_path/Frameworks \
	    -Xlinker -rpath -Xlinker @loader_path/Frameworks \
        -install_name @rpath/libKit.dylib \
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
	    $(IOSLIBS) \
	    $(IOSOBJ) \
	    -o $(IOSKIT)
ifeq ($(origin IOS_CODEID),undefined)
	@echo "please define environment variable IOS_CODEID as\n" \
	      "export IOS_CODEID=<your apple code identifier>"
	@exit -1
else
	codesign -s "$(IOS_CODEID)" $(IOSKIT)
endif


#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_link):
	rm -f $(IOSKIT).dylib



# vim: set noet sw=4 ts=4:
