# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#- Env ------------------------------------------------------------------------

LO_XCCONFIG 	:= lo.xcconfig
DEST_RESOURCE 	:= MobileLibreOffice/resource_link
BUILDID			:=$(shell cd $(SRCDIR) && git log -1 --format=%H)

#- Macros ---------------------------------------------------------------------

define MobileLibreOfficeXcodeBuild 
	CC=;xcodebuild -project shared/ios_sharedlo.xcodeproj -target ios_sharedlo -arch armv7 -configuration $(if $(ENABLE_DEBUG),Debug,Release) $(1) $(if $(verbose)$(VERBOSE),,>/dev/null)
	CC=;xcodebuild -project MobileLibreOffice/MobileLibreOffice.xcodeproj -target MobileLibreOffice -arch armv7 -configuration $(if $(ENABLE_DEBUG),Debug,Release) $(1) $(if $(verbose)$(VERBOSE),,>/dev/null)
endef

#- Targets --------------------------------------------------------------------

.PHONY: MobileLibreOffice_setup 

#==============================================================================
# Register target
$(eval $(call gb_CustomTarget_CustomTarget,ios/MobileLibreOffice))
#==============================================================================

#==============================================================================
# Build
$(call gb_CustomTarget_get_target,ios/MobileLibreOffice): MobileLibreOffice_setup
#==============================================================================
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	$(call MobileLibreOfficeXcodeBuild, clean build)

#==============================================================================
# Setup
MobileLibreOffice_setup:
#==============================================================================
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)

	# Libs #
	# Create the link flags in the xcconfig for Xcode linkage
	all_libs=`$(SRCDIR)/bin/lo-all-static-libs`; \
	sed -e "s|^\(LINK_LDFLAGS =\).*$$|\1 $$all_libs|" < $(BUILDDIR)/ios/$(LO_XCCONFIG) > $(BUILDDIR)/ios/$(LO_XCCONFIG).new && mv $(BUILDDIR)/ios/$(LO_XCCONFIG).new $(BUILDDIR)/ios/$(LO_XCCONFIG)

	# Copy lo.xcconfig to source dir for the Xcode projects
	if test $(SRCDIR) != $(BUILDDIR); then \
		cp $(BUILDDIR)/ios/$(LO_XCCONFIG) $(SRCDIR)/ios; \
	fi

	# Resources #
	rm -rf $(DEST_RESOURCE) 2>/dev/null
	mkdir -p $(DEST_RESOURCE)
	mkdir -p $(DEST_RESOURCE)/ure

	# copy rdb files
	cp $(INSTDIR)/program/types/offapi.rdb      $(DEST_RESOURCE)
	cp $(INSTDIR)/program/types/oovbaapi.rdb  	$(DEST_RESOURCE)
	cp $(INSTDIR)/program/services/services.rdb $(DEST_RESOURCE)
	cp $(INSTDIR)/ure/share/misc/services.rdb   $(DEST_RESOURCE)/ure

	# copy .res files
	# program/resource is hardcoded in tools/source/rc/resmgr.cxx. Sure,
	# we could set STAR_RESOURCE_PATH instead. sigh...
	mkdir -p $(DEST_RESOURCE)/program/resource
	cp $(INSTDIR)/program/resource/*en-US.res $(DEST_RESOURCE)/program/resource

	# soffice.cfg
	mkdir -p $(DEST_RESOURCE)/share/config
	cp -R $(INSTDIR)/share/config/soffice.cfg $(DEST_RESOURCE)/share/config

	# "registry"
	cp -R $(INSTDIR)/share/registry $(DEST_RESOURCE)/share

	# Set up rc, the "inifile". See getIniFileName_Impl().
	file=$(DEST_RESOURCE)/rc; \
	echo '[Bootstrap]'                                       >  $$file; \
	echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' >> $$file; \
	echo 'HOME=$$APP_DATA_DIR/tmp'                           >> $$file;

	# Set up fundamentalrc, unorc, bootstraprc and versionrc.
	# Do we really need all these?
	file=$(DEST_RESOURCE)/fundamentalrc; \
	echo '[Bootstrap]'                                      >  $$file; \
	echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR'             >> $$file; \
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry res:$${BRAND_BASE_DIR}/registry' >> $$file;

	file=$(DEST_RESOURCE)/unorc; \
	echo '[Bootstrap]' > $$file;

	# bootstraprc must be in $BRAND_BASE_DIR/program
	mkdir -p $(DEST_RESOURCE)/program
	file=$(DEST_RESOURCE)/program/bootstraprc; \
	echo '[Bootstrap]'                                                              >  $$file; \
	echo 'InstallMode=<installmode>'                                                >> $$file; \
	echo "ProductKey=LibreOffice $(PRODUCTVERSION)"                                 >> $$file; \
	echo 'UserInstallation=file://$$APP_DATA_DIR/../Library/Application%20Support'  >> $$file;

	# Is this really needed?
	file=$(DEST_RESOURCE)/program/versionrc; \
	echo '[Version]'            >  $$file; \
	echo 'AllLanguages=en-US'   >> $$file; \
	echo 'BuildVersion='        >> $$file; \
	echo "buildid=$(BUILDID)"   >> $$file; \
	echo 'ProductMajor=360'     >> $$file; \
	echo 'ProductMinor=1'       >> $$file; 

#==============================================================================
# Clean
$(call gb_CustomTarget_get_clean_target,ios/MobileLibreOffice):
#==============================================================================
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),APP,2)
	$(call MobileLibreOfficeXcodeBuild, clean)

#------------------------------------------------------------------------------
# vim: set noet sw=4 ts=4:
