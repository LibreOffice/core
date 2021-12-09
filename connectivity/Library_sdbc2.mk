# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdbc2))

$(eval $(call gb_Library_set_componentfile,sdbc2,connectivity/source/manager/sdbc2,services))

$(eval $(call gb_Library_use_external,sdbc2,boost_headers))

$(eval $(call gb_Library_use_sdk_api,sdbc2))

$(eval $(call gb_Library_set_include,sdbc2,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
))

$(eval $(call gb_Library_use_libraries,sdbc2,\
	cppu \
	cppuhelper \
	comphelper \
	dbtools \
	utl \
	sal \
	tl \
))

$(eval $(call gb_Library_add_exception_objects,sdbc2,\
	connectivity/source/manager/mdrivermanager \
))

# vim: set noet sw=4 ts=4:
