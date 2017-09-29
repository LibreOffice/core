# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSGEN := $(SRCDIR)/ios/generated
IOSRES := $(IOSGEN)/resources
IOSKITXC := $(BUILDDIR)/ios/loKit.xcconfig
IOSAPPXC := $(BUILDDIR)/ios/loApp.xcconfig
IOSKITPRJ := $(SRCDIR)/ios/LibreOfficeKit/LibreOfficeKit.xcodeproj
IOSAPPPRJ := $(SRCDIR)/ios/LibreOfficeLight/LibreOfficeLight.xcodeproj
IOSAPP := $(INSTDIR)/LibreOfficeLight.app
ifeq ($(ENABLE_DEBUG),TRUE)
IOSKIT := iOSkit_$(CPUNAME)_debug.a
else
IOSKIT := iOSkit_$(CPUNAME).a
endif


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/ios))

$(call gb_CustomTarget_get_target,ios/ios): $(IOSGEN)/$(IOSKIT)


#- Generate xcconfig files  ---------------------------------------------------
$(IOSKITXC) : $(BUILDDIR)/config_host.mk $(SRCDIR)/ios/CustomTarget_iOS.mk 
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	@echo "// Xcode configuration properties" > $(IOSKITXC)
	@echo "// To avoid confusion, the LO-specific ones that aren't as such used" >> $(IOSKITXC)
	@echo "// Xcode (but only expanded in option values) are prefixed with" >> $(IOSKITXC)
	@echo "// LO_.\n" >> $(IOSKITXC)
	@echo "LO_BUILDDIR = $(BUILDDIR)" >> $(IOSKITXC)
	@echo "LO_INSTDIR = $(INSTDIR)" >> $(IOSKITXC)
	@echo "LO_SRCDIR = $(SRC_ROOT)" >> $(IOSKITXC)
	@echo "LO_WORKDIR = $(WORKDIR)" >> $(IOSKITXC)
	@echo "\n// These are actual Xcode-known settings. The corresponding autoconf" >> $(IOSKITXC)
	@echo "// variables are prefixed with XCODE_ to make it clear in configure.ac" >> $(IOSKITXC)
	@echo "// what they will be used for." >> $(IOSKITXC)
	@echo "ARCHS = $(XCODE_ARCHS)" >> $(IOSKITXC)
	@echo "VALID_ARCHS = $(XCODE_ARCHS)" >> $(IOSKITXC)
	@echo "CLANG_CXX_LIBRARY = $(XCODE_CLANG_CXX_LIBRARY)" >> $(IOSKITXC)
	@echo "DEBUG_INFORMATION_FORMAT=$(XCODE_DEBUG_INFORMATION_FORMAT)" >> $(IOSKITXC)
	@echo "\n// These settings are edited in CustomTarget_Lo_Xcconfig.mk." >> $(IOSKITXC)

	@echo "OTHER_CFLAGS = $(gb_GLOBALDEFS)" >> $(IOSKITXC)
	@echo "OTHER_CPLUSPLUSFLAGS = $(gb_GLOBALDEFS)" >> $(IOSKITXC)
	@echo "LINK_LDFLAGS = -Wl,-lz,-liconv,-map,$(WORKDIR)/iosKit.map " \
              "`$(SRCDIR)/bin/lo-all-static-libs`" >> $(IOSKITXC)
	@echo "SYMROOT = $(WORKDIR)/ios/build\n" >> $(IOSKITXC)

$(IOSAPPXC) : $(BUILDDIR)/config_host.mk $(SRCDIR)/ios/CustomTarget_iOS.mk 
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	@mkdir -p $(IOSGEN);
	@echo "// Xcode configuration properties" > $(IOSAPPXC)
	@echo "// To avoid confusion, the LO-specific ones that aren't as such used" >> $(IOSAPPXC)
	@echo "// Xcode (but only expanded in option values) are prefixed with" >> $(IOSAPPXC)
	@echo "// LO_.\n" >> $(IOSAPPXC)
	@echo "LO_BUILDDIR = $(BUILDDIR)" >> $(IOSAPPXC)
	@echo "LO_INSTDIR = $(INSTDIR)" >> $(IOSAPPXC)
	@echo "LO_SRCDIR = $(SRC_ROOT)" >> $(IOSAPPXC)
	@echo "LO_WORKDIR = $(WORKDIR)" >> $(IOSAPPXC)
	@echo "\n// These are actual Xcode-known settings. The corresponding autoconf" >> $(IOSAPPXC)
	@echo "// variables are prefixed with XCODE_ to make it clear in configure.ac" >> $(IOSAPPXC)
	@echo "// what they will be used for." >> $(IOSAPPXC)
	@echo "ARCHS = $(XCODE_ARCHS)" >> $(IOSAPPXC)
	@echo "VALID_ARCHS = $(XCODE_ARCHS)" >> $(IOSAPPXC)
	@echo "CLANG_CXX_LIBRARY = $(XCODE_CLANG_CXX_LIBRARY)" >> $(IOSAPPXC)
	@echo "DEBUG_INFORMATION_FORMAT=$(XCODE_DEBUG_INFORMATION_FORMAT)" >> $(IOSAPPXC)
	@echo "\n// These settings are edited in CustomTarget_Lo_Xcconfig.mk." >> $(IOSAPPXC)

	@echo "OTHER_CFLAGS = $(gb_GLOBALDEFS)" >> $(IOSAPPXC)
	@echo "OTHER_CPLUSPLUSFLAGS = $(gb_GLOBALDEFS)" >> $(IOSAPPXC)
	@echo "LINK_LDFLAGS = -Wl,-lz,-liconv,-map,$(WORKDIR)/iosApp.map " >> $(IOSAPPXC)
	@echo "SYMROOT = $(WORKDIR)/ios/build\n" >> $(IOSAPPXC)


#- Generate ios  ------------------------------------------------------------
.PHONY : iosCopySetup
iosCopySetup: $(IOSKITXC) $(IOSAPPXC)
ifeq ("$(wildcard $(IOSRES))","")
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)

	mkdir -p $(IOSGEN) $(IOSRES) $(IOSRES)/services \
	         $(IOSRES)/share/config $(IOSRES)/share/filter $(IOSRES)/program

	# generate file with call declarations
	$(SRCDIR)/solenv/bin/native-code.py \
	    -g core -g writer -g calc -g draw -g edit \
	    > $(IOSGEN)/native-code.mm

	# generate resource files used to start/run LibreOffice
	cp $(WORKDIR)/UnpackedTarball/icu/source/data/in/icudt59l.dat $(IOSRES)/icudt59l.dat
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

	echo '[Version]'            >  $(IOSRES)/program/versionrc
	echo 'AllLanguages=en-US'   >> $(IOSRES)/program/versionrc
	echo 'BuildVersion='        >> $(IOSRES)/program/versionrc
	echo "buildid=$(BUILDID)"   >> $(IOSRES)/program/versionrc
endif


#- build  ---------------------------------------------------------------------
$(IOSGEN)/$(IOSKIT): $(IOSKITPRJ)/project.pbxproj iosCopySetup
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	mkdir -p $(WORKDIR)/ios
	CC=; \
	$(call gb_Helper_print_on_error, \
	    xcodebuild \
	        -xcconfig $(IOSKITXC) \
	        -project $(IOSKITPRJ) \
	        -target LibreOfficeKit \
	        -sdk $(XCODEBUILD_SDK) \
	        -arch $(XCODE_ARCHS) \
	        -configuration $(if $(ENABLE_DEBUG),Debug,Release) \
	        build \
	        , $(WORKDIR)/ios/build.log \
	)
	cp $(WORKDIR)/ios/build/*/libLibreOfficeKit.a $(IOSGEN)/$(IOSKIT)



$(INSTDIR)/$(IOSAPP): $(IOSAPPPRJ)/project.pbxproj $(IOSGEN)/$(IOSKIT)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	mkdir -p $(WORKDIR)/ios
	CC=; \
	$(call gb_Helper_print_on_error, \
	    xcodebuild \
	        -xcconfig $(IOSAPPXC) \
	        -project $(IOSAPPPRJ) \
	        -target LibreOfficeLight \
	        -sdk $(XCODEBUILD_SDK) \
	        -arch $(XCODE_ARCHS) \
	        -configuration $(if $(ENABLE_DEBUG),Debug,Release) \
	        build \
	        , $(WORKDIR)/ios/build.log \
	)


#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/ios):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSGEN) $(IOSKITXC) $(IOSAPPXC)


# vim: set noet sw=4 ts=4:
