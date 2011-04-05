#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,oox))

$(eval $(call gb_Library_add_package_headers,oox,oox_inc))
$(eval $(call gb_Library_add_package_headers,oox,oox_source))
$(eval $(call gb_Library_add_package_headers,oox,oox_generated))

$(eval $(call gb_Library_set_componentfile,oox,oox/oox))

$(eval $(call gb_Library_set_include,oox,\
	-I$(SRCDIR)/oox/inc \
	-I$(OUTDIR)/inc \
	-I$(OUTDIR)/inc/offuh \
	-I$(WORKDIR)/CustomTarget/oox/source \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_defs,oox,\
	$$(DEFS) \
	-DOOX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,oox,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	xcr \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(SYSTEM_OPENSSL),YES)
$(eval $(call gb_Library_add_linked_libs,oox,\
	crypto \
	ssl \
))
else
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,oox,\
	crypto \
	ssl \
))
else
$(eval $(call gb_Library_add_linked_static_libs,oox,\
	crypto \
	ssl \
))
endif
endif

# this is for openssl
ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_linked_libs,oox,\
	nsl \
	socket \
))
endif
#probably not necessary:	dl \

# object files ----------------------------------------------------------------

$(eval $(call gb_Library_add_exception_objects,oox,\
	oox/source/core/binarycodec \
	oox/source/core/binaryfilterbase \
	oox/source/core/contexthandler \
	oox/source/core/contexthandler2 \
	oox/source/core/fastparser \
	oox/source/core/fasttokenhandler \
	oox/source/core/filterbase \
	oox/source/core/filterdetect \
	oox/source/core/fragmenthandler \
	oox/source/core/fragmenthandler2 \
	oox/source/core/recordparser \
	oox/source/core/relations \
	oox/source/core/relationshandler \
	oox/source/core/services \
	oox/source/core/xmlfilterbase \
	oox/source/docprop/docprophandler \
	oox/source/docprop/ooxmldocpropimport \
	oox/source/drawingml/chart/axiscontext \
	oox/source/drawingml/chart/axisconverter \
	oox/source/drawingml/chart/axismodel \
	oox/source/drawingml/chart/chartcontextbase \
	oox/source/drawingml/chart/chartconverter \
	oox/source/drawingml/chart/chartdrawingfragment \
	oox/source/drawingml/chart/chartspaceconverter \
	oox/source/drawingml/chart/chartspacefragment \
	oox/source/drawingml/chart/chartspacemodel \
	oox/source/drawingml/chart/converterbase \
	oox/source/drawingml/chart/datasourcecontext \
	oox/source/drawingml/chart/datasourceconverter \
	oox/source/drawingml/chart/datasourcemodel \
	oox/source/drawingml/chart/modelbase \
	oox/source/drawingml/chart/objectformatter \
	oox/source/drawingml/chart/plotareacontext \
	oox/source/drawingml/chart/plotareaconverter \
	oox/source/drawingml/chart/plotareamodel \
	oox/source/drawingml/chart/seriescontext \
	oox/source/drawingml/chart/seriesconverter \
	oox/source/drawingml/chart/seriesmodel \
	oox/source/drawingml/chart/titlecontext \
	oox/source/drawingml/chart/titleconverter \
	oox/source/drawingml/chart/titlemodel \
	oox/source/drawingml/chart/typegroupcontext \
	oox/source/drawingml/chart/typegroupconverter \
	oox/source/drawingml/chart/typegroupmodel \
	oox/source/drawingml/clrscheme \
	oox/source/drawingml/clrschemecontext \
	oox/source/drawingml/color \
	oox/source/drawingml/colorchoicecontext \
	oox/source/drawingml/connectorshapecontext \
	oox/source/drawingml/customshapegeometry \
	oox/source/drawingml/customshapeproperties \
	oox/source/drawingml/diagram/datamodelcontext \
	oox/source/drawingml/diagram/diagram \
	oox/source/drawingml/diagram/diagramdefinitioncontext \
	oox/source/drawingml/diagram/diagramfragmenthandler \
	oox/source/drawingml/diagram/diagramlayoutatoms \
	oox/source/drawingml/diagram/layoutnodecontext \
	oox/source/drawingml/drawingmltypes \
	oox/source/drawingml/embeddedwavaudiofile \
	oox/source/drawingml/fillproperties \
	oox/source/drawingml/fillpropertiesgroupcontext \
	oox/source/drawingml/graphicshapecontext \
	oox/source/drawingml/guidcontext \
	oox/source/drawingml/hyperlinkcontext \
	oox/source/drawingml/lineproperties \
	oox/source/drawingml/linepropertiescontext \
	oox/source/drawingml/objectdefaultcontext \
	oox/source/drawingml/shape \
	oox/source/drawingml/shapecontext \
	oox/source/drawingml/shapegroupcontext \
	oox/source/drawingml/shapepropertiescontext \
	oox/source/drawingml/shapepropertymap \
	oox/source/drawingml/shapestylecontext \
	oox/source/drawingml/spdefcontext \
	oox/source/drawingml/table/tablebackgroundstylecontext \
	oox/source/drawingml/table/tablecell \
	oox/source/drawingml/table/tablecellcontext \
	oox/source/drawingml/table/tablecontext \
	oox/source/drawingml/table/tablepartstylecontext \
	oox/source/drawingml/table/tableproperties \
	oox/source/drawingml/table/tablerow \
	oox/source/drawingml/table/tablerowcontext \
	oox/source/drawingml/table/tablestyle \
	oox/source/drawingml/table/tablestylecellstylecontext \
	oox/source/drawingml/table/tablestylecontext \
	oox/source/drawingml/table/tablestylelist \
	oox/source/drawingml/table/tablestylelistfragmenthandler \
	oox/source/drawingml/table/tablestylepart \
	oox/source/drawingml/table/tablestyletextstylecontext \
	oox/source/drawingml/textbody \
	oox/source/drawingml/textbodycontext \
	oox/source/drawingml/textbodyproperties \
	oox/source/drawingml/textbodypropertiescontext \
	oox/source/drawingml/textcharacterproperties \
	oox/source/drawingml/textcharacterpropertiescontext \
	oox/source/drawingml/textfield \
	oox/source/drawingml/textfieldcontext \
	oox/source/drawingml/textfont \
	oox/source/drawingml/textliststyle \
	oox/source/drawingml/textliststylecontext \
	oox/source/drawingml/textparagraph \
	oox/source/drawingml/textparagraphproperties \
	oox/source/drawingml/textparagraphpropertiescontext \
	oox/source/drawingml/textrun \
	oox/source/drawingml/textspacingcontext \
	oox/source/drawingml/texttabstoplistcontext \
	oox/source/drawingml/theme \
	oox/source/drawingml/themeelementscontext \
	oox/source/drawingml/themefragmenthandler \
	oox/source/drawingml/transform2dcontext \
	oox/source/dump/biffdumper \
	oox/source/dump/dffdumper \
	oox/source/dump/dumperbase \
	oox/source/dump/oledumper \
	oox/source/dump/pptxdumper \
	oox/source/dump/xlsbdumper \
	oox/source/helper/attributelist \
	oox/source/helper/binaryinputstream \
	oox/source/helper/binaryoutputstream \
	oox/source/helper/binarystreambase \
	oox/source/helper/containerhelper \
	oox/source/helper/graphichelper \
	oox/source/helper/modelobjecthelper \
	oox/source/helper/progressbar \
	oox/source/helper/propertymap \
	oox/source/helper/propertyset \
	oox/source/helper/storagebase \
	oox/source/helper/textinputstream \
	oox/source/helper/zipstorage \
	oox/source/ole/axbinaryreader \
	oox/source/ole/axcontrol \
	oox/source/ole/axcontrolfragment \
	oox/source/ole/olehelper \
	oox/source/ole/oleobjecthelper \
	oox/source/ole/olestorage \
	oox/source/ole/vbacontrol \
	oox/source/ole/vbahelper \
	oox/source/ole/vbainputstream \
	oox/source/ole/vbamodule \
	oox/source/ole/vbaproject \
	oox/source/ole/vbaprojectfilter \
	oox/source/ppt/animationspersist \
	oox/source/ppt/animationtypes \
	oox/source/ppt/animvariantcontext \
	oox/source/ppt/backgroundproperties \
	oox/source/ppt/buildlistcontext \
	oox/source/ppt/commonbehaviorcontext \
	oox/source/ppt/commontimenodecontext \
	oox/source/ppt/conditioncontext \
	oox/source/ppt/customshowlistcontext \
	oox/source/ppt/headerfootercontext \
	oox/source/ppt/layoutfragmenthandler \
	oox/source/ppt/pptfilterhelpers \
	oox/source/ppt/pptimport \
	oox/source/ppt/pptshape \
	oox/source/ppt/pptshapecontext \
	oox/source/ppt/pptshapegroupcontext \
	oox/source/ppt/pptshapepropertiescontext \
	oox/source/ppt/presentationfragmenthandler \
	oox/source/ppt/slidefragmenthandler \
	oox/source/ppt/slidemastertextstylescontext \
	oox/source/ppt/slidepersist \
	oox/source/ppt/slidetimingcontext \
	oox/source/ppt/slidetransition \
	oox/source/ppt/slidetransitioncontext \
	oox/source/ppt/soundactioncontext \
	oox/source/ppt/timeanimvaluecontext \
	oox/source/ppt/timenode \
	oox/source/ppt/timenodelistcontext \
	oox/source/ppt/timetargetelementcontext \
	oox/source/shape/ShapeContextHandler \
	oox/source/shape/ShapeFilterBase \
	oox/source/token/namespacemap \
	oox/source/token/propertynames \
	oox/source/token/tokenmap \
	oox/source/vml/vmldrawing \
	oox/source/vml/vmldrawingfragment \
	oox/source/vml/vmlformatting \
	oox/source/vml/vmlinputstream \
	oox/source/vml/vmlshape \
	oox/source/vml/vmlshapecontainer \
	oox/source/vml/vmlshapecontext \
	oox/source/vml/vmltextbox \
	oox/source/vml/vmltextboxcontext \
	oox/source/xls/addressconverter \
	oox/source/xls/autofilterbuffer \
	oox/source/xls/autofiltercontext \
	oox/source/xls/biffcodec \
	oox/source/xls/biffdetector \
	oox/source/xls/biffhelper \
	oox/source/xls/biffinputstream \
	oox/source/xls/biffoutputstream \
	oox/source/xls/chartsheetfragment \
	oox/source/xls/commentsbuffer \
	oox/source/xls/commentsfragment \
	oox/source/xls/condformatbuffer \
	oox/source/xls/condformatcontext \
	oox/source/xls/connectionsbuffer \
	oox/source/xls/connectionsfragment \
	oox/source/xls/defnamesbuffer \
	oox/source/xls/drawingbase \
	oox/source/xls/drawingfragment \
	oox/source/xls/drawingmanager \
	oox/source/xls/excelchartconverter \
	oox/source/xls/excelfilter \
	oox/source/xls/excelhandlers \
	oox/source/xls/excelvbaproject \
	oox/source/xls/externallinkbuffer \
	oox/source/xls/externallinkfragment \
	oox/source/xls/formulabase \
	oox/source/xls/formulaparser \
	oox/source/xls/numberformatsbuffer \
	oox/source/xls/ooxformulaparser \
	oox/source/xls/pagesettings \
	oox/source/xls/pivotcachebuffer \
	oox/source/xls/pivotcachefragment \
	oox/source/xls/pivottablebuffer \
	oox/source/xls/pivottablefragment \
	oox/source/xls/querytablebuffer \
	oox/source/xls/querytablefragment \
	oox/source/xls/richstring \
	oox/source/xls/richstringcontext \
	oox/source/xls/scenariobuffer \
	oox/source/xls/scenariocontext \
	oox/source/xls/sharedstringsbuffer \
	oox/source/xls/sharedstringsfragment \
	oox/source/xls/sheetdatabuffer \
	oox/source/xls/sheetdatacontext \
	oox/source/xls/stylesbuffer \
	oox/source/xls/stylesfragment \
	oox/source/xls/tablebuffer \
	oox/source/xls/tablefragment \
	oox/source/xls/themebuffer \
	oox/source/xls/unitconverter \
	oox/source/xls/viewsettings \
	oox/source/xls/workbookfragment \
	oox/source/xls/workbookhelper \
	oox/source/xls/workbooksettings \
	oox/source/xls/worksheetbuffer \
	oox/source/xls/worksheetfragment \
	oox/source/xls/worksheethelper \
	oox/source/xls/worksheetsettings \
))

# vim: set noet sw=4 ts=4:
