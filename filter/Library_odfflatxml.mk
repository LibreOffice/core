# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,odfflatxml))

$(eval $(call gb_Library_set_componentfile,odfflatxml,filter/source/odfflatxml/odfflatxml,services))

$(eval $(call gb_Library_use_sdk_api,odfflatxml))

$(eval $(call gb_Library_set_include,odfflatxml,\
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Library_use_libraries,odfflatxml,\
	xo \
	tl \
	ucbhelper \
	comphelper \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,odfflatxml,\
	filter/source/odfflatxml/OdfFlatXml \
))

# vim: set noet sw=4 ts=4:
