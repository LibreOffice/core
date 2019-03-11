# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#- Env ------------------------------------------------------------------------
IOSGEN := $(BUILDDIR)/workdir/CustomTarget/ios
IOSRES := $(IOSGEN)/resources


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_setup))



$(call gb_CustomTarget_get_target,ios/iOS_setup): $(IOSGEN)/native-code.h



#- Generate dynamic files  ---------------------------------------------------
$(IOSGEN)/native-code.h: $(BUILDDIR)/config_host.mk $(SRCDIR)/ios/CustomTarget_iOS_setup.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	mkdir -p $(IOSGEN) $(IOSRES) $(IOSRES)/services \
	         $(IOSRES)/share/config $(IOSRES)/filter $(IOSRES)/program \
	         $(IOSRES)/config

	# generate file with call declarations
	$(SRCDIR)/solenv/bin/native-code.py \
	    -C -g core -g writer -g calc -g draw -g edit \
	    > $(IOSGEN)/native-code.h

	# generate resource files used to start/run LibreOffice
	cp $(WORKDIR)/UnpackedTarball/icu/source/data/in/icudt$(ICU_MAJOR)l.dat $(IOSRES)/icudt$(ICU_MAJOR)l.dat
	cp $(INSTDIR)/program/types.rdb             $(IOSRES)/udkapi.rdb
	cp $(INSTDIR)/program/types/offapi.rdb      $(IOSRES)
	cp $(INSTDIR)/program/types/oovbaapi.rdb    $(IOSRES)
	cp $(INSTDIR)/program/services/services.rdb $(IOSRES)/services
	cp $(INSTDIR)/program/services.rdb          $(IOSRES)
	cp -R $(INSTDIR)/program/resource $(IOSRES)/program
	mkdir -p $(IOSRES)/share/config
	cp -R $(INSTDIR)/share/config/soffice.cfg $(IOSRES)/share/config
	cp $(WORKDIR)/CustomTarget/i18npool/breakiterator/dict_*.data $(IOSRES)/share
	cp $(INSTDIR)/share/filter/oox-drawingml-adj-names $(IOSRES)/filter
	cp $(INSTDIR)/share/filter/oox-drawingml-cs-presets $(IOSRES)/filter
	cp $(INSTDIR)/share/filter/vml-shape-types $(IOSRES)/filter
	cp -R $(INSTDIR)/share/registry $(IOSRES)
	cp $(INSTDIR)/share/config/*zip $(IOSRES)/share/config
	cp -R $(INSTDIR)/share/liblangtag $(IOSRES)/share
	cp -R $(INSTDIR)/share/theme_definitions $(IOSRES)/share
	mkdir -p $(IOSRES)/share/fonts/truetype
	cp $(INSTDIR)/share/fonts/truetype/Liberation* $(IOSRES)/share/fonts/truetype

	# Set up rc, the "inifile". See getIniFileName_Impl().
	echo '[Bootstrap]' > $(IOSRES)/rc
	echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' >> $(IOSRES)/rc
	echo 'HOME=$$SYSUSERHOME' >> $(IOSRES)/rc

	# Set up fundamentalrc, unorc, bootstraprc and versionrc.
	file=$(IOSRES)/fundamentalrc; \
	echo '[Bootstrap]'                                      >  $(IOSRES)/fundamentalrc
	echo 'LO_LIB_DIR=file://$$APP_DATA_DIR/lib/'            >> $(IOSRES)/fundamentalrc
	echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR'             >> $(IOSRES)/fundamentalrc
	echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)'          >> $(IOSRES)/fundamentalrc
	echo 'BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)' >> $(IOSRES)/fundamentalrc
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/registry ' \
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
	echo "buildid=$(shell git -C $(SRCDIR) log -1 --format=%H)" >> $(IOSRES)/program/versionrc


	$(SRCDIR)/bin/lo-all-static-libs | sed -e 's/ /\
/g' >$(IOSGEN)/ios-all-static-libs.list

#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSRES) $(IOSGEN)/native-code.h $(IOSAPPXC)
	rm -rf $(WORKDIR)/ios



# vim: set noet sw=4 ts=4:
