# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#- Env ------------------------------------------------------------------------

TiledLibreOffice_resource := experimental/TiledLibreOffice/Resources
BUILDID			:=$(shell cd $(SRCDIR) && git log -1 --format=%H)

#- Macros ---------------------------------------------------------------------

define TiledLibreOfficeXcodeBuild
	CC=;xcodebuild -project experimental/TiledLibreOffice/TiledLibreOffice.xcodeproj -target TiledLibreOffice -arch $(XCODE_ARCHS) -configuration $(if $(ENABLE_DEBUG),Debug,Release) $(1) $(if $(verbose)$(VERBOSE),,>/dev/null)
endef

#- Targets --------------------------------------------------------------------

.PHONY: TiledLibreOffice_setup

# Register target
$(eval $(call gb_CustomTarget_CustomTarget,ios/TiledLibreOffice))

# Build
# Depend on the custom target that sets up lo.xcconfig
$(call gb_CustomTarget_get_target,ios/TiledLibreOffice): $(call gb_CustomTarget_get_target,ios/Lo_Xcconfig) TiledLibreOffice_setup
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	$(SRCDIR)/solenv/bin/native-code.py \
		-g extended_core -g writer \
		-s cui -s spl -s uui \
		> $(SRCDIR)/ios/experimental/TiledLibreOffice/TiledLibreOffice/native-code.mm
	$(call TiledLibreOfficeXcodeBuild, clean build)

# Setup
TiledLibreOffice_setup:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)

	# Resources #
	rm -rf $(TiledLibreOffice_resource) 2>/dev/null
	mkdir -p $(TiledLibreOffice_resource)
	mkdir -p $(TiledLibreOffice_resource)/ure

	# copy rdb files
	cp $(INSTDIR)/ure/share/misc/types.rdb 	    $(TiledLibreOffice_resource)
	cp $(INSTDIR)/program/types/offapi.rdb      $(TiledLibreOffice_resource)
	cp $(INSTDIR)/program/types/oovbaapi.rdb  	$(TiledLibreOffice_resource)
	cp $(INSTDIR)/program/services/services.rdb $(TiledLibreOffice_resource)
	cp $(INSTDIR)/ure/share/misc/services.rdb   $(TiledLibreOffice_resource)/ure

	# copy .res files
	# program/resource is hardcoded in tools/source/rc/resmgr.cxx. Sure,
	# we could set STAR_RESOURCE_PATH instead. sigh...
	mkdir -p $(TiledLibreOffice_resource)/program/resource
	cp $(INSTDIR)/program/resource/*en-US.res $(TiledLibreOffice_resource)/program/resource

	# soffice.cfg
	mkdir -p $(TiledLibreOffice_resource)/share/config
	cp -R $(INSTDIR)/share/config/soffice.cfg $(TiledLibreOffice_resource)/share/config

	# "registry"
	cp -R $(INSTDIR)/share/registry $(TiledLibreOffice_resource)/share

	# Set up rc, the "inifile". See getIniFileName_Impl().
	file=$(TiledLibreOffice_resource)/rc; \
	echo '[Bootstrap]'                                       >  $$file; \
	echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' >> $$file; \
	echo 'HOME=$$APP_DATA_DIR/tmp'                           >> $$file;

	# Set up fundamentalrc, unorc, bootstraprc and versionrc.
	# Do we really need all these?
	file=$(TiledLibreOffice_resource)/fundamentalrc; \
	echo '[Bootstrap]'                                      >  $$file; \
	echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR'             >> $$file; \
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry res:$${BRAND_BASE_DIR}/registry' >> $$file;

	file=$(TiledLibreOffice_resource)/unorc; \
	echo '[Bootstrap]' > $$file;

	# bootstraprc must be in $BRAND_BASE_DIR/program
	mkdir -p $(TiledLibreOffice_resource)/program
	file=$(TiledLibreOffice_resource)/program/bootstraprc; \
	echo '[Bootstrap]'                                                              >  $$file; \
	echo 'InstallMode=<installmode>'                                                >> $$file; \
	echo "ProductKey=LibreOffice $(PRODUCTVERSION)"                                 >> $$file; \
	echo 'UserInstallation=file://$$APP_DATA_DIR/../Library/Application%20Support'  >> $$file;

	# Is this really needed?
	file=$(TiledLibreOffice_resource)/program/versionrc; \
	echo '[Version]'            >  $$file; \
	echo 'AllLanguages=en-US'   >> $$file; \
	echo 'BuildVersion='        >> $$file; \
	echo "buildid=$(BUILDID)"   >> $$file; \
	echo 'ProductMajor=360'     >> $$file; \
	echo 'ProductMinor=1'       >> $$file;

# Clean
$(call gb_CustomTarget_get_clean_target,ios/TiledLibreOffice):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),APP,2)
	$(call TiledLibreOfficeXcodeBuild, clean)

# vim: set noet sw=4 ts=4:
