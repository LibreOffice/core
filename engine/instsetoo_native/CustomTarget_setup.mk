# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/setup))

$(eval $(call gb_CustomTarget_register_targets,instsetoo_native/setup,\
	$(call gb_Helper_get_rcfile,bootstrap) \
	$(call gb_Helper_get_rcfile,fundamental) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),$(if $(filter-out MACOSX,$(OS)), \
	    $(call gb_Helper_get_rcfile,redirect))) \
	$(call gb_Helper_get_rcfile,setup) \
	$(call gb_Helper_get_rcfile,soffice) \
	$(call gb_Helper_get_rcfile,uno) \
	$(call gb_Helper_get_rcfile,version) \
))

$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,bootstrap) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,fundamental) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,redirect) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,setup) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,soffice) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,uno) \
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,version) \
	: $(SRCDIR)/instsetoo_native/CustomTarget_setup.mk $(BUILDDIR)/config_host.mk.stamp

$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,bootstrap) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'InstallMode=<installmode>' \
		&& echo 'ProductKey=$(PRODUCTNAME) $(PRODUCTVERSION)' \
		&& echo 'UserInstallation=$$SYSUSERCONFIG/$(if $(filter-out HAIKU MACOSX WNT,$(OS)),$(shell echo $(PRODUCTNAME) | tr "[:upper:]" "[:lower:]" | tr -d " "),$(shell echo $(PRODUCTNAME) | tr -d " "))/4' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,fundamental) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'BRAND_BASE_DIR=$${ORIGIN}/..' \
		&& echo 'BRAND_INI_DIR=$${ORIGIN}' \
		&& echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)' \
		&& echo 'BRAND_SHARE_RESOURCE_SUBDIR=$(LIBO_SHARE_RESOURCE_FOLDER)' \
		&& echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry' \
                'res:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry' \
                $(if $(ENABLE_DCONF),'dconf:* ') \
                $(if $(filter WNT,$(OS)),'winreg:LOCAL_MACHINE ') \
                'sharedext:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry/sharedext.ini' \
                $(if $(filter WNT,$(OS)),'winreg:CURRENT_USER ') \
                'user:!$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/registrymodifications.xcu' \
		&& echo 'LO_DOTNET_DIR=$${BRAND_BASE_DIR}/$(LIBO_SHARE_DOTNET_FOLDER)' \
		&& echo 'LO_JAVA_DIR=$${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)' \
		&& echo 'LO_LIB_DIR=$${BRAND_BASE_DIR}/$(LIBO_LIB_FOLDER)' \
		&& echo 'URE_BIN_DIR=$${BRAND_BASE_DIR}/$(LIBO_URE_BIN_FOLDER)' \
		&& echo 'URE_MORE_JAVA_CLASSPATH_URLS=' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_SHARED_DATA=$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_USER_DATA=$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_MORE_JAVA_TYPES=$${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)/libreoffice.jar $${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)/ScriptFramework.jar' \
		&& echo 'URE_MORE_SERVICES=<$$ORIGIN/services>*' \
		&& echo 'URE_MORE_TYPES=<$$ORIGIN/types>*' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,redirect) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'URE_BOOTSTRAP=$${ORIGIN}/$(call gb_Helper_get_rcfile,fundamental)' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

.PHONY: $(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,setup)
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,setup) :
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
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,soffice) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	( \
		echo '[Bootstrap]' \
		&& echo 'HideEula=1' \
		&& echo 'Logo=1' \
		&& echo 'NativeProgress=false' \
		&& echo 'ProgressBarColor=92,41,131' \
		&& echo 'ProgressFrameColor=255,255,255' \
		&& echo 'ProgressPosition=268,106' \
		&& echo 'ProgressSize=212,4' \
		&& echo 'ProgressPositionHigh=268,137' \
		&& echo 'ProgressSizeHigh=617,12' \
		&& echo 'ProgressTextBaseline=230' \
		&& echo 'ProgressTextColor=255,255,255' \
		&& echo 'SecureUserConfig=true' \
		&& echo 'SecureUserConfigCompress=true' \
		&& echo 'SecureUserConfigMode=1' \
		&& echo 'SecureUserConfigNumCopies=2' \
		&& echo 'URE_BOOTSTRAP=$${ORIGIN}/$(call gb_Helper_get_rcfile,fundamental)' \
	) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,uno) :
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
ReferenceOOoMajorMinor=4.1
Vendor=$(OOO_VENDOR)
endef



.PHONY: $(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,version)
$(gb_CustomTarget_workdir)/instsetoo_native/setup/$(call gb_Helper_get_rcfile,version) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	$(file > $@,$(call instsetoo_native_genversionini))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

# vim: set noet sw=4 ts=4:
