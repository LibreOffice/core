# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,postgresql-sdbc))

$(eval $(call gb_Library_add_defs,postgresql-sdbc,\
	-DPQ_SDBC_MAJOR=0 \
	-DPQ_SDBC_MINOR=8 \
	-DPQ_SDBC_MICRO=2 \
))

$(eval $(call gb_Library_use_sdk_api,postgresql-sdbc))

$(eval $(call gb_Library_use_libraries,postgresql-sdbc,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
))

$(eval $(call gb_Library_set_componentfile,postgresql-sdbc,connectivity/source/drivers/postgresql/postgresql-sdbc,postgresql-sdbc))

$(eval $(call gb_Library_add_exception_objects,postgresql-sdbc,\
	connectivity/source/drivers/postgresql/pq_driver \
))

# vim: set noet sw=4 ts=4:
