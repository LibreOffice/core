# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(ENABLE_REPORTBUILDER),TRUE)
ifneq ($(SYSTEM_JFREEREPORT),YES)
include $(SRCDIR)/jfreereport/version.mk
endif
endif

$(eval $(call gb_InstallModule_InstallModule,scp2/ooo))

$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/ooo,ooo))

$(eval $(call gb_InstallModule_define_if_set,scp2/ooo,\
	DISABLE_ATL \
	DISABLE_NEON \
	ENABLE_CAIRO_CANVAS \
	ENABLE_DIRECTX \
	ENABLE_EVOAB2 \
	ENABLE_GTK \
	ENABLE_GTK3 \
	ENABLE_KAB \
	ENABLE_KDE \
	ENABLE_KDE4 \
	ENABLE_LIBLANGTAG \
	ENABLE_LPSOLVE \
	ENABLE_MACOSX_MACLIKE_APP_STRUCTURE \
	ENABLE_MACOSX_SANDBOX \
	ENABLE_NPAPI_FROM_BROWSER \
	ENABLE_NPAPI_INTO_BROWSER \
	ENABLE_ONLINE_UPDATE \
	ENABLE_OPENGL \
	ENABLE_TDE \
	ENABLE_TDEAB \
	SYSTEM_APACHE_COMMONS \
	SYSTEM_BOOST \
	SYSTEM_CAIRO \
	SYSTEM_CLUCENE \
	SYSTEM_CURL \
	SYSTEM_EXPAT \
	SYSTEM_GRAPHITE \
	SYSTEM_HSQLDB \
	SYSTEM_HUNSPELL \
	SYSTEM_HYPH \
	SYSTEM_ICU \
	SYSTEM_JFREEREPORT \
	SYSTEM_JPEG \
	SYSTEM_LCMS2 \
	SYSTEM_LIBEXTTEXTCAT \
	SYSTEM_LIBJPEG \
	SYSTEM_LIBLANGTAG \
	SYSTEM_LIBXML \
	SYSTEM_LIBXSLT \
	SYSTEM_LPSOLVE \
	SYSTEM_LUCENE \
	SYSTEM_MYTHES \
	SYSTEM_NEON \
	SYSTEM_NSS \
	SYSTEM_OPENSSL \
	SYSTEM_PANGO \
	SYSTEM_POPPLER \
	SYSTEM_PORTAUDIO \
	SYSTEM_REDLAND \
	SYSTEM_SNDFILE \
	SYSTEM_STDLIBS \
	WITH_GALLERY_BUILD \
	WITH_MOZAB4WIN \
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
	$(if $(filter YES,$(SYSTEM_HSQLDB)),\
		-DHSQLDB_JAR=\""$(call gb_Helper_make_path,$(HSQLDB_JAR))"\" \
	) \
	$(if $(filter MSC,$(COM)),$(if $(MSVC_USE_DEBUG_RUNTIME),-DMSVC_PKG_DEBUG_RUNTIME)) \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DGUIBASE_UNX \
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
else ifeq ($(SYSTEM_PYTHON),YES)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DSYSTEM_PYTHON \
))
endif

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DICU_MAJOR=$(ICU_MAJOR) \
))

ifneq ($(SYSTEM_LIBXSLT),YES)
$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DLIBXSLT_MAJOR=$(LIBXSLT_MAJOR) \
))
endif

ifneq ($(SYSTEM_REDLAND),YES)

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DRAPTOR_MAJOR=$(RAPTOR_MAJOR) \
	-DRASQAL_MAJOR=$(RASQAL_MAJOR) \
	-DREDLAND_MAJOR=$(REDLAND_MAJOR) \
))
endif

ifneq ($(SYSTEM_JFREEREPORT),YES)

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DFLOW_ENGINE_VERSION=$(FLOW_ENGINE_VERSION) \
	-DFLUTE_VERSION=$(FLUTE_VERSION) \
	-DLIBBASE_VERSION=$(LIBBASE_VERSION) \
	-DLIBFONTS_VERSION=$(LIBFONTS_VERSION) \
	-DLIBFORMULA_VERSION=$(LIBFORMULA_VERSION) \
	-DLIBLAYOUT_VERSION=$(LIBLAYOUT_VERSION) \
	-DLIBLOADER_VERSION=$(LIBLOADER_VERSION) \
	-DLIBREPOSITORY_VERSION=$(LIBREPOSITORY_VERSION) \
	-DLIBSERIALIZER_VERSION=$(LIBSERIALIZER_VERSION) \
	-DLIBXML_VERSION=$(LIBXML_VERSION) \
))

endif

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
    scp2/source/ooo/ooo_brand \
    scp2/source/ooo/profile_ooo \
    scp2/source/ooo/profileitem_ooo \
    scp2/source/ooo/scpaction_ooo \
    scp2/source/ooo/shortcut_ooo \
    scp2/source/ooo/ure_into_ooo \
    $(if $(filter WNTGCC,$(OS)$(COM)),\
		scp2/source/ooo/mingw_dlls \
	) \
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
