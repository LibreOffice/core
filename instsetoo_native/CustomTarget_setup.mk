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
	ooenv \
	$(if $(filter TRUE,$(DISABLE_PYTHON)),,$(call gb_Helper_get_rcfile,pythonloader.uno)) \
	$(call gb_Helper_get_rcfile,uno) \
	$(call gb_Helper_get_rcfile,version) \
))

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,bootstrap) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,fundamental) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/ooenv \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,pythonloader.uno) \
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,uno) \
	: $(SRCDIR)/instsetoo_native/CustomTarget_setup.mk

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,bootstrap) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[ErrorReport]' \
		&& echo 'ErrorReportPort=80' \
		&& echo 'ErrorReportServer=report.libreoffice.org' \
		&& echo '[Bootstrap]' \
		&& echo 'InstallMode=<installmode>' \
		&& echo 'ProductKey=$(PRODUCTNAME) $(PRODUCTVERSION)' \
		$(if $(ENABLE_RELEASE_BUILD),\
			&& echo 'UserInstallation=$$SYSUSERCONFIG/$(if $(filter-out MACOSX WNT,$(OS)),$(shell echo $(PRODUCTNAME) | tr "[:upper:]" "[:lower:]"),$(PRODUCTNAME))/$(LIBO_VERSION_MAJOR)', \
			&& echo 'UserInstallation=$$ORIGIN/..') \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,fundamental) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[Bootstrap]' \
		&& echo 'BRAND_BASE_DIR=$${ORIGIN}/..' \
		&& echo 'BRAND_BIN_SUBDIR=$(LIBO_BIN_FOLDER)' \
		&& echo 'BRAND_SHARE_SUBDIR=$(LIBO_SHARE_FOLDER)' \
		&& echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry res:$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/registry bundledext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,uno):BUNDLED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini sharedext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,uno):SHARED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini userext:$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,uno):UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini user:$${$$BRAND_BASE_DIR/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/registrymodifications.xcu' \
		&& echo 'LO_JAVA_DIR=$${BRAND_BASE_DIR}/$(LIBO_SHARE_JAVA_FOLDER)' \
		&& echo 'LO_LIB_DIR=$${BRAND_BASE_DIR}/$(LIBO_LIB_FOLDER)' \
		&& echo 'BAK_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):TMP_EXTENSIONS}' \
		&& echo 'BUNDLED_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):BUNDLED_EXTENSIONS}' \
		&& echo 'BUNDLED_EXTENSIONS_USER=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):BUNDLED_EXTENSIONS_USER}' \
		&& echo 'SHARED_EXTENSIONS_USER=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):SHARED_EXTENSIONS_USER}' \
		&& echo 'UNO_SHARED_PACKAGES_CACHE=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):UNO_SHARED_PACKAGES_CACHE}' \
		&& echo 'TMP_EXTENSIONS=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):TMP_EXTENSIONS}' \
		&& echo 'UNO_USER_PACKAGES_CACHE=$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):UNO_USER_PACKAGES_CACHE}' \
		&& echo 'URE_BIN_DIR=$(if $(filter WNT,$(OS)),$${.link:$${BRAND_BASE_DIR}/ure-link}/bin,$${BRAND_BASE_DIR}/ure-link/bin)' \
		&& echo 'URE_MORE_JAVA_CLASSPATH_URLS=$(if $(SYSTEM_HSQLDB),$(HSQLDB_JAR))' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_SHARED_DATA=$${BRAND_BASE_DIR}/$(LIBO_SHARE_FOLDER)/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_OVERRIDE_JAVA_JFW_USER_DATA=$${$${BRAND_BASE_DIR}/$(LIBO_ETC_FOLDER)/$(call gb_Helper_get_rcfile,bootstrap):UserInstallation}/user/config/javasettings_$${_OS}_$${_ARCH}.xml' \
		&& echo 'URE_LIB_DIR=$(if $(filter WNT,$(OS)),$${.link:$${BRAND_BASE_DIR}/ure-link}/bin,$${BRAND_BASE_DIR}/ure-link/lib)' \
		&& echo 'URE_MORE_JAVA_TYPES=$$ORIGIN/classes/unoil.jar $$ORIGIN/classes/ScriptFramework.jar $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_UserUnoFile}:UNO_JAVA_CLASSPATH} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_SharedUnoFile}:UNO_JAVA_CLASSPATH} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_BundledUnoFile}:UNO_JAVA_CLASSPATH}' \
		&& echo 'URE_MORE_SERVICES=$${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_UserUnoFile}:UNO_SERVICES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_SharedUnoFile}:UNO_SERVICES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_BundledUnoFile}:UNO_SERVICES} <$$ORIGIN/services>*' \
		&& echo 'URE_MORE_TYPES=<$$ORIGIN/types>* $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_UserUnoFile}:UNO_TYPES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_SharedUnoFile}:UNO_TYPES} $${$${$$ORIGIN/$(call gb_Helper_get_rcfile,uno):PKG_BundledUnoFile}:UNO_TYPES}' \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/ooenv :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo 'java_path=`$(gb_INSTROOT)/ure/bin/javaldx 2>/dev/null`' && \
		echo 'export LD_LIBRARY_PATH="$(gb_INSTROOT)/program:$$java_path$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}"' && \
		echo 'ulimit -c unlimited' && \
		echo 'export PATH="$(gb_INSTROOT)/program:$(gb_INSTROOT)/ure/bin:$$PATH"' && \
		echo 'export GNOME_DISABLE_CRASH_DIALOG=1' && \
		echo '# debugging assistance' && \
		echo 'export SAL_DISABLE_FLOATGRAB=1' && \
		echo 'export G_SLICE=always-malloc' && \
		echo 'export MALLOC_CHECK_=2' && \
		echo 'export MALLOC_PERTURB_=153' && \
		echo 'export OOO_DISABLE_RECOVERY=1' \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,pythonloader.uno) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		printf '[Bootstrap]\n' && \
		$(if $(filter YES,$(SYSTEM_PYTHON)),, \
			printf 'PYUNO_LOADER_PYTHONHOME=%s\n' \
				$(if $(ENABLE_MACOSX_MACLIKE_APP_STRUCTURE), \
					'$$ORIGIN/../Frameworks/LibreOfficePython.framework', \
				$(if $(filter MACOSX,$(OS)), \
					'$$ORIGIN/LibreOfficePython.framework', \
					'$$ORIGIN/python-core-$(PYTHON_VERSION)')) &&) \
		printf 'PYUNO_LOADER_PYTHONPATH=%s$$ORIGIN\n' \
			$(if $(filter YES,$(SYSTEM_PYTHON)), \
				'', \
			$(if $(ENABLE_MACOSX_MACLIKE_APP_STRUCTURE), \
				'$(foreach dir,/ /lib-dynload /lib-tk /site-packages,$(patsubst %/,%,$$ORIGIN/../Frameworks/LibreOfficePython.framework/Versions/Current/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(dir))) ', \
			$(if $(filter MACOSX,$(OS)), \
				'$(foreach dir,/ /lib-dynload /lib-tk /site-packages,$(patsubst %/,%,$$ORIGIN/LibreOfficePython.framework/Versions/Current/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(dir))) ', \
			$(if $(filter WNTMSC,$(OS)$(COM)), \
				'$(foreach dir,/ /site-packages,$(patsubst %/,%,$$ORIGIN/python-core-$(PYTHON_VERSION)/lib$(dir))) ', \
				'$(foreach dir,/ /lib-dynload /lib-tk /site-packages,$(patsubst %/,%,$$ORIGIN/python-core-$(PYTHON_VERSION)/lib$(dir))) ')))) \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,uno) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[Bootstrap]' \
		&& echo 'PKG_BundledUnoFile=$$BUNDLED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/$(call gb_Helper_get_rcfile,uno)' \
		&& echo 'PKG_SharedUnoFile=$$SHARED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/$(call gb_Helper_get_rcfile,uno)' \
		&& echo 'PKG_UserUnoFile=$$UNO_USER_PACKAGES_CACHE/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/$(call gb_Helper_get_rcfile,uno)' \
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

.PHONY: $(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,version)
$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/$(call gb_Helper_get_rcfile,version) :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[Version]' \
		&& echo 'AllLanguages=$(if $(gb_WITH_LANG),$(gb_WITH_LANG),en-US)' \
		&& echo 'BuildVersion=$(BUILD_VER_STRING)' \
		&& echo 'buildid=$(shell cd $(SRCDIR) && git log -1 --format=%H)' \
		&& echo 'ExtensionUpdateURL=http://updateexte.libreoffice.org/ExtensionUpdateService/check.Update' \
		&& echo 'ProductMajor=$(LIBO_VERSION_MAJOR)$(LIBO_VERSION_MINOR)$(LIBO_VERSION_MICRO)' \
		&& echo 'ProductMinor=$(LIBO_VERSION_PATCH)' \
		&& echo 'ReferenceOOoMajorMinor=3.4' \
		&& echo 'UpdateID=$(PRODUCTNAME)_$(LIBO_VERSION_MAJOR)_en-US' \
		&& echo 'UpdateURL=$(if $(ENABLE_ONLINE_UPDATE),http://update.libreoffice.org/check.php$(if $(filter-out WNT,$(OS)),?pkgfmt=$(PKGFORMAT)))' \
		&& echo 'UpdateUserAgent=<PRODUCT> ($${buildid}; $${_OS}; $${_ARCH}; BundledLanguages=$${AllLanguages})' \
		&& echo 'Vendor=$(OOO_VENDOR)' \
	) > $@

# vim: set noet sw=4 ts=4:
