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

$(eval $(call gb_Jar_Jar,XSLTValidate,SRCDIR))

$(eval $(call gb_Jar_add_jars,XSLTValidate,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
))

#$(eval $(call gb_Jar_use_externals,XSLTValidate,\
	xml-apis \
	xalan \
))

$(eval $(call gb_Jar_set_componentfile,XSLTValidate,filter/source/xsltvalidate/XSLTValidate,OOO))

$(eval $(call gb_Jar_set_manifest,XSLTValidate,$(SRCDIR)/filter/source/xsltvalidate/Manifest))

$(eval $(call gb_Jar_set_packageroot,XSLTValidate,XSLTValidate.class))

$(eval $(call gb_Jar_add_sourcefiles,XSLTValidate,\
	filter/source/xsltvalidate/XSLTValidate \
))

# vim: set noet sw=4 ts=4:
