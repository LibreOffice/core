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



$(eval $(call gb_Library_Library,fastsax))

$(eval $(call gb_Library_set_componentfile,fastsax,sax/source/fastparser/fastsax))

$(eval $(call gb_Library_add_api,fastsax,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,fastsax,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sax/inc \
))

$(eval $(call gb_Library_add_linked_libs,fastsax,\
	sal \
	cppu \
	cppuhelper \
	sax \
	stl \
    $(gb_STDLIBS) \
))

ifeq ($(SYSTEM_ZLIB),YES)
$(eval $(call gb_Library_add_defs,fastsax,\
	-DSYSTEM_ZLIB \
))
endif

$(call gb_Library_use_external,fastsax,expat_utf8)

# re-uses xml2utf object from sax.uno lib (see below)
$(eval $(call gb_Library_add_exception_objects,fastsax,\
	sax/source/fastparser/facreg \
	sax/source/fastparser/fastparser \
	sax/source/expatwrap/xml2utf \
))

# mba: currently it's unclear whether xml2utf needs to be compiled with the
# same compiler settings as in the sax.uno lib; in case not, we have to use 
# a custom target
#$(eval $(call gb_Library_add_generated_exception_objects,fastsax,\
#	sax/source/expatwrap/xml2utf \
#))

# vim: set noet sw=4 ts=4:
