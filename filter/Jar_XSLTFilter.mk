#***************************************************************
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#***************************************************************

$(eval $(call gb_Jar_Jar,XSLTFilter,SRCDIR))

$(eval $(call gb_Jar_add_jars,XSLTFilter,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
))

$(call gb_Jar_use_externals,XSLTFilter,saxon)

$(eval $(call gb_Jar_set_componentfile,XSLTFilter,filter/source/xsltfilter/XSLTFilter.jar,OOO))

$(eval $(call gb_Jar_set_manifest,XSLTFilter,$(SRCDIR)/filter/source/xsltfilter/Manifest))

$(eval $(call gb_Jar_set_jarclasspath,XSLTFilter,saxon9.jar))

$(eval $(call gb_Jar_set_packageroot,XSLTFilter,com))

$(eval $(call gb_Jar_add_sourcefiles,XSLTFilter,\
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/Base64 \
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/XSLTFilterOLEExtracter \
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/XSLTransformer \
))

# vim: set noet sw=4 ts=4:
