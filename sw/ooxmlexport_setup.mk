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

define sw_ooxmlexport_libraries
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sw \
	test \
	tl \
	unotest \
	utl \
	vcl \
	$(gb_UWINAPI)
endef

define sw_ooxmlexport_components
	basic/util/sb \
	canvas/source/factory/canvasfactory \
	chart2/source/chartcore \
	chart2/source/controller/chartcontroller \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	drawinglayer/drawinglayer \
	embeddedobj/util/embobj \
	filter/source/config/cache/filterconfig1 \
	forms/util/frm \
	framework/util/fwk \
	i18npool/util/i18npool \
	linguistic/source/lng \
	oox/util/oox \
	package/source/xstor/xstor \
	package/util/package2 \
	sax/source/expatwrap/expwrap \
	sw/util/sw \
	sw/util/swd \
	sw/util/msword \
	sfx2/util/sfx \
	starmath/util/sm \
	svl/source/fsstor/fsstorage \
	svl/util/svl \
	svtools/util/svt \
	svx/util/svx \
	svx/util/svxcore \
	toolkit/util/tk \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	unotools/util/utl \
	unoxml/source/service/unoxml \
	uui/util/uui \
	writerfilter/util/writerfilter \
	xmloff/util/xo
endef

# vim: set noet sw=4 ts=4:
