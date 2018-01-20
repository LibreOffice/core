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


#- Top level  -----------------------------------------------------------------
$(eval $(call gb_CustomTarget_CustomTarget,ios/iOS_setup))



$(call gb_CustomTarget_get_target,ios/iOS_setup): $(IOSGEN)/native-code.h



#- Generate dynamic files  ---------------------------------------------------
$(IOSGEN) $(WORKDIR)/ios:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),EN1,2)
	mkdir -p $(IOSGEN) $(IOSRES) $(IOSRES)/services \
	         $(IOSRES)/share/config $(IOSRES)/share/filter $(IOSRES)/program \
	         $(IOSGEN)/simulator \
	         $(IOSGEN)/debug \
	         $(IOSGEN)/release \
	         $(WORKDIR)/ios;



$(IOSGEN)/native-code.h: $(BUILDDIR)/config_host.mk \
                         $(SRCDIR)/ios/CustomTarget_iOS_setup.mk \
	                 $(SRCDIR)/solenv/bin/native-code.py \
	                 $(IOSGEN) $(WORKDIR)/ios
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),EN2,2)
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
	(echo '[Bootstrap]' \
	&& echo 'URE_BOOTSTRAP=file://$$APP_DATA_DIR/fundamentalrc' \
	&& echo 'HOME=$$SYSUSERHOME'  \
	    ) > $(IOSRES)/rc

	# Set up fundamentalrc, unorc, bootstraprc and versionrc.
	(echo '[Bootstrap]' \
        && echo 'BRAND_BASE_DIR=file://$$APP_DATA_DIR' \
        && echo 'BRAND_INI_DIR=file:://$$APP_DATA_DIR' \
        && echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)' \
        && echo '##BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)' \
        && echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry ' \
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
	&& echo 'UserInstallation=$$SYSUSERHOME/userinstallation' \
	    ) > $(IOSRES)/program/bootstraprc

	(echo '[Version]' \
	&& echo 'AllLanguages=en-US' \
	&& echo 'BuildVersion=' \
	&& echo "buildid=$(BUILDID)" \
	    ) > $(IOSRES)/program/versionrc



#- clean ios  -----------------------------------------------------------------
$(call gb_CustomTarget_get_clean_target,ios/iOS_setup):
	$(call gb_Output_announce,$(subst $(WORKDIR)/Clean/,,$@),$(false),ENV,2)
	echo $(call gb_StaticLibrary_get_target,iOS_kitBridge)
	rm -rf $(IOSRES) $(IOSGEN)/native-code.h $(IOSGEN)/build
	rm -rf $(WORKDIR)/ios
ifeq ($(ENABLE_DEBUG),TRUE)
ifeq ($(CPUNAME),X86_64)
	rm -f $(IOSGEN)/simulator/*
else
	rm -f $(IOSGEN)/debug/*
endif
else
ifeq ($(CPUNAME),ARM64)
	rm -f $(IOSGEN)/release/*
endif
endif



# vim: set noet sw=4 ts=4:
