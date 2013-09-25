# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,writerfilter))

$(eval $(call gb_Library_use_custom_headers,writerfilter,\
	officecfg/registry \
	oox/generated \
	writerfilter/source \
))

$(eval $(call gb_Library_set_precompiled_header,writerfilter,$(SRCDIR)/writerfilter/inc/pch/precompiled_writerfilter))

$(eval $(call gb_Library_set_include,writerfilter,\
    $$(INCLUDE) \
    -I$(SRCDIR)/writerfilter/inc \
    -I$(SRCDIR)/writerfilter/source \
))

$(eval $(call gb_Library_use_sdk_api,writerfilter))

$(eval $(call gb_Library_set_componentfile,writerfilter,writerfilter/util/writerfilter))

$(eval $(call gb_Library_add_defs,writerfilter,\
	-DWRITERFILTER_DOCTOK_DLLIMPLEMENTATION \
	-DWRITERFILTER_OOXML_DLLIMPLEMENTATION \
	-DWRITERFILTER_RESOURCEMODEL_DLLIMPLEMENTATION \
	-DWRITERFILTER_RTFTOK_DLLIMPLEMENTATION \
	-DWRITERFILTER_WRITERFILTER_DLLIMPLEMENTATION \
	$(if $(filter-out 0 1,$(gb_DEBUGLEVEL)), \
		-DDEBUG_DOMAINMAPPER \
		-DDEBUG_ELEMENT \
		-DDEBUG_RESOLVE \
		-DDEBUG_CONTEXT_STACK \
		-DDEBUG_ATTRIBUTES \
		-DDEBUG_PROPERTIES \
		-DDEBUG_CONTEXT_HANDLER \
		-DDEBUG_IMPORT \
		-DDEBUG_LOGGING \
		-DDEBUG_DMAPPER_PROPERTY_MAP \
		-DDEBUG_DMAPPER_TABLE_HANDLER \
		-DDEBUG_TABLE \
		-DDEBUG_DMAPPER_SETTINGS_TABLE \
	) \
))

$(eval $(call gb_Library_use_libraries,writerfilter,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    sfx \
    sot \
    svt \
	svxcore \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,writerfilter,\
	boost_headers \
	icu_headers \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,writerfilter,\
	writerfilter/source/rtftok/rtfcharsets \
	writerfilter/source/rtftok/rtfcontrolwords \
	writerfilter/source/rtftok/rtfdocumentfactory \
	writerfilter/source/rtftok/rtfdocumentimpl \
	writerfilter/source/rtftok/rtflookahead \
	writerfilter/source/rtftok/rtfreferenceproperties \
	writerfilter/source/rtftok/rtfreferencetable \
	writerfilter/source/rtftok/rtfsdrimport \
	writerfilter/source/rtftok/rtfskipdestination \
	writerfilter/source/rtftok/rtfsprm \
	writerfilter/source/rtftok/rtftokenizer \
	writerfilter/source/rtftok/rtfvalue \
    writerfilter/source/dmapper/BorderHandler \
    writerfilter/source/dmapper/CellColorHandler \
    writerfilter/source/dmapper/CellMarginHandler \
    writerfilter/source/dmapper/ConversionHelper \
    writerfilter/source/dmapper/DomainMapper \
    writerfilter/source/dmapper/DomainMapperTableHandler \
    writerfilter/source/dmapper/DomainMapperTableManager \
    writerfilter/source/dmapper/DomainMapper_Impl \
    writerfilter/source/dmapper/FFDataHandler \
    writerfilter/source/dmapper/FontTable \
    writerfilter/source/dmapper/FormControlHelper \
    writerfilter/source/dmapper/GraphicHelpers \
    writerfilter/source/dmapper/GraphicImport \
    writerfilter/source/dmapper/MeasureHandler \
    writerfilter/source/dmapper/ModelEventListener \
    writerfilter/source/dmapper/NumberingManager \
    writerfilter/source/dmapper/OLEHandler \
    writerfilter/source/dmapper/PageBordersHandler \
    writerfilter/source/dmapper/PropertyIds \
    writerfilter/source/dmapper/PropertyMap \
    writerfilter/source/dmapper/PropertyMapHelper \
    writerfilter/source/dmapper/SdtHelper \
    writerfilter/source/dmapper/SectionColumnHandler \
    writerfilter/source/dmapper/SettingsTable \
    writerfilter/source/dmapper/StyleSheetTable \
    writerfilter/source/dmapper/TDefTableHandler \
    writerfilter/source/dmapper/TablePositionHandler \
    writerfilter/source/dmapper/TablePropertiesHandler \
    writerfilter/source/dmapper/TblStylePrHandler \
    writerfilter/source/dmapper/ThemeTable \
    writerfilter/source/dmapper/WrapPolygonHandler \
    writerfilter/source/doctok/Dff \
    writerfilter/source/doctok/DffImpl \
    writerfilter/source/doctok/WW8Annotation \
    writerfilter/source/doctok/WW8Clx \
    writerfilter/source/doctok/WW8CpAndFc \
    writerfilter/source/doctok/WW8FKPImpl \
    writerfilter/source/doctok/WW8FontTable \
    writerfilter/source/doctok/WW8LFOTable \
    writerfilter/source/doctok/WW8ListTable \
    writerfilter/source/doctok/WW8OutputWithDepth \
    writerfilter/source/doctok/WW8Picture \
    writerfilter/source/doctok/WW8PropertySetImpl \
    writerfilter/source/doctok/WW8ResourceModelImpl \
    writerfilter/source/doctok/WW8StructBase \
    writerfilter/source/doctok/WW8Sttbf \
    writerfilter/source/doctok/WW8StyleSheet \
    writerfilter/source/doctok/WW8Table \
    writerfilter/source/doctok/WW8Text \
    writerfilter/source/filter/ImportFilter \
    writerfilter/source/filter/RtfFilter \
    writerfilter/source/filter/WriterFilter \
    writerfilter/source/filter/WriterFilterDetection \
    writerfilter/source/ooxml/Handler \
    writerfilter/source/ooxml/OOXMLBinaryObjectReference \
    writerfilter/source/ooxml/OOXMLDocumentImpl \
    writerfilter/source/ooxml/OOXMLFactory \
    writerfilter/source/ooxml/OOXMLFastContextHandler \
    writerfilter/source/ooxml/OOXMLFastDocumentHandler \
    writerfilter/source/ooxml/OOXMLFastTokenHandler \
    writerfilter/source/ooxml/OOXMLParserState \
    writerfilter/source/ooxml/OOXMLPropertySetImpl \
    writerfilter/source/ooxml/OOXMLStreamImpl \
    writerfilter/source/resourcemodel/Fraction \
    writerfilter/source/resourcemodel/LoggedResources \
    writerfilter/source/resourcemodel/Protocol \
    writerfilter/source/resourcemodel/ResourceModelHelper \
    writerfilter/source/resourcemodel/TagLogger \
    writerfilter/source/resourcemodel/XPathLogger \
    writerfilter/source/resourcemodel/qnametostrcore \
    writerfilter/source/resourcemodel/resourcemodel \
    writerfilter/source/resourcemodel/util \
))

$(eval $(call gb_Library_add_generated_exception_objects,writerfilter,\
	CustomTarget/writerfilter/source/resources \
    $(patsubst %,CustomTarget/writerfilter/source/OOXMLFactory_%,$(writerfilter_OOXMLNAMESPACES)) \
    CustomTarget/writerfilter/source/OOXMLFactory_generated \
    CustomTarget/writerfilter/source/OOXMLFactory_values \
    CustomTarget/writerfilter/source/sprmcodetostr \
    CustomTarget/writerfilter/source/doctok/qnametostr \
    CustomTarget/writerfilter/source/ooxml/qnametostr \
))

# vim: set noet sw=4 ts=4:
