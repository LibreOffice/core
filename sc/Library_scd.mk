# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scd))

$(eval $(call gb_Library_set_componentfile,scd,sc/util/scd,services))

$(eval $(call gb_Library_set_include,scd,\
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,scd,boost_headers))

$(eval $(call gb_Library_use_common_precompiled_header,scd))

$(eval $(call gb_Library_use_api,scd,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_use_libraries,scd,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	sot \
	tl \
	utl \
))

$(eval $(call gb_Library_add_exception_objects,scd,\
	sc/source/ui/unoobj/scdetect \
	sc/source/ui/unoobj/exceldetect \
))

# vim: set noet sw=4 ts=4:
