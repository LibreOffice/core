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



$(eval $(call gb_Library_Library,scfilt))

$(eval $(call gb_Library_add_precompiled_header,scfilt,$(SRCDIR)/sc/inc/pch/precompiled_scfilt))

$(eval $(call gb_Library_set_include,scfilt,\
        $$(INCLUDE) \
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/inc/pch \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
))

$(eval $(call gb_Library_add_api,scfilt,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,scfilt,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	for \
	msfilter \
	ootk \
	oox \
	sal \
	sax \
	sb \
	sc \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	utl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,scfilt,\
	sc/source/filter/ftools/fapihelper \
	sc/source/filter/ftools/fprogressbar \
	sc/source/filter/ftools/ftools \
	sc/source/filter/excel/colrowst \
	sc/source/filter/excel/excdoc \
	sc/source/filter/excel/excel \
	sc/source/filter/excel/excform \
	sc/source/filter/excel/excform8 \
	sc/source/filter/excel/excimp8 \
	sc/source/filter/excel/excrecds \
	sc/source/filter/excel/exctools \
	sc/source/filter/excel/expop2 \
	sc/source/filter/excel/fontbuff \
	sc/source/filter/excel/frmbase \
	sc/source/filter/excel/impop \
	sc/source/filter/excel/namebuff \
	sc/source/filter/excel/read \
	sc/source/filter/excel/tokstack \
	sc/source/filter/excel/xechart \
	sc/source/filter/excel/xecontent \
	sc/source/filter/excel/xeescher \
	sc/source/filter/excel/xeformula \
	sc/source/filter/excel/xehelper \
	sc/source/filter/excel/xelink \
	sc/source/filter/excel/xename \
	sc/source/filter/excel/xepage \
	sc/source/filter/excel/xepivot \
	sc/source/filter/excel/xerecord \
	sc/source/filter/excel/xeroot \
	sc/source/filter/excel/xestream \
	sc/source/filter/excel/xestring \
	sc/source/filter/excel/xestyle \
	sc/source/filter/excel/xetable \
	sc/source/filter/excel/xeview \
	sc/source/filter/excel/xichart \
	sc/source/filter/excel/xicontent \
	sc/source/filter/excel/xiescher \
	sc/source/filter/excel/xiformula \
	sc/source/filter/excel/xihelper \
	sc/source/filter/excel/xilink \
	sc/source/filter/excel/xiname \
	sc/source/filter/excel/xipage \
	sc/source/filter/excel/xipivot \
	sc/source/filter/excel/xiroot \
	sc/source/filter/excel/xistream \
	sc/source/filter/excel/xistring \
	sc/source/filter/excel/xistyle \
	sc/source/filter/excel/xiview \
	sc/source/filter/excel/xladdress \
	sc/source/filter/excel/xlchart \
	sc/source/filter/excel/xlescher \
	sc/source/filter/excel/xlformula \
	sc/source/filter/excel/xlpage \
	sc/source/filter/excel/xlpivot \
	sc/source/filter/excel/xlroot \
	sc/source/filter/excel/xlstyle \
	sc/source/filter/excel/xltools \
	sc/source/filter/excel/xltracer \
	sc/source/filter/excel/xlview \
	sc/source/filter/xcl97/xcl97esc \
	sc/source/filter/xcl97/xcl97rec \
	sc/source/filter/xcl97/XclImpChangeTrack \
	sc/source/filter/xcl97/XclExpChangeTrack \
	sc/source/filter/lotus/filter \
	sc/source/filter/lotus/lotus \
	sc/source/filter/lotus/lotimpop \
	sc/source/filter/lotus/lotread \
	sc/source/filter/lotus/lotform \
	sc/source/filter/lotus/memory \
	sc/source/filter/lotus/op \
	sc/source/filter/lotus/optab \
	sc/source/filter/lotus/tool \
	sc/source/filter/lotus/expop \
	sc/source/filter/lotus/export \
	sc/source/filter/lotus/lotattr \
	sc/source/filter/qpro/qpro \
	sc/source/filter/qpro/qproform \
	sc/source/filter/qpro/qprostyle \
	sc/source/filter/qpro/biff \
	sc/source/filter/dif/difimp \
	sc/source/filter/dif/difexp \
	sc/source/filter/html/htmlexp \
	sc/source/filter/html/htmlexp2 \
	sc/source/filter/html/htmlimp \
	sc/source/filter/html/htmlpars \
	sc/source/filter/rtf/eeimpars \
	sc/source/filter/rtf/expbase \
	sc/source/filter/rtf/rtfexp \
	sc/source/filter/rtf/rtfimp \
	sc/source/filter/rtf/rtfparse \
	sc/source/filter/starcalc/scflt \
	sc/source/filter/starcalc/scfobj \
))

ifeq ($(OS)$(COM)$(CPUNAME),LINUXGCCSPARC)
$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	sc/source/filter/excel/xiescher, $(gb_COMPILERNOOPTFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
