# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sc))

$(eval $(call gb_Library_add_sdi_headers,sc,sc/sdi/scslots))

$(eval $(call gb_Library_set_componentfile,sc,sc/util/sc))

$(eval $(call gb_Library_set_precompiled_header,sc,$(SRCDIR)/sc/inc/pch/precompiled_sc))

$(eval $(call gb_Library_set_include,sc,\
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/source/filter/inc \
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    -I$(WORKDIR)/SdiTarget/sc/sdi \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,sc,\
    -DSC_DLLIMPLEMENTATION \
    -DSC_INFO_OSVERSION=\"$(OS)\" \
))

$(eval $(call gb_Library_use_custom_headers,sc,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,sc))

$(eval $(call gb_Library_use_externals,sc,\
    boost_headers \
    $(call gb_Helper_optional,OPENCL, \
        clew) \
    icu_headers \
    icui18n \
    icuuc \
    libxml2 \
    mdds_headers \
))

ifneq ($(SYSTEM_LIBORCUS),)
$(eval $(call gb_Library_use_externals,sc,orcus))
else
$(eval $(call gb_Library_use_externals,sc,orcus-parser))
endif

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_Library_use_libraries,sc,tubes))

$(eval $(call gb_Library_add_exception_objects,sc,\
    sc/source/ui/collab/sccollaboration \
    sc/source/ui/collab/sendfunc \
))
endif

$(eval $(call gb_Library_use_libraries,sc,\
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    i18nutil \
    $(call gb_Helper_optional,OPENCL, \
        opencl) \
    sal \
    salhelper \
    sax \
    sb \
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
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,sc,\
    sc/source/core/data/attarray \
    sc/source/core/data/attrib \
    sc/source/core/data/autonamecache \
    sc/source/core/data/bcaslot \
    sc/source/core/data/bigrange \
    sc/source/core/data/celltextattr \
    sc/source/core/data/cellvalue \
    sc/source/core/data/cellvalues \
    sc/source/core/data/clipcontext \
    sc/source/core/data/clipparam \
    sc/source/core/data/colcontainer \
    sc/source/core/data/column \
    sc/source/core/data/column2 \
    sc/source/core/data/column3 \
    sc/source/core/data/column4 \
    sc/source/core/data/columniterator \
    sc/source/core/data/columnset \
    sc/source/core/data/columnspanset \
    sc/source/core/data/compressedarray \
    sc/source/core/data/colorscale \
    sc/source/core/data/conditio \
    sc/source/core/data/dbdocutl \
    sc/source/core/data/dociter \
    sc/source/core/data/docparam \
    sc/source/core/data/docpool \
    sc/source/core/data/documen2 \
    sc/source/core/data/documen3 \
    sc/source/core/data/documen4 \
    sc/source/core/data/documen5 \
    sc/source/core/data/documen6 \
    sc/source/core/data/documen7 \
    sc/source/core/data/documen8 \
    sc/source/core/data/documen9 \
    sc/source/core/data/document \
    sc/source/core/data/document10 \
    sc/source/core/data/documentimport \
    sc/source/core/data/documentstreamaccess \
    sc/source/core/data/dpdimsave \
    sc/source/core/data/dpfilteredcache \
    sc/source/core/data/dpglobal \
    sc/source/core/data/dpgroup \
    sc/source/core/data/dpitemdata \
    sc/source/core/data/dpnumgroupinfo \
    sc/source/core/data/dpobject \
    sc/source/core/data/dpoutput \
    sc/source/core/data/dpoutputgeometry \
    sc/source/core/data/dpresfilter \
    sc/source/core/data/dpsave \
    sc/source/core/data/dpsdbtab \
    sc/source/core/data/dpshttab \
    sc/source/core/data/dptabdat \
    sc/source/core/data/dpcache \
    sc/source/core/data/dptabres \
    sc/source/core/data/dptabsrc \
    sc/source/core/data/dputil \
    sc/source/core/data/drawpage \
    sc/source/core/data/drwlayer \
    sc/source/core/data/edittextiterator \
    sc/source/core/data/fillinfo \
    sc/source/core/data/formulacell \
    sc/source/core/data/formulaiter \
    sc/source/core/data/funcdesc \
    sc/source/core/data/global \
    sc/source/core/data/global2 \
    sc/source/core/data/globalx \
    sc/source/core/data/grouptokenconverter \
    sc/source/core/data/listenercontext \
    sc/source/core/data/markarr \
    sc/source/core/data/markdata \
    sc/source/core/data/markmulti \
    sc/source/core/data/mtvelements \
    sc/source/core/data/olinetab \
    sc/source/core/data/pagepar \
    sc/source/core/data/patattr \
    sc/source/core/data/pivot2 \
    sc/source/core/data/poolhelp \
    sc/source/core/data/postit \
    sc/source/core/data/refupdatecontext \
    sc/source/core/data/rowheightcontext \
    sc/source/core/data/segmenttree \
    sc/source/core/data/sheetevents \
    sc/source/core/data/simpleformulacalc \
    sc/source/core/data/sortparam \
    sc/source/core/data/stlpool \
    sc/source/core/data/stlsheet \
    sc/source/core/data/subtotalparam \
    sc/source/core/data/tabbgcolor \
    sc/source/core/data/table1 \
    sc/source/core/data/table2 \
    sc/source/core/data/table3 \
    sc/source/core/data/table4 \
    sc/source/core/data/table5 \
    sc/source/core/data/table6 \
    sc/source/core/data/table7 \
    sc/source/core/data/tabprotection \
    sc/source/core/data/types \
    sc/source/core/data/userdat \
    sc/source/core/data/validat \
    sc/source/core/tool/addincfg \
    sc/source/core/tool/addincol \
    sc/source/core/tool/addinhelpid \
    sc/source/core/tool/addinlis \
    sc/source/core/tool/address \
    sc/source/core/tool/adiasync \
    sc/source/core/tool/appoptio \
    sc/source/core/tool/autoform \
    sc/source/core/tool/bulkdatahint \
    sc/source/core/tool/brdcst \
    sc/source/core/tool/calcconfig \
    sc/source/core/tool/callform \
    sc/source/core/tool/cellform \
    sc/source/core/tool/cellkeytranslator \
    sc/source/core/tool/chartarr \
    sc/source/core/tool/charthelper \
    sc/source/core/tool/chartlis \
    sc/source/core/tool/chartlock \
    sc/source/core/tool/chartpos \
    sc/source/core/tool/chgtrack \
    sc/source/core/tool/chgviset \
    sc/source/core/tool/compare \
    sc/source/core/tool/compiler \
    sc/source/core/tool/consoli  \
    sc/source/core/tool/dbdata \
    sc/source/core/tool/ddelink \
    sc/source/core/tool/defaultsoptions \
    sc/source/core/tool/detdata  \
    sc/source/core/tool/detfunc \
    sc/source/core/tool/docoptio \
    sc/source/core/tool/doubleref \
    sc/source/core/tool/editdataarray \
    sc/source/core/tool/editutil \
    sc/source/core/tool/filtopt \
    sc/source/core/tool/formulagroup \
    sc/source/core/tool/formulaopt \
    sc/source/core/tool/formulaparserpool \
    sc/source/core/tool/formularesult \
    sc/source/core/tool/grouparealistener \
    sc/source/core/tool/hints \
    sc/source/core/tool/inputopt \
    sc/source/core/tool/interpr1 \
    sc/source/core/tool/interpr2 \
    sc/source/core/tool/interpr3 \
    sc/source/core/tool/interpr4 \
    sc/source/core/tool/interpr5 \
    sc/source/core/tool/interpr6 \
    sc/source/core/tool/interpr7 \
    sc/source/core/tool/interpr8 \
    sc/source/core/tool/jumpmatrix \
    sc/source/core/tool/listenerquery \
    sc/source/core/tool/lookupcache \
    sc/source/core/tool/matrixoperators \
    sc/source/core/tool/navicfg \
    sc/source/core/tool/numformat \
    sc/source/core/tool/odffmap \
    sc/source/core/tool/optutil \
    sc/source/core/tool/orcusxml \
    sc/source/core/tool/parclass \
    sc/source/core/tool/printopt \
    sc/source/core/tool/prnsave \
    sc/source/core/tool/progress \
    sc/source/core/tool/queryentry \
    sc/source/core/tool/queryparam \
    sc/source/core/tool/rangelst \
    sc/source/core/tool/rangenam \
    sc/source/core/tool/rangeseq \
    sc/source/core/tool/rangeutl \
    sc/source/core/tool/rechead  \
    sc/source/core/tool/recursionhelper \
    sc/source/core/tool/refdata \
    sc/source/core/tool/reffind \
    sc/source/core/tool/refhint \
    sc/source/core/tool/refreshtimer \
    sc/source/core/tool/reftokenhelper \
    sc/source/core/tool/refupdat \
    sc/source/core/tool/reordermap \
    sc/source/core/tool/scmatrix \
    sc/source/core/tool/scopetools \
    sc/source/core/tool/sharedformula \
    sc/source/core/tool/simplerangelist \
    sc/source/core/tool/stringutil \
    sc/source/core/tool/stylehelper \
    sc/source/core/tool/subtotal \
    sc/source/core/tool/token \
    sc/source/core/tool/tokenstringcontext \
    sc/source/core/tool/typedstrdata \
    sc/source/core/tool/unitconv \
    sc/source/core/tool/userlist \
    sc/source/core/tool/viewopti \
    sc/source/core/tool/zforauto \
    sc/source/filter/xml/datastreamimport \
    sc/source/filter/xml/XMLCalculationSettingsContext \
    sc/source/filter/xml/XMLCellRangeSourceContext \
    sc/source/filter/xml/XMLChangeTrackingExportHelper \
    sc/source/filter/xml/XMLChangeTrackingImportHelper \
    sc/source/filter/xml/XMLCodeNameProvider \
    sc/source/filter/xml/XMLColumnRowGroupExport \
    sc/source/filter/xml/XMLConsolidationContext \
    sc/source/filter/xml/XMLConverter \
    sc/source/filter/xml/XMLDDELinksContext \
    sc/source/filter/xml/XMLDetectiveContext \
    sc/source/filter/xml/XMLEmptyContext \
    sc/source/filter/xml/XMLExportDDELinks \
    sc/source/filter/xml/XMLExportDataPilot \
    sc/source/filter/xml/XMLExportDatabaseRanges \
    sc/source/filter/xml/XMLExportIterator \
    sc/source/filter/xml/XMLExportSharedData \
    sc/source/filter/xml/XMLStylesExportHelper \
    sc/source/filter/xml/XMLStylesImportHelper \
    sc/source/filter/xml/XMLTableHeaderFooterContext \
    sc/source/filter/xml/XMLTableMasterPageExport \
    sc/source/filter/xml/XMLTableShapeImportHelper \
    sc/source/filter/xml/XMLTableShapeResizer \
    sc/source/filter/xml/XMLTableShapesContext \
    sc/source/filter/xml/XMLTableSourceContext \
    sc/source/filter/xml/XMLTrackedChangesContext \
    sc/source/filter/xml/cachedattraccess \
    sc/source/filter/xml/celltextparacontext \
    sc/source/filter/xml/editattributemap \
    sc/source/filter/xml/importcontext \
    sc/source/filter/xml/pivotsource \
    sc/source/filter/xml/sheetdata \
    sc/source/filter/xml/xmlannoi \
    sc/source/filter/xml/xmlbodyi \
    sc/source/filter/xml/xmlcelli \
    sc/source/filter/xml/xmlcoli \
    sc/source/filter/xml/xmlcondformat \
    sc/source/filter/xml/xmlconti \
    sc/source/filter/xml/xmlcvali \
    sc/source/filter/xml/xmldpimp \
    sc/source/filter/xml/xmldrani \
    sc/source/filter/xml/xmlexprt \
    sc/source/filter/xml/xmlexternaltabi \
    sc/source/filter/xml/xmlfilti \
    sc/source/filter/xml/xmlfonte \
    sc/source/filter/xml/xmlimprt \
    sc/source/filter/xml/xmllabri \
    sc/source/filter/xml/xmlnexpi \
    sc/source/filter/xml/xmlrowi \
    sc/source/filter/xml/xmlsceni \
    sc/source/filter/xml/xmlsorti \
    sc/source/filter/xml/xmlstyle \
    sc/source/filter/xml/xmlstyli \
    sc/source/filter/xml/xmlsubti \
    sc/source/filter/xml/xmltabi \
    sc/source/filter/xml/xmlwrap \
    sc/source/filter/chart/chart_imp \
    sc/source/filter/importfilterdata \
    sc/source/ui/Accessibility/AccessibilityHints \
    sc/source/ui/Accessibility/AccessibleCell \
    sc/source/ui/Accessibility/AccessibleCellBase \
    sc/source/ui/Accessibility/AccessibleContextBase \
    sc/source/ui/Accessibility/AccessibleCsvControl \
    sc/source/ui/Accessibility/AccessibleDocument \
    sc/source/ui/Accessibility/AccessibleDocumentBase \
    sc/source/ui/Accessibility/AccessibleDocumentPagePreview \
    sc/source/ui/Accessibility/AccessibleEditObject \
    sc/source/ui/Accessibility/AccessibleFilterMenu \
    sc/source/ui/Accessibility/AccessibleFilterMenuItem \
    sc/source/ui/Accessibility/AccessibleFilterTopWindow \
    sc/source/ui/Accessibility/AccessibleGlobal \
    sc/source/ui/Accessibility/AccessiblePageHeader \
    sc/source/ui/Accessibility/AccessiblePageHeaderArea \
    sc/source/ui/Accessibility/AccessiblePreviewCell \
    sc/source/ui/Accessibility/AccessiblePreviewHeaderCell \
    sc/source/ui/Accessibility/AccessiblePreviewTable \
    sc/source/ui/Accessibility/AccessibleSpreadsheet \
    sc/source/ui/Accessibility/AccessibleTableBase \
    sc/source/ui/Accessibility/AccessibleText \
    sc/source/ui/Accessibility/DrawModelBroadcaster \
    sc/source/ui/app/client \
    sc/source/ui/app/drwtrans \
    sc/source/ui/app/inputhdl \
    sc/source/ui/app/inputwin \
    sc/source/ui/app/lnktrans \
    sc/source/ui/app/msgpool \
    sc/source/ui/app/rfindlst \
    sc/source/ui/app/scdll \
    sc/source/ui/app/scmod \
    sc/source/ui/app/seltrans \
    sc/source/ui/app/transobj \
    sc/source/ui/app/typemap \
    sc/source/ui/app/uiitems \
    sc/source/ui/attrdlg/scabstdlg \
    sc/source/ui/cctrl/cbuttonw \
    sc/source/ui/cctrl/checklistmenu \
    sc/source/ui/cctrl/dpcontrol \
    sc/source/ui/cctrl/popmenu \
    sc/source/ui/cctrl/tbzoomsliderctrl \
    sc/source/ui/condformat/condformatdlg \
    sc/source/ui/condformat/condformatdlgentry \
    sc/source/ui/condformat/condformathelper \
    sc/source/ui/condformat/colorformat \
    sc/source/ui/dbgui/asciiopt \
    sc/source/ui/dbgui/consdlg \
    sc/source/ui/dbgui/csvcontrol \
    sc/source/ui/dbgui/csvgrid \
    sc/source/ui/dbgui/csvruler \
    sc/source/ui/dbgui/csvsplits \
    sc/source/ui/dbgui/csvtablebox \
    sc/source/ui/dbgui/dbnamdlg \
    sc/source/ui/dbgui/filtdlg \
    sc/source/ui/dbgui/foptmgr \
    sc/source/ui/dbgui/imoptdlg \
    sc/source/ui/dbgui/PivotLayoutDialog \
    sc/source/ui/dbgui/PivotLayoutTreeListBase \
    sc/source/ui/dbgui/PivotLayoutTreeListData \
    sc/source/ui/dbgui/PivotLayoutTreeListLabel \
    sc/source/ui/dbgui/PivotLayoutTreeList \
    sc/source/ui/dbgui/sfiltdlg \
    sc/source/ui/dbgui/validate \
    sc/source/ui/dialogs/searchresults \
    sc/source/ui/docshell/arealink \
    sc/source/ui/docshell/autostyl \
    sc/source/ui/docshell/datastream \
    sc/source/ui/docshell/dbdocfun \
    sc/source/ui/docshell/dbdocimp \
    sc/source/ui/docshell/docfunc \
    sc/source/ui/docshell/docfuncutil \
    sc/source/ui/docshell/docsh \
    sc/source/ui/docshell/docsh2 \
    sc/source/ui/docshell/docsh3 \
    sc/source/ui/docshell/docsh4 \
    sc/source/ui/docshell/docsh5 \
    sc/source/ui/docshell/docsh6 \
    sc/source/ui/docshell/docsh7 \
    sc/source/ui/docshell/docsh8 \
    sc/source/ui/docshell/documentlinkmgr \
    sc/source/ui/docshell/editable \
    sc/source/ui/docshell/externalrefmgr \
    sc/source/ui/docshell/impex \
    sc/source/ui/docshell/macromgr \
    sc/source/ui/docshell/olinefun \
    sc/source/ui/docshell/pagedata \
    sc/source/ui/docshell/pntlock \
    sc/source/ui/docshell/servobj \
    sc/source/ui/docshell/sizedev \
    sc/source/ui/docshell/tablink \
    sc/source/ui/drawfunc/chartsh \
    sc/source/ui/drawfunc/drawsh \
    sc/source/ui/drawfunc/drawsh2 \
    sc/source/ui/drawfunc/drawsh4 \
    sc/source/ui/drawfunc/drawsh5 \
    sc/source/ui/drawfunc/drformsh \
    sc/source/ui/drawfunc/drtxtob \
    sc/source/ui/drawfunc/drtxtob1 \
    sc/source/ui/drawfunc/drtxtob2 \
    sc/source/ui/drawfunc/fuconarc \
    sc/source/ui/drawfunc/fuconcustomshape \
    sc/source/ui/drawfunc/fuconpol \
    sc/source/ui/drawfunc/fuconrec \
    sc/source/ui/drawfunc/fuconstr \
    sc/source/ui/drawfunc/fuconuno \
    sc/source/ui/drawfunc/fudraw \
    sc/source/ui/drawfunc/fuins1 \
    sc/source/ui/drawfunc/fuins2 \
    sc/source/ui/drawfunc/fupoor \
    sc/source/ui/drawfunc/fusel \
    sc/source/ui/drawfunc/fusel2 \
    sc/source/ui/drawfunc/futext \
    sc/source/ui/drawfunc/futext2 \
    sc/source/ui/drawfunc/futext3 \
    sc/source/ui/drawfunc/graphsh \
    sc/source/ui/drawfunc/mediash \
    sc/source/ui/drawfunc/oleobjsh \
    sc/source/ui/formdlg/dwfunctr \
    sc/source/ui/formdlg/formdata \
    sc/source/ui/formdlg/formula \
    sc/source/ui/formdlg/privsplt \
    sc/source/ui/miscdlgs/acredlin \
    sc/source/ui/miscdlgs/anyrefdg \
    sc/source/ui/miscdlgs/autofmt \
    sc/source/ui/miscdlgs/conflictsdlg \
    sc/source/ui/miscdlgs/crnrdlg \
    sc/source/ui/miscdlgs/datastreamdlg \
    sc/source/ui/miscdlgs/highred \
    sc/source/ui/miscdlgs/optsolver \
    sc/source/ui/miscdlgs/protectiondlg \
    sc/source/ui/miscdlgs/redcom \
    sc/source/ui/miscdlgs/retypepassdlg \
    sc/source/ui/miscdlgs/sharedocdlg \
    sc/source/ui/miscdlgs/simpref \
    sc/source/ui/miscdlgs/solveroptions \
    sc/source/ui/miscdlgs/solverutil \
    sc/source/ui/miscdlgs/solvrdlg \
    sc/source/ui/miscdlgs/tabopdlg \
    sc/source/ui/miscdlgs/warnbox \
    sc/source/ui/namedlg/namedefdlg \
    sc/source/ui/namedlg/namedlg \
    sc/source/ui/namedlg/namemgrtable \
    sc/source/ui/navipi/content \
    sc/source/ui/navipi/navcitem \
    sc/source/ui/navipi/navipi \
    sc/source/ui/navipi/scenwnd \
    sc/source/ui/pagedlg/areasdlg \
    sc/source/ui/pagedlg/tphfedit \
    sc/source/ui/sidebar/AlignmentPropertyPanel \
    sc/source/ui/sidebar/CellLineStyleControl \
    sc/source/ui/sidebar/CellLineStylePopup \
    sc/source/ui/sidebar/CellLineStyleValueSet \
    sc/source/ui/sidebar/CellBorderUpdater \
    sc/source/ui/sidebar/CellAppearancePropertyPanel \
    sc/source/ui/sidebar/CellBorderStyleControl \
    sc/source/ui/sidebar/CellBorderStylePopup \
    sc/source/ui/sidebar/NumberFormatPropertyPanel \
    sc/source/ui/sidebar/ScPanelFactory \
    sc/source/ui/StatisticsDialogs/AnalysisOfVarianceDialog \
    sc/source/ui/StatisticsDialogs/CorrelationDialog \
    sc/source/ui/StatisticsDialogs/CovarianceDialog \
    sc/source/ui/StatisticsDialogs/ChiSquareTestDialog \
    sc/source/ui/StatisticsDialogs/DescriptiveStatisticsDialog \
    sc/source/ui/StatisticsDialogs/ExponentialSmoothingDialog \
    sc/source/ui/StatisticsDialogs/FTestDialog \
    sc/source/ui/StatisticsDialogs/MatrixComparisonGenerator \
    sc/source/ui/StatisticsDialogs/MovingAverageDialog \
    sc/source/ui/StatisticsDialogs/RandomNumberGeneratorDialog \
    sc/source/ui/StatisticsDialogs/RegressionDialog \
    sc/source/ui/StatisticsDialogs/SamplingDialog \
    sc/source/ui/StatisticsDialogs/StatisticsInputOutputDialog \
    sc/source/ui/StatisticsDialogs/StatisticsTwoVariableDialog \
    sc/source/ui/StatisticsDialogs/TableFillingAndNavigationTools \
    sc/source/ui/StatisticsDialogs/TTestDialog \
    sc/source/ui/StatisticsDialogs/ZTestDialog \
    sc/source/ui/undo/areasave \
    sc/source/ui/undo/refundo \
    sc/source/ui/undo/target \
    sc/source/ui/undo/undobase \
    sc/source/ui/undo/undoblk \
    sc/source/ui/undo/undoblk2 \
    sc/source/ui/undo/undoblk3 \
    sc/source/ui/undo/undocell \
    sc/source/ui/undo/undocell2 \
    sc/source/ui/undo/undoconvert \
    sc/source/ui/undo/undodat \
    sc/source/ui/undo/undodraw \
    sc/source/ui/undo/undoolk \
    sc/source/ui/undo/undorangename \
    sc/source/ui/undo/undosort \
    sc/source/ui/undo/undostyl \
    sc/source/ui/undo/undotab \
    sc/source/ui/undo/undoutil \
    sc/source/ui/unoobj/ChartRangeSelectionListener \
    sc/source/ui/unoobj/addruno \
    sc/source/ui/unoobj/afmtuno \
    sc/source/ui/unoobj/appluno \
    sc/source/ui/unoobj/celllistsource \
    sc/source/ui/unoobj/cellsuno \
    sc/source/ui/unoobj/cellvaluebinding \
    sc/source/ui/unoobj/chart2uno \
    sc/source/ui/unoobj/chartuno \
    sc/source/ui/unoobj/condformatuno \
    sc/source/ui/unoobj/confuno \
    sc/source/ui/unoobj/convuno \
    sc/source/ui/unoobj/cursuno \
    sc/source/ui/unoobj/dapiuno \
    sc/source/ui/unoobj/datauno \
    sc/source/ui/unoobj/defltuno \
    sc/source/ui/unoobj/dispuno \
    sc/source/ui/unoobj/docuno \
    sc/source/ui/unoobj/drdefuno \
    sc/source/ui/unoobj/editsrc \
    sc/source/ui/unoobj/eventuno \
    sc/source/ui/unoobj/fielduno \
    sc/source/ui/unoobj/filtuno \
    sc/source/ui/unoobj/fmtuno \
    sc/source/ui/unoobj/forbiuno \
    sc/source/ui/unoobj/funcuno \
    sc/source/ui/unoobj/linkuno \
    sc/source/ui/unoobj/listenercalls \
    sc/source/ui/unoobj/miscuno \
    sc/source/ui/unoobj/nameuno \
    sc/source/ui/unoobj/notesuno \
    sc/source/ui/unoobj/optuno \
    sc/source/ui/unoobj/pageuno \
    sc/source/ui/unoobj/servuno \
    sc/source/ui/unoobj/shapeuno \
    sc/source/ui/unoobj/srchuno \
    sc/source/ui/unoobj/styleuno \
    sc/source/ui/unoobj/targuno \
    sc/source/ui/unoobj/textuno \
    sc/source/ui/unoobj/tokenuno \
    sc/source/ui/unoobj/unodoc \
    sc/source/ui/unoobj/unoreflist \
    sc/source/ui/unoobj/viewuno \
    sc/source/ui/unoobj/warnpassword \
    sc/source/ui/view/auditsh \
    sc/source/ui/view/cellmergeoption \
    sc/source/ui/view/cellsh \
    sc/source/ui/view/cellsh1 \
    sc/source/ui/view/cellsh2 \
    sc/source/ui/view/cellsh3 \
    sc/source/ui/view/cellsh4 \
    sc/source/ui/view/colrowba \
    sc/source/ui/view/cliputil \
    sc/source/ui/view/dbfunc \
    sc/source/ui/view/dbfunc2 \
    sc/source/ui/view/dbfunc3 \
    sc/source/ui/view/dbfunc4 \
    sc/source/ui/view/drawutil \
    sc/source/ui/view/drawvie2 \
    sc/source/ui/view/drawvie3 \
    sc/source/ui/view/drawvie4 \
    sc/source/ui/view/drawview \
    sc/source/ui/view/editsh \
    sc/source/ui/view/formatsh \
    sc/source/ui/view/gridmerg \
    sc/source/ui/view/gridwin \
    sc/source/ui/view/gridwin2 \
    sc/source/ui/view/gridwin3 \
    sc/source/ui/view/gridwin4 \
    sc/source/ui/view/gridwin5 \
    sc/source/ui/view/hdrcont \
    sc/source/ui/view/hintwin \
    sc/source/ui/view/imapwrap \
    sc/source/ui/view/invmerge \
    sc/source/ui/view/notemark \
    sc/source/ui/view/olinewin \
    sc/source/ui/view/olkact \
    sc/source/ui/view/output \
    sc/source/ui/view/output2 \
    sc/source/ui/view/output3 \
    sc/source/ui/view/overlayobject \
    sc/source/ui/view/pfuncache \
    sc/source/ui/view/pgbrksh \
    sc/source/ui/view/pivotsh \
    sc/source/ui/view/preview \
    sc/source/ui/view/prevloc \
    sc/source/ui/view/prevwsh \
    sc/source/ui/view/prevwsh2 \
    sc/source/ui/view/printfun \
    sc/source/ui/view/reffact \
    sc/source/ui/view/scextopt \
    sc/source/ui/view/select \
    sc/source/ui/view/selectionstate \
    sc/source/ui/view/spelldialog \
    sc/source/ui/view/spelleng \
    sc/source/ui/view/spellcheckcontext \
    sc/source/ui/view/tabcont \
    sc/source/ui/view/tabsplit \
    sc/source/ui/view/tabview \
    sc/source/ui/view/tabview2 \
    sc/source/ui/view/tabview3 \
    sc/source/ui/view/tabview4 \
    sc/source/ui/view/tabview5 \
    sc/source/ui/view/tabvwsh \
    sc/source/ui/view/tabvwsh2 \
    sc/source/ui/view/tabvwsh3 \
    sc/source/ui/view/tabvwsh4 \
    sc/source/ui/view/tabvwsh5 \
    sc/source/ui/view/tabvwsh8 \
    sc/source/ui/view/tabvwsh9 \
    sc/source/ui/view/tabvwsha \
    sc/source/ui/view/tabvwshb \
    sc/source/ui/view/tabvwshc \
    sc/source/ui/view/tabvwshd \
    sc/source/ui/view/tabvwshe \
    sc/source/ui/view/tabvwshf \
    sc/source/ui/view/tabvwshg \
    sc/source/ui/view/tabvwshh \
    sc/source/ui/view/viewdata \
    sc/source/ui/view/viewfun2 \
    sc/source/ui/view/viewfun3 \
    sc/source/ui/view/viewfun4 \
    sc/source/ui/view/viewfun5 \
    sc/source/ui/view/viewfun6 \
    sc/source/ui/view/viewfun7 \
    sc/source/ui/view/viewfunc \
    sc/source/ui/view/viewutil \
    sc/source/ui/view/waitoff \
    sc/source/ui/xmlsource/xmlsourcedlg \
))

ifneq (,$(gb_ENABLE_DBGUTIL))
$(eval $(call gb_Library_add_exception_objects,sc,\
    sc/source/ui/view/gridwin_dbgutil \
))
endif

$(eval $(call gb_Helper_optional,OPENCL,\
$(call gb_Library_add_exception_objects,sc,\
    sc/source/core/opencl/formulagroupcl \
    sc/source/core/opencl/opbase \
    sc/source/core/opencl/op_financial \
    sc/source/core/opencl/op_database \
    sc/source/core/opencl/op_math \
    sc/source/core/opencl/op_addin \
    sc/source/core/opencl/op_statistical \
    sc/source/core/opencl/op_array \
    sc/source/core/opencl/op_logical \
    sc/source/core/opencl/op_spreadsheet \
)))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,sc,\
    -lrt \
))
endif

$(eval $(call gb_SdiTarget_SdiTarget,sc/sdi/scslots,sc/sdi/scalc))

$(eval $(call gb_SdiTarget_set_include,sc/sdi/scslots,\
    -I$(SRCDIR)/sc/inc \
    -I$(SRCDIR)/sc/sdi \
    -I$(SRCDIR)/svx/sdi \
    -I$(SRCDIR)/sfx2/sdi \
    $$(INCLUDE) \
))

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,sc,sc))

# vim: set noet sw=4 ts=4:
