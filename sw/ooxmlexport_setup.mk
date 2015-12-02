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
	dbaccess/util/dba \
	drawinglayer/drawinglayer \
	embeddedobj/util/embobj \
	filter/source/config/cache/filterconfig1 \
	filter/source/odfflatxml/odfflatxml \
	filter/source/xmlfilterdetect/xmlfd \
	filter/source/xmlfilteradaptor/xmlfa \
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
	ucb/source/ucp/tdoc/ucptdoc1 \
	unotools/util/utl \
	unoxml/source/rdf/unordf \
	unoxml/source/service/unoxml \
	uui/util/uui \
	writerfilter/util/writerfilter \
	xmloff/util/xo
endef

# template for ooxmlexport tests (there are several so that they can be run in parallel)
define sw_ooxmlexport_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlexport$(1), \
    sw/qa/extras/ooxmlexport/ooxmlexport$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlexport$(1), \
	$(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlexport$(1),\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlexport$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ooxmlexport$(1),\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlexport$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_use_components,sw_ooxmlexport$(1),\
	$(sw_ooxmlexport_components) \
	filter/source/storagefilterdetect/storagefd \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_use_packages,sw_ooxmlexport$(1),\
	oox_customshapes \
	oox_generated \
))

$(call gb_CppunitTest_get_target,sw_ooxmlexport$(1)) : $(call gb_Library_get_target,iti)

endef

# vim: set noet sw=4 ts=4:
