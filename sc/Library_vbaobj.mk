# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,vbaobj))

$(eval $(call gb_Library_set_include,vbaobj,\
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,vbaobj,sc/util/vbaobj))

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
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,vbaobj))

$(eval $(call gb_Library_add_exception_objects,vbaobj,\
	sc/source/ui/vba/excelvbahelper \
	sc/source/ui/vba/service \
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
	sc/source/ui/vba/vbafont \
	sc/source/ui/vba/vbaformat \
	sc/source/ui/vba/vbaformatcondition \
	sc/source/ui/vba/vbaformatconditions \
	sc/source/ui/vba/vbaglobals \
	sc/source/ui/vba/vbahyperlink \
	sc/source/ui/vba/vbahyperlinks \
	sc/source/ui/vba/vbainterior \
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

#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50255
ifeq ($(COM)-$(OS)-$(CPUNAME),GCC-LINUX-INTEL)
sc_gccthunkBroken := $(shell expr $(gb_CCVER) \>= 40500 \& $(gb_CCVER) \< 40602)
endif

ifeq ($(sc_gccthunkBroken),1)
$(eval $(call gb_Library_add_cxxobjects,vbaobj,\
    sc/source/ui/vba/vbasheetobjects \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,vbaobj,\
    sc/source/ui/vba/vbasheetobjects \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,vbaobj,\
	advapi32 \
	$(gb_Library_win32_OLDNAMES) \
	uwinapi \
))
endif
# vim: set noet sw=4 ts=4:
