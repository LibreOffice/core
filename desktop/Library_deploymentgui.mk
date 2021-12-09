# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,deploymentgui))

$(eval $(call gb_Library_set_include,deploymentgui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Library_use_external,deploymentgui,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,deploymentgui,desktop/inc/pch/precompiled_deploymentgui))

$(eval $(call gb_Library_use_custom_headers,deploymentgui,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,deploymentgui))

$(eval $(call gb_Library_use_libraries,deploymentgui,\
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    i18nlangtag \
    sal \
    salhelper \
    sfx \
    svl \
    svt \
    svxcore \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,deploymentgui,\
    ole32 \
))

endif


$(eval $(call gb_Library_set_componentfile,deploymentgui,desktop/source/deployment/gui/deploymentgui,services))

$(eval $(call gb_Library_add_exception_objects,deploymentgui,\
    desktop/source/deployment/gui/dp_gui_dependencydialog \
    desktop/source/deployment/gui/dp_gui_dialog2 \
    desktop/source/deployment/gui/dp_gui_extensioncmdqueue \
    desktop/source/deployment/gui/dp_gui_extlistbox \
    desktop/source/deployment/gui/dp_gui_service \
    desktop/source/deployment/gui/dp_gui_theextmgr \
    desktop/source/deployment/gui/dp_gui_updatedialog \
    desktop/source/deployment/gui/dp_gui_updateinstalldialog \
    desktop/source/deployment/gui/license_dialog \
))

# vim: set ts=4 sw=4 et:
