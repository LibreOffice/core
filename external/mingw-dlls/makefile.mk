#
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
# Tor Lillqvist <tml@iki.fi> (initial developer)
# Jan Holesovsky <kendy@suse.cz>
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
#

PRJ=..
PRJNAME=external
TARGET=mingw-dlls

.INCLUDE: settings.mk

# ------------------------------------------------------------------

.IF "$(GUI)$(COM)" != "WNTGCC"

dummy:
    @echo "Nothing to do."

.ELSE

MINGW_DLLS:= \
    $(MINGW_BOOST_DATE_TIME_DLL) \
    $(MINGW_CAIRO_DLL) \
    $(MINGW_CRYPTO_DLL) \
    $(MINGW_CURL_DLL) \
    $(MINGW_DB_DLL) \
    $(MINGW_EXPAT_DLL) \
    $(MINGW_FONTCONFIG_DLL) \
    $(MINGW_FREETYPE_DLL) \
    $(MINGW_GCRYPT_DLL) \
    $(MINGW_GLIB_DLL) \
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
    $(MINGW_RAPTOR_DLL) \
    $(MINGW_RASQAL_DLL) \
    $(MINGW_READLINE_DLL) \
    $(MINGW_REDLAND_DLL) \
    $(MINGW_SQLITE3_DLL) \
    $(MINGW_SSH2_DLL) \
    $(MINGW_SSL3_DLL) \
    $(MINGW_SSL_DLL) \
    $(MINGW_TASN1_DLL) \
    $(MINGW_TERMCAP_DLL) \
    $(MINGW_ZLIB_DLL)

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
MINGW_DLLS+=$(MINGW_GCCDLL)
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
MINGW_DLLS+=$(MINGW_GXXDLL)
.ENDIF

# Guesstimate where the DLLs can be

POTENTIAL_MINGW_RUNTIME_BINDIRS = \
	$(COMPATH)/$(HOST_PLATFORM)/sys-root/mingw/bin \
	/usr/$(HOST_PLATFORM)/sys-root/mingw/bin \

all:
	@for DLL in $(MINGW_DLLS) ; do \
	    for D in $(POTENTIAL_MINGW_RUNTIME_BINDIRS); do \
	       test -f $$D/$$DLL && $(COPY) -p $$D/$$DLL $(BIN)$/ && break; \
	    done ; \
	    test -f $(BIN)$/$$DLL || { echo Could not find $$DLL in none of $(POTENTIAL_MINGW_RUNTIME_BINDIRS) ; exit 1 ; } ; \
	done

.ENDIF

# ------------------------------------------------------------------

.INCLUDE: target.mk
