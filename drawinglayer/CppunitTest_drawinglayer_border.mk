# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,drawinglayer_border))

$(eval $(call gb_CppunitTest_use_api,drawinglayer_border,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,drawinglayer_border, \
	basegfx \
	cppu \
	sal \
	salhelper \
	drawinglayer \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,drawinglayer_border,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,drawinglayer_border, \
	drawinglayer/qa/unit/border \
))

# vim: set noet sw=4 ts=4:
