# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,external))

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_moduledir,external,msc-externals))
endif

ifneq ($(ENABLE_WASM_STRIP_CLUCENE),TRUE)
$(eval $(call gb_Module_add_moduledirs,external,\
	$(call gb_Helper_optional,CLUCENE,clucene) \
))
endif

ifneq ($(ENABLE_WASM_STRIP_EPUB),TRUE)
$(eval $(call gb_Module_add_moduledirs,external,\
	$(call gb_Helper_optional,EPUBGEN,libepubgen) \
))
endif

$(eval $(call gb_Module_add_moduledirs,external,\
	$(call gb_Helper_optional,XMLSEC,xmlsec) \
	$(call gb_Helper_optional,ABW,libabw) \
	$(call gb_Helper_optional,APR,apr) \
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,BOX2D,box2d) \
	$(call gb_Helper_optional,BREAKPAD,breakpad) \
	$(call gb_Helper_optional,BSH,beanshell) \
	$(call gb_Helper_optional,BZIP2,bzip2) \
	$(call gb_Helper_optional,CAIRO,cairo) \
	$(call gb_Helper_optional,CDR,libcdr) \
	$(call gb_Helper_optional,OPENCL,clew) \
	$(call gb_Helper_optional,LIBCMIS,libcmis) \
	$(call gb_Helper_optional,COINMP,coinmp) \
	$(call gb_Helper_optional,CPPUNIT,cppunit) \
	$(call gb_Helper_optional,CT2N,ct2n) \
	$(call gb_Helper_optional,CURL,curl) \
	dtoa \
	$(call gb_Helper_optional,EBOOK,libebook) \
	$(call gb_Helper_optional,EPM,epm) \
	$(call gb_Helper_optional,EPOXY,epoxy) \
	$(call gb_Helper_optional,ETONYEK,libetonyek) \
	$(call gb_Helper_optional,EXPAT,expat) \
	$(call gb_Helper_optional,FIREBIRD,firebird) \
	$(call gb_Helper_optional,FONTCONFIG,fontconfig) \
	$(call gb_Helper_optional,FREEHAND,libfreehand) \
	$(call gb_Helper_optional,FREETYPE,freetype) \
	$(call gb_Helper_optional,GLM,glm) \
	$(call gb_Helper_optional,GPGMEPP,gpgmepp) \
	$(call gb_Helper_optional,GRAPHITE,graphite) \
	$(call gb_Helper_optional,HARFBUZZ,harfbuzz) \
	$(call gb_Helper_optional,HSQLDB,hsqldb) \
	$(call gb_Helper_optional,HUNSPELL,hunspell) \
	$(call gb_Helper_optional,HYPHEN,hyphen) \
	$(call gb_Helper_optional,ICU,icu) \
	$(call gb_Helper_optional,JFREEREPORT,jfreereport) \
	$(call gb_Helper_optional,LIBJPEG_TURBO,libjpeg-turbo) \
	$(call gb_Helper_optional,LANGUAGETOOL,languagetool) \
	$(call gb_Helper_optional,LCMS2,lcms2) \
	$(call gb_Helper_optional,LIBASSUAN,libassuan) \
	$(call gb_Helper_optional,LIBATOMIC_OPS,libatomic_ops) \
	$(call gb_Helper_optional,LIBEOT,libeot) \
	$(call gb_Helper_optional,LIBEXTTEXTCAT,libexttextcat) \
	$(call gb_Helper_optional,LIBFFI,libffi) \
	$(call gb_Helper_optional,LIBGPGERROR,libgpg-error) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	$(call gb_Helper_optional,LIBNUMBERTEXT,libnumbertext) \
	$(call gb_Helper_optional,LIBPNG,libpng) \
	$(call gb_Helper_optional,LIBXML2,libxml2) \
	$(call gb_Helper_optional,LIBXSLT,libxslt) \
	$(call gb_Helper_optional,LPSOLVE,lpsolve) \
	$(call gb_Helper_optional,LIBTOMMATH,libtommath) \
	$(call gb_Helper_optional,LXML,lxml) \
	$(call gb_Helper_optional,MARIADB_CONNECTOR_C,mariadb-connector-c) \
	$(call gb_Helper_optional,MDDS,mdds) \
	$(call gb_Helper_optional,MDNSRESPONDER,mdnsresponder) \
	$(if $(WITH_EXTRA_EXTENSIONS),misc_extensions) \
	$(call gb_Helper_optional,MORE_FONTS,more_fonts) \
	$(call gb_Helper_optional,MSPUB,libmspub) \
	$(call gb_Helper_optional,MWAW,libmwaw) \
	$(call gb_Helper_optional,MYTHES,mythes) \
	$(call gb_Helper_optional,NEON,neon) \
	$(call gb_Helper_optional,NSS,nss) \
	$(call gb_Helper_optional,ODFGEN,libodfgen) \
	$(call gb_Helper_optional,OPENLDAP,openldap) \
	$(call gb_Helper_optional,OPENSSL,openssl) \
	$(call gb_Helper_optional,ORCUS,liborcus) \
	$(call gb_Helper_optional,PAGEMAKER,libpagemaker) \
	$(call gb_Helper_optional,PDFIUM,pdfium) \
	$(call gb_Helper_optional,POPPLER,poppler) \
	$(call gb_Helper_optional,POSTGRESQL,postgresql) \
	$(call gb_Helper_optional,PYTHON,python3) \
	$(call gb_Helper_optional,QXP,libqxp) \
	$(call gb_Helper_optional,ZXING,zxing) \
	$(call gb_Helper_optional,REDLAND,redland) \
	$(call gb_Helper_optional,REVENGE,librevenge) \
	$(call gb_Helper_optional,RHINO,rhino) \
	$(call gb_Helper_optional,SERF,serf) \
	$(call gb_Helper_optional,SKIA,skia) \
	$(call gb_Helper_optional,STAROFFICE,libstaroffice) \
	$(if $(filter WNT,$(OS)),twain_dsm) \
	$(call gb_Helper_optional,UCPP,ucpp) \
	$(call gb_Helper_optional,VISIO,libvisio) \
	$(call gb_Helper_optional,WPD,libwpd) \
	$(call gb_Helper_optional,WPG,libwpg) \
	$(call gb_Helper_optional,WPS,libwps) \
	$(call gb_Helper_optional,XSLTML,xsltml) \
	$(call gb_Helper_optional,ZLIB,zlib) \
	$(call gb_Helper_optional,ZMF,libzmf) \
	$(call gb_Helper_optional,CUCKOO,cuckoo) \
))

# vim: set noet sw=4 ts=4:
