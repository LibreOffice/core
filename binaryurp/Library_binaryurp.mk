# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,binaryurp))

$(eval $(call gb_Library_set_componentfile,binaryurp,binaryurp/source/binaryurp,ure/services))

$(eval $(call gb_Library_use_udk_api,binaryurp))

$(eval $(call gb_Library_use_external,binaryurp,boost_headers))

$(eval $(call gb_Library_use_libraries,binaryurp,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
))

$(eval $(call gb_Library_add_exception_objects,binaryurp,\
	binaryurp/source/binaryany \
	binaryurp/source/bridge \
	binaryurp/source/bridgefactory \
	binaryurp/source/currentcontext \
	binaryurp/source/incomingrequest \
	binaryurp/source/lessoperators \
	binaryurp/source/marshal \
	binaryurp/source/outgoingrequests \
	binaryurp/source/proxy \
	binaryurp/source/reader \
	binaryurp/source/unmarshal \
	binaryurp/source/writer \
))

# vim: set noet sw=4 ts=4:
