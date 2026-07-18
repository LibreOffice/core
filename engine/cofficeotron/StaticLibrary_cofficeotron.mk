# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,cofficeotron))

$(eval $(call gb_StaticLibrary_set_include,cofficeotron,\
	$$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
))

$(eval $(call gb_StaticLibrary_use_sdk_api,cofficeotron))

$(eval $(call gb_StaticLibrary_use_externals,cofficeotron,\
	expat \
	rnv \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,cofficeotron,\
	cofficeotron/source/mcefilter \
	cofficeotron/source/odfsession \
	cofficeotron/source/ooxmlsession \
	cofficeotron/source/opcpackage \
	cofficeotron/source/report \
	cofficeotron/source/schemacache \
	cofficeotron/source/schemamap \
	cofficeotron/source/xmlutil \
	cofficeotron/source/zipfile \
))

# vim: set noet sw=4 ts=4:
