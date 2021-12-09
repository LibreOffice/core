# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpexpand1))

$(eval $(call gb_Library_set_componentfile,ucpexpand1,ucb/source/ucp/expand/ucpexpand1,services))

$(eval $(call gb_Library_use_sdk_api,ucpexpand1))

$(eval $(call gb_Library_use_libraries,ucpexpand1,\
	cppu \
	cppuhelper \
	sal \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucpexpand1,\
	ucb/source/ucp/expand/ucpexpand \
))

# vim: set noet sw=4 ts=4:
