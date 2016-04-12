# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scfilt))

$(eval $(call gb_Library_set_componentfile,scfilt,sc/util/scfilt))

$(eval $(call gb_Library_set_include,scfilt,\
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,scfilt,$(SRCDIR)/sc/inc/pch/precompiled_scfilt))

$(eval $(call gb_Library_use_sdk_api,scfilt))

$(eval $(call gb_Library_use_custom_headers,scfilt,\
	oox/generated \
	officecfg/registry \
))

$(eval $(call gb_Library_use_externals,scfilt,\
	boost_headers \
	$(call gb_Helper_optional,OPENCL,clew) \
	mdds_headers \
))

$(eval $(call gb_Library_use_libraries,scfilt,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	for \
	msfilter \
	oox \
	sal \
	salhelper \
	sax \
	sb \
	sc \
	sfx \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	ucbhelper \
	utl \
	vcl \
	i18nlangtag \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,scfilt,\
	sc/source/filter/dif/difexp \
	sc/source/filter/dif/difimp \
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
	sc/source/filter/excel/xedbdata \
	sc/source/filter/excel/xeescher \
	sc/source/filter/excel/xeextlst \
	sc/source/filter/excel/xeformula \
	sc/source/filter/excel/xehelper \
	sc/source/filter/excel/xelink \
	sc/source/filter/excel/xename \
	sc/source/filter/excel/xepage \
	sc/source/filter/excel/xepivot \
	sc/source/filter/excel/xepivotxml \
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
	sc/source/filter/excel/xltoolbar \
	sc/source/filter/excel/xltools \
	sc/source/filter/excel/xltracer \
	sc/source/filter/excel/xlview \
	sc/source/filter/ftools/fapihelper \
	sc/source/filter/ftools/fprogressbar \
	sc/source/filter/ftools/ftools \
	sc/source/filter/ftools/sharedformulagroups \
	sc/source/filter/html/htmlexp \
	sc/source/filter/html/htmlexp2 \
	sc/source/filter/html/htmlimp \
	sc/source/filter/html/htmlpars \
	sc/source/filter/lotus/filter \
	sc/source/filter/lotus/lotattr \
	sc/source/filter/lotus/lotform \
	sc/source/filter/lotus/lotimpop \
	sc/source/filter/lotus/lotread \
	sc/source/filter/lotus/lotus \
	sc/source/filter/lotus/memory \
	sc/source/filter/lotus/op \
	sc/source/filter/lotus/optab \
	sc/source/filter/lotus/tool \
	sc/source/filter/qpro/biff \
	sc/source/filter/qpro/qpro \
	sc/source/filter/qpro/qproform \
	sc/source/filter/qpro/qprostyle \
	sc/source/filter/rtf/eeimpars \
	sc/source/filter/rtf/expbase \
	sc/source/filter/rtf/rtfexp \
	sc/source/filter/rtf/rtfimp \
	sc/source/filter/rtf/rtfparse \
	sc/source/filter/starcalc/collect \
	sc/source/filter/starcalc/scflt \
	sc/source/filter/starcalc/scfobj \
	sc/source/filter/xcl97/XclExpChangeTrack \
	sc/source/filter/xcl97/XclImpChangeTrack \
	sc/source/filter/xcl97/xcl97esc \
	sc/source/filter/xcl97/xcl97rec \
	sc/source/filter/oox/excelfilter \
	sc/source/filter/oox/addressconverter \
	sc/source/filter/oox/autofilterbuffer \
	sc/source/filter/oox/autofiltercontext \
	sc/source/filter/oox/biffcodec \
	sc/source/filter/oox/biffhelper \
	sc/source/filter/oox/biffinputstream \
	sc/source/filter/oox/chartsheetfragment \
	sc/source/filter/oox/commentsbuffer \
	sc/source/filter/oox/commentsfragment \
	sc/source/filter/oox/condformatbuffer \
	sc/source/filter/oox/condformatcontext \
	sc/source/filter/oox/connectionsbuffer \
	sc/source/filter/oox/connectionsfragment \
	sc/source/filter/oox/defnamesbuffer \
	sc/source/filter/oox/drawingbase \
	sc/source/filter/oox/drawingfragment \
	sc/source/filter/oox/drawingmanager \
	sc/source/filter/oox/excelchartconverter \
	sc/source/filter/oox/excelhandlers \
	sc/source/filter/oox/excelvbaproject \
	sc/source/filter/oox/externallinkbuffer \
	sc/source/filter/oox/externallinkfragment \
	sc/source/filter/oox/extlstcontext \
	sc/source/filter/oox/formulabase \
	sc/source/filter/oox/formulabuffer \
	sc/source/filter/oox/formulaparser \
	sc/source/filter/oox/numberformatsbuffer \
	sc/source/filter/oox/ooxformulaparser \
	sc/source/filter/oox/pagesettings \
	sc/source/filter/oox/pivotcachebuffer \
	sc/source/filter/oox/pivotcachefragment \
	sc/source/filter/oox/pivottablebuffer \
	sc/source/filter/oox/pivottablefragment \
	sc/source/filter/oox/querytablebuffer \
	sc/source/filter/oox/querytablefragment \
	sc/source/filter/oox/revisionfragment \
	sc/source/filter/oox/richstringcontext \
	sc/source/filter/oox/richstring \
	sc/source/filter/oox/scenariobuffer \
	sc/source/filter/oox/scenariocontext \
	sc/source/filter/oox/sharedstringsbuffer \
	sc/source/filter/oox/sharedstringsfragment \
	sc/source/filter/oox/sheetdatabuffer \
	sc/source/filter/oox/sheetdatacontext \
	sc/source/filter/oox/stylesbuffer \
	sc/source/filter/oox/stylesfragment \
	sc/source/filter/oox/tablebuffer \
	sc/source/filter/oox/tablecolumnsbuffer \
	sc/source/filter/oox/tablecolumnscontext \
	sc/source/filter/oox/tablefragment \
	sc/source/filter/oox/themebuffer \
	sc/source/filter/oox/unitconverter \
	sc/source/filter/oox/viewsettings \
	sc/source/filter/oox/workbookfragment \
	sc/source/filter/oox/workbookhelper \
	sc/source/filter/oox/workbooksettings \
	sc/source/filter/oox/worksheetbuffer \
	sc/source/filter/oox/worksheetfragment \
	sc/source/filter/oox/worksheethelper \
	sc/source/filter/oox/worksheetsettings \
))

ifeq ($(ENABLE_ORCUS),TRUE)
$(eval $(call gb_Library_use_externals,scfilt,\
	orcus \
	orcus-parser \
	boost_filesystem \
	boost_system \
	boost_iostreams \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,scfilt,\
	sc/source/filter/orcus/interface \
	sc/source/filter/orcus/orcusfiltersimpl \
	sc/source/filter/orcus/xmlcontext \
	sc/source/filter/orcus/filterdetect \
))

endif

# vim: set noet sw=4 ts=4:
