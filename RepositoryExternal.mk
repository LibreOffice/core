# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# this file describes all the external libraries
# depending on the configure options these may be taken from the system,
# or the internal/bundled copy may be built.

# for every external, a function gb_LinkTarget_use__FOO is defined,
# once for the system case, once for the internal case.

# in the system case, no libraries should be registered, but the target-local
# variable LIBS should be set to FOO_LIBS, and INCLUDES to FOO_CFLAGS.

ifeq ($(SYSTEM_MESA_HEADERS),YES)

gb_LinkTarget__use_Mesa:=

else

define gb_LinkTarget__use_Mesa
$(eval $(call gb_LinkTarget_use_package,$(1),Mesa_inc))
endef

endif

ifeq ($(SYSTEM_CPPUNIT),YES)

define gb_LinkTarget__use_cppunit
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(CPPUNIT_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
    $(CPPUNIT_LIBS) \
)

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
    cppunit \
))

define gb_LinkTarget__use_cppunit
$(call gb_LinkTarget_use_libraries,$(1),\
    cppunit \
)

endef

endif

ifeq ($(SYSTEM_ZLIB),YES)

define gb_LinkTarget__use_zlib
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_ZLIB \
)
$(call gb_LinkTarget_add_libs,$(1),-lz)

endef

else # !SYSTEM_ZLIB

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	zlib \
))

define gb_LinkTarget__use_zlib
$(call gb_LinkTarget_use_static_libraries,$(1),\
	zlib \
)

endef

endif # SYSTEM_ZLIB


ifeq ($(SYSTEM_JPEG),YES)

define gb_LinkTarget__use_jpeg
$(call gb_LinkTarget_add_libs,$(1),-ljpeg)
$(call gb_LinkTarget_set_ldflags,$(1),\
	$$(filter-out -L/usr/lib/jvm%,$$(T_LDFLAGS)) \
)

endef

else # !SYSTEM_JPEG

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	jpeglib \
))

define gb_LinkTarget__use_jpeg
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/external/jpeg \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	jpeglib \
)

endef

endif # SYSTEM_JPEG


ifeq ($(SYSTEM_EXPAT),YES)

define gb_LinkTarget__use_expat
$(if $(2),,$(error gb_LinkTarget__use_expat needs additional parameter))

$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_EXPAT \
)

$(call gb_LinkTarget_add_libs,$(1),-lexpat)

endef

else # !SYSTEM_EXPAT

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	ascii_expat_xmlparse \
	expat_xmlparse \
	expat_xmltok \
))

define gb_LinkTarget__use_expat
$(if $(2),,$(error gb_LinkTarget__use_expat needs additional parameter))

$(if $(filter-out ascii_expat_xmlparse,$(2)),\
	$(call gb_LinkTarget_add_defs,$(1),\
		-DXML_UNICODE \
))

$(call gb_LinkTarget_use_static_libraries,$(1),\
	$(2) \
	expat_xmltok \
)

endef

endif # SYSTEM_EXPAT

# now define 2 wrappers that select which internal static library to use...
define gb_LinkTarget__use_expat_utf8
$(call gb_LinkTarget__use_expat,$(1),ascii_expat_xmlparse)

endef

define gb_LinkTarget__use_expat_utf16
$(call gb_LinkTarget__use_expat,$(1),expat_xmlparse)

endef


ifeq ($(SYSTEM_HUNSPELL),YES)

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(HUNSPELL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(HUNSPELL_LIBS))

endef

else # !SYSTEM_HUNSPELL

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	$(if $(filter MSC,$(COM)),libhunspell,hunspell-1.3) \
))

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_add_defs,$(1),\
	-DHUNSPELL_STATIC \
)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/hunspell \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	$(if $(filter MSC,$(COM)),libhunspell,hunspell-1.3) \
)

endef

endif # SYSTEM_HUNSPELL


ifeq ($(SYSTEM_LIBCMIS),YES)

define gb_LinkTarget__use_cmis
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBCMIS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBCMIS_LIBS))

endef

else # !SYSTEM_LIBCMIS

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	cmislib \
))

define gb_LinkTarget__use_cmis
$(call gb_LinkTarget_use_static_libraries,$(1),\
	cmislib \
)

endef

endif # SYSTEM_LIBCMIS


ifeq ($(SYSTEM_LIBEXTTEXTCAT),YES)

define gb_LinkTarget__use_libexttextcat
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBEXTTEXTCAT_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBEXTTEXTCAT_LIBS))

endef

else # !SYSTEM_LIBEXTTEXTCAT

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	libexttextcat \
))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,\
	exttextcat \
))
endif

define gb_LinkTarget__use_libexttextcat
ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	libexttextcat \
)
else
$(call gb_LinkTarget_use_static_libraries,$(1),\
	exttextcat \
)
endif

endef

endif # SYSTEM_LIBEXTTEXTCAT


ifeq ($(SYSTEM_LIBXML),YES)

define gb_LinkTarget__use_libxml2
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_LIBXML \
)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBXML_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBXML_LIBS))

endef

else # !SYSTEM_LIBXML

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	xml2 \
))

define gb_LinkTarget__use_libxml2
$(call gb_LinkTarget_use_libraries,$(1),\
	xml2 \
)

endef

endif # SYSTEM_LIBXML


ifeq ($(SYSTEM_LIBXSLT),YES)

define gb_LinkTarget__use_libxslt
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBXSLT_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBXSLT_LIBS))

endef

else # !SYSTEM_LIBXSLT

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	xslt \
))

define gb_LinkTarget__use_libxslt
$(call gb_LinkTarget_use_libraries,$(1),\
	xslt \
)

endef

endif # SYSTEM_LIBXSLT


ifeq ($(SYSTEM_NEON),YES)

define gb_LinkTarget__use_neon
ifeq ($(NEON_VERSION),)
NEON_VERSION=0295
endif

$(call gb_LinkTarget_add_defs,$(1),\
	-DNEON_VERSION=0x$(NEON_VERSION) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(NEON_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(NEON_LIBS))

endef

else # !SYSTEM_NEON

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	neon \
))

define gb_LinkTarget__use_neon
$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/external/neon \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	neon \
)

endef

endif # SYSTEM_NEON


ifeq ($(SYSTEM_REDLAND),YES)

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_REDLAND \
)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(REDLAND_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(REDLAND_LIBS))

endef

else # !SYSTEM_REDLAND

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	rdf \
))

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_use_libraries,$(1),\
	rdf \
)

ifeq ($(OS),MACOSX)

$(call gb_LinkTarget_add_libs,$(1),$(foreach replaceme,librasqal.1 libraptor.1,-dylib_file @loader_path/$(replaceme).dylib:$(gb_Library_OUTDIRLOCATION)/$(replaceme).dylib))

endif

endef

endif # SYSTEM_REDLAND


ifeq ($(SYSTEM_CAIRO),YES)

# FREETYPE_CLAGS from environment if ENABLE_CAIRO is used
define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FREETYPE_CFLAGS) \
	$(CAIRO_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CAIRO_LIBS))

endef

else # !SYSTEM_CAIRO

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	cairo \
))
ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	pixman-1 \
))
endif # WNT

define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/cairo \
	$(FREETYPE_CFLAGS) \
)
$(call gb_LinkTarget_use_libraries,$(1),\
	cairo \
)
ifneq ($(OS),WNT)
ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	pixman-1 \
)
else
$(call gb_LinkTarget_use_libraries,$(1),\
	pixman-1 \
)
endif
endif

endef

endif # SYSTEM_CAIRO

define gb_LinkTarget__use_freetype
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FREETYPE_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(FREETYPE_LIBS))

endef

define gb_LinkTarget__use_fontconfig
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FONTCONFIG_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(FONTCONFIG_LIBS))

endef

ifeq ($(SYSTEM_GRAPHITE),YES)

define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GRAPHITE_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(GRAPHITE_LIBS))

endef

else # !SYSTEM_GRAPHITE

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	graphite2_off \
))
define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_use_static_libraries,$(1),\
    graphite2_off \
)

endef

endif # SYSTEM_GRAPHITE


ifeq ($(SYSTEM_ICU),YES)

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_add_libs,$(1),-licudata)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_add_libs,$(1),-licui18n)

endef
define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_libs,$(1),-licule)

endef
define gb_LinkTarget__use_icutu
$(call gb_LinkTarget_add_libs,$(1),-licutu)

endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_libs,$(1),-licuuc)

endef

else # !SYSTEM_ICU

ifeq ($(OS),ANDROID)
gb_ICU_suffix:=lo
else
gb_ICU_suffix:=
endif

# icudata and icui18n is called icudt and icuin when built with MSVC :-(
ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icudt \
	icuin \
	icule \
	icutu \
	icuuc \
))

define gb_LinkTarget__use_icudt
$(call gb_LinkTarget_use_libraries,$(1),\
	icudt \
)

endef
define gb_LinkTarget__use_icuin
$(call gb_LinkTarget_use_libraries,$(1),\
	icuin \
)

endef
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icudata$(gb_ICU_suffix) \
	icui18n$(gb_ICU_suffix) \
	icule$(gb_ICU_suffix) \
	icutu$(gb_ICU_suffix) \
	icuuc$(gb_ICU_suffix) \
))

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_use_libraries,$(1),\
	icudata$(gb_ICU_suffix) \
)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_use_libraries,$(1),\
	icui18n$(gb_ICU_suffix) \
)

endef
endif

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_use_libraries,$(1),\
	icule$(gb_ICU_suffix) \
)

endef
define gb_LinkTarget__use_icutu
$(call gb_LinkTarget_use_libraries,$(1),\
	icutu$(gb_ICU_suffix) \
)

endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_use_libraries,$(1),\
	icuuc$(gb_ICU_suffix) \
)

endef

endif # SYSTEM_ICU


ifeq ($(SYSTEM_OPENSSL),YES)

define gb_LinkTarget__use_openssl
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(OPENSSL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(OPENSSL_LIBS))

endef

else # !SYSTEM_OPENSSL

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    crypto \
    ssl \
))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    crypto \
    ssl \
))
endif

define gb_LinkTarget__use_openssl
ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	crypto \
	ssl \
)
else
$(call gb_LinkTarget_use_static_libraries,$(1),\
	crypto \
	ssl \
)
ifeq ($(OS),SOLARIS)
$(call gb_LinkTarget_use_libraries,$(1),\
	nsl \
	socket \
)
endif
endif

endef

endif # SYSTEM_OPENSSL


ifeq ($(SYSTEM_LIBCDR),YES)

define gb_LinkTarget__use_cdr
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(CDR_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CDR_LIBS))

endef

else # !SYSTEM_LIBCDR

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	cdrlib \
))

define gb_LinkTarget__use_cdr
$(call gb_LinkTarget_use_static_libraries,$(1),\
	cdrlib \
)

endef

endif # SYSTEM_LIBCDR


ifeq ($(SYSTEM_LIBVISIO),YES)

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(VISIO_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(VISIO_LIBS))

endef

else # !SYSTEM_LIBVISIO

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	visiolib \
))

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_use_static_libraries,$(1),\
	visiolib \
)

endef

endif # SYSTEM_LIBVISIO


ifeq ($(SYSTEM_LIBWPD),YES)

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPD_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPD_LIBS))

endef

else # !SYSTEM_LIBWPD

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpdlib \
))

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_use_static_libraries,$(1),\
	wpdlib \
)

endef

endif # SYSTEM_LIBWPD


ifeq ($(SYSTEM_LIBWPG),YES)

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPG_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPG_LIBS))

endef

else # !SYSTEM_LIBWPG

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpglib \
))

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_use_static_libraries,$(1),\
	wpglib \
)

endef

endif # SYSTEM_LIBWPG


ifeq ($(SYSTEM_LIBWPS),YES)

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPS_LIBS))

endef

else # !SYSTEM_LIBWPS

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpslib \
))

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_use_static_libraries,$(1),\
	wpslib \
)

endef

endif # SYSTEM_LIBWPS


ifeq ($(SYSTEM_LCMS2),YES)

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(LCMS2_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LCMS2_LIBS))

endef

else # !SYSTEM_LCMS2

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lcms2 \
))

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_use_libraries,$(1),\
	lcms2 \
)

endef

endif # SYSTEM_LCMS2


ifeq ($(SYSTEM_LPSOLVE),YES)

define gb_LinkTarget__use_lpsolve55
$(call gb_LinkTarget_add_libs,$(1),-llpsolve55)

endef

else # !SYSTEM_LPSOLVE

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lpsolve55 \
))

define gb_LinkTarget__use_lpsolve55
$(call gb_LinkTarget_use_libraries,$(1),\
	lpsolve55 \
)

endef

endif # SYSTEM_LPSOLVE


define gb_LinkTarget__use_gio
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GIO_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(GIO_LIBS))

endef

define gb_LinkTarget__use_gtk
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GTK_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(GTK_LIBS))

ifeq ($(ENABLE_GTK_PRINT),TRUE)

$(call gb_LinkTarget_add_defs,$(1),-DENABLE_GTK_PRINT)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GTK_PRINT_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(GTK_PRINT_LIBS))

endif

endef

define gb_LinkTarget__use_gthread
$(call gb_LinkTarget_add_libs,$(1),$(GTHREAD_LIBS))

endef


ifeq ($(ENABLE_DBUS),YES)

define gb_LinkTarget__use_dbus
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(DBUS_CFLAGS) \
)

$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_DBUS \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(DBUS_LIBS) \
)

endef

else # ENABLE_DBUS

define gb_LinkTarget__use_dbus

endef

endif # ENABLE_DBUS


define gb_LinkTarget__use_dbusmenugtk
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(DBUSMENUGTK_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(DBUSMENUGTK_LIBS))

endef

ifeq ($(SYSTEM_DB),YES)

define gb_LinkTarget__use_berkeleydb
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(filter -I%,$(SYSTEM_DB_CFLAGS)) \
)

$(call gb_LinkTarget_add_defs,$(1),\
	$(filter -D%,$(SYSTEM_DB_CFLAGS)) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	-l$(DB_LIB) \
)

endef

else # !SYSTEM_DB

ifneq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	db-4.7 \
))
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	db47 \
))
endif

define gb_LinkTarget__use_berkeleydb
ifneq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	db-4.7 \
)
else
$(call gb_LinkTarget_use_libraries,$(1),\
	db47 \
)
endif

endef

endif # SYSTEM_DB

ifeq ($(SYSTEM_LIBPNG),YES)

define gb_LinkTarget__use_png
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBPNG_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(LIBPNG_LIBS) \
)

endef

else # !SYSTEM_LIBPNG

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	png \
))

define gb_LinkTarget__use_png
$(call gb_LinkTarget_use_libraries,$(1),\
	png \
)

endef

endif # !SYSTEM_LIBPNG


ifeq ($(SYSTEM_CURL),YES)

define gb_LinkTarget__use_curl
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(CURL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CURL_LIBS))

endef

else # !SYSTEM_CURL

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	curl \
))

define gb_LinkTarget__use_curl
$(call gb_LinkTarget_use_libraries,$(1),\
	curl \
)

endef

endif # SYSTEM_CURL


ifeq ($(SYSTEM_MOZILLA_HEADERS),YES)

define gb_LinkTarget__use_mozilla_headers
$(eval $(call gb_LinkTarget_use_package,$(1),np_sdk_inc))
$(eval $(call gb_LinkTarget_set_include,$(1),\
	$(MOZILLA_HEADERS_CFLAGS) \
	$$(INCLUDE) \
))

endef

else #!SYSTEM_MOZILLA_HEADERS

define gb_LinkTarget__use_mozilla_headers
$(eval $(call gb_LinkTarget_use_package,$(1),np_sdk_inc))
$(eval $(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/npsdk \
	$$(INCLUDE) \
))

endef

endif #SYSTEM_MOZILLA_HEADERS


ifneq ($(VALGRIND_CFLAGS),)

define gb_LinkTarget__use_valgrind
$(call gb_LinkTarget_add_defs,$(1),\
    -DHAVE_VALGRIND_H \
)

$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(VALGRIND_CFLAGS) \
)

endef

else # !VALGRIND_CFLAGS

define gb_LinkTarget__use_valgrind

endef

endif # VALGRIND_CFLAGS

ifeq ($(SYSTEM_POPPLER),YES)

define gb_LinkTarget__use_poppler
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_POPPLER \
)

$(call gb_LinkTarget_set_include,$(1),\
	$(POPPLER_CFLAGS) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(POPPLER_LIBS) \
)

endef

else # !SYSTEM_POPPLER

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,\
	fofi \
	Goo \
	xpdf \
))

define gb_LinkTarget__use_poppler
$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/xpdf \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	fofi \
	Goo \
	xpdf \
)

ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_use_libraries,$(1),\
	objc \
)
else ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	advapi32 \
	gdi32 \
)
endif

endef

endif # SYSTEM_POPPLER


ifeq ($(SYSTEM_CLUCENE),YES)

define gb_LinkTarget__use_clucene
$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(CLUCENE_CFLAGS)) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(CLUCENE_CFLAGS)) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(CLUCENE_LIBS) \
)

endef

else # !SYSTEM_CLUCENE

define gb_LinkTarget__use_clucene
$(call gb_LinkTarget_use_libraries,$(1),\
	clucene \
)

endef

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	clucene \
))

endif # SYSTEM_CLUCENE

# MacOSX-only frameworks ############################################
# (in alphabetical order)

define gb_LinkTarget__use_carbon
$(call gb_LinkTarget_add_libs,$(1), \
	-framework Carbon \
)

endef

define gb_LinkTarget__use_cocoa
$(call gb_LinkTarget_add_libs,$(1), \
	-framework Cocoa \
)

endef

define gb_LinkTarget__use_iokit
$(call gb_LinkTarget_add_libs,$(1), \
	-framework IOKit \
)

endef

define gb_LinkTarget__use_qtkit
$(call gb_LinkTarget_add_libs,$(1), \
	-framework QTKit \
)

endef

define gb_LinkTarget__use_quicktime
$(call gb_LinkTarget_add_libs,$(1), \
	-framework QuickTime \
)

endef

# Common MacOSX and iOS frameworks

define gb_LinkTarget__use_corefoundation
$(call gb_LinkTarget_add_libs,$(1), \
	-framework CoreFoundation \
)

endef

define gb_LinkTarget__use_coregraphics
$(call gb_Library_add_libs,$(1), \
	-framework CoreGraphics \
)

endef

define gb_LinkTarget__use_coretext
$(call gb_Library_add_libs,$(1), \
	-framework CoreText \
)

endef

define gb_LinkTarget__use_foundation
$(call gb_LinkTarget_add_libs,$(1), \
	-framework Foundation \
)

endef

# iOS-only frameworks

define gb_LinkTarget__use_uikit
$(call gb_LinkTarget_add_libs,$(1), \
	-framework UIKit \
)

endef

ifeq ($(SYSTEM_NSS),YES)

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(NSS_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
    $(NSS_LIBS) \
)

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
    nspr4 \
    nss3 \
    plc4 \
))

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_use_libraries,$(1),\
    plc4 \
)

endef

endif

### Jars ############################################################

ifeq ($(SYSTEM_SAXON),YES)

define gb_JavaClassSet__use_saxon
$(call gb_JavaClassSet_use_system_jar,$(1),$(SAXON_JAR))
endef

else # !SYSTEM_SAXON

define gb_JavaClassSet__use_saxon
$(call gb_JavaClassSet_use_jar,$(1),$(OUTDIR)/bin/saxon9.jar)
endef

endif # SYSTEM_SAXON

ifeq ($(SYSTEM_BSH),YES)

define gb_JavaClassSet__use_bsh
$(call gb_JavaClassSet_use_system_jar,$(1),$(BSH_JAR))
endef

else # !SYSTEM_BSH

define gb_JavaClassSet__use_bsh
$(call gb_JavaClassSet_use_jar,$(1),$(OUTDIR)/bin/bsh.jar)
endef

endif # SYSTEM_BSH


# vim: set noet sw=4 ts=4:
