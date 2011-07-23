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

WFDIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

include $(WFDIR)generated.mk
include $(GBUILDDIR)/gbuild_simple.mk

# doctok

doctok :
	mkdir doctok

wf_SRC_doctok_Model=$(WFDIR)doctok/resources.xmi
wf_SRC_doctok_Resources_xsl=$(WFDIR)doctok/resources.xsl
wf_SRC_doctok_ResourcesImpl_xsl=$(WFDIR)doctok/resourcesimpl.xsl
wf_SRC_doctok_ResourceIds_xsl=$(WFDIR)doctok/resourceids.xsl
wf_SRC_doctok_SprmIds_xsl=$(WFDIR)doctok/sprmids.xsl
wf_SRC_doctok_ResourceTools_xsl=$(WFDIR)doctok/resourcetools.xsl

wf_GEN_doctok_SprmIds_hxx=doctok/sprmids.hxx
wf_GEN_doctok_ResourceIds_hxx=doctok/resourceids.hxx
wf_GEN_doctok_Resources_hxx=doctok/resources.hxx
wf_GEN_doctok_Resources_cxx=resources.cxx

$(wf_GEN_doctok_SprmIds_hxx) : doctok $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_ResourceIds_hxx) : doctok $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourceIds_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_hxx) : doctok $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_Resources_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_doctok_Resources_cxx) : doctok $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model) $(wf_SRC_doctok_ResourceTools_xsl)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_ResourcesImpl_xsl) $(wf_SRC_doctok_Model)) > $@

# ooxml

ooxml :
	mkdir ooxml

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
wf_GEN_ooxml_ResourceIds_hxx=ooxml/resourceids.hxx

wf_GEN_ooxml_token_xml=token.xml
wf_GEN_ooxml_token_tmp=token.tmp

wf_GEN_ooxml_Factory_hxx=OOXMLFactory_generated.hxx
wf_GEN_ooxml_Factory_cxx=OOXMLFactory_generated.cxx
wf_GEN_ooxml_FastTokens_hxx=ooxml/OOXMLFastTokens.hxx
wf_GEN_ooxml_NamespaceIds_hxx=ooxml/OOXMLnamespaceids.hxx
wf_GEN_ooxml_FactoryValues_hxx=OOXMLFactory_values.hxx
wf_GEN_ooxml_FactoryValues_cxx=OOXMLFactory_values.cxx
wf_GEN_ooxml_GperfFastToken_hxx=gperffasttoken.hxx
wf_GEN_ooxml_Model_processed=model_preprocessed.xml
wf_GEN_ooxml_Model_analyzed=ooxml/model_analyzed.xml

$(wf_GEN_ooxml_token_tmp) : $(OUTDIR)/inc$(UPDMINOREXT)/oox/token/tokens.txt
	cat $(OUTDIR)/inc$(UPDMINOREXT)/oox/token/tokens.txt \
	| sed "s#\(.*\)#<fasttoken>\1</fasttoken>#" > $@

$(wf_GEN_ooxml_token_xml) : $(WFDIR)ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(WFDIR)ooxml/tokenxmlfooter
	cat $(WFDIR)ooxml/tokenxmlheader $(wf_GEN_ooxml_token_tmp) $(WFDIR)ooxml/tokenxmlfooter > $@

$(wf_GEN_ooxml_FastTokens_hxx) : $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FastTokens_xsl) $(wf_GEN_ooxml_token_xml)) > $@

$(wf_GEN_ooxml_Factory_hxx) : $(WFDIR)ooxml/factoryinc.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_Factory_cxx) : $(WFDIR)ooxml/factoryimpl.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $< $(wf_GEN_ooxml_Model_processed)) > $@

OOXMLFactory%.cxx : $(WFDIR)ooxml/factoryimpl_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

OOXMLFactory%.hxx : $(WFDIR)ooxml/factory_ns.xsl $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) --stringparam file $@ $< $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_hxx) : $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValues_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_FactoryValues_cxx) : $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_FactoryValuesImpl_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_ResourceIds_hxx) : ooxml $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_ResourceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_NamespaceIds_hxx) : $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_NamespaceIds_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_ooxml_GperfFastToken_hxx) : $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_GperfFastTokenHandler_xsl) $(wf_GEN_ooxml_Model_processed)) \
	| tr -d '\r' | gperf -I -t -E -S1 -c -G -LC++ > $@

$(wf_GEN_ooxml_Model_analyzed): $(wf_SRC_ooxml_Analyze_model_xsl) $(wf_SRC_ooxml_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $^) > $@

# rtftok

wf_SRC_rtftok_scanner_lex=$(WFDIR)rtftok/RTFScanner.lex
wf_SRC_rtftok_scanner_skl=$(WFDIR)rtftok/RTFScanner.skl

wf_GEN_rtftok_scanner_cxx=RTFScanner.cxx

# does not build, currently unused
#$(wf_GEN_rtftok_scanner_cxx) : $(wf_SRC_rtftok_scanner_lex) $(wf_SRC_rtftok_scanner_skl)
#	flex -+ -S$(wf_SRC_rtftok_scanner_skl) -o$@ $(wf_SRC_rtftok_scanner_lex)

# resourcemodel

wf_GEN_model_QNameToStr_cxx=qnametostr.cxx
wf_GEN_ooxml_QNameToStr_tmp=OOXMLqnameToStr.tmp
wf_GEN_model_SprmCodeToStr_cxx=sprmcodetostr.cxx
wf_GEN_model_SprmCodeToStr_tmp=sprmcodetostr.tmp
wf_GEN_doctok_QnameToStr_tmp=DOCTOKqnameToStr.tmp
wf_SRC_doctok_SprmCodeToStr_xsl=$(WFDIR)doctok/sprmcodetostr.xsl
wf_SRC_doctok_QNameToStr_xsl=$(WFDIR)doctok/qnametostr.xsl
wf_SRC_ooxml_QNameToStr_xsl=$(WFDIR)ooxml/qnametostr.xsl
wf_SRC_model_NamespacePreprocess=$(WFDIR)resourcemodel/namespace_preprocess.pl
wf_GEN_ooxml_Namespacesmap_xsl=namespacesmap.xsl
wf_DEP_ooxml_namespaces_txt=$(OUTDIR)/inc$(UPDMINOREXT)/oox/namespaces.txt
wf_GEN_ooxml_preprocess_xsl=modelpreprocess.xsl

$(wf_GEN_ooxml_QNameToStr_tmp): $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_ooxml_QNameToStr_xsl) $(wf_GEN_ooxml_Model_processed)) > $@

$(wf_GEN_doctok_QnameToStr_tmp): $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_QNameToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_QNameToStr_cxx): $(wf_GEN_ooxml_QNameToStr_tmp) $(wf_GEN_doctok_QnameToStr_tmp) $(WFDIR)resourcemodel/qnametostrheader $(WFDIR)resourcemodel/qnametostrfooter $(wf_SRC_ooxml_FactoryTools_xsl) $(wf_SRC_doctok_ResourceTools_xsl)
	cat $(WFDIR)resourcemodel/qnametostrheader $(wf_GEN_ooxml_QNameToStr_tmp) $(wf_GEN_doctok_QnameToStr_tmp) $(WFDIR)resourcemodel/qnametostrfooter > $@

$(wf_GEN_model_SprmCodeToStr_tmp) : $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_SRC_doctok_SprmCodeToStr_xsl) $(wf_SRC_doctok_Model)) > $@

$(wf_GEN_model_SprmCodeToStr_cxx): $(WFDIR)resourcemodel/sprmcodetostrheader $(wf_GEN_model_SprmCodeToStr_tmp) $(WFDIR)resourcemodel/sprmcodetostrfooter
	cat $^ > $@

$(wf_GEN_ooxml_Namespacesmap_xsl) : $(wf_SRC_ooxml_Model) $(wf_DEP_ooxml_namespaces_txt) $(wf_SRC_model_NamespacePreprocess)
	$(PERL) $(wf_SRC_model_NamespacePreprocess) $(wf_DEP_ooxml_namespaces_txt) > $@

$(wf_GEN_ooxml_preprocess_xsl) : $(wf_SRC_ooxml_Preprocess_xsl)
	cp -f $(wf_SRC_ooxml_Preprocess_xsl) $@

$(wf_GEN_ooxml_Model_processed) : $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_GEN_ooxml_preprocess_xsl) $(wf_SRC_ooxml_Model)
	$(call gb_Helper_abbreviate_dirs_native, $(gb_XSLTPROC) $(wf_GEN_ooxml_Namespacesmap_xsl) $(wf_SRC_ooxml_Model)) > $@

# all

.DEFAULT_GOAL:=all
.PHONY: all
all : \
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
	$(patsubst %,OOXMLFactory_%.hxx,$(WRITERFILTER_OOXMLNAMESPACES)) \
	$(patsubst %,OOXMLFactory_%.cxx,$(WRITERFILTER_OOXMLNAMESPACES)) \
