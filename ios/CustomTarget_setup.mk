# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSGEN := $(SRCDIR)/ios/generated
IOSKIT := $(IOSGEN)/loKit.xcconfig
IOSAPP := $(IOSGEN)/loApp.xcconfig


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/setup))

$(call gb_CustomTarget_get_target,ios/setup): $(IOSKIT) $(IOSAPP)


#- Generate xcconfig files  ---------------------------------------------------
$(IOSKIT) : $(BUILDDIR)/config_host.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	@mkdir -p $(IOSGEN);
	@echo "// Xcode configuration properties" > $(IOSKIT)
	@echo "// To avoid confusion, the LO-specific ones that aren't as such used" >> $(IOSKIT)
	@echo "// Xcode (but only expanded in option values) are prefixed with" >> $(IOSKIT)
	@echo "// LO_.\n" >> $(IOSKIT)
	@echo "LO_BUILDDIR = $(BUILDDIR)" >> $(IOSKIT)
	@echo "LO_INSTDIR = $(INSTDIR)" >> $(IOSKIT)
	@echo "LO_SRCDIR = $(SRC_ROOT)" >> $(IOSKIT)
	@echo "LO_WORKDIR = $(WORKDIR)" >> $(IOSKIT)
	@echo "\n// These are actual Xcode-known settings. The corresponding autoconf" >> $(IOSKIT)
	@echo "// variables are prefixed with XCODE_ to make it clear in configure.ac" >> $(IOSKIT)
	@echo "// what they will be used for." >> $(IOSKIT)
	@echo "ARCHS = $(XCODE_ARCHS)" >> $(IOSKIT)
	@echo "VALID_ARCHS = $(XCODE_ARCHS)" >> $(IOSKIT)
	@echo "CLANG_CXX_LIBRARY = $(XCODE_CLANG_CXX_LIBRARY)" >> $(IOSKIT)
	@echo "DEBUG_INFORMATION_FORMAT=$(XCODE_DEBUG_INFORMATION_FORMAT)" >> $(IOSKIT)
	@echo "\n// These settings are edited in CustomTarget_Lo_Xcconfig.mk." >> $(IOSKIT)

	@echo "OTHER_CFLAGS = $(gb_GLOBALDEFS)" >> $(IOSKIT)
	@echo "OTHER_CPLUSPLUSFLAGS = $(gb_GLOBALDEFS)" >> $(IOSKIT)
	@echo "LINK_LDFLAGS = -Wl,-lz,-liconv,-map,$(WORKDIR)/iosKit.map " \
              "`$(SRCDIR)/bin/lo-all-static-libs`" >> $(IOSKIT)
	@echo "SYMROOT = $(WORKDIR)/ios/build\n" >> $(IOSKIT)

$(IOSAPP) : $(BUILDDIR)/config_host.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	@mkdir -p $(IOSGEN);
	@echo "// Xcode configuration properties" > $(IOSAPP)
	@echo "// To avoid confusion, the LO-specific ones that aren't as such used" >> $(IOSAPP)
	@echo "// Xcode (but only expanded in option values) are prefixed with" >> $(IOSAPP)
	@echo "// LO_.\n" >> $(IOSAPP)
	@echo "LO_BUILDDIR = $(BUILDDIR)" >> $(IOSAPP)
	@echo "LO_INSTDIR = $(INSTDIR)" >> $(IOSAPP)
	@echo "LO_SRCDIR = $(SRC_ROOT)" >> $(IOSAPP)
	@echo "LO_WORKDIR = $(WORKDIR)" >> $(IOSAPP)
	@echo "\n// These are actual Xcode-known settings. The corresponding autoconf" >> $(IOSAPP)
	@echo "// variables are prefixed with XCODE_ to make it clear in configure.ac" >> $(IOSAPP)
	@echo "// what they will be used for." >> $(IOSAPP)
	@echo "ARCHS = $(XCODE_ARCHS)" >> $(IOSAPP)
	@echo "VALID_ARCHS = $(XCODE_ARCHS)" >> $(IOSAPP)
	@echo "CLANG_CXX_LIBRARY = $(XCODE_CLANG_CXX_LIBRARY)" >> $(IOSAPP)
	@echo "DEBUG_INFORMATION_FORMAT=$(XCODE_DEBUG_INFORMATION_FORMAT)" >> $(IOSAPP)
	@echo "\n// These settings are edited in CustomTarget_Lo_Xcconfig.mk." >> $(IOSAPP)

	@echo "OTHER_CFLAGS = $(gb_GLOBALDEFS)" >> $(IOSAPP)
	@echo "OTHER_CPLUSPLUSFLAGS = $(gb_GLOBALDEFS)" >> $(IOSAPP)
	@echo "LINK_LDFLAGS = -Wl,-lz,-liconv,-map,$(WORKDIR)/iosApp.map " >> $(IOSAPP)
	@echo "SYMROOT = $(WORKDIR)/ios/build\n" >> $(IOSAPP)



$(call gb_CustomTarget_get_clean_target,ios/setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSGEN)


# vim: set noet sw=4 ts=4:
