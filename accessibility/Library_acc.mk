# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,acc))

$(eval $(call gb_Library_set_include,acc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/accessibility/inc \
    -I$(SRCDIR)/accessibility/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,acc))

$(eval $(call gb_Library_use_libraries,acc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sot \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,acc))

$(eval $(call gb_Library_add_exception_objects,acc,\
    accessibility/source/extended/AccessibleBrowseBox \
    accessibility/source/extended/AccessibleBrowseBoxBase \
    accessibility/source/extended/AccessibleBrowseBoxCheckBoxCell \
    accessibility/source/extended/AccessibleBrowseBoxHeaderBar \
    accessibility/source/extended/AccessibleBrowseBoxHeaderCell \
    accessibility/source/extended/AccessibleBrowseBoxTable \
    accessibility/source/extended/AccessibleBrowseBoxTableBase \
    accessibility/source/extended/AccessibleBrowseBoxTableCell \
    accessibility/source/extended/AccessibleGridControl \
    accessibility/source/extended/AccessibleGridControlBase \
    accessibility/source/extended/AccessibleGridControlHeader \
    accessibility/source/extended/AccessibleGridControlHeaderCell \
    accessibility/source/extended/AccessibleGridControlTable \
    accessibility/source/extended/AccessibleGridControlTableBase \
    accessibility/source/extended/AccessibleGridControlTableCell \
    accessibility/source/extended/AccessibleToolPanelDeck \
    accessibility/source/extended/AccessibleToolPanelDeckTabBar \
    accessibility/source/extended/AccessibleToolPanelDeckTabBarItem \
    accessibility/source/extended/accessiblebrowseboxcell \
    accessibility/source/extended/accessibleeditbrowseboxcell \
    accessibility/source/extended/accessibleiconchoicectrl \
    accessibility/source/extended/accessibleiconchoicectrlentry \
    accessibility/source/extended/accessiblelistbox \
    accessibility/source/extended/accessiblelistboxentry \
    accessibility/source/extended/accessibletabbar \
    accessibility/source/extended/accessibletabbarbase \
    accessibility/source/extended/accessibletabbarpage \
    accessibility/source/extended/accessibletabbarpagelist \
    accessibility/source/extended/accessibletablistbox \
    accessibility/source/extended/accessibletablistboxtable \
    accessibility/source/extended/listboxaccessible \
    accessibility/source/extended/textwindowaccessibility \
    accessibility/source/helper/acc_factory \
    accessibility/source/helper/accresmgr \
    accessibility/source/helper/characterattributeshelper \
    accessibility/source/helper/IComboListBoxHelper \
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
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
