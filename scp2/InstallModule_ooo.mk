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

$(eval $(call gb_InstallModule_InstallModule,scp2/ooo))

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
	ENABLE_LOMENUBAR \
	ENABLE_NSPLUGIN \
	ENABLE_ONLINE_UPDATE \
	ENABLE_OPENGL \
	ENABLE_TDE \
	ENABLE_TDEAB \
	ENABLE_TELEPATHY \
	MERGELIBS \
	SYSTEM_BOOST \
	SYSTEM_CAIRO \
	SYSTEM_CLUCENE \
	SYSTEM_CURL \
	SYSTEM_DB \
	SYSTEM_EXPAT \
	SYSTEM_GDKPIXBUF \
	SYSTEM_GETTEXT \
	SYSTEM_GLIB \
	SYSTEM_GRAPHITE \
	SYSTEM_HSQLDB \
	SYSTEM_HUNSPELL \
	SYSTEM_HYPH \
	SYSTEM_ICU \
	SYSTEM_JPEG \
	SYSTEM_LCMS2 \
	SYSTEM_LIBCROCO \
	SYSTEM_LIBEXTTEXTCAT \
	SYSTEM_LIBGSF \
	SYSTEM_LIBJPEG \
	SYSTEM_LIBLANGTAG \
	SYSTEM_LIBRSVG \
	SYSTEM_LIBXML \
	SYSTEM_LIBXSLT \
	SYSTEM_LPSOLVE \
	SYSTEM_LUCENE \
	SYSTEM_MOZILLA \
	SYSTEM_MYTHES \
	SYSTEM_NEON \
	SYSTEM_NSS \
	SYSTEM_OPENSSL \
	SYSTEM_PANGO \
	SYSTEM_PORTAUDIO \
	SYSTEM_REDLAND \
	SYSTEM_SNDFILE \
	SYSTEM_STDLIBS \
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
	MINGW_DB_DLL \
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
	MINGW_LIBXML_DLL \
	MINGW_LIBXSLT_DLL \
	MINGW_LPSOLVE_DLL \
	MINGW_MINIZIP_DLL \
	MINGW_MYTHES_DLL \
	MINGW_NEON_DLL \
	MINGW_NSPR4_DLL \
	MINGW_NSS3_DLL \
	MINGW_NSSUTIL3_DLL \
	MINGW_PIXMAN_DLL \
	MINGW_PLC4_DLL \
	MINGW_PLDS4_DLL \
	MINGW_PNG15_DLL \
	MINGW_RAPTOR_DLL \
	MINGW_RASQAL_DLL \
	MINGW_REDLAND_DLL \
	MINGW_SQLITE3_DLL \
	MINGW_SSH2_DLL \
	MINGW_SSL3_DLL \
	MINGW_SSL_DLL \
	MINGW_TASN1_DLL \
	MINGW_ZLIB_DLL \
))

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	$(if $(filter INTERNAL,$(ENABLE_LIBRSVG)),\
		-DENABLE_LIBRSVG \
	) \
	$(if $(filter gcj,$(JDK)),\
		-DGCJ \
	) \
	$(if $(filter TRUE,$(ENABLE_GSTREAMER)),\
		-DGSTREAMER \
	) \
	$(if $(filter TRUE,$(ENABLE_GSTREAMER_0_10)),\
		-DGSTREAMER_0_10 \
	) \
	$(if $(WINDOWS_SDK_HOME),\
		-DHAVE_WINDOWS_SDK \
	) \
	$(if $(filter YES,$(SYSTEM_HSQLDB)),\
		-DHSQLDB_JAR=\""$(call gb_Helper_make_path,$(HSQLDB_JAR))"\" \
	) \
	$(if $(filter-out YES,$(WITH_MOZILLA)),\
		-DWITHOUT_MOZILLA \
	) \
	$(if $(filter-out YES,$(WITH_MYSPELL_DICTS)),\
		-DWITHOUT_MYSPELL_DICTS \
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
	-DICU_MICRO=$(ICU_MICRO) \
))

ifneq ($(SYSTEM_LIBXSLT),YES)
include $(OUTDIR)/inc/libxsltversion.mk

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DLIBXSLT_MAJOR=$(LIBXSLT_MAJOR) \
))
endif

ifneq ($(SYSTEM_REDLAND),YES)
include $(OUTDIR)/inc/redlandversion.mk

$(eval $(call gb_InstallModule_add_defs,scp2/ooo,\
	-DRAPTOR_MAJOR=$(RAPTOR_MAJOR) \
	-DRASQAL_MAJOR=$(RASQAL_MAJOR) \
	-DREDLAND_MAJOR=$(REDLAND_MAJOR) \
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
    $(if $(filter WNTGCC,$(GUI)$(COM)),\
		scp2/source/ooo/mingw_dlls \
	) \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/ooo,\
    scp2/source/ooo/module_helppack \
    scp2/source/ooo/module_langpack \
    scp2/source/ooo/module_ooo \
    scp2/source/ooo/module_systemint \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
