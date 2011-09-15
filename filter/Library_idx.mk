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

$(eval $(call gb_Library_Library,idx))

$(eval $(call gb_Library_add_api,idx,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,idx,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,idx,\
	vcl \
	tl \
	sal \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,idx,\
	filter/source/graphicfilter/idxf/dxf2mtf \
	filter/source/graphicfilter/idxf/dxfblkrd \
	filter/source/graphicfilter/idxf/dxfentrd \
	filter/source/graphicfilter/idxf/dxfgrprd \
	filter/source/graphicfilter/idxf/dxfreprd \
	filter/source/graphicfilter/idxf/dxftblrd \
	filter/source/graphicfilter/idxf/dxfvec \
	filter/source/graphicfilter/idxf/idxf \
))

# vim: set noet sw=4 ts=4:
