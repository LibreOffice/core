# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,protocolhandler))

$(eval $(call gb_Library_set_componentfile,protocolhandler,scripting/source/protocolhandler/protocolhandler,services))

$(eval $(call gb_Library_set_include,protocolhandler,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,protocolhandler,boost_headers))

$(eval $(call gb_Library_use_sdk_api,protocolhandler))

$(eval $(call gb_Library_use_custom_headers,protocolhandler,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,protocolhandler,\
	comphelper \
	cppu \
	cppuhelper \
	fwk \
	sal \
	sfx \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,protocolhandler,\
	scripting/source/protocolhandler/scripthandler \
))

# vim: set noet sw=4 ts=4:
