# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sw_writerfilter))

$(eval $(call gb_Library_use_custom_headers,sw_writerfilter,\
	officecfg/registry \
	oox/generated \
	sw/source/writerfilter \
))

$(eval $(call gb_Library_set_precompiled_header,sw_writerfilter,sw/inc/pch/precompiled_sw_writerfilter))

$(eval $(call gb_Library_set_include,sw_writerfilter,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/uibase/uno/ \
    -I$(SRCDIR)/sw/source/writerfilter/inc \
    -I$(SRCDIR)/sw/source/writerfilter \
))

$(eval $(call gb_Library_use_sdk_api,sw_writerfilter))

$(eval $(call gb_Library_set_componentfile,sw_writerfilter,sw/util/sw_writerfilter,services))

$(eval $(call gb_Library_use_libraries,sw_writerfilter,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
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
    sw \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_use_externals,sw_writerfilter,\
	boost_headers \
	icui18n \
	icuuc \
	icu_headers \
	libxml2 \
	frozen \
))

$(eval $(call gb_Library_add_exception_objects,sw_writerfilter,\
	sw/source/writerfilter/rtftok/rtfcharsets \
	sw/source/writerfilter/rtftok/rtfcontrolwords \
	sw/source/writerfilter/rtftok/rtfdispatchdestination \
	sw/source/writerfilter/rtftok/rtfdispatchflag \
	sw/source/writerfilter/rtftok/rtfdispatchsymbol \
	sw/source/writerfilter/rtftok/rtfdispatchvalue \
	sw/source/writerfilter/rtftok/rtfdocumentfactory \
	sw/source/writerfilter/rtftok/rtfdocumentimpl \
	sw/source/writerfilter/rtftok/rtflookahead \
	sw/source/writerfilter/rtftok/rtfreferenceproperties \
	sw/source/writerfilter/rtftok/rtfreferencetable \
	sw/source/writerfilter/rtftok/rtfsdrimport \
	sw/source/writerfilter/rtftok/rtfskipdestination \
	sw/source/writerfilter/rtftok/rtfsprm \
	sw/source/writerfilter/rtftok/rtftokenizer \
	sw/source/writerfilter/rtftok/rtfvalue \
    sw/source/writerfilter/dmapper/BorderHandler \
    sw/source/writerfilter/dmapper/CellColorHandler \
    sw/source/writerfilter/dmapper/CellMarginHandler \
    sw/source/writerfilter/dmapper/ConversionHelper \
	sw/source/writerfilter/dmapper/DocumentProtection \
    sw/source/writerfilter/dmapper/DomainMapper \
    sw/source/writerfilter/dmapper/DomainMapperTableHandler \
    sw/source/writerfilter/dmapper/DomainMapperTableManager \
    sw/source/writerfilter/dmapper/DomainMapper_Impl \
	sw/source/writerfilter/dmapper/domainmapperfactory \
    sw/source/writerfilter/dmapper/FFDataHandler \
    sw/source/writerfilter/dmapper/FontTable \
    sw/source/writerfilter/dmapper/FormControlHelper \
    sw/source/writerfilter/dmapper/GraphicHelpers \
    sw/source/writerfilter/dmapper/GraphicImport \
    sw/source/writerfilter/dmapper/LatentStyleHandler \
    sw/source/writerfilter/dmapper/LoggedResources \
    sw/source/writerfilter/dmapper/MeasureHandler \
    sw/source/writerfilter/dmapper/TrackChangesHandler \
    sw/source/writerfilter/dmapper/ModelEventListener \
    sw/source/writerfilter/dmapper/NumberingManager \
    sw/source/writerfilter/dmapper/OLEHandler \
    sw/source/writerfilter/dmapper/PageBordersHandler \
    sw/source/writerfilter/dmapper/PropertyIds \
    sw/source/writerfilter/dmapper/PropertyMap \
    sw/source/writerfilter/dmapper/PropertyMapHelper \
    sw/source/writerfilter/dmapper/SdtHelper \
    sw/source/writerfilter/dmapper/SectionColumnHandler \
    sw/source/writerfilter/dmapper/SettingsTable \
    sw/source/writerfilter/dmapper/SmartTagHandler \
    sw/source/writerfilter/dmapper/StyleSheetTable \
    sw/source/writerfilter/dmapper/TDefTableHandler \
    sw/source/writerfilter/dmapper/TableManager \
    sw/source/writerfilter/dmapper/TablePositionHandler \
    sw/source/writerfilter/dmapper/TablePropertiesHandler \
	sw/source/writerfilter/dmapper/TagLogger \
    sw/source/writerfilter/dmapper/TextEffectsHandler \
    sw/source/writerfilter/dmapper/TblStylePrHandler \
    sw/source/writerfilter/dmapper/ThemeHandler \
    sw/source/writerfilter/dmapper/WrapPolygonHandler \
	sw/source/writerfilter/dmapper/WriteProtection \
    sw/source/writerfilter/dmapper/util \
    sw/source/writerfilter/filter/RtfFilter \
    sw/source/writerfilter/filter/WriterFilter \
    sw/source/writerfilter/ooxml/Handler \
    sw/source/writerfilter/ooxml/OOXMLBinaryObjectReference \
    sw/source/writerfilter/ooxml/OOXMLDocumentImpl \
    sw/source/writerfilter/ooxml/OOXMLFactory \
    sw/source/writerfilter/ooxml/OOXMLFastContextHandler \
    sw/source/writerfilter/ooxml/OOXMLFastContextHandlerTheme \
    sw/source/writerfilter/ooxml/OOXMLFastDocumentHandler \
    sw/source/writerfilter/ooxml/OOXMLParserState \
    sw/source/writerfilter/ooxml/OOXMLPropertySet \
    sw/source/writerfilter/ooxml/OOXMLStreamImpl \
))

$(eval $(call gb_Library_add_generated_exception_objects,sw_writerfilter,\
    $(patsubst %,CustomTarget/sw/source/writerfilter/ooxml/OOXMLFactory_%,$(writerfilter_OOXMLNAMESPACES)) \
    CustomTarget/sw/source/writerfilter/ooxml/OOXMLFactory_generated \
    CustomTarget/sw/source/writerfilter/ooxml/qnametostr \
))

# vim: set noet sw=4 ts=4:
