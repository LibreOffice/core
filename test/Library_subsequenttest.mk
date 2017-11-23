# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,subsequenttest))

$(eval $(call gb_Library_add_defs,subsequenttest,\
	-DOOO_DLLIMPLEMENTATION_TEST \
))

$(eval $(call gb_Library_use_sdk_api,subsequenttest))

$(eval $(call gb_Library_use_libraries,subsequenttest,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	test \
	tl \
	utl \
	unotest \
	vcl \
))

$(eval $(call gb_Library_use_externals,subsequenttest,\
	boost_headers \
	cppunit \
))

$(eval $(call gb_Library_add_exception_objects,subsequenttest,\
	test/source/unoapi_test \
	test/source/calc_unoapi_test \
	test/source/beans/xpropertyset \
	test/source/container/xelementaccess \
	test/source/container/xindexaccess \
	test/source/container/xnamecontainer \
	test/source/container/xnamereplace \
	test/source/container/xnamed \
	test/source/sheet/cellarealink \
	test/source/sheet/cellproperties \
	test/source/sheet/databaserange \
	test/source/sheet/datapilotfield \
	test/source/sheet/datapilotitem \
	test/source/sheet/xarealink \
	test/source/sheet/xarealinks \
	test/source/sheet/xcelladdressable \
	test/source/sheet/xcellrangedata \
	test/source/sheet/xcellrangereferrer \
	test/source/sheet/xcellrangesquery \
	test/source/sheet/xcellseries \
	test/source/sheet/xgoalseek \
	test/source/sheet/xdatabaserange \
	test/source/sheet/xdatapilotdescriptor \
	test/source/sheet/xdatapilotfieldgrouping \
	test/source/sheet/xdatapilottable \
	test/source/sheet/xdatapilottable2 \
	test/source/sheet/xlabelrange \
	test/source/sheet/xlabelranges \
	test/source/sheet/xnamedrange \
	test/source/sheet/xnamedranges \
	test/source/sheet/xprintareas \
	test/source/sheet/xscenarioenhanced \
	test/source/sheet/xscenarios \
	test/source/sheet/xscenariossupplier \
	test/source/sheet/xspreadsheetdocument \
	test/source/sheet/xspreadsheet \
	test/source/sheet/xspreadsheets \
	test/source/sheet/xspreadsheets2 \
	test/source/sheet/xspreadsheetview \
	test/source/sheet/xsheetannotation \
	test/source/sheet/xsheetannotationanchor \
	test/source/sheet/xsheetannotations \
	test/source/sheet/xsheetannotationssupplier \
	test/source/sheet/xsheetannotationshapesupplier \
	test/source/sheet/xsheetauditing \
	test/source/sheet/xsheetcellrangecontainer \
	test/source/sheet/xsheetcellrange \
	test/source/sheet/xsheetcellranges \
	test/source/sheet/xsheetconditionalentries \
	test/source/sheet/xsheetconditionalentry \
	test/source/sheet/xsheetcondition \
	test/source/sheet/xsheetfilterable \
	test/source/sheet/xsheetfilterableex \
	test/source/sheet/xsheetfilterdescriptor \
	test/source/sheet/xsheetoutline \
	test/source/sheet/xsheetlinkable \
	test/source/sheet/xsheetoperation \
	test/source/sheet/xsheetpagebreak \
	test/source/sheet/xstyleloader \
	test/source/sheet/xsubtotalfield \
	test/source/sheet/xuniquecellformatrangessupplier\
	test/source/sheet/xusedareacursor \
	test/source/sheet/xviewpane \
	test/source/sheet/xviewfreezable \
	test/source/sheet/xviewsplitable \
	test/source/sheet/xsubtotalcalculatable \
	test/source/sheet/xsubtotaldescriptor \
	test/source/text/xtext \
	test/source/text/xtextfield \
	test/source/text/xtextcontent \
	test/source/util/xreplaceable \
	test/source/util/xsearchable \
))

# vim: set noet sw=4 ts=4:
