# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,subsequenttest))

$(eval $(call gb_Library_set_include,subsequenttest,\
	-I$(SRCDIR)/test/inc \
    $$(INCLUDE) \
))

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
	test/source/container/xchild \
	test/source/container/xelementaccess \
	test/source/container/xenumeration \
	test/source/container/xenumerationaccess \
	test/source/container/xindexaccess \
	test/source/container/xnameaccess \
	test/source/container/xnamecontainer \
	test/source/container/xnamed \
	test/source/container/xnamereplace \
	test/source/document/xactionlockable \
	test/source/document/xembeddedobjectsupplier \
	test/source/document/xlinktargetsupplier \
	test/source/drawing/captionshape \
	test/source/drawing/xdrawpages \
	test/source/lang/xserviceinfo \
	test/source/lang/xcomponent \
	test/source/sheet/cellarealink \
	test/source/sheet/cellproperties \
	test/source/sheet/databaseimportdescriptor \
	test/source/sheet/databaserange \
	test/source/sheet/datapilotfield \
	test/source/sheet/datapilotitem \
	test/source/sheet/documentsettings \
	test/source/sheet/functiondescription \
	test/source/sheet/globalsheetsettings \
	test/source/sheet/scenario \
	test/source/sheet/shape \
	test/source/sheet/sheetcell \
	test/source/sheet/sheetcellrange \
	test/source/sheet/sheetcellranges \
	test/source/sheet/sheetfilterdescriptor \
	test/source/sheet/sheetlink \
	test/source/sheet/sheetsortdescriptor2 \
	test/source/sheet/spreadsheet \
	test/source/sheet/spreadsheetdocumentsettings \
	test/source/sheet/spreadsheetviewsettings \
	test/source/sheet/subtotaldescriptor \
	test/source/sheet/tableautoformat \
	test/source/sheet/tablevalidation \
	test/source/sheet/xactivationbroadcaster \
	test/source/sheet/xarealink \
	test/source/sheet/xarealinks \
	test/source/sheet/xarrayformularange \
	test/source/sheet/xcalculatable \
	test/source/sheet/xcelladdressable \
	test/source/sheet/xcellformatrangessupplier \
	test/source/sheet/xcellrangeaddressable \
	test/source/sheet/xcellrangedata \
	test/source/sheet/xcellrangeformula \
	test/source/sheet/xcellrangemovement \
	test/source/sheet/xcellrangereferrer \
	test/source/sheet/xcellrangesquery \
	test/source/sheet/xcellseries \
	test/source/sheet/xconsolidatable \
	test/source/sheet/xconsolidationdescriptor \
	test/source/sheet/xdatabaserange \
	test/source/sheet/xdatabaseranges \
	test/source/sheet/xdatapilotdescriptor \
	test/source/sheet/xdatapilotfield \
	test/source/sheet/xdatapilotfieldgrouping \
	test/source/sheet/xdatapilottable \
	test/source/sheet/xdatapilottable2 \
	test/source/sheet/xdatapilottables \
	test/source/sheet/xdatapilottablessupplier \
	test/source/sheet/xddelink \
	test/source/sheet/xddelinks \
	test/source/sheet/xdocumentauditing \
	test/source/sheet/xformulaquery \
	test/source/sheet/xfunctiondescriptions \
	test/source/sheet/xgoalseek \
	test/source/sheet/xheaderfootercontent \
	test/source/sheet/xlabelrange \
	test/source/sheet/xlabelranges \
	test/source/sheet/xmultiformulatokens \
	test/source/sheet/xmultipleoperation \
	test/source/sheet/xnamedrange \
	test/source/sheet/xnamedranges \
	test/source/sheet/xprintareas \
	test/source/sheet/xrecentfunctions \
	test/source/sheet/xscenario \
	test/source/sheet/xscenarioenhanced \
	test/source/sheet/xscenarios \
	test/source/sheet/xscenariossupplier \
	test/source/sheet/xsheetannotation \
	test/source/sheet/xsheetannotationanchor \
	test/source/sheet/xsheetannotations \
	test/source/sheet/xsheetannotationshapesupplier \
	test/source/sheet/xsheetannotationssupplier \
	test/source/sheet/xsheetauditing \
	test/source/sheet/xsheetcellcursor \
	test/source/sheet/xsheetcellrange \
	test/source/sheet/xsheetcellrangecontainer \
	test/source/sheet/xsheetcellranges \
	test/source/sheet/xsheetcondition \
	test/source/sheet/xsheetconditionalentries \
	test/source/sheet/xsheetconditionalentry \
	test/source/sheet/xsheetfilterable \
	test/source/sheet/xsheetfilterableex \
	test/source/sheet/xsheetfilterdescriptor \
	test/source/sheet/xsheetfilterdescriptor2 \
	test/source/sheet/xsheetfilterdescriptor3 \
	test/source/sheet/xsheetlinkable \
	test/source/sheet/xsheetoperation \
	test/source/sheet/xsheetoutline \
	test/source/sheet/xsheetpagebreak \
	test/source/sheet/xspreadsheet \
	test/source/sheet/xspreadsheetdocument \
	test/source/sheet/xspreadsheets \
	test/source/sheet/xspreadsheets2 \
	test/source/sheet/xspreadsheetview \
	test/source/sheet/xsubtotalcalculatable \
	test/source/sheet/xsubtotaldescriptor \
	test/source/sheet/xsubtotalfield \
	test/source/sheet/xuniquecellformatrangessupplier\
	test/source/sheet/xusedareacursor \
	test/source/sheet/xviewfreezable \
	test/source/sheet/xviewpane \
	test/source/sheet/xviewsplitable \
	test/source/style/xstyleloader \
	test/source/table/xcell \
	test/source/table/xcellcursor \
	test/source/table/xcolumnrowrange \
	test/source/table/xtablechart \
	test/source/table/xtablecharts \
	test/source/table/xtablechartssupplier \
	test/source/table/xtablecolumns \
	test/source/table/xtablerows \
	test/source/text/baseindex \
	test/source/text/xsimpletext \
	test/source/text/xtext \
	test/source/text/xtextcontent \
	test/source/text/xtextfield \
	test/source/text/xtextrange \
    test/source/text/xdocumentindex \
    test/source/text/textdocumentsettings \
    test/source/text/textprintersettings \
    test/source/text/textsettings \
    test/source/text/textdocumentindex \
	test/source/util/xindent \
	test/source/util/xmergeable \
	test/source/util/xrefreshable \
	test/source/util/xreplaceable \
	test/source/util/xsearchable \
	test/source/view/xcontrolaccess \
))

# vim: set noet sw=4 ts=4:
