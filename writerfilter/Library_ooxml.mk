# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

include $(realpath $(SRCDIR)/writerfilter/debug_setup.mk)
include $(realpath $(SRCDIR)/writerfilter/source/generated.mk)

$(eval $(call gb_Library_Library,ooxml))

$(eval $(call gb_Library_add_package_headers,ooxml,writerfilter_generated))

$(eval $(call gb_Library_set_include,ooxml,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
    -I$(realpath $(SRCDIR)/writerfilter/source/ooxml) \
    -I$(realpath $(SRCDIR)/writerfilter/source/dmapper) \
	-I$(WORKDIR)/CustomTarget/writerfilter/source \
	-I$(WORKDIR)/CustomTarget/writerfilter/source/ooxml \
	-I$(WORKDIR)/CustomTarget/writerfilter/source/doctok \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_defs,ooxml,\
	-DWRITERFILTER_OOXML_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
))

$(eval $(call gb_Library_add_api,ooxml,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ooxml,\
    comphelper \
    cppu \
    cppuhelper \
	doctok \
	i18nisolang1 \
	i18npaper \
	resourcemodel \
    sal \
    tl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_external,ooxml,libxml2))

$(eval $(call gb_Library_add_exception_objects,ooxml,\
    writerfilter/source/dmapper/BorderHandler \
    writerfilter/source/dmapper/CellColorHandler \
    writerfilter/source/dmapper/CellMarginHandler \
    writerfilter/source/dmapper/ConversionHelper \
    writerfilter/source/dmapper/DomainMapper \
    writerfilter/source/dmapper/DomainMapper_Impl \
    writerfilter/source/dmapper/DomainMapperTableHandler \
    writerfilter/source/dmapper/DomainMapperTableManager \
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
    writerfilter/source/dmapper/TablePropertiesHandler \
    writerfilter/source/dmapper/TblStylePrHandler \
    writerfilter/source/dmapper/TDefTableHandler \
    writerfilter/source/dmapper/ThemeTable \
    writerfilter/source/dmapper/WrapPolygonHandler \
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
))

$(eval $(call gb_Library_add_generated_exception_objects,ooxml,\
    $(patsubst %,CustomTarget/writerfilter/source/OOXMLFactory_%,$(WRITERFILTER_OOXMLNAMESPACES)) \
    CustomTarget/writerfilter/source/OOXMLFactory_generated \
    CustomTarget/writerfilter/source/OOXMLFactory_values \
))

# vim: set noet sw=4 ts=4:
