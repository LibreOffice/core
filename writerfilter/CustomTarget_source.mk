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


$(eval $(call gb_CustomTarget_CustomTarget,writerfilter/source,new_style))

WFSG := $(call gb_CustomTarget_get_workdir,writerfilter/source)
WFDIR := $(SRCDIR)/writerfilter/source/

include $(WFDIR)generated.mk

# doctok

wf_SRC_doctok_Model=$(WFDIR)doctok/resources.xmi
wf_SRC_doctok_Resources_xsl=$(WFDIR)doctok/resources.xsl
wf_SRC_doctok_ResourcesImpl_xsl=$(WFDIR)doctok/resourcesimpl.xsl
wf_SRC_doctok_ResourceIds_xsl=$(WFDIR)doctok/resourceids.xsl
wf_SRC_doctok_SprmIds_xsl=$(WFDIR)doctok/sprmids.xsl
wf_SRC_doctok_ResourceTools_xsl=$(WFDIR)doctok/resourcetools.xsl

wf_GEN_doctok_SprmIds_hxx := $(WFSG)/doctok/sprmids.hxx
wf_GEN_doctok_ResourceIds_hxx := $(WFSG)/doctok/resourceids.hxx
wf_GEN_doctok_Resources_hxx := $(WFSG)/doctok/resources.hxx
wf_GEN_doctok_Resources_cxx := $(WFSG)/resources.cxx

$(wf_GEN_doctok_SprmIds_hxx) : $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model) | $(WFSG)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_ResourceIds_hxx) : $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model) | $(WFSG)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_hxx) : $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model) | $(WFSG)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_cxx) : $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model) $(wf_SRC_doctok_ResourceTools_xsl) | $(WFSG)/doctok/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model)) > $@

# ooxml

wf_SRC_ooxml_Model=$(WFDIR)ooxml/model.xml
wf_SRC_ooxml_Preprocess_xsl=$(WFDIR)ooxml/modelpreprocess.xsl
wf_SRC_ooxml_FactoryTools_xsl=$(WFDIR)ooxml/factorytools.xsl
wf_SRC_ooxml_FastTokens_xsl=$(WFDIR)ooxml/fasttokens.xsl
wf_SRC_ooxml_NamespaceIds_xsl=$(WFDIR)ooxml/namespaceids.xsl
wf_SRC_ooxml_FactoryValues_xsl=$(WFDIR)ooxml/factory_values.xsl
wf_SRC_ooxml_FactoryValuesImpl_xsl=$(WFDIR)ooxml/factoryimpl_values.xsl
wf_SRC_ooxml_ResourceIds_xsl=$(WFDIR)ooxml/resourceids.xsl
wf_SRC_ooxml_GperfFastTokenHandler_xsl=$(WFDIR)ooxml/gperffasttokenhandler.xsl
wf_SRC_ooxml_Analyze_model_xsl=$(WFDIR)ooxml/analyzemodel.xsl
wf_GEN_ooxml_ResourceIds_hxx=$(WFSG)/ooxml/resourceids.hxx

wf_GEN_ooxml_token_xml=$(WFSG)/token.xml
wf_GEN_ooxml_token_tmp=$(WFSG)/token.tmp

wf_GEN_ooxml_Factory_hxx=$(WFSG)/OOXMLFactory_generated.hxx
wf_GEN_ooxml_Factory_cxx=$(WFSG)/OOXMLFactory_generated.cxx
wf_GEN_ooxml_FastTokens_hxx=$(WFSG)/ooxml/OOXMLFastTokens.hxx
wf_GEN_ooxml_NamespaceIds_hxx=$(WFSG)/ooxml/OOXMLnamespaceids.hxx
wf_GEN_ooxml_FactoryValues_hxx=$(WFSG)/OOXMLFactory_values.hxx
wf_GEN_ooxml_FactoryValues_cxx=$(WFSG)/OOXMLFactory_values.cxx
wf_GEN_ooxml_GperfFastToken_hxx=$(WFSG)/gperffasttoken.hxx
wf_GEN_ooxml_Model_processed=$(WFSG)/model_preprocessed.xml
wf_GEN_ooxml_Model_analyzed=$(WFSG)/ooxml/model_analyzed.xml

$(wf_GEN_ooxml_token_tmp) : $(OUTDIR)/inc/oox/tokens.txt
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $(OUTDIR)/inc/oox/tokens.txt \
	| sed "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@

$(wf_GEN_ooxml_token_xml) : $(WFDIR)ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(WFDIR)ooxml/tokenxmlfooter
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $(WFDIR)ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(WFDIR)ooxml/tokenxmlfooter > $@

$(wf_GEN_ooxml_FastTokens_hxx) : $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)) > $@

$(wf_GEN_ooxml_Factory_hxx) : $(WFDIR)ooxml/factoryinc.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_Factory_cxx) : $(WFDIR)ooxml/factoryimpl.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

$(WFSG)/OOXMLFactory%.cxx : $(WFDIR)ooxml/factoryimpl_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

$(WFSG)/OOXMLFactory%.hxx : $(WFDIR)ooxml/factory_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_hxx) : $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_cxx) : $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_ResourceIds_hxx) : $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed) | $(WFSG)/ooxml/.dir
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_NamespaceIds_hxx) : $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_GperfFastToken_hxx) : $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_token_xml)
	$(call gb_Output_announce,$@,build,GPF,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_token_xml)) \
	| tr -d '\r' | $(GPERF) -I -t -E -S1 -c -G -LC++ > $@

$(wf_GEN_ooxml_Model_analyzed): $(wf_SRC_ooxml_Analyze_model_xsl) $(wf_SRC_ooxml_Model)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $^) > $@

# rtftok

wf_SRC_rtftok_scanner_lex=$(WFDIR)rtftok/RTFScanner.lex
wf_SRC_rtftok_scanner_skl=$(WFDIR)rtftok/RTFScanner.skl

wf_GEN_rtftok_scanner_cxx=$(WFSG)/RTFScanner.cxx

# does not build, currently unused
#$(wf_GEN_rtftok_scanner_cxx) : $(wf_SRC_rtftok_scanner_lex) $(wf_SRC_rtftok_scanner_skl)
#	flex -+ -S$(wf_SRC_rtftok_scanner_skl) -o$@ $(wf_SRC_rtftok_scanner_lex)

# resourcemodel

wf_GEN_model_QNameToStr_cxx=$(WFSG)/qnametostr.cxx
wf_GEN_ooxml_QNameToStr_tmp=$(WFSG)/OOXMLqnameToStr.tmp
wf_GEN_model_SprmCodeToStr_cxx=$(WFSG)/sprmcodetostr.cxx
wf_GEN_model_SprmCodeToStr_tmp=$(WFSG)/sprmcodetostr.tmp
wf_GEN_doctok_QnameToStr_tmp=$(WFSG)/DOCTOKqnameToStr.tmp
wf_SRC_doctok_SprmCodeToStr_xsl=$(WFDIR)doctok/sprmcodetostr.xsl
wf_SRC_doctok_QNameToStr_xsl=$(WFDIR)doctok/qnametostr.xsl
wf_SRC_ooxml_QNameToStr_xsl=$(WFDIR)ooxml/qnametostr.xsl
wf_SRC_model_NamespacePreprocess=$(WFDIR)resourcemodel/namespace_preprocess.pl
wf_GEN_ooxml_Namespacesmap_xsl=$(WFSG)/namespacesmap.xsl
wf_DEP_ooxml_namespaces_txt=$(OUTDIR)/inc/oox/namespaces.txt
wf_GEN_ooxml_preprocess_xsl=$(WFSG)/modelpreprocess.xsl

$(wf_GEN_ooxml_QNameToStr_tmp): $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_doctok_QnameToStr_tmp): $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_QNameToStr_cxx): $(wf_GEN_ooxml_QNameToStr_tmp) $(wf_GEN_doctok_QnameToStr_tmp) $(WFDIR)resourcemodel/qnametostrheader $(WFDIR)resourcemodel/qnametostrfooter $(wf_SRC_ooxml_FactoryTools_xsl) $(wf_SRC_doctok_ResourceTools_xsl)
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $(WFDIR)resourcemodel/qnametostrheader $(wf_GEN_ooxml_QNameToStr_tmp) $(wf_GEN_doctok_QnameToStr_tmp) $(WFDIR)resourcemodel/qnametostrfooter > $@

$(wf_GEN_model_SprmCodeToStr_tmp) : $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_SprmCodeToStr_cxx): $(WFDIR)resourcemodel/sprmcodetostrheader $(wf_GEN_model_SprmCodeToStr_tmp) $(WFDIR)resourcemodel/sprmcodetostrfooter
	$(call gb_Output_announce,$@,build,CAT,1)
	cat $^ > $@

$(wf_GEN_ooxml_Namespacesmap_xsl) : $(wf_SRC_ooxml_Model) $(wf_DEP_ooxml_namespaces_txt) $(wf_SRC_model_NamespacePreprocess)
	$(call gb_Output_announce,$@,build,PRL,1)
	$(PERL) $(wf_SRC_model_NamespacePreprocess) $(wf_DEP_ooxml_namespaces_txt) > $@

$(wf_GEN_ooxml_preprocess_xsl) : $(wf_SRC_ooxml_Preprocess_xsl)
	$(call gb_Output_announce,$@,build,CPY,1)
	cp -f $(wf_SRC_ooxml_Preprocess_xsl) $@

$(wf_GEN_ooxml_Model_processed) : $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_GEN_ooxml_preprocess_xsl) $(wf_SRC_ooxml_Model)
	$(call gb_Output_announce,$@,build,XSL,1)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_SRC_ooxml_Model)) > $@

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
	$(wf_GEN_model_QNameToStr_cxx) \
	$(wf_GEN_model_SprmCodeToStr_cxx) \
	$(patsubst %,$(WFSG)/OOXMLFactory_%.hxx,$(WRITERFILTER_OOXMLNAMESPACES)) \
	$(patsubst %,$(WFSG)/OOXMLFactory_%.cxx,$(WRITERFILTER_OOXMLNAMESPACES)) \

$(call gb_CustomTarget_get_target,writerfilter/source) : $(wf_all)

$(wf_all) :| $(gb_XSLTPROCTARGET) $(WFSG)/.dir

# vim: set noet sw=4 ts=4:
