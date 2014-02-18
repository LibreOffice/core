# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,writerfilter/source))

writerfilter_WORK := $(call gb_CustomTarget_get_workdir,writerfilter/source)
writerfilter_SRC := $(SRCDIR)/writerfilter/source

writerfilter_XSLTCOMMAND := $(call gb_ExternalExecutable_get_command,xsltproc)

writerfilter_OOXMLNAMESPACES= \
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
	mce \
	wp14 \
	w14 \
	wml

writerfilter_ALL = \
	$(writerfilter_GEN_doctok_SprmIds_hxx) \
	$(writerfilter_GEN_model_SprmCodeToStr_cxx) \
	$(writerfilter_GEN_ooxml_Factory_cxx) \
	$(writerfilter_GEN_ooxml_Factory_hxx) \
	$(writerfilter_GEN_ooxml_FactoryValues_hxx) \
	$(writerfilter_GEN_ooxml_FastTokens_hxx) \
	$(writerfilter_GEN_ooxml_GperfFastToken_hxx) \
	$(writerfilter_GEN_ooxml_NamespaceIds_hxx) \
	$(writerfilter_GEN_ooxml_QNameToStr_cxx) \
	$(writerfilter_GEN_ooxml_ResourceIds_hxx) \
	$(writerfilter_GEN_ooxml_Model_processed) \
	$(writerfilter_GEN_model_SprmCodeToStr_tmp) \
	$(patsubst %,$(writerfilter_WORK)/OOXMLFactory_%.hxx,$(writerfilter_OOXMLNAMESPACES)) \
	$(patsubst %,$(writerfilter_WORK)/OOXMLFactory_%.cxx,$(writerfilter_OOXMLNAMESPACES)) \

writerfilter_DEP_ooxml_Namespaces_txt=$(call gb_CustomTarget_get_workdir,oox/generated)/misc/namespaces.txt
writerfilter_GEN_doctok_SprmIds_hxx=$(writerfilter_WORK)/doctok/sprmids.hxx
writerfilter_GEN_model_SprmCodeToStr_cxx=$(writerfilter_WORK)/sprmcodetostr.cxx
writerfilter_GEN_model_SprmCodeToStr_tmp=$(writerfilter_WORK)/sprmcodetostr.tmp
writerfilter_GEN_ooxml_FactoryValues_hxx=$(writerfilter_WORK)/OOXMLFactory_values.hxx
writerfilter_GEN_ooxml_Factory_cxx=$(writerfilter_WORK)/OOXMLFactory_generated.cxx
writerfilter_GEN_ooxml_Factory_hxx=$(writerfilter_WORK)/OOXMLFactory_generated.hxx
writerfilter_GEN_ooxml_FastTokens_hxx=$(writerfilter_WORK)/ooxml/OOXMLFastTokens.hxx
writerfilter_GEN_ooxml_GperfFastToken_hxx=$(writerfilter_WORK)/gperffasttoken.hxx
writerfilter_GEN_ooxml_Model_analyzed=$(writerfilter_WORK)/ooxml/model_analyzed.xml
writerfilter_GEN_ooxml_Model_processed=$(writerfilter_WORK)/model_preprocessed.xml
writerfilter_GEN_ooxml_NamespaceIds_hxx=$(writerfilter_WORK)/ooxml/OOXMLnamespaceids.hxx
writerfilter_GEN_ooxml_Namespacesmap_xsl=$(writerfilter_WORK)/namespacesmap.xsl
writerfilter_GEN_ooxml_Preprocess_xsl=$(writerfilter_WORK)/modelpreprocess.xsl
writerfilter_GEN_ooxml_QNameToStr_cxx=$(writerfilter_WORK)/ooxml/qnametostr.cxx
writerfilter_GEN_ooxml_ResourceIds_hxx=$(writerfilter_WORK)/ooxml/resourceids.hxx
writerfilter_GEN_ooxml_Token_tmp=$(writerfilter_WORK)/token.tmp
writerfilter_GEN_ooxml_Token_xml=$(writerfilter_WORK)/token.xml
writerfilter_SRC_doctok_Model=$(writerfilter_SRC)/doctok/resources.xmi
writerfilter_SRC_doctok_ResourceTools_xsl=$(writerfilter_SRC)/doctok/resourcetools.xsl
writerfilter_SRC_doctok_SprmCodeToStr_xsl=$(writerfilter_SRC)/doctok/sprmcodetostr.xsl
writerfilter_SRC_doctok_SprmIds_xsl=$(writerfilter_SRC)/doctok/sprmids.xsl
writerfilter_SRC_model_NamespacePreprocess=$(writerfilter_SRC)/resourcemodel/namespace_preprocess.pl
writerfilter_SRC_ooxml_Analyze_model_xsl=$(writerfilter_SRC)/ooxml/analyzemodel.xsl
writerfilter_SRC_ooxml_FactoryTools_xsl=$(writerfilter_SRC)/ooxml/factorytools.xsl
writerfilter_SRC_ooxml_FactoryValues_xsl=$(writerfilter_SRC)/ooxml/factory_values.xsl
writerfilter_SRC_ooxml_FastTokens_xsl=$(writerfilter_SRC)/ooxml/fasttokens.xsl
writerfilter_SRC_ooxml_GperfFastTokenHandler_xsl=$(writerfilter_SRC)/ooxml/gperffasttokenhandler.xsl
writerfilter_SRC_ooxml_Model=$(writerfilter_SRC)/ooxml/model.xml
writerfilter_SRC_ooxml_NamespaceIds_xsl=$(writerfilter_SRC)/ooxml/namespaceids.xsl
writerfilter_SRC_ooxml_Preprocess_xsl=$(writerfilter_SRC)/ooxml/modelpreprocess.xsl
writerfilter_SRC_ooxml_QNameToStr_xsl=$(writerfilter_SRC)/ooxml/qnametostr.xsl
writerfilter_SRC_ooxml_ResourceIds_xsl=$(writerfilter_SRC)/ooxml/resourceids.xsl

$(writerfilter_GEN_doctok_SprmIds_hxx) : $(writerfilter_SRC_doctok_Model) $(writerfilter_SRC_doctok_SprmIds_xsl) | $(writerfilter_WORK)/doctok/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_doctok_SprmIds_xsl) $(writerfilter_SRC_doctok_Model)) > $@

$(writerfilter_GEN_model_SprmCodeToStr_cxx): $(writerfilter_SRC)/resourcemodel/sprmcodetostrheader $(writerfilter_GEN_model_SprmCodeToStr_tmp) $(writerfilter_SRC)/resourcemodel/sprmcodetostrfooter
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CAT,1)
	cat $^ > $@

$(writerfilter_GEN_model_SprmCodeToStr_tmp) : $(writerfilter_SRC_doctok_SprmCodeToStr_xsl) $(writerfilter_SRC_doctok_Model) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_doctok_SprmCodeToStr_xsl) $(writerfilter_SRC_doctok_Model)) > $@

$(writerfilter_GEN_ooxml_Factory_cxx) : $(writerfilter_SRC)/ooxml/factoryimpl.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_Factory_hxx) : $(writerfilter_SRC)/ooxml/factoryinc.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_FactoryValues_hxx) : $(writerfilter_SRC_ooxml_FactoryValues_xsl) $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_FactoryValues_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_FastTokens_hxx) : $(writerfilter_SRC_ooxml_FastTokens_xsl) $(writerfilter_GEN_ooxml_Token_xml) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_FastTokens_xsl) $(writerfilter_GEN_ooxml_Token_xml)) > $@

$(writerfilter_GEN_ooxml_GperfFastToken_hxx) : $(writerfilter_SRC_ooxml_GperfFastTokenHandler_xsl) $(writerfilter_GEN_ooxml_Token_xml)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,GPF,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_GperfFastTokenHandler_xsl) $(writerfilter_GEN_ooxml_Token_xml)) \
	| tr -d '\r' | $(GPERF) -c -E -G -I  -LC++ -S1 -t  > $@

$(writerfilter_GEN_ooxml_Model_analyzed) : $(writerfilter_SRC_ooxml_Analyze_model_xsl) $(writerfilter_SRC_ooxml_Model) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $^) > $@

$(writerfilter_GEN_ooxml_Model_processed) : $(writerfilter_GEN_ooxml_Namespacesmap_xsl) $(writerfilter_GEN_ooxml_Preprocess_xsl) $(writerfilter_SRC_ooxml_Model)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_GEN_ooxml_Namespacesmap_xsl) $(writerfilter_SRC_ooxml_Model)) > $@

$(writerfilter_GEN_ooxml_NamespaceIds_hxx) : $(writerfilter_SRC_ooxml_NamespaceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_NamespaceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_Namespacesmap_xsl) : $(writerfilter_SRC_ooxml_Model) $(writerfilter_DEP_ooxml_Namespaces_txt) \
		$(writerfilter_SRC_model_NamespacePreprocess) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PRL,1)
	$(PERL) $(writerfilter_SRC_model_NamespacePreprocess) $(writerfilter_DEP_ooxml_Namespaces_txt) > $@

$(writerfilter_GEN_ooxml_Preprocess_xsl) : $(writerfilter_SRC_ooxml_Preprocess_xsl) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CPY,1)
	cp -f $(writerfilter_SRC_ooxml_Preprocess_xsl) $@

$(writerfilter_GEN_ooxml_QNameToStr_cxx): $(writerfilter_SRC_ooxml_QNameToStr_xsl) $(writerfilter_SRC_ooxml_FactoryTools_xsl) $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_QNameToStr_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_ResourceIds_hxx) : $(writerfilter_SRC_ooxml_ResourceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_ResourceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_Token_tmp) : $(SRCDIR)/oox/source/token/tokens.txt | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CAT,1)
	sed "s/\(.*\)/<fasttoken>\1<\/fasttoken>/" \
		< $(SRCDIR)/oox/source/token/tokens.txt > $@

$(writerfilter_GEN_ooxml_Token_xml) : $(writerfilter_SRC)/ooxml/tokenxmlheader $(writerfilter_GEN_ooxml_Token_tmp) $(writerfilter_SRC)/ooxml/tokenxmlfooter
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CAT,1)
	cat $(writerfilter_SRC)/ooxml/tokenxmlheader $(writerfilter_GEN_ooxml_Token_tmp) $(writerfilter_SRC)/ooxml/tokenxmlfooter > $@

$(writerfilter_WORK)/OOXMLFactory%.cxx : $(writerfilter_SRC)/ooxml/factoryimpl_ns.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) --stringparam file $@ $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_WORK)/OOXMLFactory%.hxx : $(writerfilter_SRC)/ooxml/factory_ns.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) --stringparam file $@ $< $(writerfilter_GEN_ooxml_Model_processed)) > $@


$(call gb_CustomTarget_get_target,writerfilter/source) : $(writerfilter_ALL)

$(writerfilter_ALL) :| $(call gb_ExternalExecutable_get_dependencies,xsltproc) $(writerfilter_WORK)/.dir

# vim: set noet sw=4 ts=4:
