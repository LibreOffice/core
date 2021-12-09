# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,basprov))

$(eval $(call gb_Library_set_componentfile,basprov,scripting/source/basprov/basprov,services))

$(eval $(call gb_Library_set_include,basprov,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,basprov,boost_headers))

$(eval $(call gb_Library_use_sdk_api,basprov))

$(eval $(call gb_Library_use_libraries,basprov,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sb \
	sfx \
	svl \
	tl \
	ucbhelper \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,basprov,\
	scripting/source/basprov/baslibnode \
	scripting/source/basprov/basmethnode \
	scripting/source/basprov/basmodnode \
	scripting/source/basprov/basprov \
	scripting/source/basprov/basscript \
))

# vim: set noet sw=4 ts=4:
