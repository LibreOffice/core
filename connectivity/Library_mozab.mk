# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mozab2))

$(eval $(call gb_Library_set_componentfile,mozab2,connectivity/source/drivers/mozab/mozab))

$(eval $(call gb_Library_set_include,mozab2,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_external,mozab2,boost_headers))

$(eval $(call gb_Library_use_sdk_api,mozab2))

$(eval $(call gb_Library_use_libraries,mozab2,\
	cppu \
	cppuhelper \
	dbtools \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,mozab2,\
	connectivity/source/drivers/mozab/MDriver \
	connectivity/source/drivers/mozab/MServices \
))

# vim: set noet sw=4 ts=4:
