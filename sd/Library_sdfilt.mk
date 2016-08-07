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



$(eval $(call gb_Library_Library,sdfilt))

$(eval $(call gb_Library_add_api,sdfilt,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,sdfilt,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sd/inc \
	-I$(SRCDIR)/sd/inc/pch \
))

$(eval $(call gb_Library_add_linked_libs,sdfilt,\
	editeng \
	svxcore \
	sd \
	sfx \
	svl \
	svt \
	stl \
	sot \
	tl \
	vcl \
	msfilter \
	cppu \
	cppuhelper \
	ucbhelper \
	i18nisolang1 \
	sal \
	comphelper \
	utl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sdfilt,\
	sd/source/filter/ppt/propread \
	sd/source/filter/ppt/pptin \
	sd/source/filter/ppt/pptinanimations \
	sd/source/filter/ppt/pptatom \
	sd/source/filter/ppt/ppt97animations \
	sd/source/filter/eppt/eppt \
	sd/source/filter/eppt/escherex \
	sd/source/filter/eppt/pptexanimations \
	sd/source/filter/eppt/pptexsoundcollection \
))

# the following source file can't be compiled with optimization by some compilers (crash or endless loop):
# Solaris Sparc with Sun compiler, gcc on MacOSX and Linux PPC
# the latter is currently not supported by gbuild and needs a fix here later
ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxobjects,sdfilt,\
    sd/source/filter/eppt/epptso \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,sdfilt,\
    sd/source/filter/eppt/epptso \
))
endif

# vim: set noet sw=4 ts=4:
