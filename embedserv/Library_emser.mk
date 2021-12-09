# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,emser))

$(eval $(call gb_Library_set_componentfile,emser,embedserv/util/emser,services))

$(eval $(call gb_Library_set_include,emser,\
	-I$(SRCDIR)/embedserv/source/inc \
	-I$(ATL_INCLUDE) \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,emser,\
	-UNOMINMAX \
))

$(eval $(call gb_Library_use_external,emser,boost_headers))

$(eval $(call gb_Library_use_sdk_api,emser))

$(eval $(call gb_Library_use_libraries,emser,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,emser,\
	gdi32 \
	ole32 \
	oleaut32 \
	uuid \
))

$(eval $(call gb_Library_add_libs,emser,\
	$(ATL_LIB)/atls.lib \
))

$(eval $(call gb_Library_add_ldflags,emser,\
	-LIBPATH:$(ATL_LIB) \
))

$(eval $(call gb_Library_add_exception_objects,emser,\
	embedserv/source/embed/docholder \
	embedserv/source/embed/ed_idataobj \
	embedserv/source/embed/ed_iinplace \
	embedserv/source/embed/ed_ioleobject \
	embedserv/source/embed/ed_ipersiststr \
	embedserv/source/embed/esdll \
	embedserv/source/embed/guid \
	embedserv/source/embed/iipaobj \
	embedserv/source/embed/intercept \
	embedserv/source/embed/servprov \
	embedserv/source/embed/syswinwrapper \
	embedserv/source/embed/tracker \
))

# vim: set noet sw=4 ts=4:
