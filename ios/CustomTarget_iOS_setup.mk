# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


#- Env ------------------------------------------------------------------------
IOSGEN  = $(BUILDDIR)/workdir/CustomTarget/ios
IOSRES  = $(IOSGEN)/resources
IOSDIRS = $(IOSGEN) \
          $(IOSGEN)/Debug_x86_64 \
		  $(IOSGEN)/Debug_arm64 \
		  $(IOSGEN)/Release_x86_64 \
		  $(IOSGEN)/Release_arm64 \
		  $(IOSRES) \
		  $(IOSRES)/services \
		  $(IOSRES)/program \
		  $(IOSRES)/share \
		  $(IOSRES)/config \
		  $(IOSRES)/filter


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_setup))

$(call gb_CustomTarget_get_target,ios/iOS_setup): $(IOSGEN)/native-code.h


#- create directories  --------------------------------------------------------
$(IOSDIRS):
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),MKD,2)
	mkdir -p $(IOSDIRS)


#- Generate resources  --------------------------------------------------------
$(IOSGEN)/native-code.h: $(BUILDDIR)/config_host.mk \
                         $(SRCDIR)/ios/CustomTarget_iOS_setup.mk \
	                 $(SRCDIR)/solenv/bin/native-code.py \
	                 $(IOSGEN) \
	                 $(IOSDIRS)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),EN2,2)

	# generate native-code.h (used by LibreOffice.c)
	$(SRCDIR)/solenv/bin/native-code.py \
	    -C -g core -g writer -g calc -g draw -g edit \
	    > $(IOSGEN)/native-code.h

	# copy resource files used to start/run LibreOffice
	cp $(WORKDIR)/UnpackedTarball/icu/source/data/in/icudt$(ICU_MAJOR)l.dat $(IOSRES)/icudt$(ICU_MAJOR)l.dat
	cp $(INSTDIR)/program/types.rdb             $(IOSRES)/udkapi.rdb
	cp $(INSTDIR)/program/types/offapi.rdb      $(IOSRES)
	cp $(INSTDIR)/program/types/oovbaapi.rdb    $(IOSRES)
	cp $(INSTDIR)/program/services/services.rdb $(IOSRES)/services
	cp $(INSTDIR)/program/services.rdb          $(IOSRES)
	if test -d $(INSTDIR)/program/resource; then \
		cp -R $(INSTDIR)/program/resource $(IOSRES)/program; \
	fi
	mkdir -p $(IOSRES)/share/config
	cp -R $(INSTDIR)/share/config/soffice.cfg $(IOSRES)/share/config
	cp $(INSTDIR)/share/filter/oox-drawingml-adj-names $(IOSRES)/filter
	cp $(INSTDIR)/share/filter/oox-drawingml-cs-presets $(IOSRES)/filter
	cp $(INSTDIR)/share/filter/vml-shape-types $(IOSRES)/filter
	cp -R $(INSTDIR)/share/registry $(IOSRES)
	cp $(INSTDIR)/share/config/*zip $(IOSRES)/share/config
	cp -R $(INSTDIR)/share/liblangtag $(IOSRES)/share
	cp -R $(INSTDIR)/share/theme_definitions $(IOSRES)/share
	mkdir -p $(IOSRES)/share/fonts/truetype
	cp $(INSTDIR)/share/fonts/truetype/Liberation* $(IOSRES)/share/fonts/truetype
	cp $(SRCDIR)/ios/welcome.odt $(IOSRES)

	# Set up rc (the "inifile", fundamentalrc, unorc, bootstraprc and versionrc.
	(echo '[Bootstrap]' \
	&& echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' \
	&& echo 'HOME=$$SYSUSERHOME'  \
	    ) > $(IOSRES)/rc

	(echo '[Bootstrap]' \
        && echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR' \
        && echo 'BRAND_INI_DIR=file:://$$APP_DATA_DIR' \
        && echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)' \
        && echo 'BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)' \
        && echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/registry ' \
	        'res:$${BRAND_BASE_DIR}/registry' \
	&& echo 'LO_LIB_DIR=file://$$APP_DATA_DIR/lib/' \
	&& echo 'UNO_TYPES=file://$$APP_DATA_DIR/udkapi.rdb ' \
	        'file://$$APP_DATA_DIR/offapi.rdb' \
	&& echo 'UNO_SERVICES=file://$$APP_DATA_DIR/services.rdb ' \
	        'file://$$APP_DATA_DIR/services/services.rdb' \
	&& echo 'OSL_SOCKET_PATH=$$APP_DATA_DIR/cache' \
	    ) > $(IOSRES)/fundamentalrc;

	echo '[Bootstrap]' > $(IOSRES)/unorc

	# bootstraprc must be in $BRAND_BASE_DIR/program
	(echo '[Bootstrap]' \
	&& echo 'InstallMode=<installmode>' \
	&& echo "ProductKey=LibreOffice $(PRODUCTVERSION)" \
	&& echo 'UserInstallation=$$SYSUSERHOME' \
	    ) > $(IOSRES)/program/bootstraprc

	(echo '[Version]' \
	&& echo 'AllLanguages=en-US' \
	&& echo 'BuildVersion=' \
	&& echo "buildid=$(shell git -C $(SRCDIR) log -1 --format=%H)" \
	    ) > $(IOSRES)/program/versionrc

	$(SRCDIR)/bin/lo-all-static-libs | sed -e 's/ /\
/g' >$(IOSGEN)/ios-all-static-libs.list

#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSRES)/* $(IOSGEN)/native-code.h $(IOSGEN)

# vim: set noet sw=4 ts=4:
