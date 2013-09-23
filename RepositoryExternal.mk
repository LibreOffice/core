# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# this file describes all the external libraries
# depending on the configure options these may be taken from the system,
# or the internal/bundled copy may be built.

# for every external, a function gb_LinkTarget_use__FOO is defined,
# once for the system case, once for the internal case.

# in the system case, no libraries should be registered, but the target-local
# variable LIBS should be set to FOO_LIBS, and INCLUDES to FOO_CFLAGS.

# External headers

ifeq ($(SYSTEM_MARIADB),YES)

define gb_LinkTarget__use_mariadb
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(MARIADB_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MARIADB_LIBS))

endef
gb_ExternalProject__use_mariadb :=

else # !SYSTEM_MARIADB

define gb_LinkTarget__use_mariadb
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(MARIADB_CFLAGS) \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	mariadblib \
)

endef
define gb_ExternalProject__use_mariadb
$(call gb_ExternalProject_use_static_libraries,$(1),mariadblib)

endef

endif # SYSTEM_MARIADB


ifeq ($(SYSTEM_MESA_HEADERS),YES)

gb_LinkTarget__use_mesa_headers :=

else

define gb_LinkTarget__use_mesa_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(SRCDIR)/Mesa/inc \
	$$(INCLUDE) \
)

endef

endif

ifeq ($(SYSTEM_NPAPI_HEADERS),YES)

# yes this uses internal headers too...
# they are split across 2 dirs for this reason
define gb_LinkTarget__use_npapi_headers
$(call gb_LinkTarget_set_include,$(1),\
	$(NPAPI_HEADERS_CFLAGS) \
	-I$(SRCDIR)/np_sdk \
	$$(INCLUDE) \
)

endef

else #!SYSTEM_NPAPI_HEADERS

define gb_LinkTarget__use_npapi_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(SRCDIR)/np_sdk/inc \
	-I$(SRCDIR)/np_sdk \
	$$(INCLUDE) \
)

endef

endif #SYSTEM_NPAPI_HEADERS

ifeq ($(SYSTEM_ODBC_HEADERS),YES)

define gb_LinkTarget__use_odbc_headers
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_ODBC_HEADERS \
)

endef

else

define gb_LinkTarget__use_odbc_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(SRCDIR)/unixODBC/inc \
	$$(INCLUDE) \
)

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

ifeq (SANE,$(filter SANE,$(BUILD_TYPE)))

define gb_LinkTarget__use_sane_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(SRCDIR)/$(if $(filter WNT,$(OS)),twain,sane)/inc \
	$$(INCLUDE) \
)

endef

else

gb_LinkTarget__use_sane_headers :=

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
$(call gb_LinkTarget_use_package,$(1),cppunit)

$(call gb_LinkTarget_use_libraries,$(1),\
    cppunit \
)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,cppunit/include)\
	$$(INCLUDE) \
)

endef

endif

define gb_LinkTarget__use_iconv
$(call gb_LinkTarget_add_libs,$(1),-liconv)

endef

ifeq ($(SYSTEM_MARIADB),YES)

define gb_LinkTarget__use_mysql

$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_MARIADB \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(MARIADB_LIBS) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$(MARIADB_CFLAGS) \
	$$(INCLUDE) \
)
endef

else

define gb_LinkTarget__use_mysql

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
)

endef

endif

ifeq ($(SYSTEM_MYSQL_CPPCONN),YES)

define gb_LinkTarget__use_mysqlcppconn
$(call gb_LinkTarget_add_libs,$(1),\
	-lmysqlcppconn \
)

$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_MYSQL_CPPCONN \
)
endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,\
	mysqlcppconn \
))

# note: this does not link mysqlcppconn, it is loaded via osl_loadModuleRelative
define gb_LinkTarget__use_mysqlcppconn

$(call gb_LinkTarget_use_unpacked,$(1),mysqlcppconn)

$(call gb_LinkTarget_add_defs,$(1),\
	-DCPPCONN_LIB_BUILD \
)

$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,mysqlcppconn) \
	-I$(call gb_UnpackedTarball_get_dir,mysqlcppconn)/cppconn \
	$$(INCLUDE) \
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

# nothing on system
define gb_LinkTarget__use_zlib_x64

endef

gb_ExternalProject__use_zlib :=

else # !SYSTEM_ZLIB

define gb_LinkTarget__use_zlib_multiarch
$(if $(2),,$(call gb_Output_error,gb_LinkTarget__use_zlib_multiarch needs two arguments))

$(call gb_LinkTarget_set_include,$(1),\
	$(ZLIB_CFLAGS) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	$(2) \
)

endef

define gb_LinkTarget__use_zlib
$(call gb_LinkTarget__use_zlib_multiarch,$(1),zlib)

endef

define gb_LinkTarget__use_zlib_x64
$(call gb_LinkTarget__use_zlib_multiarch,$(1),zlib_x64)

endef

define gb_ExternalProject__use_zlib
$(call gb_ExternalProject_use_static_libraries,$(1),zlib)

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

define gb_LinkTarget__use_jpeg
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,jpeg) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	jpeg \
)

endef

endif # SYSTEM_JPEG

ifeq ($(SYSTEM_MYTHES),YES)

define gb_LinkTarget__use_mythes
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(MYTHES_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MYTHES_LIBS))

endef

else # !SYSTEM_MYTHES

define gb_LinkTarget__use_mythes
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,mythes) \
	$$(INCLUDE) \
)

ifeq ($(COM),MSC)
$(call gb_LinkTarget_use_package,$(1),\
	mythes \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	mythes \
)
else
$(call gb_LinkTarget_add_libs,$(1),$(MYTHES_LIBS))
$(call gb_LinkTarget_use_external_project,$(1),mythes)
endif

endef

endif # SYSTEM_MYTHES


ifeq ($(SYSTEM_EXPAT),YES)

define gb_LinkTarget__use_expat_impl
$(if $(2),,$(error gb_LinkTarget__use_expat_impl needs additional parameter))

$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_EXPAT \
)

$(call gb_LinkTarget_add_libs,$(1),-lexpat)

endef

gb_ExternalProject__use_expat :=

else # !SYSTEM_EXPAT

define gb_LinkTarget__use_expat_impl
$(if $(2),,$(error gb_LinkTarget__use_expat_impl needs additional parameter))

$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,expat)/lib \
    $$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	$(2) \
)

endef

define gb_ExternalProject__use_expat
$(call gb_ExternalProject_use_static_libraries,$(1),expat)

endef

endif # SYSTEM_EXPAT

define gb_LinkTarget__use_expat
$(call gb_LinkTarget__use_expat_impl,$(1),expat)

endef

define gb_LinkTarget__use_expat_x64
$(call gb_LinkTarget__use_expat_impl,$(1),expat_x64)

endef

ifeq ($(SYSTEM_HYPH),YES)

define gb_LinkTarget__use_hyphen
$(call gb_LinkTarget_add_libs,$(1),$(HYPHEN_LIB))

endef

else # !SYSTEM_HYPH

define gb_LinkTarget__use_hyphen
$(call gb_LinkTarget_use_unpacked,$(1),hyphen)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,hyphen)\
	$$(INCLUDE) \
)

ifeq ($(COM),MSC)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	hyphen \
)
else
$(call gb_LinkTarget_add_libs,$(1),$(HYPHEN_LIB))
$(call gb_LinkTarget_use_external_project,$(1),hyphen)
endif

endef

endif # SYSTEM_HYPH

ifeq ($(SYSTEM_HUNSPELL),YES)

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(HUNSPELL_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(HUNSPELL_LIBS))

endef

gb_ExternalProject__use_hunspell :=

else # !SYSTEM_HUNSPELL

define gb_LinkTarget__use_hunspell
$(call gb_LinkTarget_add_defs,$(1),\
	-DHUNSPELL_STATIC \
)
$(call gb_LinkTarget_use_unpacked,$(1),hunspell)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,hunspell/src/hunspell)\
	$$(INCLUDE) \
)

ifeq ($(COM),MSC)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	hunspell \
)
else
$(call gb_LinkTarget_add_libs,$(1),$(HUNSPELL_LIBS))
$(call gb_LinkTarget_use_external_project,$(1),hunspell)
endif

endef

define gb_ExternalProject__use_hunspell
$(call gb_ExternalProject_use_external_project,$(1),hunspell)

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

define gb_LinkTarget__use_boostsystem
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(BOOST_CPPFLAGS) \
)

$(call gb_LinkTarget_add_ldflags,$(1),\
	$(BOOST_LDFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(BOOST_SYSTEM_LIB) \
)

endef

gb_ExternalProject__use_boostsystem :=

define gb_LinkTarget__use_boost_headers
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(BOOST_CPPFLAGS) \
)

endef

gb_ExternalProject__use_boost_headers:=

else # !SYSTEM_BOOST

ifeq ($(OS),WNT)
define gb_LinkTarget__use_boostthread
$(call gb_LinkTarget_add_defs,$(1),\
	-DBOOST_ALL_NO_LIB \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	boostthread \
)
endef
endif

define gb_LinkTarget__use_boostdatetime
$(call gb_LinkTarget_add_defs,$(1),\
	-DBOOST_ALL_NO_LIB \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	boostdatetime \
)

endef

define gb_LinkTarget__use_boostsystem
$(call gb_LinkTarget_add_defs,$(1),\
	-DBOOST_ALL_NO_LIB \
)

$(call gb_LinkTarget_use_static_libraries,$(1),\
	boostsystem \
)

endef

define gb_ExternalProject__use_boostsystem
$(call gb_ExternalProject_use_static_libraries,$(1),boostsystem)
endef

define gb_LinkTarget__use_boost_headers
$(call gb_LinkTarget_use_unpacked,$(1),boost)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,boost) \
	$$(INCLUDE) \
)

endef

define gb_ExternalProject__use_boost_headers
$(call gb_ExternalProject_get_preparation_target,$(1)) :| $(call gb_UnpackedTarball_get_final_target,boost)

endef
endif # SYSTEM_BOOST


ifeq ($(SYSTEM_CMIS),YES)

define gb_LinkTarget__use_cmis
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(CMIS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CMIS_LIBS))

endef

else # !SYSTEM_CMIS

define gb_LinkTarget__use_cmis
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,cmis)/src \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	cmislib \
)

endef

endif # SYSTEM_CMIS

ifeq ($(OS)$(COM),WNTGCC)

define gb_LinkTarget__use_jawt
$(call gb_LinkTarget_use_packages,$(1),\
	jawt \
)

endef

else # $(OS)$(COM) != WNTGCC

gb_LinkTarget__use_jawt :=

endif # $(OS)$(COM) = WNTGCC


ifeq ($(SYSTEM_LIBATOMIC_OPS),YES)

define gb_LinkTarget__use_libatomic_ops
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBATOMIC_OPS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1), $(LIBATOMIC_OPS_LIBS))

endef
gb_ExternalProject__use_libatomic_ops :=

else # !SYSTEM_LIBATOMIC_OPS

define gb_LinkTarget__use_libatomic_ops
$(call gb_LinkTarget_set_include,$(1),\
$(LIBATOMIC_OPS_CFLAGS) \
	$$(INCLUDE) \
	$(LIBATOMIC_OPS_CFLAGS) \
)
$(call gb_LinkTarget_use_package,$(1),\
	libatomic_ops \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	libatomic_ops \
)

endef

define gb_ExternalProject__use_libatomic_ops
$(call gb_ExternalProject_use_package,$(1),libatomic_ops)

endef

endif # SYSTEM_LIBATOMIC_OPS


ifeq ($(SYSTEM_LIBEXTTEXTCAT),YES)

define gb_LinkTarget__use_libexttextcat
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBEXTTEXTCAT_CFLAGS) \
)
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_LIBEXTTEXTCAT \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBEXTTEXTCAT_LIBS))

endef

else # !SYSTEM_LIBEXTTEXTCAT

define gb_LinkTarget__use_libexttextcat
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,exttextcat/src) \
	$$(INCLUDE) \
)

ifeq ($(COM),MSC)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	exttextcat \
)
else
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,exttextcat)/src/.libs/libexttextcat-2.0.a\
)
$(call gb_LinkTarget_use_external_project,$(1),mythes)
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
gb_ExternalProject__use_libxml2:=

else # !SYSTEM_LIBXML

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	xml2 \
))

define gb_LinkTarget__use_libxml2
$(call gb_LinkTarget_use_package,$(1),xml2)
$(call gb_LinkTarget_use_libraries,$(1),\
	xml2 \
)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,xml2)/include \
)

endef
define gb_ExternalProject__use_libxml2
$(call gb_ExternalProject_use_package,$(1),xml2)

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
$(call gb_LinkTarget_use_package,$(1),xslt)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,xslt) \
)
$(call gb_LinkTarget_use_libraries,$(1),\
	xslt \
)

endef

define gb_LinkTarget__use_libexslt
$(call gb_LinkTarget_use_package,$(1),xslt)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,xslt) \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	exslt \
)

endef

endif # SYSTEM_LIBXSLT

define gb_LinkTarget__use_glib
$(error gb_LinkTarget__use_glib should not be called any more)
endef

ifeq ($(ENABLE_LIBLANGTAG),TRUE)

ifeq ($(SYSTEM_LIBLANGTAG),YES)

define gb_LinkTarget__use_liblangtag
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBLANGTAG_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(LIBLANGTAG_LIBS))

endef

else # !SYSTEM_LIBLANGTAG

define gb_LinkTarget__use_liblangtag
$(call gb_LinkTarget_use_unpacked,$(1),langtag)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,langtag) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LIBLANGTAG_LIBS))
$(call gb_LinkTarget_use_external_project,$(1),langtag)

endef

endif # SYSTEM_LIBLANGTAG

else

gb_LinkTarget__use_liblangtag :=

endif # ENABLE_LIBLANGTAG


ifeq ($(SYSTEM_NEON),YES)

define gb_LinkTarget__use_neon
$(call gb_LinkTarget_add_defs,$(1),\
	-DNEON_VERSION=0x$(NEON_VERSION) \
	-DSYSTEM_NEON \
)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(NEON_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(NEON_LIBS))

endef

else # !SYSTEM_NEON

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	neon \
))

define gb_LinkTarget__use_neon
$(call gb_LinkTarget_use_unpacked,$(1),neon)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,neon/src) \
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

gb_LinkTarget__use_redland_headers:=

gb_LinkTarget__use_raptor_headers:=

gb_LinkTarget__use_rasqal_headers:=

else # !SYSTEM_REDLAND

define gb_LinkTarget__use_redland_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,redland)/src \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_raptor_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,raptor)/src \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_rasqal_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,rasqal)/src \
	$$(INCLUDE) \
)

endef

ifneq ($(OS),ANDROID)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	raptor2 \
	rasqal \
	rdf \
))

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_use_packages,$(1),raptor rasqal redland)
$(call gb_LinkTarget_use_libraries,$(1),\
	raptor2 \
	rdf \
)

endef

else # ANDROID

define gb_LinkTarget__use_librdf
$(call gb_LinkTarget_use_packages,$(1),raptor rasqal redland)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	rdf \
	rasqal \
	raptor2 \
)

endef

endif # ANDROID

endif # SYSTEM_REDLAND


ifeq ($(SYSTEM_CAIRO),YES)

define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(CAIRO_CFLAGS) \
)
$(call gb_LinkTarget_use_external,$(1),freetype_headers)
$(call gb_LinkTarget_add_libs,$(1),$(CAIRO_LIBS))

endef

else ifeq ($(SYSTEM_CAIRO),NO)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	cairo \
	$(if $(filter-out MACOSX WNT,$(OS)), \
		pixman-1 \
	) \
))

define gb_LinkTarget__use_cairo
$(call gb_LinkTarget_use_package,$(1),cairo)
$(call gb_LinkTarget_use_package,$(1),pixman)
$(call gb_LinkTarget_use_external,$(1),freetype_headers)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,cairo) \
	-I$(call gb_UnpackedTarball_get_dir,cairo)/src \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_libraries,$(1),\
	cairo \
	$(if $(filter-out MACOSX WNT,$(OS)), \
		pixman-1 \
	) \
)

endef

endif # SYSTEM_CAIRO

ifeq ($(SYSTEM_FREETYPE),YES)

define gb_LinkTarget__use_freetype_headers
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FREETYPE_CFLAGS) \
)

endef

define gb_LinkTarget__use_freetype
$(call gb_LinkTarget_use_external,$(1),freetype_headers)
$(call gb_LinkTarget_add_libs,$(1),$(FREETYPE_LIBS))

endef

else # ! SYSTEM_FREETYPE

define gb_LinkTarget__use_freetype_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,freetype)/include \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_freetype
$(call gb_LinkTarget_use_external,$(1),freetype_headers)
$(call gb_LinkTarget_use_static_libraries,$(1),freetype)

endef

endif # SYSTEM_FREETYPE

ifeq ($(SYSTEM_FONTCONFIG),YES)

define gb_LinkTarget__use_fontconfig
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(FONTCONFIG_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(FONTCONFIG_LIBS))

endef

else # ! SYSTEM_FONTCONFIG

define gb_LinkTarget__use_fontconfig
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,fontconfig) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_use_static_libraries,$(1),fontconfig)

endef

endif # SYSTEM_FONTCONFIG

ifeq ($(SYSTEM_GRAPHITE),YES)

define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GRAPHITE_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(GRAPHITE_LIBS))

endef

else ifeq ($(SYSTEM_GRAPHITE),NO)

define gb_LinkTarget__use_graphite
$(call gb_LinkTarget_use_unpacked,$(1),graphite)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,graphite/include) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
    graphite \
)

endef

else # DISABLED GRAPHITE

gb_LinkTarget__use_graphite :=

endif # SYSTEM_GRAPHITE

ifeq ($(SYSTEM_ICU),YES)

gb_LinkTarget__use_icu_headers:=
gb_ExternalProject__use_icu:=

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_add_libs,$(1),-licudata)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_add_libs,$(1),-licui18n)

endef
define gb_LinkTarget__use_icutu
$(call gb_LinkTarget_add_libs,$(1),-licutu)

endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_add_libs,$(1),-licuuc)

endef

else # !SYSTEM_ICU

ifeq ($(OS)$(DISABLE_DYNLOADING),ANDROID)
gb_ICU_suffix:=lo
else
gb_ICU_suffix:=
endif

define gb_LinkTarget__use_icu_headers
$(call gb_LinkTarget_use_unpacked,$(1),icu)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,icu)/source \
	-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
	-I$(call gb_UnpackedTarball_get_dir,icu)/source/common \
	$$(INCLUDE) \
)

endef

define gb_ExternalProject__use_icu
$(call gb_ExternalProject_use_package,$(1),icu)

endef

# icudata and icui18n is called icudt and icuin when built with MSVC :-/
ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icudt \
	icuin \
))

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icudt \
)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icuin \
)

endef
else
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icudata$(gb_ICU_suffix) \
	icui18n$(gb_ICU_suffix) \
))

define gb_LinkTarget__use_icudata
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icudata$(gb_ICU_suffix) \
)

endef
define gb_LinkTarget__use_icui18n
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icui18n$(gb_ICU_suffix) \
)

endef
endif

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	icutu$(gb_ICU_suffix) \
	icuuc$(gb_ICU_suffix) \
))

define gb_LinkTarget__use_icutu
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icutu$(gb_ICU_suffix) \
)

endef
define gb_LinkTarget__use_icuuc
$(call gb_LinkTarget_use_package,$(1),icu)
$(call gb_LinkTarget_use_libraries,$(1),\
	icuuc$(gb_ICU_suffix) \
)

endef

endif # SYSTEM_ICU

ifeq ($(ENABLE_HARFBUZZ),TRUE)
ifeq ($(SYSTEM_HARFBUZZ),YES)

define gb_LinkTarget__use_harfbuzz
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(HARFBUZZ_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(HARFBUZZ_LIBS))

endef

else # SYSTEM_HARFBUZZ != YES

define gb_LinkTarget__use_harfbuzz
$(call gb_LinkTarget_set_include,$(1),\
	$(HARFBUZZ_CFLAGS) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),$(HARFBUZZ_LIBS))
$(call gb_LinkTarget_use_external_project,$(1),harfbuzz)

endef

endif # SYSTEM_HARFBUZZ
else # ENABLE_HARFBUZZ != YES

gb_LinkTarget__use_harfbuzz :=

endif # ENABLE_HARFBUZZ

ifeq ($(DISABLE_OPENSSL),YES)

gb_ExternalProject__use_openssl:=
gb_LinkTarget__use_openssl_headers:=
gb_LinkTarget__use_openssl:=

else # !DISABLE_OPENSSL

ifeq ($(SYSTEM_OPENSSL),YES)

gb_LinkTarget__use_openssl_headers:=
gb_ExternalProject__use_openssl:=

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
endif

define gb_ExternalProject__use_openssl
$(call gb_ExternalProject_use_package,$(1),openssl)

endef

define gb_LinkTarget__use_openssl_headers
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,openssl)/include \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_openssl
$(call gb_LinkTarget_use_package,$(1),openssl)
ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_libraries,$(1),\
	crypto \
	ssl \
)
else
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,openssl)/libssl.a \
	$(call gb_UnpackedTarball_get_dir,openssl)/libcrypto.a \
)
$(call gb_LinkTarget_use_external_project,$(1),openssl)
ifeq ($(OS),SOLARIS)
$(call gb_LinkTarget_add_libs,$(1),\
	-lnsl \
	-lsocket \
)
endif
endif

endef

endif # SYSTEM_OPENSSL
endif # DISABLE_OPENSSL


ifeq ($(DISABLE_OPENSSL),YES)

define gb_LinkTarget__use_gnutls
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(GNUTLS_CFLAGS) \
)

$(call gb_LinkTarget_add_defs,$(1),\
    -DDISABLE_OPENSSL \
)

$(call gb_LinkTarget_add_libs,$(1),$(GNUTLS_LIBS))

endef

define gb_LinkTarget__use_libgcrypt
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LIBGCRYPT_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),$(LIBGCRYPT_LIBS))

endef

else # !DISABLE_OPENSSL

gb_LinkTarget__use_gnutls:=
gb_LinkTarget__use_libgcrypt:=

endif # DISABLE_OPENSSL


ifeq ($(SYSTEM_CDR),YES)

define gb_LinkTarget__use_cdr
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(CDR_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(CDR_LIBS))

endef

else # !SYSTEM_CDR

define gb_LinkTarget__use_cdr
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libcdr)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libcdr)/src/lib/.libs/libcdr-0.0$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libcdr)
endef

endif # SYSTEM_CDR


ifeq ($(SYSTEM_ODFGEN),YES)

define gb_LinkTarget__use_odfgen
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(ODFGEN_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(ODFGEN_LIBS))

endef

else # !SYSTEM_ODFGEN

define gb_LinkTarget__use_odfgen
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libodfgen)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libodfgen)/src/.libs/libodfgen-0.0$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libodfgen)

endef

endif # SYSTEM_ODFGEN


ifeq ($(SYSTEM_MSPUB),YES)

define gb_LinkTarget__use_mspub
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(MSPUB_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MSPUB_LIBS))

endef

else # !SYSTEM_MSPUB

define gb_LinkTarget__use_mspub
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libmspub)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libmspub)/src/lib/.libs/libmspub-0.0$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libmspub)

endef

endif # SYSTEM_MSPUB


ifeq ($(SYSTEM_VISIO),YES)

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(VISIO_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(VISIO_LIBS))

endef

else # !SYSTEM_VISIO

define gb_LinkTarget__use_visio
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libvisio)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libvisio)/src/lib/.libs/libvisio-0.0$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libvisio)

endef

endif # SYSTEM_VISIO


ifeq ($(SYSTEM_WPD),YES)

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPD_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPD_LIBS))

endef
gb_ExternalProject__use_wpd :=

else # !SYSTEM_WPD

define gb_LinkTarget__use_wpd
$(call gb_LinkTarget_set_include,$(1),\
	$(WPD_CFLAGS) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libwpd)/src/lib/.libs/libwpd-0.9$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libwpd)

endef
define gb_ExternalProject__use_wpd
$(call gb_ExternalProject_use_external_project,$(1),libwpd)

endef

endif # SYSTEM_WPD


ifeq ($(SYSTEM_WPG),YES)

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPG_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPG_LIBS))

endef
gb_ExternalProject__use_wpg :=

else # !SYSTEM_WPG

define gb_LinkTarget__use_wpg
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libwpg)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libwpg)/src/lib/.libs/libwpg-0.2$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libwpg)

endef
define gb_ExternalProject__use_wpg
$(call gb_ExternalProject_use_external_project,$(1),libwpg)

endef

endif # SYSTEM_WPG


ifeq ($(SYSTEM_WPS),YES)

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(WPS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(WPS_LIBS))

endef
gb_ExternalProject__use_wps :=

else # !SYSTEM_WPS

define gb_LinkTarget__use_wps
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libwps)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libwps)/src/lib/.libs/libwps-0.2$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libwps)

endef
define gb_ExternalProject__use_wps
$(call gb_ExternalProject_use_external_project,$(1),libwps)

endef

endif # SYSTEM_WPS


ifeq ($(SYSTEM_MWAW),YES)

define gb_LinkTarget__use_mwaw
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(MWAW_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(MWAW_LIBS))

endef

else # !SYSTEM_MWAW

define gb_LinkTarget__use_mwaw
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,libmwaw)/inc \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,libmwaw)/src/lib/.libs/libmwaw-0.1$(gb_StaticLibrary_PLAINEXT) \
)
$(call gb_LinkTarget_use_external_project,$(1),libmwaw)

endef

endif # SYSTEM_MWAW


ifeq ($(SYSTEM_LCMS2),YES)

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(LCMS2_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(LCMS2_LIBS))

endef

gb_ExternalProject__use_lcms2 :=

else # !SYSTEM_LCMS2

define gb_ExternalProject__use_lcms2
$(call gb_ExternalProject_use_package,$(1),lcms2)

endef

ifeq ($(OS),ANDROID)

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_use_package,$(1),lcms2)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,lcms2/include) \
	$$(INCLUDE) \
)

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lcms2 \
))

define gb_LinkTarget__use_lcms2
$(call gb_LinkTarget_use_package,$(1),lcms2)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,lcms2/include) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_libraries,$(1),\
	lcms2 \
)

endef

endif # ANDROID
endif # SYSTEM_LCMS2


ifeq ($(SYSTEM_LPSOLVE),YES)

define gb_LinkTarget__use_lpsolve
$(call gb_LinkTarget_add_libs,$(1),-llpsolve55)
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_LPSOLVE \
)

endef

else # !SYSTEM_LPSOLVE

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	lpsolve55 \
))

define gb_LinkTarget__use_lpsolve
$(call gb_LinkTarget_use_unpacked,$(1),lpsolve)
$(call gb_LinkTarget_use_libraries,$(1),\
	lpsolve55 \
)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,lpsolve) \
	$$(INCLUDE) \
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

ifeq ($(ENABLE_AVAHI),TRUE)

define gb_LinkTarget__use_avahi
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(AVAHI_CFLAGS) \
)

$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_AVAHI \
)

$(call gb_LinkTarget_add_libs,$(1),$(AVAHI_LIBS))

endef

else # ENABLE_AVAHI

define gb_LinkTarget__use_AVAHI

endef

endif # ENABLE_AVAHI


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

ifeq ($(ENABLE_CUPS),TRUE)

define gb_LinkTarget__use_cups
$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_CUPS \
)

$(call gb_LinkTarget_add_libs,$(1),\
	-lcups \
)

endef

else # ENABLE_CUPS

define gb_LinkTarget__use_cups

endef

endif # ENABLE_DBUS

ifeq ($(ENABLE_DBUS),TRUE)

define gb_LinkTarget__use_dbus
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(DBUS_CFLAGS) \
)

$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_DBUS \
)

ifeq ($(ENABLE_PACKAGEKIT),TRUE)
$(call gb_LinkTarget_add_defs,$(1),\
    -DENABLE_PACKAGEKIT \
)
endif # ENABLE_PACKAGEKIT

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

define gb_LinkTarget__use_croco
$(error gb_LinkTarget__use_croco should not be used any more)
endef

define gb_LinkTarget__use_pango
$(error gb_LinkTarget__use_pango should not be used any more)
endef

define gb_LinkTarget__use_gsf
$(error gb_LinkTarget__use_gsf should not be used any more)
endef

define gb_LinkTarget__use_pixbuf
$(error gb_LinkTarget__use_pixbuf should not be used any more)
endef

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

define gb_LinkTarget__use_png
$(call gb_LinkTarget_set_include,$(1),\
	$(LIBPNG_CFLAGS) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_static_libraries,$(1),\
	png \
)
$(call gb_LinkTarget__use_zlib,$(1))

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
$(call gb_LinkTarget_use_package,$(1),curl)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,curl/include) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_libraries,$(1),\
	curl \
)

endef

endif # SYSTEM_CURL

ifeq ($(ENABLE_VALGRIND),TRUE)

define gb_LinkTarget__use_valgrind
$(call gb_LinkTarget_add_defs,$(1),\
	-DHAVE_VALGRIND_HEADERS \
)

$(call gb_LinkTarget_set_include,$(1),\
    $$(INCLUDE) \
    $(VALGRIND_CFLAGS) \
)

endef

else # !ENABLE_VALGRIND

define gb_LinkTarget__use_valgrind

endef

endif # ENABLE_VALGRIND

ifeq ($(SYSTEM_POPPLER),YES)

define gb_LinkTarget__use_poppler
$(call gb_LinkTarget_set_include,$(1),\
	$(POPPLER_CFLAGS) \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(POPPLER_LIBS) \
)

endef

else # !SYSTEM_POPPLER

define gb_LinkTarget__use_poppler
$(call gb_LinkTarget_use_external_project,$(1),poppler)

$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,poppler) \
	-I$(call gb_UnpackedTarball_get_dir,poppler)/poppler \
	-I$(call gb_UnpackedTarball_get_dir,poppler)/goo \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,poppler)/fofi/.libs/libfofi$(gb_StaticLibrary_PLAINEXT) \
	$(call gb_UnpackedTarball_get_dir,poppler)/goo/.libs/libgoo$(gb_StaticLibrary_PLAINEXT) \
	$(call gb_UnpackedTarball_get_dir,poppler)/poppler/.libs/libpoppler$(gb_StaticLibrary_PLAINEXT) \
)

ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_add_libs,$(1),\
	-lobjc \
)
else ifeq ($(OS),WNT)
$(call gb_LinkTarget_use_system_win32_libs,$(1),\
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
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,clucene)/src/core \
	-I$(call gb_UnpackedTarball_get_dir,clucene)/src/shared \
	-I$(call gb_UnpackedTarball_get_dir,clucene)/src/contribs-lib \
	$$(INCLUDE) \
)

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

gb_ExternalProject__use_openldap :=

else ifeq ($(SYSTEM_OPENLDAP),NO)

define gb_LinkTarget__use_openldap
$(call gb_LinkTarget_use_unpacked,$(1),openldap)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,openldap/include) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_use_external_project,$(1),openldap)
$(call gb_LinkTarget_add_libs,$(1), \
	$(call gb_UnpackedTarball_get_dir,openldap)/libraries/libldap/.libs/libldap.a \
	$(call gb_UnpackedTarball_get_dir,openldap)/libraries/liblber/.libs/liblber.a \
)

endef

define gb_ExternalProject__use_openldap
$(call gb_ExternalProject_use_external_project,$(1),openldap)

endef

else # no openldap

gb_LinkTarget__use_openldap :=
gb_ExternalProject__use_openldap :=

endif # SYSTEM_OPENLDAP


ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)

ifeq ($(SYSTEM_FIREBIRD),YES)

define gb_LinkTarget__use_libfbembed
$(call gb_LinkTarget_set_include,$(1),\
	$(FIREBIRD_CFLAGS) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),$(FIREBIRD_LIBS))

endef

# gb_LinkTarget__use_atomic_ops :=
# gb_LinkTarget__use_tommath :=

else # !SYSTEM_FIREBIRD

#$(call gb_LinkTarget__use_libatomic_ops,$(1))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    fbembed \
))

define gb_LinkTarget__use_libfbembed
$(call gb_LinkTarget_use_package,$(1),firebird)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,firebird)/gen/firebird/include \
)
$(call gb_LinkTarget_use_libraries,$(1),\
    fbembed \
)

endef

# define gb_LinkTarget__use_tommath
# $(call gb_LinkTarget_set_include,$(1),\
# 	$(TOMMATH_CFLAGS) \
# 	$$(INCLUDE) \
# )

# $(call gb_LinkTarget_add_libs,$(1),$(TOMMATH_LIBS))

# endef

endif # SYSTEM_FIREBIRD

else # !ENABLE_FIREBIRD_SDBC

gb_LinkTarget__use_firebird :=
# gb_LinkTarget__use_atomic_ops :=
# gb_LinkTarget__use_tommath :=

endif # ENABLE_FIREBIRD_SDBC


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

$(call gb_LinkTarget_use_external_project,$(1),postgresql)

$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,postgresql)/src/include \
	-I$(call gb_UnpackedTarball_get_dir,postgresql)/src/interfaces/libpq \
	$$(INCLUDE) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,postgresql)/src/interfaces/libpq/libpq$(gb_StaticLibrary_PLAINEXT) \
)

ifeq ($(OS)$(COM),WNTMSC)
$(call gb_LinkTarget_use_external,$(1),openssl)

$(call gb_LinkTarget_use_system_win32_libs,$(1),\
	secur32 \
	ws2_32 \
)

endif

endef

endif # SYSTEM_POSTGRESQL

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
define gb_LinkTarget__use_mozilla

$(call gb_LinkTarget_add_defs,$(1),\
	-DMOZILLA_INTERNAL_API \
)

$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/mozilla \
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

endef

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE,\
	xpcom \
	xpcom_core \
))

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
# extra python_headers external because pyuno wrapper must not link python
ifeq ($(SYSTEM_PYTHON),YES)

define gb_LinkTarget__use_python_headers
$(call gb_LinkTarget_add_defs,$(1),\
	$(filter-out -I%,$(PYTHON_CFLAGS)) \
)

$(call gb_LinkTarget_set_include,$(1),\
	$(filter -I%,$(PYTHON_CFLAGS)) \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_python
$(call gb_LinkTarget__use_python_headers,$(1))

$(call gb_LinkTarget_add_libs,$(1),\
	$(PYTHON_LIBS) \
)

endef

else # !SYSTEM_PYTHON

# depend on external project because on MACOSX the Package is disabled...
define gb_LinkTarget__use_python_headers
$(call gb_LinkTarget_use_external_project,$(1),python3)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,python3) \
	-I$(call gb_UnpackedTarball_get_dir,python3)/PC \
	-I$(call gb_UnpackedTarball_get_dir,python3)/Include \
	$$(INCLUDE) \
)

endef

define gb_LinkTarget__use_python
$(call gb_LinkTarget__use_python_headers,$(1))

ifeq ($(OS),WNT)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,python3)/PCbuild$(if $(filter X86_64,$(CPUNAME)),/amd64)/python$(PYTHON_VERSION_MAJOR)$(PYTHON_VERSION_MINOR)$(if $(MSVC_USE_DEBUG_RUNTIME),_d).lib \
)
else ifeq ($(OS),MACOSX)
$(call gb_LinkTarget_add_libs,$(1),\
	-F$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO -framework LibreOfficePython \
)
else
$(call gb_LinkTarget_add_libs,$(1),\
	-L$(call gb_UnpackedTarball_get_dir,python3) \
	-lpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m \
)
endif

endef

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
	python$(PYTHON_VERSION_MAJOR) \
	python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m \
))

endif # SYSTEM_PYTHON

# ORCUS
ifeq ($(ENABLE_ORCUS),TRUE)

ifeq ($(SYSTEM_LIBORCUS),YES)

define gb_LinkTarget__use_orcus
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
    $(ORCUS_CFLAGS) \
)
$(call gb_LinkTarget_add_libs,$(1),$(ORCUS_LIBS))
endef

define gb_LinkTarget__use_orcus-parser

endef

else # !SYSTEM_LIBORCUS

define gb_LinkTarget__use_orcus
$(call gb_LinkTarget_use_external_project,$(1),liborcus)
$(call gb_LinkTarget_set_include,$(1),\
	-I$(call gb_UnpackedTarball_get_dir,liborcus/include) \
	$$(INCLUDE) \
)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,liborcus)/src/liborcus/.libs/liborcus-0.6$(gb_StaticLibrary_PLAINEXT) \
)

$(if $(filter YES,$(SYSTEM_BOOST)), \
    $(call gb_LinkTarget_add_ldflags,$(1),$(BOOST_LDFLAGS)) \
    $(call gb_LinkTarget_add_libs,$(1),$(BOOST_SYSTEM_LIB)) \
)

endef

define gb_LinkTarget__use_orcus-parser
$(call gb_LinkTarget_use_external_project,$(1),liborcus)
$(call gb_LinkTarget_add_libs,$(1),\
	$(call gb_UnpackedTarball_get_dir,liborcus)/src/parser/.libs/liborcus-parser-0.6$(gb_StaticLibrary_PLAINEXT) \
)

endef

endif # SYSTEM_LIBORCUS

else # ENABLE_ORCUS != TRUE

gb_LinkTarget__use_orcus :=
gb_LinkTarget__use_orcus-parser :=

endif

### X11 stuff ###

ifeq ($(GUIBASE),unx)

ifeq ($(XRANDR_DLOPEN),FALSE)

define gb_LinkTarget__use_Xrandr
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(XRANDR_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(XRANDR_LIBS) \
)
endef

else # XRANDR_DLOPEN

define gb_LinkTarget__use_Xrandr
$(call gb_LinkTarget_add_defs,$(1),\
	-DXRANDR_DLOPEN \
)
endef

endif # XRANDR_DLOPEN

define gb_LinkTarget__use_Xrender
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	$(XRENDER_CFLAGS) \
)

$(call gb_LinkTarget_add_libs,$(1),\
	$(XRENDER_LIBS) \
)
endef

endif # GUIBASE=unx


gb_ExternalProject__use_nss3:=

ifeq (,$(filter DESKTOP,$(BUILD_TYPE)))

gb_LinkTarget__use_nss3:=

else

ifeq ($(SYSTEM_NSS),YES)

define gb_LinkTarget__use_nss3
$(call gb_LinkTarget_add_defs,$(1),\
	-DSYSTEM_NSS \
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

define gb_LinkTarget__use_ssl3
$(call gb_LinkTarget__use_nss3,$(1))

endef

else

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO,\
    nspr4 \
    nss3 \
    plc4 \
    smime3 \
	ssl3 \
))

define gb_LinkTarget__use_nss3
$(call gb_LinkTarget_use_package,$(1),nss)
$(call gb_LinkTarget_set_include,$(1),\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/public/nss \
	-I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/out/include \
)

$(call gb_LinkTarget_use_libraries,$(1),\
	nspr4 \
	nss3 \
	smime3 \
)

endef

define gb_LinkTarget__use_plc4
$(call gb_LinkTarget_use_package,$(1),nss)
$(call gb_LinkTarget_use_libraries,$(1),\
    plc4 \
)

endef

define gb_LinkTarget__use_ssl3
$(call gb_LinkTarget_use_package,$(1),nss)
$(call gb_LinkTarget_use_libraries,$(1),\
	ssl3 \
)

endef

define gb_ExternalProject__use_nss3
$(call gb_ExternalProject_use_package,$(1),nss)
$(call gb_ExternalProject_use_libraries,$(1),\
	nspr4 \
	nss3 \
	smime3 \
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
gb_ExternalProject__use_commons-codec :=

define gb_Jar__use_commons-httpclient
$(call gb_Jar_use_system_jar,$(1),$(COMMONS_HTTPCLIENT_JAR))
endef
gb_ExternalProject__use_commons-httpclient :=

define gb_Jar__use_commons-lang
$(call gb_Jar_usadd_linked_libse_system_jar,$(1),$(COMMONS_LANG_JAR))
endef
gb_ExternalProject__use_commons-lang :=

define gb_Jar__use_commons-logging
$(call gb_Jar_use_system_jar,$(1),$(COMMONS_LOGGING_JAR))
endef
gb_ExternalProject__use_commons-logging :=

else # !SYSTEM_APACHE_COMMONS

$(eval $(call gb_Helper_register_jars,OXT,\
	commons-codec-1.6 \
	commons-httpclient-3.1 \
	commons-lang-2.4 \
	commons-logging-1.1.1 \
))

define gb_Jar__use_commons-codec
$(call gb_Jar_use_jar,$(1),commons-codec-1.6)
endef
define gb_ExternalProject__use_commons-codec
$(call gb_ExternalProject_use_external_project,$(1),apache_commons_codec)
endef

define gb_Jar__use_commons-httpclient
$(call gb_Jar_use_jar,$(1),commons-httpclient-3.1)
endef
define gb_ExternalProject__use_commons-httpclient
$(call gb_ExternalProject_use_external_project,$(1),apache_commons_httpclient)
endef

define gb_Jar__use_commons-lang
$(call gb_Jar_use_jar,$(1),commons-lang-2.4)
endef
define gb_ExternalProject__use_commons-lang
$(call gb_ExternalProject_use_external_project,$(1),apache_commons_lang)
endef

define gb_Jar__use_commons-logging
$(call gb_Jar_use_jar,$(1),commons-logging-1.1.1)
endef
define gb_ExternalProject__use_commons-logging
$(call gb_ExternalProject_use_external_project,$(1),apache_commons_logging)
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


# Executables

# FIXME: the library target should be for build too
define gb_Executable__register_bestreversemap
$(call gb_Executable_add_runtime_dependencies,bestreversemap,\
	$(if $(filter-out ANDROID,$(OS)),$(call gb_Library_get_target,sal_textenc)) \
)
endef

# TODO depending on the whole URE might be overkill, but I do not have a
# Windows machine to debug it...
# FIXME: the library target should be for build too
define gb_Executable__register_climaker
$(call gb_Executable_add_runtime_dependencies,climaker,\
	$(call gb_Library_get_target,$(gb_CPPU_ENV)_uno) \
	$(call gb_Package_get_target_for_build,cppuhelper_unorc) \
	$(call gb_Rdb_get_target_for_build,ure/services) \
	$(INSTROOT)/$(LIBO_URE_SHARE_FOLDER)/misc/services.rdb \
	$(call gb_UnoApi_get_target,udkapi) \
)
endef

# Better duplication with gb_Gallery__UNO_COMPONENTS than nothing.
# This is used to determine what we need for 'build' platform.
define gb_Executable__register_gengal
$(call gb_Executable_add_runtime_dependencies,gengal,\
	$(foreach component, \
		comphelper/util/comphelp \
		configmgr/source/configmgr \
		fileaccess/source/fileacc \
		framework/util/fwk \
		i18npool/util/i18npool \
		package/source/xstor/xstor \
		package/util/package2 \
		sfx2/util/sfx \
		svx/util/svx \
		svx/util/svxcore \
		ucb/source/core/ucb1 \
		ucb/source/ucp/file/ucpfile1 \
		unoxml/source/service/unoxml \
	,$(call gb_ComponentTarget_get_target_for_build,$(component))) \
	$(call gb_AllLangResTarget_get_target,ofa) \
	$(call gb_Configuration_get_target,registry) \
	$(call gb_Configuration_get_target,fcfg_langpack) \
	$(call gb_Library_get_target,$(gb_CPPU_ENV)_uno) \
	$(if $(filter-out MACOSX WNT,$(OS_FOR_BUILD)),$(if $(ENABLE_HEADLESS),, \
		$(call gb_Library_get_target,vclplug_svp) \
	)) \
	$(call gb_Package_get_target_for_build,cppuhelper_unorc) \
	$(call gb_Rdb_get_target_for_build,ure/services) \
	$(INSTROOT)/$(LIBO_URE_SHARE_FOLDER)/misc/services.rdb \
	$(call gb_UnoApi_get_target,offapi) \
	$(call gb_UnoApi_get_target,udkapi) \
)
endef

ifneq ($(SYSTEM_ICU),YES)

define gb_Executable__register_gendict
$(call gb_Executable_add_runtime_dependencies,gendict,\
	$(call gb_Package_get_target_for_build,icu) \
)
endef

endif

ifeq (,$(SYSTEM_UCPP))
define gb_Executable__register_idlc
$(call gb_Executable_add_runtime_dependencies,idlc,\
	$(call gb_Executable_get_target_for_build,ucpp) \
)
endef
endif

define gb_Executable__register_localize
$(call gb_Executable_add_runtime_dependencies,localize,\
	$(foreach exec,cfgex helpex propex transex3 treex uiex ulfex xrmex,\
		$(call gb_Executable_get_runtime_dependencies,$(exec)) \
	) \
)
endef

# FIXME ure/services.rdb needs cleanup
# The dependencies on ure/services.rdb and udkapi.rdb are implicitly required
# due to the settings for URE_SERVICES and URE_TYPES in cppuhelper/source/unorc:
# FIXME: the library target should be for build too
define gb_Executable__register_saxparser
$(call gb_Executable_add_runtime_dependencies,saxparser,\
	$(call gb_Library_get_target,$(gb_CPPU_ENV)_uno) \
	$(call gb_Package_get_target_for_build,cppuhelper_unorc) \
	$(call gb_Rdb_get_target_for_build,saxparser) \
	$(call gb_Rdb_get_target_for_build,ure/services) \
	$(INSTROOT)/$(LIBO_URE_SHARE_FOLDER)/misc/services.rdb \
	$(call gb_UnoApi_get_target,udkapi) \
)
endef

# NOTE: the dependencies on ure/services.rdb and udkapi.rdb are implicitly
# required due to the settings for URE_SERVICES and URE_TYPES in
# cppuhelper/source/unorc
# FIXME: the library target should be for build too
define gb_Executable__register_uno
$(call gb_Executable_add_runtime_dependencies,uno,\
	$(call gb_Library_get_target,$(gb_CPPU_ENV)_uno) \
	$(call gb_Package_get_target_for_build,cppuhelper_unorc) \
	$(call gb_Rdb_get_target_for_build,ure/services) \
	$(INSTROOT)/$(LIBO_URE_SHARE_FOLDER)/misc/services.rdb \
	$(call gb_UnoApi_get_target,udkapi) \
)
endef


# External executables

ifneq ($(SYSTEM_LIBXML_FOR_BUILD),YES)

gb_ExternalExecutable__register_xmllint :=

else # ! SYSTEM_LIBXML_FOR_BUILD

define gb_ExternalExecutable__register_xmllint
$(call gb_ExternalExecutable_set_internal,xmllint,$(OUTDIR_FOR_BUILD)/bin/xmllint$(gb_Executable_EXT_for_build))

endef

endif # SYSTEM_LIBXML_FOR_BUILD

ifeq ($(SYSTEM_LIBXSLT_FOR_BUILD),YES)

gb_ExternalExecutable__register_xsltproc :=

else # ! SYSTEM_LIBXSLT_FOR_BUILD

define gb_ExternalExecutable__register_xsltproc
$(call gb_ExternalExecutable_set_internal,xsltproc,$(OUTDIR_FOR_BUILD)/bin/xsltproc$(gb_Executable_EXT_for_build))
$(call gb_ExternalExecutable_add_dependencies,xsltproc,$(call gb_Package_get_target,xslt))

endef

endif # SYSTEM_LIBXSLT_FOR_BUILD

ifeq (,$(PYTHON_FOR_BUILD))

define gb_ExternalExecutable__register_python
ifeq ($(OS),MACOSX)

# use set_external, to avoid having the command added as prerequisite for the
# targets that make use of it. (Otherwise make will choke as it doesn't have a
# matching rule to build that specific file)
$(call gb_ExternalExecutable_set_external,python,$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO/LibreOfficePython.framework/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR))
# the Zip ensures that internal python has been built (cannot use the Package
# target, as that is not used on Mac)
$(call gb_ExternalExecutable_add_dependencies,python,$(call gb_GeneratedPackage_get_target_for_build,python3))

else

$(call gb_ExternalExecutable_set_internal,python,$(INSTROOT)/$(LIBO_BIN_FOLDER)/$(if $(filter WNT,$(OS)),python-core-$(PYTHON_VERSION)/bin/python.exe,python.bin))
$(call gb_ExternalExecutable_set_precommand,python,$(gb_Python_PRECOMMAND))
$(call gb_ExternalExecutable_add_dependencies,python,$(call gb_Package_get_target_for_build,python3))

endif

endef

else

define gb_ExternalExecutable__register_python
$(call gb_ExternalExecutable_set_external,python,$(PYTHON_FOR_BUILD))

endef

endif # PYTHON_FOR_BUILD

ifneq ($(SYSTEM_GENBRK),)

define gb_ExternalExecutable__register_genbrk
$(call gb_ExternalExecutable_set_external,genbrk,$(SYSTEM_GENBRK))

endef

else # ! SYSTEM_GENBRK

define gb_ExternalExecutable__register_genbrk
$(call gb_ExternalExecutable_set_internal,genbrk,$(OUTDIR_FOR_BUILD)/bin/genbrk$(gb_Executable_EXT_for_build))
$(call gb_ExternalExecutable_add_dependencies,genbrk,\
	$(call gb_Package_get_target_for_build,icu) \
)

endef

endif

ifneq ($(SYSTEM_GENCCODE),)

define gb_ExternalExecutable__register_genccode
$(call gb_ExternalExecutable_set_external,genccode,$(SYSTEM_GENCCODE))

endef

else # ! SYSTEM_GENCCODE

define gb_ExternalExecutable__register_genccode
$(call gb_ExternalExecutable_set_internal,genccode,$(OUTDIR_FOR_BUILD)/bin/genccode$(gb_Executable_EXT_for_build))
$(call gb_ExternalExecutable_add_dependencies,genccode,\
	$(call gb_Package_get_target_for_build,icu) \
)

endef

endif

ifneq ($(SYSTEM_GENCMN),)

define gb_ExternalExecutable__register_gencmn
$(call gb_ExternalExecutable_set_external,gencmn,$(SYSTEM_GENCMN))

endef

else # ! SYSTEM_GENCMN

define gb_ExternalExecutable__register_gencmn
$(call gb_ExternalExecutable_set_internal,gencmn,$(OUTDIR_FOR_BUILD)/bin/gencmn$(gb_Executable_EXT_for_build))
$(call gb_ExternalExecutable_add_dependencies,gencmn,\
	$(call gb_Package_get_target_for_build,icu) \
)

endef

endif

# vim: set noet sw=4 ts=4:
