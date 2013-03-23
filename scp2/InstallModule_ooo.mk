# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	ENABLE_GSTREAMER \
	ENABLE_GSTREAMER_0_10 \
	ENABLE_GTK \
	ENABLE_GTK3 \
	ENABLE_KAB \
	ENABLE_KDE \
	ENABLE_KDE4 \
	ENABLE_LIBLANGTAG \
	ENABLE_NPAPI_FROM_BROWSER \
	ENABLE_NPAPI_INTO_BROWSER \
	ENABLE_ONLINE_UPDATE \
	ENABLE_OPENGL \
	ENABLE_TDE \
	ENABLE_TDEAB \
	ENABLE_TELEPATHY \
	MERGELIBS \
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
	WITH_MOZAB4WIN \
	WITH_MYSPELL_DICTS \
))

$(eval $(call gb_InstallModule_define_value_if_set,scp2/ooo,\
	MINGW_GCCDLL \
	MINGW_GXXDLL \
))

$(eval $(call gb_InstallModule_define_mingw_dll_if_set,scp2/ooo,\
	MINGW_BOOST_DATE_TIME_DLL \
	MINGW_CAIRO_DLL \
	MINGW_CRYPTO_DLL \
	MINGW_CURL_DLL \
	MINGW_EXPAT_DLL \
	MINGW_FONTCONFIG_DLL \
	MINGW_FREETYPE_DLL \
	MINGW_GCRYPT_DLL \
	MINGW_GNUTLS_DLL \
	MINGW_GPG_ERROR_DLL \
	MINGW_GRAPHITE2_DLL \
	MINGW_HUNSPELL_DLL \
	MINGW_HYPHEN_DLL \
	MINGW_ICONV_DLL \
	MINGW_ICUDATA_DLL \
	MINGW_ICUI18N_DLL \
	MINGW_ICUUC_DLL \
	MINGW_IDN_DLL \
	MINGW_INTL_DLL \
	MINGW_JPEG_DLL \
	MINGW_LCMS2_DLL \
	MINGW_LIBEXSLT_DLL \
	MINGW_LIBLANGTAG_DLL \
	MINGW_LIBXML_DLL \
	MINGW_LIBXSLT_DLL \
	MINGW_LPSOLVE_DLL \
	MINGW_MYTHES_DLL \
	MINGW_NEON_DLL \
	MINGW_NSPR4_DLL \
	MINGW_NSS3_DLL \
	MINGW_NSSUTIL3_DLL \
	MINGW_PIXMAN_DLL \
	MINGW_PLC4_DLL \
	MINGW_PLDS4_DLL \
	MINGW_PNG15_DLL \
	MINGW_POPPLER_DLL \
	MINGW_RAPTOR_DLL \
	MINGW_RASQAL_DLL \
	MINGW_REDLAND_DLL \
	MINGW_SMIME3_DLL \
	MINGW_SQLITE3_DLL \
	MINGW_SSH2_DLL \
	MINGW_SSL3_DLL \
	MINGW_SSL_DLL \
	MINGW_TASN1_DLL \
	MINGW_ZLIB_DLL \
))

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	$(if $(WINDOWS_SDK_HOME),\
		-DHAVE_WINDOWS_SDK \
	) \
	$(if $(filter YES,$(SYSTEM_HSQLDB)),\
		-DHSQLDB_JAR=\""$(call gb_Helper_make_path,$(HSQLDB_JAR))"\" \
	) \
))

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
	-DICU_MINOR=$(ICU_MINOR) \
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
