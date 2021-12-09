# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,vbaevents))

$(eval $(call gb_Library_set_componentfile,vbaevents,scripting/source/vbaevents/vbaevents,services))

$(eval $(call gb_Library_set_include,vbaevents,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,vbaevents,boost_headers))

$(eval $(call gb_Library_use_api,vbaevents,\
	offapi \
	oovbaapi \
	udkapi \
))

$(eval $(call gb_Library_use_libraries,vbaevents,\
	comphelper \
	cppu \
	cppuhelper \
	msfilter \
	sal \
	sfx \
	sb \
	tl \
))

$(eval $(call gb_Library_add_exception_objects,vbaevents,\
	scripting/source/vbaevents/eventhelper \
))

# vim: set noet sw=4 ts=4:
