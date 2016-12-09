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



$(eval $(call gb_Jar_Jar,juh,SRCDIR))

$(eval $(call gb_Jar_add_jars,juh,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/unoil.jar \
))

$(eval $(call gb_Jar_set_packageroot,juh,com))

$(eval $(call gb_Jar_set_manifest,juh,$(SRCDIR)/javaunohelper/util/manifest))

$(eval $(call gb_Jar_set_jarclasspath,juh,\
	ridl.jar \
	jurt.jar \
	../../lib/ \
	../bin/ \
))

$(eval $(call gb_Jar_add_sourcefiles,juh,\
	javaunohelper/com/sun/star/lib/uno/helper/UnoUrl \
	javaunohelper/com/sun/star/lib/uno/helper/Factory \
	javaunohelper/com/sun/star/lib/uno/helper/ComponentBase \
	javaunohelper/com/sun/star/lib/uno/helper/WeakAdapter \
	javaunohelper/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer \
	javaunohelper/com/sun/star/lib/uno/helper/WeakBase \
	javaunohelper/com/sun/star/lib/uno/helper/PropertySetMixin \
	javaunohelper/com/sun/star/lib/uno/helper/PropertySet \
	javaunohelper/com/sun/star/lib/uno/helper/InterfaceContainer \
	javaunohelper/com/sun/star/lib/uno/adapter/ByteArrayToXInputStreamAdapter \
	javaunohelper/com/sun/star/lib/uno/adapter/XOutputStreamToByteArrayAdapter \
	javaunohelper/com/sun/star/lib/uno/adapter/XOutputStreamToOutputStreamAdapter \
	javaunohelper/com/sun/star/lib/uno/adapter/InputStreamToXInputStreamAdapter \
	javaunohelper/com/sun/star/lib/uno/adapter/XInputStreamToInputStreamAdapter \
	javaunohelper/com/sun/star/lib/uno/adapter/OutputStreamToXOutputStreamAdapter \
	javaunohelper/com/sun/star/comp/JavaUNOHelperServices \
	javaunohelper/com/sun/star/comp/juhtest/SmoketestCommandEnvironment \
	javaunohelper/com/sun/star/comp/helper/RegistryServiceFactory \
	javaunohelper/com/sun/star/comp/helper/UnoInfo \
	javaunohelper/com/sun/star/comp/helper/ComponentContext \
	javaunohelper/com/sun/star/comp/helper/BootstrapException \
	javaunohelper/com/sun/star/comp/helper/SharedLibraryLoader \
	javaunohelper/com/sun/star/comp/helper/Bootstrap \
	javaunohelper/com/sun/star/comp/helper/ComponentContextEntry \
))

$(eval $(call gb_Jar_set_componentfile,juh,javaunohelper/util/juh,URE))

# vim: set noet sw=4 ts=4:
