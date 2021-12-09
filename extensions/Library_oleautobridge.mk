# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,oleautobridge))

$(eval $(call gb_Library_set_componentfile,oleautobridge,extensions/source/ole/oleautobridge,services))

$(eval $(call gb_Library_use_api,oleautobridge,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_Library_set_include,oleautobridge,\
	$(foreach inc,$(ATL_INCLUDE),-I$(inc)) \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_ldflags,oleautobridge,\
	-LIBPATH:$(ATL_LIB) \
))

$(eval $(call gb_Library_use_external,oleautobridge,boost_headers))

$(eval $(call gb_Library_use_libraries,oleautobridge,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_use_system_win32_libs,oleautobridge,\
	advapi32 \
	ole32 \
	oleaut32 \
	uuid \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_libs,oleautobridge,\
	$(ATL_LIB)/atls.lib \
))
endif

$(eval $(call gb_Library_add_exception_objects,oleautobridge,\
	extensions/source/ole/jscriptclasses \
	extensions/source/ole/ole2uno \
	extensions/source/ole/oledll \
	extensions/source/ole/oleobjw \
	extensions/source/ole/olethread \
	extensions/source/ole/servprov \
	extensions/source/ole/servreg \
	extensions/source/ole/unoobjw \
	extensions/source/ole/unotypewrapper \
))

# vim:set noet sw=4 ts=4:
