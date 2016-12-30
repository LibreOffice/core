#**************************************************************
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
#**************************************************************



$(eval $(call gb_Library_Library,basctl))

$(eval $(call gb_Library_add_precompiled_header,basctl,$(SRCDIR)/basctl/inc/pch/precompiled_basctl))

$(eval $(call gb_Library_add_sdi_headers,basctl,basctl/sdi/basslots))

$(eval $(call gb_Library_set_componentfile,basctl,basctl/util/basctl))

$(eval $(call gb_Library_set_include,basctl,\
        $$(INCLUDE) \
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/inc/pch \
	-I$(SRCDIR)/basctl/source/inc \
	-I$(WORKDIR)/SdiTarget/basctl/sdi \
))

$(eval $(call gb_Library_add_api,basctl,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,basctl,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	fwe \
	ootk \
	sal \
	sb \
	sfx \
	sot \
	stl \
	svl \
	svt \
	svx \
	svxcore \
	tl \
	ucbhelper \
	utl \
	vcl \
	xcr \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,basctl,\
	shell32 \
))
endif

$(eval $(call gb_Library_add_exception_objects,basctl,\
	basctl/source/accessibility/accessibledialogwindow \
	basctl/source/accessibility/accessibledialogcontrolshape \
	basctl/source/basicide/basicrenderable \
	basctl/source/basicide/scriptdocument \
	basctl/source/basicide/basicbox \
	basctl/source/basicide/basidesh \
	basctl/source/basicide/basides1 \
	basctl/source/basicide/basides2 \
	basctl/source/basicide/basides3 \
	basctl/source/basicide/baside2 \
	basctl/source/basicide/baside3 \
	basctl/source/basicide/basobj2 \
	basctl/source/basicide/basobj3 \
	basctl/source/basicide/bastypes \
	basctl/source/basicide/bastype2 \
	basctl/source/basicide/bastype3 \
	basctl/source/basicide/iderdll \
	basctl/source/basicide/macrodlg \
	basctl/source/basicide/moduldlg \
	basctl/source/basicide/moduldl2 \
	basctl/source/basicide/unomodel \
	basctl/source/basicide/register \
	basctl/source/basicide/basdoc \
	basctl/source/basicide/tbxctl \
	basctl/source/basicide/basidectrlr \
	basctl/source/basicide/localizationmgr \
	basctl/source/basicide/doceventnotifier \
	basctl/source/basicide/docsignature \
	basctl/source/basicide/documentenumeration \
	basctl/source/dlged/dlged \
	basctl/source/dlged/dlgedobj \
	basctl/source/dlged/dlgedfac \
	basctl/source/dlged/dlgedlist \
	basctl/source/dlged/dlgedclip \
	basctl/source/dlged/propbrw \
))

$(eval $(call gb_Library_add_noexception_objects,basctl,\
	basctl/source/basicide/baside2b \
	basctl/source/basicide/brkdlg \
	basctl/source/basicide/objdlg \
	basctl/source/dlged/dlgedfunc \
	basctl/source/dlged/dlgedmod \
	basctl/source/dlged/dlgedpage \
	basctl/source/dlged/dlgedview \
	basctl/source/dlged/managelang \
))

$(eval $(call gb_SdiTarget_SdiTarget,basctl/sdi/basslots,basctl/sdi/baside))

$(eval $(call gb_SdiTarget_set_include,basctl/sdi/basslots,\
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/sdi \
	$$(INCLUDE) \
))


# vim: set noet sw=4 ts=4:
