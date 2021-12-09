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

$(eval $(call gb_Library_set_precompiled_header,writerfilter,writerfilter/inc/pch/precompiled_writerfilter))

$(eval $(call gb_Library_set_include,writerfilter,\
    $$(INCLUDE) \
    -I$(SRCDIR)/writerfilter/inc \
    -I$(SRCDIR)/writerfilter/source \
))

$(eval $(call gb_Library_use_sdk_api,writerfilter))

$(eval $(call gb_Library_set_componentfile,writerfilter,writerfilter/util/writerfilter,services))

$(eval $(call gb_Library_use_libraries,writerfilter,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sfx \
    sot \
    svt \
	svxcore \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_use_externals,writerfilter,\
	boost_headers \
	icui18n \
	icuuc \
	icu_headers \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,writerfilter,\
	writerfilter/source/rtftok/rtfcharsets \
	writerfilter/source/rtftok/rtfcontrolwords \
	writerfilter/source/rtftok/rtfdispatchdestination \
	writerfilter/source/rtftok/rtfdispatchflag \
	writerfilter/source/rtftok/rtfdispatchsymbol \
	writerfilter/source/rtftok/rtfdispatchvalue \
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
	writerfilter/source/dmapper/DocumentProtection \
    writerfilter/source/dmapper/DomainMapper \
    writerfilter/source/dmapper/DomainMapperTableHandler \
    writerfilter/source/dmapper/DomainMapperTableManager \
    writerfilter/source/dmapper/DomainMapper_Impl \
	writerfilter/source/dmapper/domainmapperfactory \
    writerfilter/source/dmapper/FFDataHandler \
    writerfilter/source/dmapper/FontTable \
    writerfilter/source/dmapper/FormControlHelper \
    writerfilter/source/dmapper/GraphicHelpers \
    writerfilter/source/dmapper/GraphicImport \
    writerfilter/source/dmapper/LatentStyleHandler \
    writerfilter/source/dmapper/LoggedResources \
    writerfilter/source/dmapper/MeasureHandler \
    writerfilter/source/dmapper/TrackChangesHandler \
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
    writerfilter/source/dmapper/SmartTagHandler \
    writerfilter/source/dmapper/StyleSheetTable \
    writerfilter/source/dmapper/TDefTableHandler \
    writerfilter/source/dmapper/TableManager \
    writerfilter/source/dmapper/TablePositionHandler \
    writerfilter/source/dmapper/TablePropertiesHandler \
	writerfilter/source/dmapper/TagLogger \
    writerfilter/source/dmapper/TextEffectsHandler \
    writerfilter/source/dmapper/TblStylePrHandler \
    writerfilter/source/dmapper/ThemeTable \
    writerfilter/source/dmapper/WrapPolygonHandler \
	writerfilter/source/dmapper/WriteProtection \
    writerfilter/source/dmapper/util \
    writerfilter/source/filter/RtfFilter \
    writerfilter/source/filter/WriterFilter \
    writerfilter/source/ooxml/Handler \
    writerfilter/source/ooxml/OOXMLBinaryObjectReference \
    writerfilter/source/ooxml/OOXMLDocumentImpl \
    writerfilter/source/ooxml/OOXMLFactory \
    writerfilter/source/ooxml/OOXMLFastContextHandler \
    writerfilter/source/ooxml/OOXMLFastDocumentHandler \
    writerfilter/source/ooxml/OOXMLParserState \
    writerfilter/source/ooxml/OOXMLPropertySet \
    writerfilter/source/ooxml/OOXMLStreamImpl \
))

$(eval $(call gb_Library_add_generated_exception_objects,writerfilter,\
    $(patsubst %,CustomTarget/writerfilter/source/ooxml/OOXMLFactory_%,$(writerfilter_OOXMLNAMESPACES)) \
    CustomTarget/writerfilter/source/ooxml/OOXMLFactory_generated \
    CustomTarget/writerfilter/source/ooxml/qnametostr \
))

# vim: set noet sw=4 ts=4:
