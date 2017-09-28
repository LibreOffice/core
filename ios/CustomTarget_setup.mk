# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSGEN := $(SRCDIR)/ios/generated
IOSKIT := $(BUILDDIR)/ios/loKit.xcconfig
IOSAPP := $(BUILDDIR)/ios/loApp.xcconfig
IOSRES := $(IOSGEN)/resources


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/setup))

$(call gb_CustomTarget_get_target,ios/setup): $(IOSKIT) $(IOSAPP) iosCopySetup


#- Generate xcconfig files  ---------------------------------------------------
$(IOSKIT) : $(BUILDDIR)/config_host.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
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

#- Generate setup  ------------------------------------------------------------
.PHONY : iosCopySetup
iosCopySetup:
ifeq ("$(wildcard $(IOSRES))","")
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)

	mkdir -p $(IOSGEN) $(IOSRES) $(IOSRES)/services \
	         $(IOSRES)/share/config $(IOSRES)/share/filter $(IOSRES)/program

	# generate file with call declarations
	$(SRCDIR)/solenv/bin/native-code.py \
	    -g core -g writer -g calc -g draw -g edit \
	    > $(IOSGEN)/native-code.mm

	# generate resource files used to start/run LibreOffice
	cp $(INSTDIR)/program/types.rdb             $(IOSRES)/udkapi.rdb
	cp $(INSTDIR)/program/types/offapi.rdb      $(IOSRES)
	cp $(INSTDIR)/program/types/oovbaapi.rdb    $(IOSRES)
	cp $(INSTDIR)/program/services/services.rdb $(IOSRES)/services
	cp $(INSTDIR)/program/services.rdb          $(IOSRES)
	cp -R $(INSTDIR)/share/config/soffice.cfg $(IOSRES)/share/config
	cp $(WORKDIR)/CustomTarget/i18npool/breakiterator/dict_*.data $(IOSRES)/share
	cp $(INSTDIR)/share/filter/oox-drawingml-adj-names $(IOSRES)/share/filter
	cp $(INSTDIR)/share/filter/oox-drawingml-cs-presets $(IOSRES)/share/filter
	cp $(INSTDIR)/share/filter/vml-shape-types $(IOSRES)/share/filter
	cp -R $(INSTDIR)/share/registry $(IOSRES)/share

	# Set up rc, the "inifile". See getIniFileName_Impl().
	echo '[Bootstrap]' > $(IOSRES)/rc
	echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' >> $(IOSRES)/rc
	echo 'HOME=$$SYSUSERHOME' >> $(IOSRES)/rc

	# Set up fundamentalrc, unorc, bootstraprc and versionrc.
	file=$(IOSRES)/fundamentalrc; \
	echo '[Bootstrap]'                                      >  $(IOSRES)/fundamentalrc
	echo 'LO_LIB_DIR=file://$$APP_DATA_DIR/lib/'            >> $(IOSRES)/fundamentalrc
	echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR'             >> $(IOSRES)/fundamentalrc
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry ' \
	     'res:$${BRAND_BASE_DIR}/registry' >> $(IOSRES)/fundamentalrc
	echo 'UNO_TYPES=file://$$APP_DATA_DIR/udkapi.rdb ' \
	     'file://$$APP_DATA_DIR/offapi.rdb' >> $(IOSRES)/fundamentalrc
	echo 'UNO_SERVICES=file://$$APP_DATA_DIR/services.rdb ' \
	     'file://$$APP_DATA_DIR/services/services.rdb' >> $(IOSRES)/fundamentalrc
	echo 'OSL_SOCKET_PATH=$$APP_DATA_DIR/cache' >> $(IOSRES)/fundamentalrc

	echo '[Bootstrap]' > $(IOSRES)/unorc

	# bootstraprc must be in $BRAND_BASE_DIR/program
	echo '[Bootstrap]'                                     >  $(IOSRES)/program/bootstraprc
	echo 'InstallMode=<installmode>'                       >> $(IOSRES)/program/bootstraprc
	echo "ProductKey=LibreOffice $(PRODUCTVERSION)"        >> $(IOSRES)/program/bootstraprc
	echo 'UserInstallation=$$SYSUSERHOME/userinstallation' >> $(IOSRES)/program/bootstraprc

	# Is this really needed?
	echo '[Version]'            >  $(IOSRES)/program/versionrc
	echo 'AllLanguages=en-US'   >> $(IOSRES)/program/versionrc
	echo 'BuildVersion='        >> $(IOSRES)/program/versionrc
	echo "buildid=$(BUILDID)"   >> $(IOSRES)/program/versionrc
endif


#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSGEN) $(IOSKIT) $(IOSAPP)


# vim: set noet sw=4 ts=4:
