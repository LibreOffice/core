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

$(eval $(call gb_Jar_Jar,report-builder))

$(eval $(call gb_Jar_use_jars,report-builder,\
	java_uno \
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_use_externals,report-builder,\
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

$(eval $(call gb_Jar_set_manifest,report-builder,$(SRCDIR)/reportbuilder/util/manifest.mf))

$(eval $(call gb_Jar_set_packageroot,report-builder,org))

$(eval $(call gb_Jar_add_sourcefiles,report-builder,\
    reportbuilder/java/org/libreoffice/report/DataRow \
    reportbuilder/java/org/libreoffice/report/DataSource \
    reportbuilder/java/org/libreoffice/report/DataSourceException \
    reportbuilder/java/org/libreoffice/report/DataSourceFactory \
    reportbuilder/java/org/libreoffice/report/ImageService \
    reportbuilder/java/org/libreoffice/report/InputRepository \
    reportbuilder/java/org/libreoffice/report/JobDefinitionException \
    reportbuilder/java/org/libreoffice/report/JobProgressIndicator \
    reportbuilder/java/org/libreoffice/report/JobProperties \
    reportbuilder/java/org/libreoffice/report/OfficeToken \
    reportbuilder/java/org/libreoffice/report/OutputRepository \
    reportbuilder/java/org/libreoffice/report/ParameterMap \
    reportbuilder/java/org/libreoffice/report/ReportAddIn \
    reportbuilder/java/org/libreoffice/report/ReportEngineMetaData \
    reportbuilder/java/org/libreoffice/report/ReportEngineParameterNames \
    reportbuilder/java/org/libreoffice/report/ReportExecutionException \
    reportbuilder/java/org/libreoffice/report/ReportExpression \
    reportbuilder/java/org/libreoffice/report/ReportExpressionMetaData \
    reportbuilder/java/org/libreoffice/report/ReportFunction \
    reportbuilder/java/org/libreoffice/report/ReportJob \
    reportbuilder/java/org/libreoffice/report/ReportJobDefinition \
    reportbuilder/java/org/libreoffice/report/ReportJobFactory \
    reportbuilder/java/org/libreoffice/report/SDBCReportData \
    reportbuilder/java/org/libreoffice/report/SDBCReportDataFactory \
    reportbuilder/java/org/libreoffice/report/SOImageService \
    reportbuilder/java/org/libreoffice/report/StorageRepository \
    reportbuilder/java/org/libreoffice/report/function/metadata/AuthorFunction \
    reportbuilder/java/org/libreoffice/report/function/metadata/AuthorFunctionDescription \
    reportbuilder/java/org/libreoffice/report/function/metadata/MetaDataFunctionCategory \
    reportbuilder/java/org/libreoffice/report/function/metadata/TitleFunction \
    reportbuilder/java/org/libreoffice/report/function/metadata/TitleFunctionDescription \
    reportbuilder/java/org/libreoffice/report/pentaho/DefaultNameGenerator \
    reportbuilder/java/org/libreoffice/report/pentaho/OfficeNamespaces \
    reportbuilder/java/org/libreoffice/report/pentaho/PentahoFormulaContext \
    reportbuilder/java/org/libreoffice/report/pentaho/PentahoReportAddIn \
    reportbuilder/java/org/libreoffice/report/pentaho/PentahoReportEngine \
    reportbuilder/java/org/libreoffice/report/pentaho/PentahoReportEngineMetaData \
    reportbuilder/java/org/libreoffice/report/pentaho/PentahoReportJob \
    reportbuilder/java/org/libreoffice/report/pentaho/SOFormulaOpCodeMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/SOFormulaParser \
    reportbuilder/java/org/libreoffice/report/pentaho/SOFunctionManager \
    reportbuilder/java/org/libreoffice/report/pentaho/SOReportJobFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/StarFunctionCategory \
    reportbuilder/java/org/libreoffice/report/pentaho/StarFunctionDescription \
    reportbuilder/java/org/libreoffice/report/pentaho/StarReportData \
    reportbuilder/java/org/libreoffice/report/pentaho/StarReportDataFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/StarReportModule \
    reportbuilder/java/org/libreoffice/report/pentaho/expressions/SumExpression \
    reportbuilder/java/org/libreoffice/report/pentaho/expressions/SumExpressionMetaData \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/AbstractReportElementLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/FixedTextLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/FormatValueUtility \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/FormattedTextLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/ImageElementContext \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/ImageElementLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/ObjectOleLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeDetailLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeGroupInstanceSectionLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeGroupLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeGroupSectionLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficePageSectionLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeRepeatingStructureLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeReportLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeTableLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/OfficeTableTemplateLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/TableCellLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/VariablesCollection \
    reportbuilder/java/org/libreoffice/report/pentaho/layoutprocessor/VariablesDeclarationLayoutController \
    reportbuilder/java/org/libreoffice/report/pentaho/loader/InputRepositoryLoader \
    reportbuilder/java/org/libreoffice/report/pentaho/loader/InputRepositoryResourceData \
    reportbuilder/java/org/libreoffice/report/pentaho/loader/InputResourceKey \
    reportbuilder/java/org/libreoffice/report/pentaho/model/DataStyle \
    reportbuilder/java/org/libreoffice/report/pentaho/model/FixedTextElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/FontFaceDeclsSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/FontFaceElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/FormatCondition \
    reportbuilder/java/org/libreoffice/report/pentaho/model/FormattedTextElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/ImageElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/ObjectOleElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeDetailSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeDocument \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeGroup \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeGroupInstanceSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeGroupSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeMasterPage \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeMasterStyles \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeReport \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeStyle \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeStyles \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeStylesCollection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/OfficeTableSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/PageLayout \
    reportbuilder/java/org/libreoffice/report/pentaho/model/PageSection \
    reportbuilder/java/org/libreoffice/report/pentaho/model/RawText \
    reportbuilder/java/org/libreoffice/report/pentaho/model/ReportElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/TableCellElement \
    reportbuilder/java/org/libreoffice/report/pentaho/model/VariablesDeclarationSection \
    reportbuilder/java/org/libreoffice/report/pentaho/output/ImageProducer \
    reportbuilder/java/org/libreoffice/report/pentaho/output/OfficeDocumentReportTarget \
    reportbuilder/java/org/libreoffice/report/pentaho/output/OleProducer \
    reportbuilder/java/org/libreoffice/report/pentaho/output/StyleUtilities \
    reportbuilder/java/org/libreoffice/report/pentaho/output/StylesWriter \
    reportbuilder/java/org/libreoffice/report/pentaho/output/chart/ChartRawReportProcessor \
    reportbuilder/java/org/libreoffice/report/pentaho/output/chart/ChartRawReportTarget \
    reportbuilder/java/org/libreoffice/report/pentaho/output/spreadsheet/SpreadsheetRawReportProcessor \
    reportbuilder/java/org/libreoffice/report/pentaho/output/spreadsheet/SpreadsheetRawReportTarget \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/MasterPageFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/PageBreakDefinition \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/PageContext \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/TextRawReportProcessor \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/TextRawReportTarget \
    reportbuilder/java/org/libreoffice/report/pentaho/output/text/VariablesDeclarations \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/AttributeSpecification \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/ElementReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/OfficeDocumentXmlResourceFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/OfficeParserUtil \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/OfficeStylesXmlResourceFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/StarStyleXmlFactoryModule \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/StarXmlFactoryModule \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/StyleMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/chart/ChartReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/data/DataStyleReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/draw/ObjectOleReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/office/BodyReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/office/DocumentContentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/office/DocumentStylesReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/office/FontFaceDeclsReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/office/MasterStylesReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/ConditionalPrintExpressionReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/DetailRootTableReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/FixedContentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/FormatConditionReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/FormattedTextReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/FunctionReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/GroupReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/GroupSectionReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/ImageReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/MasterDetailReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/ReportElementReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/ReportReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/RootTableReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt/SubDocumentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/FontFaceReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/MasterPageReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/OfficeStyleReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/OfficeStylesReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/PageLayoutReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/style/StyleDefinitionReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/OneOfConstantsMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/draw/TextAreaVerticalAlignMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/BackgroundColorMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/BorderRightMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/ColorMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/FontSizeMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/FontStyleMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/FontWeightMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/fo/TextAlignMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/FontFamilyGenericMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/FontFamilyMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/FontNameMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/FontPitchMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/FontReliefMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/TextEmphasizeMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/TextUnderlineColorMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/TextUnderlineStyleMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/TextUnderlineWidthMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/TextUnderlineWordMode \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/style/VerticalAlignMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/table/ColumnWidthMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/stylemapper/table/RowHeightMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/CoveredCellReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableCellReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableColumnReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableColumnsReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableRowReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/table/TableRowsReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/text/NoCDATATextContentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/text/TextContentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/parser/xlink/XLinkReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/LengthCalculator \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMapper \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMapperKey \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMapperXmlFactoryModule \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMapperXmlResourceFactory \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMappingDocumentReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMappingReadHandler \
    reportbuilder/java/org/libreoffice/report/pentaho/styles/StyleMappingRule \
    reportbuilder/java/org/libreoffice/report/util/DefaultJobProperties \
    reportbuilder/java/org/libreoffice/report/util/DefaultParameterMap \
    reportbuilder/java/org/libreoffice/report/util/DefaultReportJobDefinition \
    reportbuilder/java/org/libreoffice/report/util/ManifestWriter \
))

$(eval $(call gb_Jar_add_packagefiles,report-builder,,\
	$(SRCDIR)/reportbuilder/java/jfreereport.properties \
	$(SRCDIR)/reportbuilder/java/libformula.properties \
	$(SRCDIR)/reportbuilder/java/loader.properties \
))

$(eval $(call gb_Jar_add_packagefiles,report-builder,org/libreoffice/report/function/metadata,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/Author-Function.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/Author-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/Title-Function.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/Title-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/category.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/function/metadata/category_en_US.properties \
))

$(eval $(call gb_Jar_add_packagefiles,report-builder,org/libreoffice/report/pentaho,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/configuration.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/module.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-datastyle.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-draw.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-form.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-style.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-table.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/oasis-text.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/smil.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/star-office.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/star-report.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/star-rpt.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/svg.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/xsl-fo.css \
))

$(eval $(call gb_Jar_add_packagefiles,report-builder,org/libreoffice/report/pentaho/parser,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/parser/rpt-schema-v1.0-os.xsd \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/parser/selectors.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/parser/style-mapping.txt \
))

$(eval $(call gb_Jar_add_packagefiles,report-builder,org/libreoffice/report/pentaho/styles,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/styles/stylemapper.xml \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/report/pentaho/styles/stylemapper.xsd \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
