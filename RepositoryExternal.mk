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

# External headers

ifeq ($(SYSTEM_MESA_HEADERS),YES)

gb_LinkTarget__use_mesa_headers :=

else

define gb_LinkTarget__use_mesa_headers
$(eval $(call gb_LinkTarget_use_package,$(1),Mesa_inc))

endef

endif

ifeq ($(SYSTEM_MOZILLA_HEADERS),YES)

define gb_LinkTarget__use_mozilla_headers
$(call gb_LinkTarget_set_include,$(1),\
	$(MOZILLA_HEADERS_CFLAGS) \
	$$(INCLUDE) \
)

endef

else #!SYSTEM_MOZILLA_HEADERS

define gb_LinkTarget__use_mozilla_headers
$(call gb_LinkTarget_use_package,$(1),mozilla_inc)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/external/npsdk \
	$$(INCLUDE) \
)

endef

endif #SYSTEM_MOZILLA_HEADERS

ifeq ($(SYSTEM_ODBC_HEADERS),YES)

define gb_LinkTarget__use_odbc_headers
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_ODBC_HEADERS \
)

endef

else

define gb_LinkTarget__use_odbc_headers
$(call gb_LinkTarget_use_package,$(1),odbc_inc)

endef

endif

ifeq ($(SYSTEM_VIGRA),YES)

gb_LinkTarget__use_vigra_headers :=

else

define gb_LinkTarget__use_vigra_headers
$(call gb_LinkTarget_use_unpacked,$(1),vigra)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,vigra/include) \
	$$(INCLUDE) \
)

endef

endif

ifeq ($(SYSTEM_MDDS),YES)

gb_LinkTarget__use_mdds_headers :=

else

define gb_LinkTarget__use_mdds_headers
$(call gb_LinkTarget_use_unpacked,$(1),mdds)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,mdds/include) \
	$$(INCLUDE) \
)

endef

endif

# External libraries

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

ifeq ($(OS),ANDROID)
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	fontconfig \
	freetype \
))
endif

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


ifeq ($(SYSTEM_BOOST),YES)

define gb_LinkTarget__use_boostdatetime
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(BOOST_CPPFLAGS) \
)

$(call gb_LinkTarget_add_ldflags,$(1),\
	$(BOOST_LDFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(BOOST_DATE_TIME_LIB) \
)

endef

else # !SYSTEM_BOOST

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	boostdatetime \
))

define gb_LinkTarget__use_boostdatetime
$(call gb_LinkTarget_use_static_libraries,$(1),\
	boostdatetime \
)

endef

endif # SYSTEM_BOOST


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
	exttextcat-1.0 \
))
endif

define gb_LinkTarget__use_libexttextcat
ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	libexttextcat \
)
else
$(call gb_LinkTarget_use_static_libraries,$(1),\
	exttextcat-1.0 \
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

define gb_LinkTarget__use_libexslt
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBEXSLT_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(LIBEXSLT_LIBS))

endef

else # !SYSTEM_LIBXSLT

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	exslt \
	xslt \
))

define gb_LinkTarget__use_libxslt
$(call gb_LinkTarget_use_libraries,$(1),\
	xslt \
)

endef

define gb_LinkTarget__use_libexslt
$(call gb_LinkTarget_use_libraries,$(1),\
	exslt \
)

endef

endif # SYSTEM_LIBXSLT


ifeq ($(SYSTEM_GLIB),YES)

define gb_LinkTarget__use_glib
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GLIB_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(GLIB_LIBS))

endef

else # !SYSTEM_GLIB

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	glib-2.0 \
	gmodule-2.0 \
))

define gb_LinkTarget__use_glib
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/external/glib-2.0 \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	glib-2.0 \
	gmodule-2.0 \
)

endef

endif # SYSTEM_GLIB


ifeq ($(ENABLE_LIBLANGTAG),YES)

ifeq ($(SYSTEM_LIBLANGTAG),YES)

define gb_LinkTarget__use_liblangtag
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBLANGTAG_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(LIBLANGTAG_LIBS))

endef

else # !SYSTEM_LIBLANGTAG

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	langtag \
))

define gb_LinkTarget__use_liblangtag
$(call gb_LinkTarget_use_libraries,$(1),\
	langtag \
)

ifeq ($(OS),MACOSX)

$(call gb_LinkTarget_add_libs,$(1),$(foreach replaceme,libglib-2.0.0 libgmodule-2.0.0,-dylib_file @loader_path/$(replaceme).dylib:$(gb_Library_OUTDIRLOCATION)/$(replaceme).dylib))

endif

endef

endif # SYSTEM_LIBLANGTAG

endif # ENABLE_LIBLANGTAG


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

ifneq ($(OS),ANDROID)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	rdf \
))

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_use_libraries,$(1),\
	rdf \
)

ifeq ($(OS),MACOSX)

$(call gb_LinkTarget_add_libs,$(1),$(foreach replaceme,librasqal-lo.1 libraptor-lo.1,-dylib_file @loader_path/$(replaceme).dylib:$(gb_Library_OUTDIRLOCATION)/$(replaceme).dylib))

endif

endef

else # ANDROID

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	rdf \
	rasqal \
	raptor \
))

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_use_static_libraries,$(1),\
	rdf \
	rasqal \
	raptor \
	crypto \
)

endef

endif # ANDROID

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
ifeq ($(OS),MACOSX)
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	pixman-1 \
))
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	pixman-1 \
))
endif # MACOSX
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

ifneq ($(OS),ANDROID)
$(call gb_LinkTarget_add_libs,$(1),$(FREETYPE_LIBS))
else
$(call gb_LinkTarget_use_static_libraries,$(1),freetype)
endif

endef

define gb_LinkTarget__use_fontconfig
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FONTCONFIG_CFLAGS) \
)

ifneq ($(OS),ANDROID)
$(call gb_LinkTarget_add_libs,$(1),$(FONTCONFIG_LIBS))
else
$(call gb_LinkTarget_use_static_libraries,$(1),fontconfig)
endif

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


ifeq ($(SYSTEM_LIBMSPUB),YES)

define gb_LinkTarget__use_mspub
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(MSPUB_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MSPUB_LIBS))

endef

else # !SYSTEM_LIBMSPUB

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	mspublib \
))

define gb_LinkTarget__use_mspub
$(call gb_LinkTarget_use_static_libraries,$(1),\
	mspublib \
)

endef

endif # SYSTEM_LIBMSPUB


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

ifeq ($(OS),ANDROID)

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	lcms2 \
))

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_use_static_libraries,$(1),\
	lcms2 \
)

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lcms2 \
))

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_use_libraries,$(1),\
	lcms2 \
)

endef

endif # ANDROID
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

ifeq ($(ENABLE_GIO),TRUE)

define gb_LinkTarget__use_gio
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GIO_CFLAGS) \
)

$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_GIO \
)

$(call gb_LinkTarget_add_libs,$(1),$(GIO_LIBS))

endef

else # ENABLE_GIO

define gb_LinkTarget__use_gio

endef

endif # ENABLE_GIO

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


ifeq ($(ENABLE_DBUS),TRUE)

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


ifeq ($(ENABLE_TELEPATHY),TRUE)

define gb_LinkTarget__use_telepathy
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(TELEPATHY_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(TELEPATHY_LIBS) \
)

endef

else # !ENABLE_TELEPATHY

gb_LinkTarget__use_telepathy :=

endif # ENABLE_TELEPATHY


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

define gb_LinkTarget__use_gobject
$(call gb_LinkTarget_add_libs,$(1),\
	$(GOBJECT_LIBS) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GOBJECT_CFLAGS) \
)
endef


ifeq ($(SYSTEM_HSQLDB),YES)

define gb_LinkTarget__use_hsqldb

$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_HSQLDB \
	-DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\" \
)

endef

else # !SYSTEM_HSQLDB

define gb_LinkTarget__use_hsqldb

endef

endif # SYSTEM_HSQLDB


ifeq ($(SYSTEM_OPENLDAP),YES)

define gb_LinkTarget__use_openldap

$(call gb_LinkTarget_add_libs,$(1),\
	-lldap \
	-llber \
)

endef

else # !SYSTEM_OPENLDAP

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,\
	ldap \
	lber \
))

define gb_LinkTarget__use_openldap

$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/openldap \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	ldap \
	lber \
)

endef

endif # SYSTEM_OPENLDAP


ifeq ($(SYSTEM_POSTGRESQL),YES)

define gb_LinkTarget__use_postgresql

$(call gb_LinkTarget_set_include,$(1),\
	$(POSTGRESQL_INC) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	-lpq \
)

$(call gb_LinkTarget_add_ldflags,$(1),\
	$(POSTGRESQL_LIB) \
)

endef

else # !SYSTEM_POSTGRESQL

define gb_LinkTarget__use_postgresql

$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/postgresql \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	$(if $(filter MSC,$(COM)),lib)pq \
)

ifeq ($(GUI)$(COM),WNTMSC)
$(call gb_LinkTarget_use_external,$(1),openssl)

$(call gb_LinkTarget_use_libraries,$(1),\
	secur32 \
	ws2_32 \
)

endif

endef

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,\
	$(if $(filter MSC,$(COM)),lib)pq \
))

endif # SYSTEM_POSTGRESQL

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
ifeq ($(SYSTEM_MOZILLA),YES)

# Nothing needed here ATM
define gb_LinkTarget__use_mozilla

endef

else # !SYSTEM_MOZILLA

define gb_LinkTarget__use_mozilla

$(call gb_LinkTarget_add_defs,$(1),\
	-DMOZILLA_INTERNAL_API \
)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/mozilla/moz \
	-I$(OUTDIR)/inc/mozilla/nspr \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	nspr4 \
	xpcom \
	xpcom_core \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	embed_base_s \
	mozreg_s \
)

ifeq ($(GUI),WNT)

$(call gb_LinkTarget_add_defs,$(1),\
	-DMOZILLA_CLIENT \
	-DMOZ_REFLOW_PERF \
	-DMOZ_REFLOW_PERF_DSP \
	-DMOZ_XUL \
	-DOJI \
	-DWIN32 \
	-DXP_PC \
	-DXP_WIN \
	-DXP_WIN32 \
	-D_WINDOWS \
)

ifeq ($(COM),GCC)

$(call gb_LinkTarget_add_cxxflags,$(1),\
	-Wall \
	-Wcast-align \
	-Wconversion \
	-Wno-long-long \
	-Woverloaded-virtual \
	-Wpointer-arith \
	-Wsynth \
	-fno-rtti \
)

else

ifneq ($(DBG_LEVEL),0)
$(call gb_LinkTarget_add_defs,$(1),\
	-D_STL_NOFORCE_MANIFEST \
)
endif

$(call gb_LinkTarget_add_libs,$(1),\
	$(OUTDIR)/lib/embed_base_s.lib \
	$(OUTDIR)/lib/mozreg_s.lib \
)

endif # !GCC

else ifeq ($(GUI),UNX)

$(call gb_LinkTarget_add_defs,$(1),\
	-DMOZILLA_CLIENT \
	-DXP_UNIX \
	$(if $(filter LINUX,$(OS)),-DOJI) \
	$(if $(filter LINUX MACOSX NETBSD,$(OS)),-DTRACING) \
)

$(call gb_LinkTarget_add_cflags,$(1),\
	$(if $(filter LINUX,$(OS)),-g) \
	$(if $(filter LINUX MACOSX NETBSD,$(OS)),-fPIC) \
)

$(call gb_LinkTarget_add_cxxflags,$(1),\
	-Wcast-align \
	-Wconversion \
	-Wno-long-long \
	-Woverloaded-virtual \
	-Wpointer-arith \
	-Wsynth \
	$(if $(filter LINUX,$(OS)),-pthread) \
	$(if $(filter MACOSX NETBSD,$(OS)),-Wno-deprecated) \
)

endif # UNX

endef

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,\
	xpcom \
	xpcom_core \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS,\
	embed_base_s \
	mozreg_s \
))

endif # SYSTEM_MOZILLA
endif # DESKTOP

ifeq ($(ENABLE_KDE),TRUE)

define gb_LinkTarget__use_kde
$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(KDE_CFLAGS)) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(KDE_CFLAGS)) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(KDE_LIBS) \
)

ifeq ($(COM),GCC)
$(call gb_LinkTarget_add_cxxflags,$(1),\
	-Wno-shadow \
)
endif

endef

else # !ENABLE_KDE

define gb_LinkTarget__use_kde

endef

endif # ENABLE_KDE


ifeq ($(ENABLE_KDE4),TRUE)

define gb_LinkTarget__use_kde4
$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(KDE4_CFLAGS)) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(KDE4_CFLAGS)) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(KDE4_LIBS) \
)

ifeq ($(COM),GCC)
$(call gb_LinkTarget_add_cxxflags,$(1),\
	-Wno-shadow \
)
endif

endef

else # !ENABLE_KDE4

define gb_LinkTarget__use_kde4

endef

endif # ENABLE_KDE4


ifeq ($(ENABLE_TDE),TRUE)

define gb_LinkTarget__use_tde
$(call gb_LinkTarget_add_libs,$(1),\
	$(TDE_LIBS) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(TDE_CFLAGS) \
)
endef

else # ! ENABLE_TDE

define gb_LinkTarget__use_tde

endef

endif # ENABLE_TDE


ifeq ($(ENABLE_GCONF),TRUE)

define gb_LinkTarget__use_gconf
$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(GCONF_CFLAGS)) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(GCONF_CFLAGS)) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(GCONF_LIBS) \
)

endef

else # !ENABLE_GCONF

define gb_LinkTarget__use_gconf

endef

endif # ENABLE_GCONF


# PYTHON
ifeq ($(SYSTEM_PYTHON),YES)

define gb_LinkTarget__use_python
$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(PYTHON_CFLAGS)) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(PYTHON_CFLAGS)) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(PYTHON_LIBS) \
)

endef

else # !SYSTEM_PYTHON

define gb_LinkTarget__use_python

ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_libs,$(1),\
    python26.lib \
)
else ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_add_libs,$(1),\
    -F$(OUTDIR)/lib -framework OOoPython \
)
else
$(call gb_LinkTarget_use_libraries,$(1),\
    python2.6 \
)
endif

$(call gb_LinkTarget_set_include,$(1),\
	-I$(OUTDIR)/inc/python \
	$$(INCLUDE) \
)

endef

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	python2.6 \
))

endif # SYSTEM_PYTHON


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

define gb_LinkTarget__use_coreservices
$(call gb_Library_add_libs,$(1), \
	-framework CoreServices \
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

define gb_LinkTarget__use_systemconfiguration
$(call gb_LinkTarget_add_libs,$(1), \
	-framework SystemConfiguration \
)

endef

# iOS-only frameworks

define gb_LinkTarget__use_uikit
$(call gb_LinkTarget_add_libs,$(1), \
	-framework UIKit \
)

endef

ifeq (,$(filter DESKTOP,$(BUILD_TYPE)))

define gb_LinkTarget__use_nss3

endef

else

ifeq ($(SYSTEM_NSS),YES)

define gb_LinkTarget__use_nss3
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_MOZILLA \
)

$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(NSS_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
    $(NSS_LIBS) \
)

endef

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget__use_nss3,$(1))

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
    nspr4 \
    nss3 \
    plc4 \
    smime3 \
))

define gb_LinkTarget__use_nss3
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/mozilla/nspr \
	-I$(OUTDIR)/inc/mozilla/nss \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	nspr4 \
	nss3 \
	smime3 \
)

endef

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_use_libraries,$(1),\
    plc4 \
)

endef

endif
endif # DESKTOP

### Jars ############################################################

ifeq ($(SYSTEM_HSQLDB),YES)

define gb_Jar__use_hsqldb
$(call gb_Jar_use_system_jar,$(1),$(HSQLDB_JAR))
endef
define gb_JunitTest__use_hsqldb
$(call gb_JunitTest_use_system_jar,$(1),$(HSQLDB_JAR))
endef

else # !SYSTEM_HSQLDB

$(eval $(call gb_Helper_register_jars,OOO,\
	hsqldb \
))

define gb_Jar__use_hsqldb
$(call gb_Jar_use_jar,$(1),hsqldb)
endef
define gb_JunitTest__use_hsqldb
$(call gb_JunitTest_use_jar,$(1),hsqldb)
endef

endif # SYSTEM_HSQLDB

ifeq ($(SYSTEM_SAXON),YES)

define gb_Jar__use_saxon
$(call gb_Jar_use_system_jar,$(1),$(SAXON_JAR))
endef

else # !SYSTEM_SAXON

$(eval $(call gb_Helper_register_jars,OOO,\
	saxon9 \
))

define gb_Jar__use_saxon
$(call gb_Jar_use_jar,$(1),saxon9)
endef

endif # SYSTEM_SAXON

ifeq ($(SYSTEM_BSH),YES)

define gb_Jar__use_bsh
$(call gb_Jar_use_system_jar,$(1),$(BSH_JAR))
endef

else # !SYSTEM_BSH

$(eval $(call gb_Helper_register_jars,OOO,\
	bsh \
))

define gb_Jar__use_bsh
$(call gb_Jar_use_jar,$(1),bsh)
endef

endif # SYSTEM_BSH


ifeq ($(SYSTEM_RHINO),YES)

define gb_Jar__use_rhino
$(call gb_Jar_use_system_jar,$(1),$(RHINO_JAR))
endef

else # !SYSTEM_RHINO

$(eval $(call gb_Helper_register_jars,OOO,\
	js \
))

define gb_Jar__use_rhino
$(call gb_Jar_use_jar,$(1),js)
endef

endif # SYSTEM_RHINO

ifeq ($(SYSTEM_APACHE_COMMONS),YES)

define gb_Jar__use_commons-codec
$(call gb_Jar_use_system_jar,$(1),$(COMMONS_CODEC_JAR))
endef

define gb_Jar__use_commons-httpclient
$(call gb_Jar_use_system_jar,$(1),$(COMMONS_HTTPCLIENT_JAR))
endef

define gb_Jar__use_commons-lang
$(call gb_Jar_usadd_linked_libse_system_jar,$(1),$(COMMONS_LANG_JAR))
endef

define gb_Jar__use_commons-logging
$(call gb_Jar_use_system_jar,$(1),$(COMMONS_LOGGING_JAR))
endef

else # !SYSTEM_APACHE_COMMONS

$(eval $(call gb_Helper_register_jars,OXT,\
	commons-codec-1.3 \
	commons-httpclient-3.1 \
	commons-lang-2.3 \
	commons-logging-1.1.1 \
))

define gb_Jar__use_commons-codec
$(call gb_Jar_use_jar,$(1),commons-codec-1.3)
endef

define gb_Jar__use_commons-httpclient
$(call gb_Jar_use_jar,$(1),commons-httpclient-3.1)
endef

define gb_Jar__use_commons-lang
$(call gb_Jar_use_jar,$(1),commons-lang-2.3)
endef

define gb_Jar__use_commons-logging
$(call gb_Jar_use_jar,$(1),commons-logging-1.1.1)
endef

endif # SYSTEM_APACHE_COMMONS


ifeq ($(SYSTEM_JFREEREPORT),YES)

define gb_Jar__use_flow-engine
$(call gb_Jar_use_system_jar,$(1),$(JFREEREPORT_JAR))
endef

define gb_Jar__use_flute
$(call gb_Jar_use_system_jar,$(1),$(FLUTE_JAR))
endef

define gb_Jar__use_libbase
$(call gb_Jar_use_system_jar,$(1),$(LIBBASE_JAR))
endef

define gb_Jar__use_libfonts
$(call gb_Jar_use_system_jar,$(1),$(LIBFONTS_JAR))
endef

define gb_Jar__use_libformula
$(call gb_Jar_use_system_jar,$(1),$(LIBFORMULA_JAR))
endef

define gb_Jar__use_liblayout
$(call gb_Jar_use_system_jar,$(1),$(LIBLAYOUT_JAR))
endef

define gb_Jar__use_libloader
$(call gb_Jar_use_system_jar,$(1),$(LIBLOADER_JAR))
endef

define gb_Jar__use_librepository
$(call gb_Jar_use_system_jar,$(1),$(LIBREPOSITORY_JAR))
endef

define gb_Jar__use_libserializer
$(call gb_Jar_use_system_jar,$(1),$(LIBSERIALIZER_JAR))
endef

define gb_Jar__use_libxml
$(call gb_Jar_use_system_jar,$(1),$(LIBXML_JAR))
endef

define gb_Jar__use_sac
$(call gb_Jar_use_system_jar,$(1),$(SAC_JAR))
endef

else # !SYSTEM_JFREEREPORT

$(eval $(call gb_Helper_register_jars,OXT,\
	flow-engine-0.9.4 \
	flute-1.1.6 \
	libbase-1.1.6 \
	libfonts-1.1.6 \
	libformula-1.1.7 \
	liblayout-0.2.10 \
	libloader-1.1.6 \
	librepository-1.1.6 \
	libserializer-1.1.6 \
	libxml-1.1.7 \
	sac \
))

define gb_Jar__use_flow-engine
$(call gb_Jar_use_jar,$(1),flow-engine-0.9.4)
endef

define gb_Jar__use_flute
$(call gb_Jar_use_jar,$(1),flute-1.1.6)
endef

define gb_Jar__use_libbase
$(call gb_Jar_use_jar,$(1),libbase-1.1.6)
endef

define gb_Jar__use_libfonts
$(call gb_Jar_use_jar,$(1),libfonts-1.1.6)
endef

define gb_Jar__use_libformula
$(call gb_Jar_use_jar,$(1),libformula-1.1.7)
endef

define gb_Jar__use_liblayout
$(call gb_Jar_use_jar,$(1),liblayout-0.2.10)
endef

define gb_Jar__use_libloader
$(call gb_Jar_use_jar,$(1),libloader-1.1.6)
endef

define gb_Jar__use_librepository
$(call gb_Jar_use_jar,$(1),librepository-1.1.6)
endef

define gb_Jar__use_libserializer
$(call gb_Jar_use_jar,$(1),libserializer-1.1.6)
endef

define gb_Jar__use_libxml
$(call gb_Jar_use_jar,$(1),libxml-1.1.7)
endef

define gb_Jar__use_sac
$(call gb_Jar_use_jar,$(1),sac)
endef

endif # SYSTEM_JFREEREPORT

# vim: set noet sw=4 ts=4:
