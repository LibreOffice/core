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

$(eval $(call gb_Library_Library,ooxml))

$(eval $(call gb_Library_set_include,ooxml,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/writerfilter/inc) \
    -I$(realpath $(SRCDIR)/writerfilter/source/ooxml) \
    -I$(WORKDIR)/writerfilter/inc \
    -I$(WORKDIR)/writerfilter/inc/ooxml \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter -I%,$(LIBXML_CFLAGS))) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

include debug_setup.mk

$(eval $(call gb_Library_set_defs,ooxml,\
	$$(DEFS) \
	-DWRITERFILTER_DLLIMPLEMENTATION \
	$(writerfilter_debug_flags) \
	$(if $(filter YES,$(SYSTEM_LIBXML)),$(filter-out -I%,$(LIBXML_CFLAGS))) \
))

$(eval $(call gb_Library_add_linked_libs,ooxml,\
    comphelper \
    cppu \
    cppuhelper \
	i18nisolang1 \
	i18npaper \
	resourcemodel \
    sal \
    tl \
    $(gb_STDLIBS) \
))

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

ooxml_NAMESPACES := \
    dml-baseStylesheet \
    dml-baseTypes \
    dml-chartDrawing \
    dml-documentProperties \
    dml-graphicalObject \
    dml-shape3DCamera \
    dml-shape3DLighting \
    dml-shape3DScene \
    dml-shape3DStyles \
    dml-shapeEffects \
    dml-shapeGeometry \
    dml-shapeLineProperties \
    dml-shapeProperties \
    dml-styleDefaults \
    dml-stylesheet \
    dml-textCharacter \
    dml-wordprocessingDrawing \
    shared-math \
    shared-relationshipReference \
    sml-customXmlMappings \
    vml-main \
    vml-officeDrawing \
    vml-wordprocessingDrawing \
    wml

ooxml_factory_source_stem = writerfilter/source/ooxml/OOXMLFactory_$(1)

define ooxml_factory_source
$(call gb_GenCxxObject_get_source,$(call ooxml_factory_source_stem,$(1)))
endef

define ooxml_namespace_sources
$(foreach namespace,$(ooxml_NAMESPACES),$(call ooxml_factory_source,$(namespace)))
endef

$(eval $(call gb_Library_add_generated_exception_objects,ooxml,\
    writerfilter/source/ooxml/OOXMLFactory_generated \
    writerfilter/source/ooxml/OOXMLFactory_values \
	$(foreach namespace,$(ooxml_NAMESPACES),$(call ooxml_factory_source_stem,$(namespace))) \
))

SLOFACTORIESNAMESPACES= \
    $(SLO)/ooxml_Factory_{$(NAMESPACES)}.obj

ooxml_HXXOUTDIR := $(WORKDIR)/writerfilter/inc/ooxml
ooxml_CXXOUTDIR := $(dir $(call ooxml_factory_source,generated))
ooxml_MISCDIR := $(WORKDIR)/writerfilter/ooxml

ooxml_HXXOUTDIRCREATED := $(ooxml_HXXOUTDIR)/created

define ooxml_factory_header
$(ooxml_HXXOUTDIR)/OOXMLFactory_$(1).hxx
endef

define ooxml_namespace_headers
$(foreach namespace,$(ooxml_NAMESPACES),$(call ooxml_factory_header,$(namespace)))
endef

ooxml_MODEL := $(SRCDIR)/writerfilter/source/ooxml/model.xml

ooxml_FACTORYIMPLNSXSL := $(SRCDIR)/writerfilter/source/ooxml/factoryimpl_ns.xsl
ooxml_FACTORYIMPLXSL := $(SRCDIR)/writerfilter/source/ooxml/factoryimpl.xsl
ooxml_FACTORYINCXSL := $(SRCDIR)/writerfilter/source/ooxml/factoryinc.xsl
ooxml_FACTORYNSXSL := $(SRCDIR)/writerfilter/source/ooxml/factory_ns.xsl
ooxml_FACTORYTOOLSXSL := $(SRCDIR)/writerfilter/source/ooxml/factorytools.xsl
ooxml_FACTORYVALUESIMPLXSL := $(SRCDIR)/writerfilter/source/ooxml/factoryimpl_values.xsl
ooxml_FACTORYVALUESXSL := $(SRCDIR)/writerfilter/source/ooxml/factory_values.xsl
ooxml_FASTTOKENSXSL := $(SRCDIR)/writerfilter/source/ooxml/fasttokens.xsl
ooxml_GPERFFASTTOKENXSL := $(SRCDIR)/writerfilter/source/ooxml/gperffasttokenhandler.xsl
ooxml_NAMESPACEIDSXSL := $(SRCDIR)/writerfilter/source/ooxml/namespaceids.xsl
ooxml_PREPROCESSXSL := $(SRCDIR)/writerfilter/source/ooxml/modelpreprocess.xsl
ooxml_QNAMETOSTRXSL := $(SRCDIR)/writerfilter/source/ooxml/qnametostr.xsl
ooxml_RESORUCETOKENSXSL := $(SRCDIR)/writerfilter/source/ooxml/resourcestokens.xsl
ooxml_RESOURCEIDSXSL := $(SRCDIR)/writerfilter/source/ooxml/resourceids.xsl
ooxml_RESOURCESIMPLXSL := $(SRCDIR)/writerfilter/source/ooxml/resourcesimpl.xsl

ooxml_NSPROCESS := $(SRCDIR)/writerfilter/source/resourcemodel/namespace_preprocess.pl

ooxml_RESOURCEIDSHXX := $(ooxml_HXXOUTDIR)/resourceids.hxx

ooxml_TOKENXML := $(ooxml_MISCDIR)/token.xml
ooxml_TOKENXMLTMP := $(ooxml_MISCDIR)/token.tmp

ooxml_FACTORYGENERATEDHXX := $(call ooxml_factory_header,generated)
ooxml_FACTORYGENERATEDCXX := $(call ooxml_factory_source,generated)
ooxml_FASTTOKENSHXX := $(ooxml_HXXOUTDIR)/OOXMLFastTokens.hxx
ooxml_NAMESPACEIDSHXX := $(ooxml_HXXOUTDIR)/OOXMLnamespaceids.hxx
ooxml_FACTORYVALUESHXX := $(call ooxml_factory_header,values)
ooxml_FACTORYVALUESCXX := $(call ooxml_factory_source,values)
ooxml_GPERFFASTTOKENHXX := $(ooxml_HXXOUTDIR)/gperffasttoken.hxx
ooxml_GPERFFASTTOKENTMP := $(ooxml_MISCDIR)/gperffasttoken.tmp
ooxml_QNAMETOSTRTMP := $(WORKDIR)/writerfilter/ooxml_qnameToStr.tmp

ooxml_MODELPROCESSED := $(ooxml_MISCDIR)/model_preprocessed.xml
ooxml_NSXSL := $(ooxml_MISCDIR)/namespacesmap.xsl
ooxml_PREPROCESSXSLCOPIED := $(ooxml_MISCDIR)/modelpreprocess.xsl

ooxml_NAMESPACESTXT := $(OUTDIR)/inc/oox/namespaces.txt

ooxml_GENHEADERS = \
    $(ooxml_FACTORYGENERATEDHXX) \
    $(ooxml_FASTTOKENSHXX) \
    $(ooxml_FACTORYVALUESHXX) \
    $(ooxml_GPERFFASTTOKENHXX) \
    $(ooxml_RESOURCEIDSHXX) \
    $(ooxml_NAMESPACEIDSHXX) \
    $(ooxml_namespace_headers)

ooxml_GENERATEDFILES = \
    $(ooxml_GENHEADERS) \
    $(ooxml_GPERFFASTTOKENTMP) \
    $(ooxml_FACTORYGENERATEDCXX) \
    $(ooxml_MODELPROCESSED) \
    $(ooxml_namespace_sources) \
    $(ooxml_NSXSL) \
    $(ooxml_FACTORYVALUESCXX) \
    $(ooxml_PREPROCESSXSLCOPIED) \
    $(ooxml_QNAMETOSTRTMP) \
    $(ooxml_TOKENXMLTMP) \
    $(ooxml_TOKENXML)

$(ooxml_TOKENXMLTMP) : $(OUTDIR)/inc/oox/tokens.txt
	mkdir -p $(dir $@) && sed -e 's#.*#<fasttoken>&</fasttoken>#' $< > $@

$(ooxml_TOKENXML) : $(SRCDIR)/writerfilter/source/ooxml/tokenxmlheader $(ooxml_TOKENXMLTMP) $(SRCDIR)/writerfilter/source/ooxml/tokenxmlfooter
	mkdir -p $(dir $@) && cat $(SRCDIR)/writerfilter/source/ooxml/tokenxmlheader $(ooxml_TOKENXMLTMP) $(SRCDIR)/writerfilter/source/ooxml/tokenxmlfooter > $@

$(ooxml_HXXOUTDIRCREATED) :
	mkdir -p $(dir $@) && touch $@

$(ooxml_GENHEADERS) : $(ooxml_HXXOUTDIRCREATED)

$(call ooxml_factory_source,%) :| $(call ooxml_factory_header,%)

$(call ooxml_factory_source,%) : $(ooxml_FACTORYIMPLNSXSL) $(ooxml_MODELPROCESSED)
	mkdir -p $(dir $@) && $(gb_XSLTPROC) --nonet --stringparam file $@ $(ooxml_FACTORYIMPLNSXSL) $(ooxml_MODELPROCESSED) > $@

$(call ooxml_factory_header,%) : $(ooxml_FACTORYNSXSL) $(ooxml_MODELPROCESSED)
	mkdir -p $(dir $@) && $(gb_XSLTPROC) --nonet --stringparam file $@ $(ooxml_FACTORYNSXSL) $(ooxml_MODELPROCESSED) > $@

define ooxml_xsl_process
$(1) : $(2) $(3)
	mkdir -p $(dir $(1)) && $(gb_XSLTPROC) --nonet $(2) $(3) > $(1)
endef

define ooxml_xsl_process_model
$(call ooxml_xsl_process,$(1),$(2),$(ooxml_MODELPROCESSED))
endef

$(eval $(call ooxml_xsl_process,$(ooxml_MODELPROCESSED),$(ooxml_NSXSL),$(ooxml_MODEL)))
$(eval $(call ooxml_xsl_process,$(ooxml_FASTTOKENSHXX),$(ooxml_FASTTOKENSXSL),$(ooxml_TOKENXML)))

$(eval $(call ooxml_xsl_process_model,$(ooxml_FACTORYGENERATEDHXX),$(ooxml_FACTORYINCXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_FACTORYGENERATEDCXX),$(ooxml_FACTORYIMPLXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_FACTORYVALUESHXX),$(ooxml_FACTORYVALUESXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_FACTORYVALUESCXX),$(ooxml_FACTORYVALUESIMPLXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_QNAMETOSTRTMP),$(ooxml_QNAMETOSTRXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_RESOURCEIDSHXX),$(ooxml_RESOURCEIDSXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_NAMESPACEIDSHXX),$(ooxml_NAMESPACEIDSXSL)))
$(eval $(call ooxml_xsl_process_model,$(ooxml_GPERFFASTTOKENTMP),$(ooxml_GPERFFASTTOKENXSL)))

$(ooxml_NSXSL) : $(ooxml_NAMESPACESTXT) $(ooxml_NSPROCESS) $(ooxml_PREPROCESSXSLCOPIED)
	mkdir -p $(dir $@) && perl $(ooxml_NSPROCESS) $(ooxml_NAMESPACESTXT) > $@

# this is included by relative path from $(ooxml_NSXSL)
$(ooxml_PREPROCESSXSLCOPIED) : $(ooxml_PREPROCESSXSL)
	mkdir -p $(dir $@) && cp $< $@

$(ooxml_GPERFFASTTOKENHXX) : $(ooxml_GPERFFASTTOKENTMP) $(ooxml_HXXOUTDIRCREATED)
	tr -d '\r' < $< | gperf -I -t -E -S1 -c -G -LC++ > $@

$(ooxml_GENERATEDFILES) : $(ooxml_FACTORYTOOLSXSL)

$(ooxml_FACTORYVALUESCXX) :| $(ooxml_FACTORYVALUESHXX)
$(ooxml_FACTORYGENERATEDCXX) :| $(ooxml_FACTORYGENERATEDHXX) $(ooxml_GENHEADERS)

ooxml_clean :
	rm -rf $(ooxml_GENERATEDFILES) $(ooxml_HXXOUTDIR)
.PHONY : ooxml_clean

$(call gb_Library_get_clean_target,ooxml) : ooxml_clean

# vim: set noet ts=4 sw=4:
