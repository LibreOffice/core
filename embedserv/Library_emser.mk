# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Library_Library,emser))

$(eval $(call gb_Library_set_componentfile,emser,embedserv/util/emser))

$(eval $(call gb_Library_set_include,emser,\
	-I$(SRCDIR)/embedserv/source/inc \
	-I$(ATL_INCLUDE) \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,emser,\
	-UNOMINMAX \
))

$(eval $(call gb_Library_use_sdk_api,emser))

$(eval $(call gb_Library_use_libraries,emser,\
	comphelper \
	cppu \
	cppuhelper \
	gdi32 \
	ole32 \
	oleaut32 \
	sal \
	uuid \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,emser))

ifeq ($(USE_DEBUG_RUNTIME),)
$(eval $(call gb_Library_add_libs,emser,\
	$(ATL_LIB)/atls.lib \
))
else
$(eval $(call gb_Library_add_libs,emser,\
	$(ATL_LIB)/atlsd.lib \
))
endif

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
	embedserv/source/embed/register \
	embedserv/source/embed/servprov \
	embedserv/source/embed/syswinwrapper \
	embedserv/source/embed/tracker \
))

# vim: set noet sw=4 ts=4:
