# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/ooo))

$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/ooo,\
	brand \
	ooo \
	ooobinarytable \
	reportbuilder \
	pdfimport \
))

$(eval $(call gb_InstallModule_define_if_set,scp2/ooo,\
	ENABLE_GTK \
	ENABLE_MACOSX_SANDBOX \
	ENABLE_NPAPI_FROM_BROWSER \
	ENABLE_ONLINE_UPDATE \
	ENABLE_TDE \
	SYSTEM_CURL \
	SYSTEM_HSQLDB \
	SYSTEM_LIBXSLT \
	SYSTEM_REDLAND \
	WITH_GALLERY_BUILD \
	WITH_MYSPELL_DICTS \
))

$(eval $(call gb_InstallModule_define_value_if_set,scp2/ooo,\
	MINGW_GCCDLL \
	MINGW_GXXDLL \
))

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	$(if $(CUSTOM_BRAND_DIR),-DCUSTOM_BRANDING) \
	$(if $(WINDOWS_SDK_HOME),\
		-DHAVE_WINDOWS_SDK \
	) \
	$(if $(SYSTEM_HSQLDB),\
		-DHSQLDB_JAR=\""$(call gb_Helper_make_path,$(HSQLDB_JAR))"\" \
	) \
	$(if $(MACOSX_SDK_VERSION),\
		-DMACOSX_SDK_VERSION=$(MACOSX_SDK_VERSION) \
	) \
	$(if $(filter MSC,$(COM)),$(if $(MSVC_USE_DEBUG_RUNTIME),-DMSVC_PKG_DEBUG_RUNTIME)) \
))

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DUSING_X11 \
))
endif

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DDBCONNECTIVITY \
))
endif

ifeq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DDISABLE_PYUNO \
))
else ifneq ($(SYSTEM_PYTHON),)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DSYSTEM_PYTHON \
))
endif

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DICU_MAJOR=$(ICU_MAJOR) \
))

$(eval $(call gb_InstallModule_add_templates,scp2/ooo,\
    scp2/source/templates/module_helppack \
    scp2/source/templates/module_helppack_root \
    scp2/source/templates/module_langpack \
    scp2/source/templates/module_langpack_root \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/ooo,\
    scp2/source/ooo/common_brand \
    scp2/source/ooo/common_brand_readme \
    scp2/source/ooo/directory_ooo \
    scp2/source/ooo/directory_ooo_macosx \
    scp2/source/ooo/file_extra_ooo \
    scp2/source/ooo/file_font_ooo \
    scp2/source/ooo/file_library_ooo \
    scp2/source/ooo/file_ooo \
    scp2/source/ooo/file_resource_ooo \
    scp2/source/ooo/installation_ooo \
    scp2/source/ooo/module_help_template \
    scp2/source/ooo/module_hidden_ooo \
    scp2/source/ooo/module_lang_template \
    scp2/source/ooo/profileitem_ooo \
    scp2/source/ooo/scpaction_ooo \
    $(if $(filter WNTGCC,$(OS)$(COM)),\
		scp2/source/ooo/mingw_dlls \
	) \
    scp2/source/ooo/module_filter \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/ooo,\
    scp2/source/ooo/module_helppack \
    scp2/source/ooo/module_langpack \
    scp2/source/ooo/module_ooo \
    scp2/source/ooo/module_systemint \
))

ifeq ($(ENABLE_PDFIMPORT),TRUE)
$(eval $(call gb_InstallModule_add_scpfiles,scp2/ooo,\
    scp2/source/ooo/module_pdfimport \
))
endif

ifeq ($(ENABLE_REPORTBUILDER),TRUE)
$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/ooo,\
    scp2/source/ooo/module_reportbuilder \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
