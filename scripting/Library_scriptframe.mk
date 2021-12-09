# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scriptframe))

$(eval $(call gb_Library_set_componentfile,scriptframe,scripting/util/scriptframe,services))

$(eval $(call gb_Library_set_include,scriptframe,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,scriptframe,boost_headers))

$(eval $(call gb_Library_use_sdk_api,scriptframe))

$(eval $(call gb_Library_use_libraries,scriptframe,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	ucbhelper \
	utl \
))

$(eval $(call gb_Library_add_exception_objects,scriptframe,\
	scripting/source/provider/ActiveMSPList \
	scripting/source/provider/BrowseNodeFactoryImpl \
	scripting/source/provider/MasterScriptProvider \
	scripting/source/provider/MasterScriptProviderFactory \
	scripting/source/provider/ProviderCache \
	scripting/source/provider/URIHelper \
))

# vim: set noet sw=4 ts=4:
