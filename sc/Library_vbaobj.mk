# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,vbaobj))

$(eval $(call gb_Library_set_include,vbaobj,\
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,vbaobj,sc/inc/pch/precompiled_vbaobj))

$(eval $(call gb_Library_set_componentfile,vbaobj,sc/util/vbaobj,services))

$(eval $(call gb_Library_use_externals,vbaobj,\
	boost_headers \
	mdds_headers \
))

$(eval $(call gb_Library_use_api,vbaobj,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,vbaobj,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	msfilter \
	sal \
	sb \
	sc \
	sfx \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	utl \
	vbahelper \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,vbaobj,\
	sc/source/ui/vba/excelvbahelper \
	sc/source/ui/vba/vbaapplication \
	sc/source/ui/vba/vbaassistant \
	sc/source/ui/vba/vbaaxes \
	sc/source/ui/vba/vbaaxis \
	sc/source/ui/vba/vbaaxistitle \
	sc/source/ui/vba/vbaborders \
	sc/source/ui/vba/vbacharacters \
	sc/source/ui/vba/vbachart \
	sc/source/ui/vba/vbachartobject \
	sc/source/ui/vba/vbachartobjects \
	sc/source/ui/vba/vbacharttitle \
	sc/source/ui/vba/vbacomment \
	sc/source/ui/vba/vbacomments \
	sc/source/ui/vba/vbacondition \
	sc/source/ui/vba/vbadialog \
	sc/source/ui/vba/vbadialogs \
	sc/source/ui/vba/vbaeventshelper \
	sc/source/ui/vba/vbafiledialog \
	sc/source/ui/vba/vbafiledialogitems \
	sc/source/ui/vba/vbafont \
	sc/source/ui/vba/vbaformat \
	sc/source/ui/vba/vbaformatcondition \
	sc/source/ui/vba/vbaformatconditions \
	sc/source/ui/vba/vbaglobals \
	sc/source/ui/vba/vbahyperlink \
	sc/source/ui/vba/vbahyperlinks \
	sc/source/ui/vba/vbainterior \
	sc/source/ui/vba/vbalineshape \
	sc/source/ui/vba/vbamenubar  \
	sc/source/ui/vba/vbamenubars \
	sc/source/ui/vba/vbamenu \
	sc/source/ui/vba/vbamenus \
	sc/source/ui/vba/vbamenuitem \
	sc/source/ui/vba/vbamenuitems \
	sc/source/ui/vba/vbaname \
	sc/source/ui/vba/vbanames \
	sc/source/ui/vba/vbaoleobject \
	sc/source/ui/vba/vbaoleobjects \
	sc/source/ui/vba/vbaoutline \
	sc/source/ui/vba/vbaovalshape \
	sc/source/ui/vba/vbapagebreak \
	sc/source/ui/vba/vbapagebreaks \
	sc/source/ui/vba/vbapagesetup \
	sc/source/ui/vba/vbapalette \
	sc/source/ui/vba/vbapane \
	sc/source/ui/vba/vbapivotcache \
	sc/source/ui/vba/vbapivottable \
	sc/source/ui/vba/vbapivottables \
	sc/source/ui/vba/vbarange \
	sc/source/ui/vba/vbasheetobject \
	sc/source/ui/vba/vbasheetobjects \
	sc/source/ui/vba/vbastyle \
	sc/source/ui/vba/vbastyles \
	sc/source/ui/vba/vbatextboxshape \
	sc/source/ui/vba/vbatextframe \
	sc/source/ui/vba/vbavalidation \
	sc/source/ui/vba/vbawindow \
	sc/source/ui/vba/vbawindows \
	sc/source/ui/vba/vbaworkbook \
	sc/source/ui/vba/vbaworkbooks \
	sc/source/ui/vba/vbaworksheet \
	sc/source/ui/vba/vbaworksheets \
	sc/source/ui/vba/vbawsfunction \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,vbaobj,\
	advapi32 \
	oldnames \
))
endif

# vim: set noet sw=4 ts=4:
