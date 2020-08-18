# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/setup))

$(eval $(call gb_CustomTarget_register_targets,instsetoo_native/setup,\
	$(call gb_Helper_get_rcfile,bootstrap) \
	$(call gb_Helper_get_rcfile,fundamental) \
	$(call gb_Helper_get_rcfile,louno) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),$(if $(filter-out MACOSX,$(OS)), \
	    $(call gb_Helper_get_rcfile,redirect))) \
	$(call gb_Helper_get_rcfile,setup) \
	$(call gb_Helper_get_rcfile,soffice) \
	$(call gb_Helper_get_rcfile,uno) \
	$(call gb_Helper_get_rcfile,version) \
	$(call gb_Helper_get_rcfile,crashreport) \
))

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,bootstrap) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,fundamental) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,louno) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,redirect) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,setup) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,soffice) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,uno) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,version) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,crashreport) \
	: $(SRCDIR)/instsetoo_native/CustomTarget_setup.mk

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,bootstrap) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'InstallMode=<installmode>' \
		&& echo 'ProductKey=$(PRODUCTNAME) $(PRODUCTVERSION)' \
		$(if $(ENABLE_RELEASE_BUILD),\
			&& echo 'UserInstallation=$$SYSUSERCONFIG/$(if $(filter-out HAIKU MACOSX WNT,$(OS)),$(shell echo $(PRODUCTNAME) | tr "[:upper:]" "[:lower:]"),$(shell echo $(PRODUCTNAME) | sed -e 's/ /%20/g'))/4', \
			&& echo 'UserInstallation=$$ORIGIN/..') \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,fundamental) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'BRAND_BASE_DIR=$${ORIGIN}/..' \
		&& echo 'BRAND_INI_DIR=$${ORIGIN}' \
		&& echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)' \
		&& echo 'BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)' \
		&& echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry res:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry $(if $(ENABLE_DCONF),dconf:* )$(if $(filter WNT,$(OS)),winreg:LOCAL_MACHINE )bundledext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,louno):BUNDLED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini sharedext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,louno):SHARED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini userext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,louno):UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini $(if $(filter WNT,$(OS)),winreg:CURRENT_USER )user:!$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/registrymodifications.xcu' \
		&& echo 'LO_JAVA_DIR=$${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)' \
		&& echo 'LO_LIB_DIR=$${BRAND_BASE_DIR}/$(LIBO_LIB_FOLDER)' \
		&& echo 'BAK_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):TMP_EXTENSIONS}' \
		&& echo 'BUNDLED_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):BUNDLED_EXTENSIONS}' \
		&& echo 'BUNDLED_EXTENSIONS_USER=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):BUNDLED_EXTENSIONS_USER}' \
		&& echo 'SHARED_EXTENSIONS_USER=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):SHARED_EXTENSIONS_USER}' \
		&& echo 'UNO_SHARED_PACKAGES_CACHE=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):UNO_SHARED_PACKAGES_CACHE}' \
		&& echo 'TMP_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):TMP_EXTENSIONS}' \
		&& echo 'UNO_USER_PACKAGES_CACHE=$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):UNO_USER_PACKAGES_CACHE}' \
		&& echo 'URE_BIN_DIR=$${BRAND_BASE_DIR}/$(LIBO_URE_BIN_FOLDER)' \
		&& echo 'URE_MORE_JAVA_CLASSPATH_URLS=$(if $(SYSTEM_HSQLDB),$(HSQLDB_JAR))' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_SHARED_DATA=$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_USER_DATA=$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_MORE_JAVA_TYPES=$${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)/libreoffice.jar $${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)/ScriptFramework.jar $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_UserUnoFile}:UNO_JAVA_CLASSPATH} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_SharedUnoFile}:UNO_JAVA_CLASSPATH} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_BundledUnoFile}:UNO_JAVA_CLASSPATH}' \
		&& echo 'URE_MORE_SERVICES=$${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_UserUnoFile}:UNO_SERVICES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_SharedUnoFile}:UNO_SERVICES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_BundledUnoFile}:UNO_SERVICES} <$$ORIGIN/services>*' \
		&& echo 'URE_MORE_TYPES=<$$ORIGIN/types>* $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_UserUnoFile}:UNO_TYPES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_SharedUnoFile}:UNO_TYPES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,louno):PKG_BundledUnoFile}:UNO_TYPES}' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,louno) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'PKG_BundledUnoFile=$$BUNDLED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc' \
		&& echo 'PKG_SharedUnoFile=$$SHARED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc' \
		&& echo 'PKG_UserUnoFile=$$UNO_USER_PACKAGES_CACHE/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc' \
		&& echo 'BAK_EXTENSIONS=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/extensions/bak' \
		&& echo 'BUNDLED_EXTENSIONS=$$BRAND_BASE_DIR/$(LIBO_SHARE_FOLDER)/extensions' \
		&& echo 'BUNDLED_EXTENSIONS_USER=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/extensions/bundled' \
		&& echo 'TMP_EXTENSIONS=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/extensions/tmp' \
		&& echo 'SHARED_EXTENSIONS_USER=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/extensions/shared' \
		&& echo 'UNO_SHARED_PACKAGES=$$BRAND_BASE_DIR/$(LIBO_SHARE_FOLDER)/uno_packages' \
		&& echo 'UNO_SHARED_PACKAGES_CACHE=$$UNO_SHARED_PACKAGES/cache' \
		&& echo 'UNO_USER_PACKAGES=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/uno_packages' \
		&& echo 'UNO_USER_PACKAGES_CACHE=$$UNO_USER_PACKAGES/cache' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,redirect) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'URE_BOOTSTRAP=$${ORIGIN}/$(call gb_Helper_get_rcfile,fundamental)' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

.PHONY: $(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,setup)
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,setup) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'buildid=$(shell cd $(SRCDIR) && git log -1 --format=%H)' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# for release-builds (building installers) adjust values in openoffice.lst.in
# Added 'SecureUserConfig' flags to enable and safe user config files
#  SecureUserConfig :           boolean - switches securing on/off - default false
#  SecureUserConfigCompress :   boolean - defines if backup data will be compressed - default true
#  SecureUserConfigNumCopies :  integer - defines how many compressed copies of saved content will be kept - default 2
#  SecureUserConfigMode:        integer - defines what to secure, default is 1
#                                           0 : only registrymodifications.xcu
#                                           1 : a selected amount of user-defined configs
#                                           2 : everything in the user config directory
#  SecureUserConfigExtensions:  boolean - defines to also safe the extension configuration (which extensions
#                                         are installed, which are activated) - default is true
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,soffice) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'CrashDirectory=$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/crash' \
		&& echo 'CrashDumpEnable=$(DEFAULT_CRASHDUMP_VALUE)' \
		&& echo 'HideEula=1' \
		&& echo 'Logo=1' \
		&& echo 'NativeProgress=false' \
		&& echo 'ProgressBarColor=0,0,0' \
		&& echo 'ProgressFrameColor=102,102,102' \
		&& echo 'ProgressPosition=30,145' \
		&& echo 'ProgressSize=385,8' \
		&& echo 'ProgressPositionHigh=30,200' \
		&& echo 'ProgressSizeHigh=650,12' \
		&& echo 'ProgressTextBaseline=170' \
		&& echo 'ProgressTextColor=0,0,0' \
		&& echo 'SecureUserConfig=true' \
		&& echo 'SecureUserConfigCompress=true' \
		&& echo 'SecureUserConfigExtensions=true' \
		&& echo 'SecureUserConfigMode=1' \
		&& echo 'SecureUserConfigNumCopies=2' \
		&& echo 'URE_BOOTSTRAP=$${ORIGIN}/$(call gb_Helper_get_rcfile,fundamental)' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,uno) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'URE_INTERNAL_LIB_DIR=$${ORIGIN}$(if $(filter MACOSX,$(OS)),/../../../Frameworks)' \
		&& echo 'URE_INTERNAL_JAVA_DIR=$(if $(filter MACOSX,$(OS)),$${ORIGIN}/../../java,$${ORIGIN}/classes)' \
		&& echo 'URE_INTERNAL_JAVA_CLASSPATH=$${URE_MORE_JAVA_TYPES}' \
		&& echo 'UNO_TYPES=$(if $(filter MACOSX,$(OS)),$${ORIGIN}/../share/misc/,$${ORIGIN}/)types.rdb $${URE_MORE_TYPES}' \
		&& echo 'UNO_SERVICES=$(if $(filter MACOSX,$(OS)),$${ORIGIN}/../share/misc/,$${ORIGIN}/)services.rdb $${URE_MORE_SERVICES}' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)





define instsetoo_native_genversionini
[Version]
AllLanguages=$(if $(gb_WITH_LANG),$(gb_WITH_LANG),en-US)
buildid=$(shell cd $(SRCDIR) && git log -1 --format=%H)
ExtensionUpdateURL=https://updateexte.libreoffice.org/ExtensionUpdateService/check.Update
UpdateChannel=$(if $(ENABLE_ONLINE_UPDATE_MAR),$(shell cd $(SRCDIR) && bin/update/get_update_channel.py $(UPDATE_CONFIG)))
ReferenceOOoMajorMinor=4.1
UpdateID=$(PRODUCTNAME)_$(LIBO_VERSION_MAJOR)_en-US
UpdateURL=$(if $(ENABLE_ONLINE_UPDATE),https://update.libreoffice.org/check.php$(if $(filter-out WNT,$(OS)),?pkgfmt=$(PKGFORMAT)))
UpdateUserAgent=<PRODUCT> ($${buildid}; $${_OS}; $${_ARCH}; <OPTIONAL_OS_HW_DATA>)
Vendor=$(OOO_VENDOR)
endef



.PHONY: $(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,version)
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,version) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
ifeq ($(HAVE_GNUMAKE_FILE_FUNC),)
	printf '[Version]\n' > $@ && \
	printf 'AllLanguages=$(if $(gb_WITH_LANG),$(gb_WITH_LANG),en-US)\n' >> $@ && \
	printf 'buildid=$(shell cd $(SRCDIR) && git log -1 --format=%H)\n' >> $@ && \
	printf 'ExtensionUpdateURL=https://updateexte.libreoffice.org/ExtensionUpdateService/check.Update\n' >> $@ && \
	printf 'UpdateChannel=$(if $(ENABLE_ONLINE_UPDATE_MAR),$(shell cd $(SRCDIR) && bin/update/get_update_channel.py $(UPDATE_CONFIG)))\n' >> $@ && \
	printf 'ReferenceOOoMajorMinor=4.1\n' >> $@ && \
	printf 'UpdateID=$(PRODUCTNAME)_$(LIBO_VERSION_MAJOR)_en-US\n' >> $@ && \
	printf 'UpdateURL=$(if $(ENABLE_ONLINE_UPDATE),https://update.libreoffice.org/check.php$(if $(filter-out WNT,$(OS)),?pkgfmt=$(PKGFORMAT)))\n' >> $@ && \
	printf 'UpdateUserAgent=<PRODUCT> ($${buildid}; $${_OS}; $${_ARCH}; <OPTIONAL_OS_HW_DATA>)\n' >> $@ && \
	printf 'Vendor=$(OOO_VENDOR)\n' >> $@
else
	$(file > $@,$(call instsetoo_native_genversionini))
endif
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# vim: set noet sw=4 ts=4:
