# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Jar_Jar,sun-report-builder))

$(eval $(call gb_Jar_use_jars,sun-report-builder,\
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
))

$(eval $(call gb_Jar_use_externals,sun-report-builder,\
	commons-logging \
	flow-engine \
	flute \
	libbase \
	libfonts \
	libformula \
	liblayout \
	libloader \
	librepository \
	libserializer \
	libxml \
	sac \
))

$(eval $(call gb_Jar_set_manifest,sun-report-builder,$(SRCDIR)/reportbuilder/util/manifest.mf))

$(eval $(call gb_Jar_set_packageroot,sun-report-builder,com))

$(eval $(call gb_Jar_add_sourcefiles,sun-report-builder,\
    reportbuilder/java/com/sun/star/report/DataRow \
    reportbuilder/java/com/sun/star/report/DataSource \
    reportbuilder/java/com/sun/star/report/DataSourceException \
    reportbuilder/java/com/sun/star/report/DataSourceFactory \
    reportbuilder/java/com/sun/star/report/ImageService \
    reportbuilder/java/com/sun/star/report/InputRepository \
    reportbuilder/java/com/sun/star/report/JobDefinitionException \
    reportbuilder/java/com/sun/star/report/JobProgressIndicator \
    reportbuilder/java/com/sun/star/report/JobProperties \
    reportbuilder/java/com/sun/star/report/OfficeToken \
    reportbuilder/java/com/sun/star/report/OutputRepository \
    reportbuilder/java/com/sun/star/report/ParameterMap \
    reportbuilder/java/com/sun/star/report/ReportAddIn \
    reportbuilder/java/com/sun/star/report/ReportEngine \
    reportbuilder/java/com/sun/star/report/ReportEngineMetaData \
    reportbuilder/java/com/sun/star/report/ReportEngineParameterNames \
    reportbuilder/java/com/sun/star/report/ReportExecutionException \
    reportbuilder/java/com/sun/star/report/ReportExpression \
    reportbuilder/java/com/sun/star/report/ReportExpressionMetaData \
    reportbuilder/java/com/sun/star/report/ReportFunction \
    reportbuilder/java/com/sun/star/report/ReportJob \
    reportbuilder/java/com/sun/star/report/ReportJobDefinition \
    reportbuilder/java/com/sun/star/report/ReportJobFactory \
    reportbuilder/java/com/sun/star/report/SDBCReportData \
    reportbuilder/java/com/sun/star/report/SDBCReportDataFactory \
    reportbuilder/java/com/sun/star/report/SOImageService \
    reportbuilder/java/com/sun/star/report/StorageRepository \
    reportbuilder/java/com/sun/star/report/function/metadata/AuthorFunction \
    reportbuilder/java/com/sun/star/report/function/metadata/AuthorFunctionDescription \
    reportbuilder/java/com/sun/star/report/function/metadata/MetaDataFunctionCategory \
    reportbuilder/java/com/sun/star/report/function/metadata/TitleFunction \
    reportbuilder/java/com/sun/star/report/function/metadata/TitleFunctionDescription \
    reportbuilder/java/com/sun/star/report/pentaho/DefaultNameGenerator \
    reportbuilder/java/com/sun/star/report/pentaho/OfficeNamespaces \
    reportbuilder/java/com/sun/star/report/pentaho/PentahoFormulaContext \
    reportbuilder/java/com/sun/star/report/pentaho/PentahoReportAddIn \
    reportbuilder/java/com/sun/star/report/pentaho/PentahoReportEngine \
    reportbuilder/java/com/sun/star/report/pentaho/PentahoReportEngineMetaData \
    reportbuilder/java/com/sun/star/report/pentaho/PentahoReportJob \
    reportbuilder/java/com/sun/star/report/pentaho/SOFormulaOpCodeMapper \
    reportbuilder/java/com/sun/star/report/pentaho/SOFormulaParser \
    reportbuilder/java/com/sun/star/report/pentaho/SOFunctionManager \
    reportbuilder/java/com/sun/star/report/pentaho/SOReportJobFactory \
    reportbuilder/java/com/sun/star/report/pentaho/StarFunctionCategory \
    reportbuilder/java/com/sun/star/report/pentaho/StarFunctionDescription \
    reportbuilder/java/com/sun/star/report/pentaho/StarReportData \
    reportbuilder/java/com/sun/star/report/pentaho/StarReportDataFactory \
    reportbuilder/java/com/sun/star/report/pentaho/StarReportModule \
    reportbuilder/java/com/sun/star/report/pentaho/expressions/SumExpression \
    reportbuilder/java/com/sun/star/report/pentaho/expressions/SumExpressionMetaData \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/AbstractReportElementLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/FixedTextLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/FormatValueUtility \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/FormattedTextLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/ImageElementContext \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/ImageElementLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/ObjectOleLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeDetailLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeGroupInstanceSectionLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeGroupLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeGroupSectionLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficePageSectionLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeRepeatingStructureLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeReportLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeTableLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/OfficeTableTemplateLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/TableCellLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/VariablesCollection \
    reportbuilder/java/com/sun/star/report/pentaho/layoutprocessor/VariablesDeclarationLayoutController \
    reportbuilder/java/com/sun/star/report/pentaho/loader/InputRepositoryLoader \
    reportbuilder/java/com/sun/star/report/pentaho/loader/InputRepositoryResourceData \
    reportbuilder/java/com/sun/star/report/pentaho/loader/InputResourceKey \
    reportbuilder/java/com/sun/star/report/pentaho/model/DataStyle \
    reportbuilder/java/com/sun/star/report/pentaho/model/FixedTextElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/FontFaceDeclsSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/FontFaceElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/FormatCondition \
    reportbuilder/java/com/sun/star/report/pentaho/model/FormattedTextElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/ImageElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/ObjectOleElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeDetailSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeDocument \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeGroup \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeGroupInstanceSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeGroupSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeMasterPage \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeMasterStyles \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeReport \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeStyle \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeStyles \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeStylesCollection \
    reportbuilder/java/com/sun/star/report/pentaho/model/OfficeTableSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/PageLayout \
    reportbuilder/java/com/sun/star/report/pentaho/model/PageSection \
    reportbuilder/java/com/sun/star/report/pentaho/model/RawText \
    reportbuilder/java/com/sun/star/report/pentaho/model/ReportElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/TableCellElement \
    reportbuilder/java/com/sun/star/report/pentaho/model/VariablesDeclarationSection \
    reportbuilder/java/com/sun/star/report/pentaho/output/ImageProducer \
    reportbuilder/java/com/sun/star/report/pentaho/output/OfficeDocumentReportTarget \
    reportbuilder/java/com/sun/star/report/pentaho/output/OleProducer \
    reportbuilder/java/com/sun/star/report/pentaho/output/StyleUtilities \
    reportbuilder/java/com/sun/star/report/pentaho/output/StylesWriter \
    reportbuilder/java/com/sun/star/report/pentaho/output/chart/ChartRawReportProcessor \
    reportbuilder/java/com/sun/star/report/pentaho/output/chart/ChartRawReportTarget \
    reportbuilder/java/com/sun/star/report/pentaho/output/spreadsheet/SpreadsheetRawReportProcessor \
    reportbuilder/java/com/sun/star/report/pentaho/output/spreadsheet/SpreadsheetRawReportTarget \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/MasterPageFactory \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/PageBreakDefinition \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/PageContext \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/TextRawReportProcessor \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/TextRawReportTarget \
    reportbuilder/java/com/sun/star/report/pentaho/output/text/VariablesDeclarations \
    reportbuilder/java/com/sun/star/report/pentaho/parser/AttributeSpecification \
    reportbuilder/java/com/sun/star/report/pentaho/parser/ElementReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/OfficeDocumentXmlResourceFactory \
    reportbuilder/java/com/sun/star/report/pentaho/parser/OfficeParserUtil \
    reportbuilder/java/com/sun/star/report/pentaho/parser/OfficeStylesXmlResourceFactory \
    reportbuilder/java/com/sun/star/report/pentaho/parser/StarStyleXmlFactoryModule \
    reportbuilder/java/com/sun/star/report/pentaho/parser/StarXmlFactoryModule \
    reportbuilder/java/com/sun/star/report/pentaho/parser/StyleMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/chart/ChartReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/data/DataStyleReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/draw/ObjectOleReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/office/BodyReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/office/DocumentContentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/office/DocumentStylesReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/office/FontFaceDeclsReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/office/MasterStylesReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/ConditionalPrintExpressionReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/DetailRootTableReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/FixedContentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/FormatConditionReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/FormattedTextReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/FunctionReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/GroupReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/GroupSectionReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/ImageReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/MasterDetailReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/ReportElementReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/ReportReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/RootTableReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/rpt/SubDocumentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/FontFaceReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/MasterPageReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/OfficeStyleReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/OfficeStylesReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/PageLayoutReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/style/StyleDefinitionReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/OneOfConstantsMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/draw/TextAreaVerticalAlignMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/BackgroundColorMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/BorderRightMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/ColorMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/FontSizeMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/FontStyleMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/FontWeightMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/fo/TextAlignMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/FontFamilyGenericMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/FontFamilyMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/FontNameMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/FontPitchMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/FontReliefMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/TextEmphasizeMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/TextUnderlineColorMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/TextUnderlineStyleMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/TextUnderlineWidthMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/TextUnderlineWordMode \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/style/VerticalAlignMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/table/ColumnWidthMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/stylemapper/table/RowHeightMapper \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/CoveredCellReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableCellReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableColumnReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableColumnsReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableRowReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/table/TableRowsReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/text/NoCDATATextContentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/text/TextContentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/parser/xlink/XLinkReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/styles/LengthCalculator \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMapper \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMapperKey \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMapperXmlFactoryModule \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMapperXmlResourceFactory \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMappingDocumentReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMappingReadHandler \
    reportbuilder/java/com/sun/star/report/pentaho/styles/StyleMappingRule \
    reportbuilder/java/com/sun/star/report/util/DefaultJobProperties \
    reportbuilder/java/com/sun/star/report/util/DefaultParameterMap \
    reportbuilder/java/com/sun/star/report/util/DefaultReportJobDefinition \
    reportbuilder/java/com/sun/star/report/util/ManifestWriter \
))

$(eval $(call gb_Jar_add_packagefiles,sun-report-builder,,\
	$(SRCDIR)/reportbuilder/java/jfreereport.properties \
	$(SRCDIR)/reportbuilder/java/libformula.properties \
	$(SRCDIR)/reportbuilder/java/loader.properties \
))

$(eval $(call gb_Jar_add_packagefiles,sun-report-builder,com/sun/star/report/function/metadata,\
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/Author-Function.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/Author-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/Title-Function.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/Title-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/category.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/function/metadata/category_en_US.properties \
))

$(eval $(call gb_Jar_add_packagefiles,sun-report-builder,com/sun/star/report/pentaho,\
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/configuration.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/module.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-datastyle.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-draw.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-form.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-style.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-table.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/oasis-text.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/smil.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/star-office.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/star-report.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/star-rpt.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/svg.css \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/xsl-fo.css \
))

$(eval $(call gb_Jar_add_packagefiles,sun-report-builder,com/sun/star/report/pentaho/parser,\
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/parser/rpt-schema-v1.0-os.xsd \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/parser/selectors.properties \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/parser/style-mapping.txt \
))

$(eval $(call gb_Jar_add_packagefiles,sun-report-builder,com/sun/star/report/pentaho/styles,\
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/styles/stylemapper.xml \
	$(SRCDIR)/reportbuilder/java/com/sun/star/report/pentaho/styles/stylemapper.xsd \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
