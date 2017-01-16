#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,vbaobj))

$(eval $(call gb_Library_set_componentfile,vbaobj,sc/util/vbaobj))

$(eval $(call gb_Library_set_include,vbaobj,\
        $$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/inc/pch \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
))

$(eval $(call gb_Library_add_api,vbaobj,\
	offapi \
	oovbaapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,vbaobj,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	msfilter \
	ootk \
	sal \
	sb \
	sc \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svxcore \
	tl \
	utl \
	vbahelper \
	vcl \
	$(gb_STDLIBS) \
))

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
	sc/source/ui/vba/vbacharts \
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
	sc/source/ui/vba/vbaseriescollection \
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

# vim: set noet sw=4 ts=4:
