# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,acc))

$(eval $(call gb_Library_set_componentfile,acc,accessibility/util/acc,services))

$(eval $(call gb_Library_set_include,acc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/accessibility/inc \
    -I$(SRCDIR)/accessibility/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,acc,accessibility/inc/pch/precompiled_acc))

$(eval $(call gb_Library_use_external,acc,boost_headers))

$(eval $(call gb_Library_use_sdk_api,acc))

$(eval $(call gb_Library_use_libraries,acc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    i18nlangtag \
    sot \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_add_defs,acc,\
    -DVCL_INTERNALS \
))

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
    accessibility/source/extended/accessiblebrowseboxcell \
    accessibility/source/extended/accessibleeditbrowseboxcell \
    accessibility/source/extended/accessibleiconchoicectrl \
    accessibility/source/extended/accessibleiconchoicectrlentry \
    accessibility/source/extended/AccessibleIconView \
    accessibility/source/extended/accessiblelistbox \
    accessibility/source/extended/accessiblelistboxentry \
    accessibility/source/extended/accessibletablistbox \
    accessibility/source/extended/accessibletablistboxtable \
    accessibility/source/extended/textwindowaccessibility \
    accessibility/source/helper/acc_factory \
    accessibility/source/helper/accresmgr \
    accessibility/source/helper/characterattributeshelper \
    accessibility/source/helper/IComboListBoxHelper \
    accessibility/source/standard/accessiblemenubasecomponent \
    accessibility/source/standard/accessiblemenucomponent \
    accessibility/source/standard/accessiblemenuitemcomponent \
    accessibility/source/standard/floatingwindowaccessible \
    accessibility/source/standard/svtaccessiblenumericfield \
    accessibility/source/standard/vclxaccessiblebox \
    accessibility/source/standard/vclxaccessiblebutton \
    accessibility/source/standard/vclxaccessiblecheckbox \
    accessibility/source/standard/vclxaccessiblecombobox \
    accessibility/source/standard/vclxaccessibledropdowncombobox \
    accessibility/source/standard/vclxaccessibledropdownlistbox \
    accessibility/source/standard/vclxaccessibleedit \
    accessibility/source/standard/vclxaccessiblefixedhyperlink \
    accessibility/source/standard/vclxaccessiblefixedtext \
    accessibility/source/standard/vclxaccessibleheaderbar \
    accessibility/source/standard/vclxaccessibleheaderbaritem \
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

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)
$(eval $(call gb_Library_add_exception_objects,acc,\
    accessibility/source/extended/accessibletabbar \
    accessibility/source/extended/accessibletabbarbase \
    accessibility/source/extended/accessibletabbarpage \
    accessibility/source/extended/accessibletabbarpagelist \
))
endif

# vim:set noet sw=4 ts=4:
