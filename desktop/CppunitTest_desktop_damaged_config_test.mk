# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,desktop_damaged_config))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_damaged_config, \
	desktop/qa/extras/shutdown \
))

$(eval $(call gb_CppunitTest_use_external,desktop_damaged_config,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,desktop_damaged_config, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	for \
	forui \
	i18nlangtag \
	msfilter \
	oox \
	sal \
	salhelper \
	sax \
	sb \
	sc \
	scqahelper \
	sfx \
	sot \
	subsequenttest \
	svl \
	svt \
	svx \
	svxcore \
	test \
	tk \
	tl \
	ucbhelper \
	unotest \
	utl \
	vbahelper \
	vcl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_damaged_config))
$(eval $(call gb_CppunitTest_use_ure,desktop_damaged_config))
$(eval $(call gb_CppunitTest_use_vcl,desktop_damaged_config))
$(eval $(call gb_CppunitTest_use_configuration,desktop_damaged_config))

# vim: set noet sw=4 ts=4:
