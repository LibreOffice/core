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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,writerfilter))

$(eval $(call gb_Library_use_custom_headers,writerfilter,\
	writerfilter/source \
))

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
    i18nisolang1 \
    i18nutil \
    msfilter \
    oox \
    sal \
    sot \
    svt \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_external,writerfilter,libxml2))

$(eval $(call gb_Library_add_exception_objects,writerfilter,\
	writerfilter/source/rtftok/rtfcharsets \
	writerfilter/source/rtftok/rtfcontrolwords \
	writerfilter/source/rtftok/rtfdocumentfactory \
	writerfilter/source/rtftok/rtfdocumentimpl \
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
    writerfilter/source/doctok/WW8BinTableImpl \
    writerfilter/source/doctok/WW8Clx \
    writerfilter/source/doctok/WW8CpAndFc \
    writerfilter/source/doctok/WW8DocumentImpl \
    writerfilter/source/doctok/WW8FKPImpl \
    writerfilter/source/doctok/WW8FontTable \
    writerfilter/source/doctok/WW8LFOTable \
    writerfilter/source/doctok/WW8ListTable \
    writerfilter/source/doctok/WW8OutputWithDepth \
    writerfilter/source/doctok/WW8Picture \
    writerfilter/source/doctok/WW8PieceTableImpl \
    writerfilter/source/doctok/WW8PropertySetImpl \
    writerfilter/source/doctok/WW8ResourceModelImpl \
    writerfilter/source/doctok/WW8StreamImpl \
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
    writerfilter/source/resourcemodel/WW8Analyzer \
    writerfilter/source/resourcemodel/XPathLogger \
    writerfilter/source/resourcemodel/resourcemodel \
    writerfilter/source/resourcemodel/util \
))

$(eval $(call gb_Library_add_generated_exception_objects,writerfilter,\
	CustomTarget/writerfilter/source/resources \
    $(patsubst %,CustomTarget/writerfilter/source/OOXMLFactory_%,$(writerfilter_OOXMLNAMESPACES)) \
    CustomTarget/writerfilter/source/OOXMLFactory_generated \
    CustomTarget/writerfilter/source/OOXMLFactory_values \
    CustomTarget/writerfilter/source/sprmcodetostr \
))

ifneq ($(COM)-$(OS)-$(CPUNAME),GCC-LINUX-POWERPC64)
#Apparently some compilers, according to the original .mk this was converted
#from, require this to be noopt or they fail to compile it, probably good to
#revisit that and narrow this down to where it's necessary
$(eval $(call gb_Library_add_generated_cxxobjects,writerfilter,\
    CustomTarget/writerfilter/source/qnametostr \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
#Ironically, on RHEL-6 PPC64 with no-opt the output is too large for the
#toolchain, "Error: operand out of range", but it build fine with
#normal flags
$(eval $(call gb_Library_add_generated_cxxobjects,writerfilter,\
    CustomTarget/writerfilter/source/qnametostr \
	, $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
