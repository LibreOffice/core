# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,updatecheckui))

$(eval $(call gb_Library_set_include,updatecheckui,\
       -I$(SRCDIR)/extensions/inc \
       $$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,updatecheckui,extensions/source/update/ui/updchk,services))

$(eval $(call gb_Library_use_external,updatecheckui,boost_headers))

$(eval $(call gb_Library_use_sdk_api,updatecheckui))

$(eval $(call gb_Library_use_libraries,updatecheckui,\
	vcl \
	tl \
	utl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	i18nlangtag \
))

$(eval $(call gb_Library_add_exception_objects,updatecheckui,\
	extensions/source/update/ui/updatecheckui \
))

# vim:set noet sw=4 ts=4:
