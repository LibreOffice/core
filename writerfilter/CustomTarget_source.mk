# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_CustomTarget_CustomTarget,writerfilter/source))

writerfilter_WORK := $(call gb_CustomTarget_get_workdir,writerfilter/source)
writerfilter_SRC := $(SRCDIR)/writerfilter/source

include $(writerfilter_SRC)/generated.mk

# doctok

wf_SRC_doctok_Model=$(writerfilter_SRC)/doctok/resources.xmi
wf_SRC_doctok_Resources_xsl=$(writerfilter_SRC)/doctok/resources.xsl
wf_SRC_doctok_ResourcesImpl_xsl=$(writerfilter_SRC)/doctok/resourcesimpl.xsl
wf_SRC_doctok_ResourceIds_xsl=$(writerfilter_SRC)/doctok/resourceids.xsl
wf_SRC_doctok_SprmIds_xsl=$(writerfilter_SRC)/doctok/sprmids.xsl
wf_SRC_doctok_ResourceTools_xsl=$(writerfilter_SRC)/doctok/resourcetools.xsl

wf_GEN_doctok_SprmIds_hxx := $(writerfilter_WORK)/doctok/sprmids.hxx
wf_GEN_doctok_ResourceIds_hxx := $(writerfilter_WORK)/doctok/resourceids.hxx
wf_GEN_doctok_Resources_hxx := $(writerfilter_WORK)/doctok/resources.hxx
wf_GEN_doctok_Resources_cxx := $(writerfilter_WORK)/resources.cxx

$(wf_GEN_doctok_SprmIds_hxx) : $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model) | $(writerfilter_WORK)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_ResourceIds_hxx) : $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model) | $(writerfilter_WORK)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_hxx) : $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model) | $(writerfilter_WORK)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_cxx) : $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model) $(wf_SRC_doctok_ResourceTools_xsl) | $(writerfilter_WORK)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model)) > $@

# ooxml

wf_SRC_ooxml_Model=$(writerfilter_SRC)/ooxml/model.xml
wf_SRC_ooxml_Preprocess_xsl=$(writerfilter_SRC)/ooxml/modelpreprocess.xsl
wf_SRC_ooxml_FactoryTools_xsl=$(writerfilter_SRC)/ooxml/factorytools.xsl
wf_SRC_ooxml_FastTokens_xsl=$(writerfilter_SRC)/ooxml/fasttokens.xsl
wf_SRC_ooxml_NamespaceIds_xsl=$(writerfilter_SRC)/ooxml/namespaceids.xsl
wf_SRC_ooxml_FactoryValues_xsl=$(writerfilter_SRC)/ooxml/factory_values.xsl
wf_SRC_ooxml_FactoryValuesImpl_xsl=$(writerfilter_SRC)/ooxml/factoryimpl_values.xsl
wf_SRC_ooxml_ResourceIds_xsl=$(writerfilter_SRC)/ooxml/resourceids.xsl
wf_SRC_ooxml_GperfFastTokenHandler_xsl=$(writerfilter_SRC)/ooxml/gperffasttokenhandler.xsl
wf_SRC_ooxml_Analyze_model_xsl=$(writerfilter_SRC)/ooxml/analyzemodel.xsl
wf_GEN_ooxml_ResourceIds_hxx=$(writerfilter_WORK)/ooxml/resourceids.hxx

wf_GEN_ooxml_token_xml=$(writerfilter_WORK)/token.xml
wf_GEN_ooxml_token_tmp=$(writerfilter_WORK)/token.tmp

wf_GEN_ooxml_Factory_hxx=$(writerfilter_WORK)/OOXMLFactory_generated.hxx
wf_GEN_ooxml_Factory_cxx=$(writerfilter_WORK)/OOXMLFactory_generated.cxx
wf_GEN_ooxml_FastTokens_hxx=$(writerfilter_WORK)/ooxml/OOXMLFastTokens.hxx
wf_GEN_ooxml_NamespaceIds_hxx=$(writerfilter_WORK)/ooxml/OOXMLnamespaceids.hxx
wf_GEN_ooxml_FactoryValues_hxx=$(writerfilter_WORK)/OOXMLFactory_values.hxx
wf_GEN_ooxml_FactoryValues_cxx=$(writerfilter_WORK)/OOXMLFactory_values.cxx
wf_GEN_ooxml_GperfFastToken_hxx=$(writerfilter_WORK)/gperffasttoken.hxx
wf_GEN_ooxml_Model_processed=$(writerfilter_WORK)/model_preprocessed.xml
wf_GEN_ooxml_Model_analyzed=$(writerfilter_WORK)/ooxml/model_analyzed.xml

$(wf_GEN_ooxml_token_tmp) : $(OUTDIR)/inc/oox/tokens.txt | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $(OUTDIR)/inc/oox/tokens.txt \
	| sed "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@

$(wf_GEN_ooxml_token_xml) : $(writerfilter_SRC)/ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(writerfilter_SRC)/ooxml/tokenxmlfooter
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $(writerfilter_SRC)/ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(writerfilter_SRC)/ooxml/tokenxmlfooter > $@

$(wf_GEN_ooxml_FastTokens_hxx) : $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)) > $@

$(wf_GEN_ooxml_Factory_hxx) : $(writerfilter_SRC)/ooxml/factoryinc.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_Factory_cxx) : $(writerfilter_SRC)/ooxml/factoryimpl.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

$(writerfilter_WORK)/OOXMLFactory%.cxx : $(writerfilter_SRC)/ooxml/factoryimpl_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

$(writerfilter_WORK)/OOXMLFactory%.hxx : $(writerfilter_SRC)/ooxml/factory_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_hxx) : $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_cxx) : $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_ResourceIds_hxx) : $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_NamespaceIds_hxx) : $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_GperfFastToken_hxx) : $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_token_xml)
	$(call gb_Output_announce,$@,build,GPF,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_token_xml)) \
	| tr -d '\r' | $(GPERF) -I -t -E -S1 -c -G -LC++ > $@

$(wf_GEN_ooxml_Model_analyzed) : $(wf_SRC_ooxml_Analyze_model_xsl) $(wf_SRC_ooxml_Model) | $(writerfilter_WORK)/ooxml/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $^) > $@

# rtftok

wf_SRC_rtftok_scanner_lex=$(writerfilter_SRC)/rtftok/RTFScanner.lex
wf_SRC_rtftok_scanner_skl=$(writerfilter_SRC)/rtftok/RTFScanner.skl

wf_GEN_rtftok_scanner_cxx=$(writerfilter_WORK)/RTFScanner.cxx

# does not build, currently unused
#$(wf_GEN_rtftok_scanner_cxx) : $(wf_SRC_rtftok_scanner_lex) $(wf_SRC_rtftok_scanner_skl)
#	flex -+ -S$(wf_SRC_rtftok_scanner_skl) -o$@ $(wf_SRC_rtftok_scanner_lex)

# resourcemodel

wf_GEN_doctok_QNameToStr_cxx=$(writerfilter_WORK)/doctok/qnametostr.cxx
wf_GEN_ooxml_QNameToStr_cxx=$(writerfilter_WORK)/ooxml/qnametostr.cxx
wf_GEN_model_SprmCodeToStr_cxx=$(writerfilter_WORK)/sprmcodetostr.cxx
wf_GEN_model_SprmCodeToStr_tmp=$(writerfilter_WORK)/sprmcodetostr.tmp
wf_GEN_doctok_QnameToStr_tmp=$(writerfilter_WORK)/DOCTOKqnameToStr.tmp
wf_SRC_doctok_SprmCodeToStr_xsl=$(writerfilter_SRC)/doctok/sprmcodetostr.xsl
wf_SRC_doctok_QNameToStr_xsl=$(writerfilter_SRC)/doctok/qnametostr.xsl
wf_SRC_ooxml_QNameToStr_xsl=$(writerfilter_SRC)/ooxml/qnametostr.xsl
wf_SRC_model_NamespacePreprocess=$(writerfilter_SRC)/resourcemodel/namespace_preprocess.pl
wf_GEN_ooxml_Namespacesmap_xsl=$(writerfilter_WORK)/namespacesmap.xsl
wf_DEP_ooxml_namespaces_txt=$(OUTDIR)/inc/oox/namespaces.txt
wf_GEN_ooxml_preprocess_xsl=$(writerfilter_WORK)/modelpreprocess.xsl

$(wf_GEN_ooxml_QNameToStr_cxx): $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed) $(wf_SRC_ooxml_FactoryTools_xsl)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_doctok_QNameToStr_cxx): $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model) $(wf_SRC_doctok_ResourceTools_xsl) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_SprmCodeToStr_tmp) : $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_SprmCodeToStr_cxx): $(writerfilter_SRC)/resourcemodel/sprmcodetostrheader $(wf_GEN_model_SprmCodeToStr_tmp) $(writerfilter_SRC)/resourcemodel/sprmcodetostrfooter
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $^ > $@

$(wf_GEN_ooxml_Namespacesmap_xsl) : $(wf_SRC_ooxml_Model) $(wf_DEP_ooxml_namespaces_txt) \
		$(wf_SRC_model_NamespacePreprocess) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$@,build,PRL,1)
	$(PERL) $(wf_SRC_model_NamespacePreprocess) $(wf_DEP_ooxml_namespaces_txt) > $@

$(wf_GEN_ooxml_preprocess_xsl) : $(wf_SRC_ooxml_Preprocess_xsl) | $(writerfilter_WORK)/.dir
	$(call gb_Output_announce,$@,build,CPY,1)
	cp -f $(wf_SRC_ooxml_Preprocess_xsl) $@

$(wf_GEN_ooxml_Model_processed) : $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_GEN_ooxml_preprocess_xsl) $(wf_SRC_ooxml_Model)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs, $(gb_XSLTPROC) $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_SRC_ooxml_Model)) > $@

# all

wf_all := \
	$(wf_GEN_doctok_SprmIds_hxx) \
	$(wf_GEN_doctok_ResourceIds_hxx) \
	$(wf_GEN_doctok_Resources_hxx) \
	$(wf_GEN_doctok_Resources_cxx) \
	$(wf_GEN_ooxml_ResourceIds_hxx) \
	$(wf_GEN_ooxml_Factory_hxx) \
	$(wf_GEN_ooxml_Factory_cxx) \
	$(wf_GEN_ooxml_FactoryValues_hxx) \
	$(wf_GEN_ooxml_FactoryValues_cxx) \
	$(wf_GEN_ooxml_FastTokens_hxx) \
	$(wf_GEN_ooxml_GperfFastToken_hxx) \
	$(wf_GEN_ooxml_NamespaceIds_hxx) \
	$(wf_GEN_doctok_QNameToStr_cxx) \
	$(wf_GEN_ooxml_QNameToStr_cxx) \
	$(wf_GEN_model_SprmCodeToStr_cxx) \
	$(patsubst %,$(writerfilter_WORK)/OOXMLFactory_%.hxx,$(WRITERFILTER_OOXMLNAMESPACES)) \
	$(patsubst %,$(writerfilter_WORK)/OOXMLFactory_%.cxx,$(WRITERFILTER_OOXMLNAMESPACES)) \

$(call gb_CustomTarget_get_target,writerfilter/source) : $(wf_all)

$(wf_all) :| $(gb_XSLTPROCTARGET) $(writerfilter_WORK)/.dir

# vim: set noet sw=4 ts=4:
