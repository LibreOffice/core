# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xstor))

$(eval $(call gb_Library_set_componentfile,xstor,package/source/xstor/xstor))

$(eval $(call gb_Library_set_include,xstor,\
	$$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
))

$(eval $(call gb_Library_set_precompiled_header,xstor,$(SRCDIR)/package/inc/pch/precompiled_xstor))

$(eval $(call gb_Library_use_sdk_api,xstor))

$(eval $(call gb_Library_use_libraries,xstor,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
))

$(eval $(call gb_Library_add_exception_objects,xstor,\
	package/source/xstor/disposelistener \
	package/source/xstor/ocompinstream \
	package/source/xstor/ohierarchyholder \
	package/source/xstor/oseekinstream \
	package/source/xstor/owriteablestream \
	package/source/xstor/register \
	package/source/xstor/selfterminatefilestream \
	package/source/xstor/switchpersistencestream \
	package/source/xstor/xfactory \
	package/source/xstor/xstorage \
))

# vim: set noet sw=4 ts=4:
