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



$(eval $(call gb_Executable_Executable,srvdepy))

$(eval $(call gb_Executable_set_targettype_gui,srvdepy,NO))

#$(eval $(call gb_Executable_add_api,srvdepy,\
#	udkapi \
#	offapi \
#))

$(eval $(call gb_Executable_set_include,srvdepy,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xml2cmp/source/inc \
))

$(eval $(call gb_Executable_add_linked_libs,srvdepy,\
	stl \
	$(filter-out uwinapi,$(gb_STDLIBS)) \
))


$(eval $(call gb_Executable_add_exception_objects,srvdepy,\
	xml2cmp/source/finder/dependy \
	xml2cmp/source/finder/dep_main \
	xml2cmp/source/xcd/cr_html \
	xml2cmp/source/xcd/cr_index \
	xml2cmp/source/xcd/cr_metho \
	xml2cmp/source/xcd/filebuff \
	xml2cmp/source/xcd/parse \
	xml2cmp/source/xcd/xmlelem \
	xml2cmp/source/xcd/xmltree \
	xml2cmp/source/support/cmdline \
	xml2cmp/source/support/heap \
	xml2cmp/source/support/sistr \
	xml2cmp/source/support/syshelp \
	xml2cmp/source/support/badcast \
))

# vim: set noet sw=4 ts=4:
