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



$(eval $(call gb_Library_Library,acc))

$(eval $(call gb_Library_add_precompiled_header,acc,$(SRCDIR)/accessibility/inc/pch/precompiled_accessibility))

$(eval $(call gb_Library_set_include,acc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/accessibility/inc \
	-I$(SRCDIR)/accessibility/inc/pch \
	-I$(SRCDIR)/accessibility/source/inc \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,acc, \
	offapi \
        udkapi \
))


$(eval $(call gb_Library_add_defs,acc,\
	-DACCESSIBILITY_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_versionmap,acc,$(SRCDIR)/accessibility/util/acc.map))

$(eval $(call gb_Library_add_linked_libs,acc,\
	comphelper \
	cppu \
	cppuhelper \
	ootk \
	sal \
	sot \
	stl \
	svl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,acc,\
	accessibility/source/standard/accessiblemenubasecomponent \
	accessibility/source/standard/accessiblemenucomponent \
	accessibility/source/standard/accessiblemenuitemcomponent \
	accessibility/source/standard/floatingwindowaccessible \
	accessibility/source/standard/vclxaccessiblebox \
	accessibility/source/standard/vclxaccessiblebutton \
	accessibility/source/standard/vclxaccessiblecheckbox \
	accessibility/source/standard/vclxaccessiblecombobox \
	accessibility/source/standard/vclxaccessibledropdowncombobox \
	accessibility/source/standard/vclxaccessibledropdownlistbox \
	accessibility/source/standard/vclxaccessibleedit \
	accessibility/source/standard/vclxaccessiblefixedhyperlink \
	accessibility/source/standard/vclxaccessiblefixedtext \
	accessibility/source/standard/vclxaccessiblelist \
	accessibility/source/standard/vclxaccessiblelistbox \
	accessibility/source/standard/vclxaccessiblelistitem \
	accessibility/source/standard/vclxaccessiblemenu \
	accessibility/source/standard/vclxaccessiblemenubar \
	accessibility/source/standard/vclxaccessiblemenuitem \
	accessibility/source/standard/vclxaccessiblemenuseparator \
	accessibility/source/standard/vclxaccessiblepopupmenu \
	accessibility/source/standard/vclxaccessibleradiobutton \
	accessibility/source/standard/vclxaccessiblescrollbar \
	accessibility/source/standard/vclxaccessiblestatusbar \
	accessibility/source/standard/vclxaccessiblestatusbaritem \
	accessibility/source/standard/vclxaccessibletabcontrol \
	accessibility/source/standard/vclxaccessibletabpage \
	accessibility/source/standard/vclxaccessibletabpagewindow \
	accessibility/source/standard/vclxaccessibletextcomponent \
	accessibility/source/standard/vclxaccessibletextfield \
	accessibility/source/standard/vclxaccessibletoolbox \
	accessibility/source/standard/vclxaccessibletoolboxitem \
	accessibility/source/extended/AccessibleBrowseBoxCheckBoxCell \
	accessibility/source/extended/AccessibleBrowseBoxBase \
	accessibility/source/extended/AccessibleBrowseBox \
	accessibility/source/extended/AccessibleBrowseBoxTableCell \
	accessibility/source/extended/AccessibleBrowseBoxHeaderCell \
	accessibility/source/extended/AccessibleBrowseBoxTableBase \
	accessibility/source/extended/AccessibleBrowseBoxTable \
	accessibility/source/extended/AccessibleBrowseBoxHeaderBar \
	accessibility/source/extended/accessibleiconchoicectrl \
	accessibility/source/extended/accessibleiconchoicectrlentry \
	accessibility/source/extended/accessiblelistbox \
	accessibility/source/extended/accessiblelistboxentry \
	accessibility/source/extended/accessibletabbarbase \
	accessibility/source/extended/accessibletabbar \
	accessibility/source/extended/accessibletabbarpage \
	accessibility/source/extended/accessibletabbarpagelist \
	accessibility/source/extended/accessibletablistbox \
	accessibility/source/extended/accessibletablistboxtable \
	accessibility/source/extended/listboxaccessible \
	accessibility/source/extended/accessiblebrowseboxcell \
	accessibility/source/extended/accessibleeditbrowseboxcell \
	accessibility/source/extended/textwindowaccessibility \
	accessibility/source/extended/AccessibleGridControlBase \
	accessibility/source/extended/AccessibleGridControl \
	accessibility/source/extended/AccessibleGridControlTableBase \
	accessibility/source/extended/AccessibleGridControlHeader \
	accessibility/source/extended/AccessibleGridControlTableCell \
	accessibility/source/extended/AccessibleGridControlHeaderCell \
	accessibility/source/extended/AccessibleGridControlTable \
	accessibility/source/extended/AccessibleToolPanelDeck \
	accessibility/source/extended/AccessibleToolPanelDeckTabBar \
	accessibility/source/extended/AccessibleToolPanelDeckTabBarItem \
	accessibility/source/helper/acc_factory \
	accessibility/source/helper/accresmgr \
	accessibility/source/helper/characterattributeshelper \
))


ifeq ($(OS)$(COM),SOLARISI)
$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	accessibility/source/extended/accessibletabbarpagelist, $(gb_COMPILERNOOPTFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:

