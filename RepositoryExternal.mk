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
$(call gb_LinkTarget_add_linked_libs,$(1),\
	pixman-1 \
)
endif
endef

endif # SYSTEM_CAIRO


ifeq ($(SYSTEM_GRAPHITE),YES)

#check for building with stlport, but graphite was not built with stlport
define gb_LinkTarget__use_graphite
ifneq ($(USE_SYSTEM_STL),YES)
$(call gb_LinkTarget_add_defs,$(1),\
	-DADAPT_EXT_STL \
)
endif
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GRAPHITE_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(GRAPHITE_LIBS))
endef

else # !SYSTEM_GRAPHITE

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    graphite \
))
define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_add_linked_static_libs,$(1),\
    graphite \
)
endef

endif # SYSTEM_GRAPHITE


ifeq ($(SYSTEM_ICU),YES)

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_libs,$(1),-licule)
endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_libs,$(1),-licuuc)
endef

else # !SYSTEM_ICU

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icule \
	icuuc \
))

define gb_LinkTarget__use_icule
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icule \
)
endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_linked_libs,$(1),\
	icuuc \
)
endef

endif # SYSTEM_ICU

# vim: set noet sw=4 ts=4:
