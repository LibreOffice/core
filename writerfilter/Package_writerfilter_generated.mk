###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



$(eval $(call \
	gb_Package_Package,writerfilter_generated,$(WORKDIR)/CustomTarget/writerfilter/source))

$(eval $(call \
	gb_Package_add_customtarget,writerfilter_generated,writerfilter/source,SRCDIR))

$(eval $(call \
	gb_CustomTarget_add_outdir_dependencies,writerfilter/source,\
		$(gb_XSLTPROCTARGET) \
		$(OUTDIR)/inc$(UPDMINOREXT)/oox/token/tokens.txt \
		$(OUTDIR)/inc$(UPDMINOREXT)/oox/token/namespaces.txt \
))

$(eval $(call \
	gb_CustomTarget_add_dependencies,writerfilter/source,\
		writerfilter/source/doctok/resources.xmi \
		writerfilter/source/doctok/resources.xsl \
		writerfilter/source/doctok/resourcesimpl.xsl \
		writerfilter/source/doctok/resourceids.xsl \
		writerfilter/source/doctok/sprmids.xsl \
		writerfilter/source/doctok/resourcetools.xsl \
		writerfilter/source/ooxml/model.xml \
		writerfilter/source/ooxml/modelpreprocess.xsl \
		writerfilter/source/ooxml/factorytools.xsl \
		writerfilter/source/ooxml/fasttokens.xsl \
		writerfilter/source/ooxml/namespaceids.xsl \
		writerfilter/source/ooxml/factory_values.xsl \
		writerfilter/source/ooxml/factoryimpl_values.xsl \
		writerfilter/source/ooxml/resourceids.xsl \
		writerfilter/source/ooxml/gperffasttokenhandler.xsl \
		writerfilter/source/ooxml/gperffasttokenhandler.xsl \
		writerfilter/source/ooxml/tokenxmlheader \
		writerfilter/source/ooxml/tokenxmlfooter \
		writerfilter/source/ooxml/factoryinc.xsl \
		writerfilter/source/ooxml/factoryimpl.xsl \
		writerfilter/source/ooxml/factoryimpl_ns.xsl \
		writerfilter/source/ooxml/factory_ns.xsl \
		writerfilter/source/rtftok/RTFScanner.lex \
		writerfilter/source/rtftok/RTFScanner.skl \
		writerfilter/source/doctok/sprmcodetostr.xsl \
		writerfilter/source/doctok/qnametostr.xsl \
		writerfilter/source/ooxml/qnametostr.xsl \
		writerfilter/source/resourcemodel/namespace_preprocess.pl \
		writerfilter/source/resourcemodel/qnametostrheader \
		writerfilter/source/resourcemodel/qnametostrfooter \
		writerfilter/source/resourcemodel/sprmcodetostrheader \
		writerfilter/source/resourcemodel/sprmcodetostrfooter \
))

$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/doctok/sprmids.hxx,doctok/sprmids.hxx))
$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/doctok/resourceids.hxx,doctok/resourceids.hxx))
$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/ooxml/resourceids.hxx,ooxml/resourceids.hxx))

