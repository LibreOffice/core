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



$(eval $(call gb_Library_Library,expwrap))

$(eval $(call gb_Library_set_componentfile,expwrap,sax/source/expatwrap/expwrap))

$(eval $(call gb_Library_add_api,expwrap,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,expwrap,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sax/inc \
))

$(eval $(call gb_Library_add_linked_libs,expwrap,\
	sal \
	cppu \
	cppuhelper \
	stl \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_external,expwrap,expat_utf16)

$(eval $(call gb_Library_add_exception_objects,expwrap,\
	sax/source/expatwrap/attrlistimpl \
	sax/source/expatwrap/sax_expat \
	sax/source/expatwrap/saxwriter \
	sax/source/expatwrap/xml2utf \
))

# vim: set noet sw=4 ts=4:
