# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,fpicker))

$(eval $(call gb_Library_set_componentfile,fpicker,fpicker/source/generic/fpicker))

$(eval $(call gb_Library_use_external,fpicker,boost_headers))

$(eval $(call gb_Library_use_sdk_api,fpicker))

$(eval $(call gb_Library_use_libraries,fpicker,\
	cppu \
	cppuhelper \
	sal \
	svl \
	svt \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,fpicker,\
	fpicker/source/generic/fpicker \
))

# vim: set noet sw=4 ts=4:
