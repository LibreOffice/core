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

$(eval $(call gb_Jar_Jar,lo-report-builder))

$(eval $(call gb_Jar_use_jars,lo-report-builder,\
	java_uno \
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_use_externals,lo-report-builder,\
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

$(eval $(call gb_Jar_set_manifest,lo-report-builder,$(SRCDIR)/reportbuilder/util/manifest.mf))

$(eval $(call gb_Jar_set_packageroot,lo-report-builder,org))

$(eval $(call gb_Jar_add_sourcefiles,lo-report-builder,\
    reportbuilder/java/org/libreoffice/ext/report/DataRow \
    reportbuilder/java/org/libreoffice/ext/report/DataSource \
    reportbuilder/java/org/libreoffice/ext/report/DataSourceException \
    reportbuilder/java/org/libreoffice/ext/report/DataSourceFactory \
    reportbuilder/java/org/libreoffice/ext/report/ImageService \
    reportbuilder/java/org/libreoffice/ext/report/InputRepository \
    reportbuilder/java/org/libreoffice/ext/report/JobDefinitionException \
    reportbuilder/java/org/libreoffice/ext/report/JobProgressIndicator \
    reportbuilder/java/org/libreoffice/ext/report/JobProperties \
    reportbuilder/java/org/libreoffice/ext/report/OfficeToken \
    reportbuilder/java/org/libreoffice/ext/report/OutputRepository \
    reportbuilder/java/org/libreoffice/ext/report/ParameterMap \
    reportbuilder/java/org/libreoffice/ext/report/ReportAddIn \
    reportbuilder/java/org/libreoffice/ext/report/ReportEngineMetaData \
    reportbuilder/java/org/libreoffice/ext/report/ReportEngineParameterNames \
    reportbuilder/java/org/libreoffice/ext/report/ReportExecutionException \
    reportbuilder/java/org/libreoffice/ext/report/ReportExpression \
    reportbuilder/java/org/libreoffice/ext/report/ReportExpressionMetaData \
    reportbuilder/java/org/libreoffice/ext/report/ReportFunction \
    reportbuilder/java/org/libreoffice/ext/report/ReportJob \
    reportbuilder/java/org/libreoffice/ext/report/ReportJobDefinition \
    reportbuilder/java/org/libreoffice/ext/report/ReportJobFactory \
    reportbuilder/java/org/libreoffice/ext/report/SDBCReportData \
    reportbuilder/java/org/libreoffice/ext/report/SDBCReportDataFactory \
    reportbuilder/java/org/libreoffice/ext/report/SOImageService \
    reportbuilder/java/org/libreoffice/ext/report/StorageRepository \
    reportbuilder/java/org/libreoffice/ext/report/function/metadata/AuthorFunction \
    reportbuilder/java/org/libreoffice/ext/report/function/metadata/AuthorFunctionDescription \
    reportbuilder/java/org/libreoffice/ext/report/function/metadata/MetaDataFunctionCategory \
    reportbuilder/java/org/libreoffice/ext/report/function/metadata/TitleFunction \
    reportbuilder/java/org/libreoffice/ext/report/function/metadata/TitleFunctionDescription \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/DefaultNameGenerator \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/OfficeNamespaces \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/PentahoFormulaContext \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/PentahoReportAddIn \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/PentahoReportEngine \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/PentahoReportEngineMetaData \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/PentahoReportJob \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/SOFormulaOpCodeMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/SOFormulaParser \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/SOFunctionManager \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/SOReportJobFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/StarFunctionCategory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/StarFunctionDescription \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/StarReportData \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/StarReportDataFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/StarReportModule \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/expressions/SumExpression \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/expressions/SumExpressionMetaData \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/AbstractReportElementLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/FixedTextLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/FormatValueUtility \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/FormattedTextLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/ImageElementContext \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/ImageElementLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/ObjectOleLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeDetailLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeGroupInstanceSectionLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeGroupLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeGroupSectionLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficePageSectionLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeRepeatingStructureLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeReportLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeTableLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/OfficeTableTemplateLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/TableCellLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/VariablesCollection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/layoutprocessor/VariablesDeclarationLayoutController \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/loader/InputRepositoryLoader \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/loader/InputRepositoryResourceData \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/loader/InputResourceKey \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/DataStyle \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/FixedTextElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/FontFaceDeclsSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/FontFaceElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/FormatCondition \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/FormattedTextElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/ImageElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/ObjectOleElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeDetailSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeDocument \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeGroup \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeGroupInstanceSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeGroupSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeMasterPage \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeMasterStyles \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeReport \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeStyle \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeStyles \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeStylesCollection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/OfficeTableSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/PageLayout \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/PageSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/RawText \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/ReportElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/TableCellElement \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/model/VariablesDeclarationSection \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/ImageProducer \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/OfficeDocumentReportTarget \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/OleProducer \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/StyleUtilities \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/StylesWriter \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/chart/ChartRawReportProcessor \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/chart/ChartRawReportTarget \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/spreadsheet/SpreadsheetRawReportProcessor \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/spreadsheet/SpreadsheetRawReportTarget \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/MasterPageFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/PageBreakDefinition \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/PageContext \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/TextRawReportProcessor \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/TextRawReportTarget \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/output/text/VariablesDeclarations \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/AttributeSpecification \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/ElementReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/OfficeDocumentXmlResourceFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/OfficeParserUtil \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/OfficeStylesXmlResourceFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/StarStyleXmlFactoryModule \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/StarXmlFactoryModule \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/StyleMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/chart/ChartReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/data/DataStyleReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/draw/ObjectOleReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/office/BodyReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/office/DocumentContentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/office/DocumentStylesReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/office/FontFaceDeclsReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/office/MasterStylesReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/ConditionalPrintExpressionReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/DetailRootTableReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/FixedContentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/FormatConditionReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/FormattedTextReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/FunctionReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/GroupReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/GroupSectionReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/ImageReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/MasterDetailReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/ReportElementReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/ReportReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/RootTableReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt/SubDocumentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/FontFaceReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/MasterPageReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/OfficeStyleReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/OfficeStylesReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/PageLayoutReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style/StyleDefinitionReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/OneOfConstantsMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/draw/TextAreaVerticalAlignMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/BackgroundColorMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/BorderRightMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/ColorMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/FontSizeMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/FontStyleMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/FontWeightMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/fo/TextAlignMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/FontFamilyGenericMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/FontFamilyMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/FontNameMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/FontPitchMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/FontReliefMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/TextEmphasizeMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/TextUnderlineColorMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/TextUnderlineStyleMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/TextUnderlineWidthMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/TextUnderlineWordMode \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/style/VerticalAlignMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/table/ColumnWidthMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/stylemapper/table/RowHeightMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/CoveredCellReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableCellReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableColumnReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableColumnsReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableRowReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/table/TableRowsReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/text/NoCDATATextContentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/text/TextContentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/xlink/XLinkReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/LengthCalculator \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMapper \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMapperKey \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMapperXmlFactoryModule \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMapperXmlResourceFactory \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMappingDocumentReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMappingReadHandler \
    reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/StyleMappingRule \
    reportbuilder/java/org/libreoffice/ext/report/util/DefaultJobProperties \
    reportbuilder/java/org/libreoffice/ext/report/util/DefaultParameterMap \
    reportbuilder/java/org/libreoffice/ext/report/util/DefaultReportJobDefinition \
    reportbuilder/java/org/libreoffice/ext/report/util/ManifestWriter \
))

$(eval $(call gb_Jar_add_packagefiles,lo-report-builder,,\
	$(SRCDIR)/reportbuilder/java/jfreereport.properties \
	$(SRCDIR)/reportbuilder/java/libformula.properties \
	$(SRCDIR)/reportbuilder/java/loader.properties \
))

$(eval $(call gb_Jar_add_packagefiles,lo-report-builder,org/libreoffice/ext/report/function/metadata,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/Author-Function.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/Author-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/Title-Function.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/Title-Function_en_US.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/category.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/function/metadata/category_en_US.properties \
))

$(eval $(call gb_Jar_add_packagefiles,lo-report-builder,org/libreoffice/ext/report/pentaho,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/configuration.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/module.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-datastyle.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-draw.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-form.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-style.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-table.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/oasis-text.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/smil.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/star-office.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/star-report.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/star-rpt.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/svg.css \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/xsl-fo.css \
))

$(eval $(call gb_Jar_add_packagefiles,lo-report-builder,org/libreoffice/ext/report/pentaho/parser,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/rpt-schema-v1.0-os.xsd \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/selectors.properties \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/parser/style-mapping.txt \
))

$(eval $(call gb_Jar_add_packagefiles,lo-report-builder,org/libreoffice/ext/report/pentaho/styles,\
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/stylemapper.xml \
	$(SRCDIR)/reportbuilder/java/org/libreoffice/ext/report/pentaho/styles/stylemapper.xsd \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
