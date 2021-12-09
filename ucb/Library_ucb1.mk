# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucb1))

$(eval $(call gb_Library_set_componentfile,ucb1,ucb/source/core/ucb1,services))

$(eval $(call gb_Library_set_include,ucb1,\
	-I$(SRCDIR)/ucb/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,ucb1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucb1))

$(eval $(call gb_Library_use_libraries,ucb1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	ucbhelper \
	utl \
))

$(eval $(call gb_Library_add_exception_objects,ucb1,\
	ucb/source/core/cmdenv \
	ucb/source/core/identify \
	ucb/source/core/provprox \
	ucb/source/core/ucb \
	ucb/source/core/ucbcmds \
	ucb/source/core/ucbprops \
	ucb/source/core/ucbstore \
	ucb/source/core/FileAccess \
	ucb/source/regexp/regexp \
))

# vim: set noet sw=4 ts=4:
