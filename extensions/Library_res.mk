# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,res))

$(eval $(call gb_Library_set_componentfile,res,extensions/source/resource/res))

$(eval $(call gb_Library_use_external,res,boost_headers))

$(eval $(call gb_Library_use_sdk_api,res))

$(eval $(call gb_Library_use_libraries,res,\
	vcl \
    i18nlangtag \
	tl \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,res,\
	extensions/source/resource/ResourceIndexAccess \
	extensions/source/resource/oooresourceloader \
	extensions/source/resource/resourceservices \
))

# vim:set noet sw=4 ts=4:
