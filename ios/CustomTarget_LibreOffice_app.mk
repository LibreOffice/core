# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,ios/LibreOffice_app))

# We distinguish between builds for the simulator and device by
# looking for the "iarmv7" or "i386" in the -arch option that is part
# of $(CC)

ifneq ($(filter i386,$(CC)),)
xcode_sdk=iphonesimulator
xcode_arch=i386
else
xcode_sdk=iphoneos
xcode_arch=armv7
endif

# If run from Xcode, check that its configuration (device or
# simulator) matches that of gbuild. We detect being run from Xcode by
# looking for $(SCRIPT_OUTPUT_FILE_0). The Run Script build phase in
# our project has as its (single) output file the location of the app
# executable in its app bundle.

ifneq ($(SCRIPT_OUTPUT_FILE_0),)

export CCACHE_CPP2=y

ifneq ($(CURRENT_ARCH),$(xcode_arch))
$(error Xcode platform ($(CURRENT_ARCH)) does not match that of this build tree ($(xcode_arch)))
endif

endif

ifneq ($(SCRIPT_OUTPUT_FILE_0),)
ifeq ($(EXECUTABLE_NAME),LibreOffice)
# When run from Xcode, we move the LibreOffice executable from solver into
# the LibreOffice.app directory that Xcode uses. We also set up/copy all
# the run-time configuration etc files that the app needs.
$(call gb_CustomTarget_get_target,ios/LibreOffice_app) : $(SCRIPT_OUTPUT_FILE_0)

appdir=$(dir $(SCRIPT_OUTPUT_FILE_0))
buildid=$(shell cd $(SRCDIR) && git log -1 --format=%H)

$(SCRIPT_OUTPUT_FILE_0) : $(call gb_Executable_get_target,LibreOffice)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),APP,2)
	mkdir -p $(appdir)/ure
	mv $(call gb_Executable_get_target,LibreOffice) $(SCRIPT_OUTPUT_FILE_0)
#
# Copy rdb files
#
	cp $(OUTDIR)/bin/offapi.rdb $(appdir)
	cp $(OUTDIR)/bin/udkapi.rdb $(appdir)
	cp $(OUTDIR)/bin/oovbaapi.rdb $(appdir)
	cp $(OUTDIR)/xml/services.rdb $(appdir)
	cp $(OUTDIR)/xml/ure/services.rdb $(appdir)/ure
#
# Copy "registry" files
#
	mkdir -p $(appdir)/registry/modules $(appdir)/registry/res
	cp $(OUTDIR)/xml/*.xcd $(appdir)/registry
	mv $(appdir)/registry/fcfg_langpack_en-US.xcd $(appdir)/registry/res
	cp -R $(OUTDIR)/xml/registry/* $(appdir)/registry
#
# Copy .res files
#
# program/resource is hardcoded in tools/source/rc/resmgr.cxx. Sure,
# we could set STAR_RESOURCE_PATH instead. sigh...
#
	mkdir -p $(appdir)/program/resource
	cp $(OUTDIR)/bin/*en-US.res $(appdir)/program/resource
#
# Artwork
#
	mkdir -p $(appdir)/share/config
	cp -R $(OUTDIR)/bin/images_tango.zip $(appdir)/share/config/images.zip
#
# soffice.cfg
#
	cp -R $(gb_INSTROOT)/share/config/soffice.cfg $(appdir)/share/config
#
# "registry"
#
	mkdir -p $(appdir)/share/registry/res
	cp $(OUTDIR)/xml/*.xcd $(appdir)/share/registry
	mv $(appdir)/share/registry/fcfg_langpack_en-US.xcd $(appdir)/share/registry/res
	cp -R $(OUTDIR)/xml/registry/* $(appdir)/share/registry
#
# Set up rc, the "inifile". See getIniFileName_Impl().
#
	( \
		echo '[Bootstrap]' && \
		echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' && \
		echo 'HOME=$$APP_DATA_DIR/tmp' && \
	: ) > $(appdir)/rc
#
# Set up fundamentalrc, unorc, bootstraprc and versionrc.
#
# Do we really need all these?
#
	( \
		echo '[Bootstrap]' && \
		echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR' && \
		echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/registry module:$${BRAND_BASE_DIR}/registry/modules res:$${BRAND_BASE_DIR}/registry' && \
	: ) > $(appdir)/fundamentalrc
#
	( \
		echo '[Bootstrap]' && \
		: UNO_TYPES and UNO_SERVICES are set up in lo-viewer.mm, is that sane? && \
	: ) > $(appdir)/unorc
#
# bootstraprc must be in $BRAND_BASE_DIR/program
#
	mkdir -p $(appdir)/program
	( \
		echo '[Bootstrap]' && \
		echo 'InstallMode=<installmode>' && \
		echo 'ProductKey=LibreOffice $(PRODUCTVERSION)' && \
		echo 'UserInstallation=file://$$APP_DATA_DIR/../Library/Application%20Support' && \
	: ) > $(appdir)/program/bootstraprc
#
# Is this really needed?
#
	( \
		echo '[Version]' && \
		echo 'AllLanguages=en-US' && \
		echo 'BuildVersion=' && \
		echo 'buildid=$(buildid)' && \
		echo 'ProductMajor=360' && \
		echo 'ProductMinor=1' && \
	: ) > $(appdir)/program/versionrc
#
# Copy a sample document... good old test1.odt...
#
	cp $(SRC_ROOT)/odk/examples/java/DocumentHandling/test/test1.odt $(appdir)
endif
else
# When run just from the command line, we don't have any app bundle to
# copy or move the executable to. So do nothing. Except one trick:
# Copy the Xcode project to BUILDDIR if SRCDIR!=BUILDDIR, so that one
# can then open it from there in Xcode.
$(call gb_CustomTarget_get_target,ios/LibreOffice_app) : $(gb_Helper_PHONY)
	if test $(SRCDIR) != $(BUILDDIR); then \
		(cd $(SRCDIR) && tar cf - ios/experimental/LibreOffice/LibreOffice.xcodeproj/project.pbxproj) | (cd $(BUILDDIR) && tar xf -); \
	fi

$(call gb_CustomTarget_get_clean_target,ios/LibreOffice_app) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),APP,2)
# Here we just assume that Xcode's settings are default, or something
	rm -rf experimental/LibreOffice/build

endif

# vim: set noet sw=4 ts=4:
