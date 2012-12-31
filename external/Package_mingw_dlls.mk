# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,mingw_dlls,$(MINGW_SYSROOT)/mingw/bin))

$(eval $(call gb_Package_add_files,mingw_dlls,bin,\
    $(MINGW_BOOST_DATE_TIME_DLL) \
    $(MINGW_CAIRO_DLL) \
    $(MINGW_CRYPTO_DLL) \
    $(MINGW_CURL_DLL) \
    $(MINGW_EXPAT_DLL) \
    $(MINGW_FONTCONFIG_DLL) \
    $(MINGW_FREETYPE_DLL) \
    $(MINGW_GCRYPT_DLL) \
    $(MINGW_GLIB_DLL) \
    $(MINGW_GMODULE_DLL) \
    $(MINGW_GNUTLS_DLL) \
    $(MINGW_GPG_ERROR_DLL) \
    $(MINGW_GRAPHITE2_DLL) \
    $(MINGW_HUNSPELL_DLL) \
    $(MINGW_HYPHEN_DLL) \
    $(MINGW_ICONV_DLL) \
    $(MINGW_ICUDATA_DLL) \
    $(MINGW_ICUI18N_DLL) \
    $(MINGW_ICUUC_DLL) \
    $(MINGW_IDN_DLL) \
    $(MINGW_INTL_DLL) \
    $(MINGW_JPEG_DLL) \
    $(MINGW_LCMS2_DLL) \
    $(MINGW_LIBEXSLT_DLL) \
    $(MINGW_LIBLANGTAG_DLL) \
    $(MINGW_LIBXML_DLL) \
    $(MINGW_LIBXSLT_DLL) \
    $(MINGW_LPSOLVE_DLL) \
    $(MINGW_MYTHES_DLL) \
    $(MINGW_NEON_DLL) \
    $(MINGW_NSPR4_DLL) \
    $(MINGW_NSS3_DLL) \
    $(MINGW_NSSUTIL3_DLL) \
    $(MINGW_PIXMAN_DLL) \
    $(MINGW_PYTHON_DLL) \
    $(MINGW_PYTHON_EXE) \
    $(MINGW_PLC4_DLL) \
    $(MINGW_PLDS4_DLL) \
    $(MINGW_PNG15_DLL) \
    $(MINGW_POPPLER_DLL) \
    $(MINGW_RAPTOR_DLL) \
    $(MINGW_RASQAL_DLL) \
    $(MINGW_READLINE_DLL) \
    $(MINGW_REDLAND_DLL) \
    $(MINGW_SQLITE3_DLL) \
    $(MINGW_SMIME3_DLL) \
    $(MINGW_SSH2_DLL) \
    $(MINGW_SSL3_DLL) \
    $(MINGW_SSL_DLL) \
    $(MINGW_TASN1_DLL) \
    $(MINGW_TERMCAP_DLL) \
    $(MINGW_ZLIB_DLL) \
    $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),$(MINGW_GCCDLL)) \
    $(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_GXXDLL))
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
