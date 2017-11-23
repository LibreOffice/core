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
IOSAPPXC := $(WORKDIR)/ios/loApp.xcconfig


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_setup))



$(call gb_CustomTarget_get_target,ios/iOS_setup): $(IOSAPPXC) $(IOSGEN)/native-code.h



#- Generate dynamic files  ---------------------------------------------------
$(IOSAPPXC): $(SRCDIR)/ios/loApp.xcconfig.in $(BUILDDIR)/config_host.mk $(SRCDIR)/ios/CustomTarget_iOS_setup.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	@mkdir -p $(WORKDIR)/ios
	sed -e "s'@BUILDDIR@'$(BUILDDIR)'g" \
	    -e "s'@INSTDIR@'$(INSTDIR)'g" \
	    -e "s'@SRCDIR@'$(SRC_ROOT)'g" \
	    -e "s'@WORKDIR@'$(WORKDIR)'g" \
	    -e "s'@CFLAGS@'$(gb_GLOBALDEFS)'g" \
	    -e "s'@CPLUSPLUSFLAGS@'$(gb_GLOBALDEFS)'g" \
	    -e "s'@SYMROOT@'$(WORKDIR)/ios/build'g" \
	    $(SRCDIR)/ios/loApp.xcconfig.in > $(IOSAPPXC)



$(IOSGEN)/native-code.h: $(BUILDDIR)/config_host.mk $(SRCDIR)/ios/CustomTarget_iOS_setup.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ENV,2)
	mkdir -p $(IOSGEN) $(IOSRES) $(IOSRES)/services \
	         $(IOSRES)/share/config $(IOSRES)/share/filter $(IOSRES)/program \
	         $(IOSGEN) $(WORKDIR)/ios;

	# generate file with call declarations
	$(SRCDIR)/solenv/bin/native-code.py \
	    -C -g core -g writer -g calc -g draw -g edit \
	    > $(IOSGEN)/native-code.h

	# generate resource files used to start/run LibreOffice
	cp $(WORKDIR)/UnpackedTarball/icu/source/data/in/icudt60l.dat $(IOSRES)/icudt60l.dat
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



#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	rm -rf $(IOSRES) $(IOSGEN)/native-code.h $(IOSAPPXC)
	rm -rf $(WORKDIR)/ios



# vim: set noet sw=4 ts=4:
