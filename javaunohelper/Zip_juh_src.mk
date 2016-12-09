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



$(eval $(call gb_Zip_Zip,juh_src,$(SRCDIR)/javaunohelper))

$(eval $(call gb_Zip_add_files,juh_src,\
	com/sun/star/lib/uno/helper/UnoUrl.java \
	com/sun/star/lib/uno/helper/Factory.java \
	com/sun/star/lib/uno/helper/ComponentBase.java \
	com/sun/star/lib/uno/helper/WeakAdapter.java \
	com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer.java \
	com/sun/star/lib/uno/helper/WeakBase.java \
	com/sun/star/lib/uno/helper/PropertySetMixin.java \
	com/sun/star/lib/uno/helper/PropertySet.java \
	com/sun/star/lib/uno/helper/InterfaceContainer.java \
	com/sun/star/lib/uno/adapter/ByteArrayToXInputStreamAdapter.java \
	com/sun/star/lib/uno/adapter/XOutputStreamToByteArrayAdapter.java \
	com/sun/star/lib/uno/adapter/XOutputStreamToOutputStreamAdapter.java \
	com/sun/star/lib/uno/adapter/InputStreamToXInputStreamAdapter.java \
	com/sun/star/lib/uno/adapter/XInputStreamToInputStreamAdapter.java \
	com/sun/star/lib/uno/adapter/OutputStreamToXOutputStreamAdapter.java \
	com/sun/star/comp/JavaUNOHelperServices.java \
	com/sun/star/comp/juhtest/SmoketestCommandEnvironment.java \
	com/sun/star/comp/helper/RegistryServiceFactory.java \
	com/sun/star/comp/helper/UnoInfo.java \
	com/sun/star/comp/helper/ComponentContext.java \
	com/sun/star/comp/helper/BootstrapException.java \
	com/sun/star/comp/helper/SharedLibraryLoader.java \
	com/sun/star/comp/helper/Bootstrap.java \
	com/sun/star/comp/helper/ComponentContextEntry.java \
))

# vim: set noet sw=4 ts=4:
