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
writerfilter_PYTHONCOMMAND := $(call gb_ExternalExecutable_get_command,python)

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
	a14 \
	wml

writerfilter_ALL = \
	$(writerfilter_GEN_ooxml_Factory_cxx) \
	$(writerfilter_GEN_ooxml_Factory_hxx) \
	$(writerfilter_GEN_ooxml_FactoryValues_hxx) \
	$(writerfilter_GEN_ooxml_FastTokens_hxx) \
	$(writerfilter_GEN_ooxml_GperfFastToken_hxx) \
	$(writerfilter_GEN_ooxml_NamespaceIds_hxx) \
	$(writerfilter_GEN_ooxml_QNameToStr_cxx) \
	$(writerfilter_GEN_ooxml_ResourceIds_hxx) \
	$(writerfilter_GEN_ooxml_Model_processed) \
	$(patsubst %,$(writerfilter_WORK)/ooxml/OOXMLFactory_%.hxx,$(writerfilter_OOXMLNAMESPACES)) \
	$(patsubst %,$(writerfilter_WORK)/ooxml/OOXMLFactory_%.cxx,$(writerfilter_OOXMLNAMESPACES)) \

writerfilter_DEP_ooxml_Namespaces_txt=$(call gb_CustomTarget_get_workdir,oox/generated)/misc/namespaces.txt
writerfilter_GEN_ooxml_FactoryValues_hxx=$(writerfilter_WORK)/ooxml/OOXMLFactory_values.hxx
writerfilter_GEN_ooxml_Factory_cxx=$(writerfilter_WORK)/ooxml/OOXMLFactory_generated.cxx
writerfilter_GEN_ooxml_Factory_hxx=$(writerfilter_WORK)/ooxml/OOXMLFactory_generated.hxx
writerfilter_GEN_ooxml_FastTokens_hxx=$(writerfilter_WORK)/ooxml/OOXMLFastTokens.hxx
writerfilter_GEN_ooxml_GperfFastToken_hxx=$(writerfilter_WORK)/ooxml/gperffasttoken.hxx
writerfilter_GEN_ooxml_Model_processed=$(writerfilter_WORK)/ooxml/model_preprocessed.xml
writerfilter_GEN_ooxml_NamespaceIds_hxx=$(writerfilter_WORK)/ooxml/OOXMLnamespaceids.hxx
writerfilter_GEN_ooxml_Namespacesmap_xsl=$(writerfilter_WORK)/ooxml/namespacesmap.xsl
writerfilter_GEN_ooxml_QNameToStr_cxx=$(writerfilter_WORK)/ooxml/qnametostr.cxx
writerfilter_GEN_ooxml_ResourceIds_hxx=$(writerfilter_WORK)/ooxml/resourceids.hxx
writerfilter_GEN_ooxml_Token_xml=$(writerfilter_WORK)/ooxml/token.xml
writerfilter_SRC_ooxml_FactoryTools_xsl=$(writerfilter_SRC)/ooxml/factorytools.xsl
writerfilter_SRC_ooxml_FactoryValues_py=$(writerfilter_SRC)/ooxml/factory_values.py
writerfilter_SRC_ooxml_FastTokens_py=$(writerfilter_SRC)/ooxml/fasttokens.py
writerfilter_SRC_ooxml_GperfFastTokenHandler_py=$(writerfilter_SRC)/ooxml/gperffasttokenhandler.py
writerfilter_SRC_ooxml_Model=$(writerfilter_SRC)/ooxml/model.xml
writerfilter_SRC_ooxml_NamespaceIds_xsl=$(writerfilter_SRC)/ooxml/namespaceids.xsl
writerfilter_SRC_ooxml_Preprocess_py=$(writerfilter_SRC)/ooxml/modelpreprocess.py
writerfilter_SRC_ooxml_QNameToStr_xsl=$(writerfilter_SRC)/ooxml/qnametostr.xsl
writerfilter_SRC_ooxml_ResourceIds_py=$(writerfilter_SRC)/ooxml/resourceids.py

$(writerfilter_GEN_ooxml_Factory_cxx) : $(writerfilter_SRC)/ooxml/factoryimpl.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_Factory_hxx) : $(writerfilter_SRC)/ooxml/factoryinc.py $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_FactoryValues_hxx) : $(writerfilter_SRC_ooxml_FactoryValues_py) $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $(writerfilter_SRC_ooxml_FactoryValues_py) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_FastTokens_hxx) : $(writerfilter_SRC_ooxml_FastTokens_py) $(writerfilter_GEN_ooxml_Token_xml) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $(writerfilter_SRC_ooxml_FastTokens_py) $(writerfilter_GEN_ooxml_Token_xml)) > $@

$(writerfilter_GEN_ooxml_GperfFastToken_hxx) : $(writerfilter_SRC_ooxml_GperfFastTokenHandler_py) $(writerfilter_GEN_ooxml_Token_xml)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,GPF,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $(writerfilter_SRC_ooxml_GperfFastTokenHandler_py) $(writerfilter_GEN_ooxml_Token_xml)) \
	| tr -d '\r' | $(GPERF) -c -E -G -I  -LC++ -S1 -t  > $@

$(writerfilter_GEN_ooxml_Model_processed) : $(writerfilter_SRC_ooxml_Preprocess_py) $(writerfilter_DEP_ooxml_Namespaces_txt) $(writerfilter_SRC_ooxml_Model) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $(writerfilter_SRC_ooxml_Preprocess_py) $(writerfilter_DEP_ooxml_Namespaces_txt) $(writerfilter_SRC_ooxml_Model)) > $@

$(writerfilter_GEN_ooxml_NamespaceIds_hxx) : $(writerfilter_SRC_ooxml_NamespaceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_NamespaceIds_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_QNameToStr_cxx): $(writerfilter_SRC_ooxml_QNameToStr_xsl) $(writerfilter_SRC_ooxml_FactoryTools_xsl) $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) $(writerfilter_SRC_ooxml_QNameToStr_xsl) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_ResourceIds_hxx) : $(writerfilter_SRC_ooxml_ResourceIds_py) $(writerfilter_GEN_ooxml_Model_processed) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,PY ,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_PYTHONCOMMAND) $(writerfilter_SRC_ooxml_ResourceIds_py) $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_GEN_ooxml_Token_xml) : $(SRCDIR)/oox/source/token/tokens.txt $(writerfilter_SRC)/ooxml/tokens-to-xml.sed | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,CAT,1)
	sed -f $(writerfilter_SRC)/ooxml/tokens-to-xml.sed \
		< $(SRCDIR)/oox/source/token/tokens.txt > $@

$(writerfilter_WORK)/ooxml/OOXMLFactory%.cxx : $(writerfilter_SRC)/ooxml/factoryimpl_ns.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) --stringparam file $@ $< $(writerfilter_GEN_ooxml_Model_processed)) > $@

$(writerfilter_WORK)/ooxml/OOXMLFactory%.hxx : $(writerfilter_SRC)/ooxml/factory_ns.xsl $(writerfilter_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(writerfilter_XSLTCOMMAND) --stringparam file $@ $< $(writerfilter_GEN_ooxml_Model_processed)) > $@


$(call gb_CustomTarget_get_target,writerfilter/source) : $(writerfilter_ALL)

$(writerfilter_ALL) :| $(call gb_ExternalExecutable_get_dependencies,xsltproc) $(call gb_ExternalExecutable_get_dependencies,python) $(writerfilter_WORK)/ooxml/.dir

# vim: set noet sw=4 ts=4:
