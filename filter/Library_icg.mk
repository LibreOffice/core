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

$(eval $(call gb_Library_Library,icg))

$(eval $(call gb_Library_add_api,icg,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,icg,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,icg,\
	tk \
	vcl \
	utl \
	tl \
	cppu \
	sal \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,icg,\
	filter/source/graphicfilter/icgm/actimpr \
	filter/source/graphicfilter/icgm/bitmap \
	filter/source/graphicfilter/icgm/bundles \
	filter/source/graphicfilter/icgm/cgm \
	filter/source/graphicfilter/icgm/chart \
	filter/source/graphicfilter/icgm/class0 \
	filter/source/graphicfilter/icgm/class1 \
	filter/source/graphicfilter/icgm/class2 \
	filter/source/graphicfilter/icgm/class3 \
	filter/source/graphicfilter/icgm/class4 \
	filter/source/graphicfilter/icgm/class5 \
	filter/source/graphicfilter/icgm/class7 \
	filter/source/graphicfilter/icgm/classx \
	filter/source/graphicfilter/icgm/elements \
	filter/source/graphicfilter/icgm/outact \
))

# vim: set noet sw=4 ts=4:
