#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
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

include $(SRCDIR)/writerfilter/source/generated.mk

$(eval $(call gb_Library_Library,writerfilter))

$(eval $(call \
	gb_Library_add_package_headers,writerfilter,writerfilter_inc writerfilter_generated))

#$(eval $(call gb_Library_add_precompiled_header,writerfilter,$(SRCDIR)/writerfilter/PCH_FILE))

$(eval $(call gb_Library_set_componentfile,writerfilter,writerfilter/util/writerfilter))

$(eval $(call gb_Library_set_include,writerfilter,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(SRCDIR)/writerfilter/inc \
	-I$(WORKDIR)/CustomTarget/writerfilter/source \
	-I$(WORKDIR)/CustomTarget/writerfilter/source/ooxml \
	-I$(WORKDIR)/CustomTarget/writerfilter/source/doctok \
	-I$(SRCDIR)/writerfilter/source/doctok \
	-I$(SRCDIR)/writerfilter/source/ooxml \
))

$(eval $(call gb_Library_set_defs,writerfilter,\
	$$(DEFS) \
	-DWRITERFILTER_DLLIMPLEMENTATION \
	-DDEBUG_CONTEXT_HANDLER \
	-DDEBUG_FACTORY \
	-DDEBUG_IMPORT \
))

$(eval $(call gb_Library_add_linked_libs,writerfilter,\
	oox \
	i18nisolang1 \
	i18npaper \
	sot \
	tl \
	utl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,writerfilter,\
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
	writerfilter/source/dmapper/NumberingManager  \
	writerfilter/source/dmapper/OLEHandler \
	writerfilter/source/dmapper/PageBordersHandler \
	writerfilter/source/dmapper/PropertyIds \
	writerfilter/source/dmapper/PropertyMap \
	writerfilter/source/dmapper/PropertyMapHelper \
	writerfilter/source/dmapper/SectionColumnHandler \
	writerfilter/source/dmapper/SettingsTable \
	writerfilter/source/dmapper/StyleSheetTable \
	writerfilter/source/dmapper/TDefTableHandler \
	writerfilter/source/dmapper/TablePropertiesHandler \
	writerfilter/source/dmapper/TblStylePrHandler \
	writerfilter/source/dmapper/ThemeTable \
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
	writerfilter/source/ooxml/OOXMLBinaryObjectReference\
	writerfilter/source/ooxml/OOXMLDocumentImpl \
	writerfilter/source/ooxml/OOXMLFactory \
	writerfilter/source/ooxml/OOXMLFastContextHandler \
	writerfilter/source/ooxml/OOXMLFastDocumentHandler \
	writerfilter/source/ooxml/OOXMLFastTokenHandler \
	writerfilter/source/ooxml/OOXMLParserState \
	writerfilter/source/ooxml/OOXMLPropertySetImpl \
	writerfilter/source/ooxml/OOXMLStreamImpl \
	writerfilter/source/resourcemodel/Protocol \
	writerfilter/source/resourcemodel/ResourceModelHelper \
	writerfilter/source/resourcemodel/TagLogger \
	writerfilter/source/resourcemodel/Token \
	writerfilter/source/resourcemodel/WW8Analyzer \
	writerfilter/source/resourcemodel/XPathLogger \
	writerfilter/source/resourcemodel/resourcemodel \
	writerfilter/source/resourcemodel/util \
	writerfilter/source/rtftok/RTFParseException \
))

$(eval $(call gb_Library_add_generated_exception_objects,writerfilter,\
	CustomTarget/writerfilter/source/resources \
	CustomTarget/writerfilter/source/qnametostr \
	CustomTarget/writerfilter/source/sprmcodetostr \
	$(patsubst %,CustomTarget/writerfilter/source/OOXMLFactory_%,$(WRITERFILTER_OOXMLNAMESPACES)) \
	CustomTarget/writerfilter/source/OOXMLFactory_generated \
	CustomTarget/writerfilter/source/OOXMLFactory_values \
))

#disabled
#	CustomTarget/writerfilter/generated/RTFScanner \

# vim: set noet sw=4 ts=4:

