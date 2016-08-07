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



$(eval $(call gb_Library_Library,sm))

$(eval $(call gb_Library_add_sdi_headers,sm,starmath/sdi/smslots))

$(eval $(call gb_Library_add_precompiled_header,sm,$(SRCDIR)/starmath/inc/pch/precompiled_starmath))

$(eval $(call gb_Library_set_componentfile,sm,starmath/util/sm))

$(eval $(call gb_Library_add_api,sm,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,sm,\
	-I$(SRCDIR)/starmath/inc/pch \
	-I$(SRCDIR)/starmath/inc \
	-I$(WORKDIR)/SdiTarget/starmath/sdi \
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_linked_libs,sm,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	i18npaper \
	ootk \
	sal \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	utl \
	vcl \
	xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sm,\
	starmath/source/accessibility \
	starmath/source/action \
	starmath/source/cfgitem \
	starmath/source/config \
	starmath/source/dialog \
	starmath/source/document \
	starmath/source/edit \
	starmath/source/eqnolefilehdr \
	starmath/source/format \
	starmath/source/mathmlexport \
	starmath/source/mathmlimport \
	starmath/source/mathtype \
	starmath/source/node \
	starmath/source/parse \
	starmath/source/rect \
	starmath/source/register \
	starmath/source/smdll \
	starmath/source/smmod \
	starmath/source/symbol \
	starmath/source/toolbox \
	starmath/source/typemap \
	starmath/source/types \
	starmath/source/unodoc \
	starmath/source/unomodel \
	starmath/source/utility \
	starmath/source/view \
))

$(eval $(call gb_SdiTarget_SdiTarget,starmath/sdi/smslots,starmath/sdi/smath))

$(eval $(call gb_SdiTarget_set_include,starmath/sdi/smslots,\
	-I$(SRCDIR)/starmath/inc \
	-I$(SRCDIR)/starmath/sdi \
	$$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
