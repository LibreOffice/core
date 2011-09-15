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
$(eval $(call gb_LinkTarget_add_external_headers,$(1),Mesa_inc))
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
$(call gb_LinkTarget_add_linked_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
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

$(call gb_LinkTarget_add_linked_static_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	xslt \
)

endef

endif # SYSTEM_LIBXSLT


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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	rdf \
)

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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	cairo \
)
ifneq ($(OS),WNT)
ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_add_static_libs,$(1),\
	pixman-1 \
)
else
$(call gb_LinkTarget_add_linked_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icudt \
)

endef
define gb_LinkTarget__use_icuin
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icuin \
)

endef
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icudata \
	icui18n \
	icule \
	icutu \
	icuuc \
))

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icudata \
)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icui18n \
)

endef
endif

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icule \
)

endef
define gb_LinkTarget__use_icutu
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icutu \
)

endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icuuc \
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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	crypto \
	ssl \
)
else
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
	crypto \
	ssl \
)
ifeq ($(OS),SOLARIS)
$(call gb_LinkTarget_add_libs,$(1),\
	-lnsl \
	-lsocket \
)
endif
endif

endef

endif # SYSTEM_OPENSSL


ifeq ($(SYSTEM_LIBVISIO),YES)

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(LIBVISIO_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBVISIO_LIBS))

endef

else # !SYSTEM_LIBVISIO

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	visiolib \
))

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
	visiolib \
)

endef

endif # SYSTEM_LIBVISIO


ifeq ($(SYSTEM_LIBWPD),YES)

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(LIBWPD_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBWPD_LIBS))

endef

else # !SYSTEM_LIBWPD

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpdlib \
))

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
	wpdlib \
)

endef

endif # SYSTEM_LIBWPD


ifeq ($(SYSTEM_LIBWPG),YES)

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(LIBWPG_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBWPG_LIBS))

endef

else # !SYSTEM_LIBWPG

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpglib \
))

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
	wpglib \
)

endef

endif # SYSTEM_LIBWPG


ifeq ($(SYSTEM_LIBWPS),YES)

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(LIBWPS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBWPS_LIBS))

endef

else # !SYSTEM_LIBWPS

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	wpslib \
))

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
	wpslib \
)

endef

endif # SYSTEM_LIBWPS


ifeq ($(SYSTEM_LPSOLVE),YES)

define gb_LinkTarget__use_lpsolve55
$(call gb_LinkTarget_add_libs,$(1),-llpsolve55)

endef

else # !SYSTEM_LPSOLVE

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lpsolve55 \
))

define gb_LinkTarget__use_lpsolve55
$(call gb_LinkTarget_add_linked_libs,$(1),\
	lpsolve55 \
)

endef

endif # SYSTEM_LPSOLVE


define gb_LinkTarget__use_gtk
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GTK_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(GTK_LIBS))

endef

define gb_LinkTarget__use_gthread
$(call gb_LinkTarget_add_libs,$(1),$(GTHREAD_LIBS))

endef

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

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	berkeleydb \
))

define gb_LinkTarget__use_berkeleydb
$(call gb_LinkTarget_add_linked_libs,$(1),\
	db \
)

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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	png \
)

endef

endif # !SYSTEM_LIBPNG

# MacOSX-only frameworks
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

ifeq ($(SYSTEM_MOZILLA),YES)

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(MOZ_NSS_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
    $(MOZ_NSS_LIBS) \
)

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
    nspr4 \
    nss3 \
    plc4 \
))

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_add_linked_libs,$(1),\
    plc4 \
)

endef

endif

### Jars ############################################################

ifeq ($(SYSTEM_SAXON),YES)

define gb_JavaClassSet__use_saxon
$(call gb_JavaClassSet_add_system_jar,$(1),$(SAXON_JAR))
endef

else # !SYSTEM_SAXON

define gb_JavaClassSet__use_saxon
$(call gb_JavaClassSet_add_jar,$(1),$(OUTDIR)/bin/saxon9.jar)
endef

endif # SYSTEM_SAXON


# vim: set noet sw=4 ts=4:
